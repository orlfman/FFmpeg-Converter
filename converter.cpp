#include "converter.h"
#include <QTimer>
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QRegularExpression>
#include <QTime>
#include <QMutexLocker>

Converter::Converter(QObject *parent) : QObject(parent) {
    currentPass = 0;
}

void Converter::startConversion(const QString &inputFile, const QString &outputDir, const QString &baseName, const QStringList &args, bool twoPass, const QString &extension, const QString &codec, const QString &ffmpegPath, const QProcessEnvironment &env, bool overwrite) {
    file = inputFile;
    this->outputDir = outputDir;
    this->baseName = baseName;
    ffmpegArgs = args;
    this->codec = codec;
    this->ffmpegPath = ffmpegPath;
    processEnv = env;
    this->overwriteFlag = overwrite;
    duration = getDuration(inputFile);
    if (duration <= 0) {
        emit logMessage("Could not determine duration for: " + inputFile);
        emit conversionFinished();
        return;
    }
    emit logMessage("Using time-based progress for: " + inputFile + ", duration: " + QString::number(duration) + " seconds");
    QString originalOutputFile = QDir(outputDir).filePath(baseName + extension);
    finalOutputFile = getUniqueOutputFile(outputDir, baseName, extension);
    if (overwriteFlag && QFile::exists(originalOutputFile)) {
        emit logMessage("🗑️ Overwriting existing file: " + originalOutputFile);
    } else if (finalOutputFile != originalOutputFile) {
        emit logMessage("Output file " + originalOutputFile + " already exists, using " + finalOutputFile + " instead.");
    }
    emit logMessage("Starting conversion for: " + file + " -> " + finalOutputFile);

    if (codec == "vp9" && args.contains("-crf") && args.contains("-b:v") && args.contains("0")) {
        emit logMessage("VP9 CRF detected: Using single-pass mode (two-pass not supported for CRF).");
        twoPass = false;
    }

    currentPass = twoPass ? 1 : 0;
    if (twoPass) {
        QString statsFile;
        if (codec == "vp9") {
            statsFile = QDir(outputDir).filePath(baseName);
        } else {
            statsFile = QDir(outputDir).filePath(baseName + ".stats");
        }
        processFile(1, statsFile);
    } else {
        processFile(0, "");
    }
}

void Converter::processFile(int pass, const QString &statsFile) {
    QMutexLocker locker(&processesMutex);
    QStringList args;
    args << "-y" << "-i" << file;
    for (int i = 0; i < ffmpegArgs.size(); ++i) {
        if (ffmpegArgs[i] == "-vf" && i + 1 < ffmpegArgs.size()) {
            args << "-vf" << ffmpegArgs[i + 1];
            ++i;
        } else if (ffmpegArgs[i] == "-svtav1-params" && i + 1 < ffmpegArgs.size()) {
            args << "-svtav1-params" << ffmpegArgs[i + 1];
            ++i;
        } else {
            args << ffmpegArgs[i];
        }
    }
    args << "-loglevel" << "verbose";
    args << "-progress" << "pipe:2";
    int vfIndex = ffmpegArgs.indexOf("-vf");
    if (vfIndex != -1 && vfIndex + 1 < ffmpegArgs.size()) {
        QString filters = ffmpegArgs[vfIndex + 1];
        emit logMessage("🛠️ Filters applied: " + filters);
    } else {
        emit logMessage("🛠️ Filters applied: None");
    }
    if (pass > 0) {
        if (codec == "av1") {
            QStringList svtParamsList = {"pass=" + QString::number(pass), "stat-file=" + statsFile};
            int svtIndex = ffmpegArgs.indexOf("-svtav1-params");
            if (svtIndex != -1 && svtIndex + 1 < ffmpegArgs.size()) {
                svtParamsList << ffmpegArgs[svtIndex + 1].split(":");
            }
            QString allParams = svtParamsList.join(":");
            args << "-svtav1-params" << allParams;
        } else if (codec == "vp9") {
            QString passPrefix = QFileInfo(statsFile).baseName();
            args << "-pass" << QString::number(pass) << "-passlogfile" << passPrefix;
            emit logMessage("Using VP9 passlog prefix: " + passPrefix + " for pass " + QString::number(pass));
        } else if (codec == "x265") {
            QStringList x265ParamsList = {"pass=" + QString::number(pass), "stats=" + statsFile};
            if (pass == 1) {
                x265ParamsList << "no-slow-firstpass=1";
            }
            int x265Index = ffmpegArgs.indexOf("-x265-params");
            if (x265Index != -1 && x265Index + 1 < ffmpegArgs.size()) {
                x265ParamsList << ffmpegArgs[x265Index + 1].split(":");
            }
            QString allParams = x265ParamsList.join(":");
            args << "-x265-params" << allParams;
        }
    }
    if (pass == 1) {
        args << "-f" << "null" << "-";
    } else {
        args << finalOutputFile;
    }
    QString effectiveFFmpegPath = ffmpegPath.isEmpty() ? "ffmpeg" : ffmpegPath;
    if (effectiveFFmpegPath.contains("ffprobe")) {
        emit logMessage("⚠️ Error: FFmpeg path incorrectly set to ffprobe: " + effectiveFFmpegPath);
        emit conversionFinished();
        return;
    }
    emit logMessage("🔍 Debug: Executing FFmpeg command: " + effectiveFFmpegPath + " " + args.join(" "));
    QProcess* process = new QProcess(this);
    QPointer<QProcess> processPtr = process;
    processes.append(processPtr);
    process->setProcessEnvironment(processEnv);
    connect(process, &QProcess::readyReadStandardError, this, &Converter::readProcessOutput);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, processPtr, pass, statsFile](int exitCode, QProcess::ExitStatus exitStatus) {
                processFinished(processPtr, exitCode, exitStatus, pass, statsFile);
            });
    process->start(effectiveFFmpegPath, args);
    if (!process->waitForStarted(5000)) {
        emit logMessage("Failed to start FFmpeg process for pass " + QString::number(pass));
        processes.removeOne(processPtr);
        process->deleteLater();
        emit conversionFinished();
        return;
    }
    else if (pass == 2) {
        emit logMessage("🔄 Starting second pass for: " + file + " -> " + finalOutputFile);
    } else {
        emit logMessage("🔄 Starting conversion for: " + file + " -> " + finalOutputFile);
    }
}

void Converter::cancel() {
    QMutexLocker locker(&processesMutex);
    emit logMessage("🛑 Cancel button clicked, stopping process...");
    QList<QPointer<QProcess>> processesToRemove = processes;
    for (QPointer<QProcess> process : processesToRemove) {
        if (process && processes.contains(process)) {
            emit logMessage("Killing process: " + QString::number((qint64)process.data(), 16));
            QObject::disconnect(process.data(), nullptr, nullptr, nullptr);
            process->kill();
            process->waitForFinished(3000);
            processes.removeOne(process);
            process->deleteLater();
        }
    }
    processes.clear();
    emit conversionFinished();
}

void Converter::readProcessOutput() {
    QProcess* process = qobject_cast<QProcess*>(sender());
    if (process) {
        QString stderrOutput = process->readAllStandardError();
        QStringList stderrLines = stderrOutput.split("\n", Qt::SkipEmptyParts);
        for (const QString &line : stderrLines) {
            if (line.contains("ffmpeg version") || line.contains("ffprobe version") ||
                line.contains("libavutil") || line.contains("libavcodec") ||
                line.contains("libavformat") || line.contains("libavdevice") ||
                line.contains("libavfilter") || line.contains("libswscale") ||
                line.contains("libswresample") || line.contains("configuration:") ||
                line.contains("built with") || line.contains("Queue input") ||
                line.contains("Past duration")) {
                continue;
                }
                if (line.startsWith("frame=") ||
                    line.startsWith("fps=") ||
                    line.startsWith("stream_") ||
                    line.startsWith("bitrate=") ||
                    line.startsWith("total_size=") ||
                    line.startsWith("out_time_ms=") ||
                    line.startsWith("out_time_us=") ||
                    line.startsWith("out_time=") ||
                    line.startsWith("dup_frames=") ||
                    line.startsWith("drop_frames=") ||
                    line.startsWith("speed=") ||
                    line.startsWith("progress=")) {
                    continue;
                    }
                    emit logMessage(line.trimmed());
        }
        QString stdoutOutput = process->readAllStandardOutput();
        QStringList stdoutLines = stdoutOutput.split("\n", Qt::SkipEmptyParts);
        for (const QString &line : stdoutLines) {
            if (!line.contains("ffmpeg version") && !line.contains("ffprobe version") &&
                !line.contains("libavutil") && !line.contains("libavcodec") &&
                !line.contains("libavformat") && !line.contains("libavdevice") &&
                !line.contains("libavfilter") && !line.contains("libswscale") &&
                !line.contains("libswresample") && !line.contains("configuration:") &&
                !line.contains("built with") && !line.contains("Queue input")) {
                emit logMessage(line.trimmed());
                }
        }
        QRegularExpression timeRegex("time=(\\d{2}:\\d{2}:\\d{2}\\.\\d{2})");
        QRegularExpressionMatch match = timeRegex.match(stderrOutput);
        if (match.hasMatch()) {
            QString timeStr = match.captured(1);
            QTime time = QTime::fromString(timeStr, "hh:mm:ss.zz");
            double currentTime = time.hour() * 3600 + time.minute() * 60 + time.second() + time.msec() / 1000.0;
            if (duration > 0) {
                double progressFraction = currentTime / duration;
                int progress;
                if (currentPass == 1) {
                    progress = static_cast<int>(progressFraction * 50);
                } else if (currentPass == 2) {
                    progress = 50 + static_cast<int>(progressFraction * 50);
                } else {
                    progress = static_cast<int>(progressFraction * 100);
                }
                emit progressUpdated(progress);
            }
        }
    }
}

void Converter::processFinished(QPointer<QProcess> processPtr, int exitCode, QProcess::ExitStatus exitStatus, int pass, const QString &statsFile) {
    QMutexLocker locker(&processesMutex);
    if (!processPtr) {
        emit logMessage("Process has been deleted, skipping.");
        return;
    }
    if (!processes.contains(processPtr)) {
        emit logMessage("Process not found in list, skipping.");
        processPtr->deleteLater();
        return;
    }
    if (exitCode != 0) {
        QString errorMsg = (pass == 1) ?
        "First pass failed for: " + file + " with exit code: " + QString::number(exitCode) :
        "Conversion failed for: " + finalOutputFile + " with exit code: " + QString::number(exitCode);
        emit logMessage("❌ " + errorMsg);
        QMessageBox::critical(nullptr, "Error", errorMsg + "\nCheck the Console tab for details.");
    } else {
        if (pass == 1) {
            emit logMessage("🔄 First pass completed successfully for: " + file);
            QString actualStats = statsFile;
            if (codec == "vp9") {
                actualStats = statsFile + "-0.log";
                if (!QFile::exists(actualStats)) {
                    emit logMessage("❌ VP9 stats file missing after first pass: " + actualStats);
                    emit conversionFinished();
                    return;
                }
                emit logMessage("✅ VP9 stats file created: " + actualStats);
            } else {
                // x265/AV1: Plain .stats
                if (!QFile::exists(actualStats)) {
                    emit logMessage("❌ Stats file missing after first pass: " + actualStats);
                    emit conversionFinished();
                    return;
                }
                emit logMessage("✅ Stats file created: " + actualStats);
            }
            emit logMessage("⏳ First pass done, scheduling second pass...");
            currentPass = 2;
            QTimer::singleShot(100, this, [this, statsFile]() {
                emit logMessage("🔄 Starting second pass now...");
                processFile(2, statsFile);
            });
            return;
        } else {
            emit logMessage("✅ Conversion completed successfully: " + finalOutputFile);
        }
    }
    emit logMessage("🧹 Cleaning up process for: " + (pass == 1 ? "First pass" : finalOutputFile));
    QObject::disconnect(processPtr, nullptr, nullptr, nullptr);
    processes.removeOne(processPtr);
    processPtr->deleteLater();
    emit logMessage("✅ Process cleanup complete");
    if (pass == 2 || pass == 0) {
        if (pass == 2) {
            // Clean up stats (codec-specific)
            QString cleanupFile = statsFile;
            if (codec == "vp9") {
                cleanupFile += "-0.log";
            }
            if (QFile::exists(cleanupFile)) {
                if (QFile::remove(cleanupFile)) {
                    emit logMessage("🗑️ Stats file removed: " + cleanupFile);
                } else {
                    emit logMessage("⚠️ Failed to remove stats file: " + cleanupFile);
                }
            }
        }
        emit logMessage("🎉 All passes done, signaling conversion finished.");
        emit conversionFinished();
    }
}

double Converter::getDuration(const QString &inputFile) {
    QFileInfo fileInfo(inputFile);
    if (!fileInfo.exists() || !fileInfo.isReadable()) {
        emit logMessage("⚠️ Input file is invalid or not readable: " + inputFile);
        return 0.0;
    }
    QProcess ffprobe;
    ffprobe.setProcessEnvironment(processEnv);
    QString ffprobePath = ffmpegPath.isEmpty() ? "ffprobe" : QString(ffmpegPath).replace("ffmpeg", "ffprobe");
    QStringList args = {"-v", "error", "-show_entries", "format=duration", "-of", "default=noprint_wrappers=1:nokey=1", inputFile};
    emit logMessage("Executing ffprobe: " + ffprobePath + " " + args.join(" "));
    ffprobe.start(ffprobePath, args);
    if (!ffprobe.waitForFinished(10000)) {
        QString error = ffprobe.readAllStandardError();
        emit logMessage("⚠️ ffprobe failed: " + error);
        QMessageBox::warning(nullptr, "Error", "Failed to retrieve duration: " + error);
        return 0.0;
    }
    QString output = ffprobe.readAllStandardOutput().trimmed();
    QString error = ffprobe.readAllStandardError();
    if (!error.isEmpty()) {
        QStringList lines = error.split("\n", Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            if (!line.contains("ffprobe version") && !line.contains("libavutil") &&
                !line.contains("libavcodec") && !line.contains("libavformat") &&
                !line.contains("libavdevice") && !line.contains("libavfilter") &&
                !line.contains("libswscale") && !line.contains("libswresample") &&
                !line.contains("configuration:") && !line.contains("built with")) {
                emit logMessage("ffprobe stderr: " + line.trimmed());
                }
        }
    }
    bool ok;
    double dur = output.toDouble(&ok);
    if (!ok || dur <= 0) {
        emit logMessage("⚠️ Failed to parse duration from: " + output + ", error: " + error);
        return 0.0;
    }
    emit logMessage("Duration retrieved: " + QString::number(dur) + " seconds");
    return dur;
}

QString Converter::getUniqueOutputFile(const QString& outputDir, const QString& baseName, const QString& extension) {
    QString path = QDir(outputDir).filePath(baseName + extension);
    if (overwriteFlag) {
        return path;
    } else {
        if (!QFile::exists(path)) {
            return path;
        }
        int suffix = 1;
        while (true) {
            path = QDir(outputDir).filePath(baseName + "-" + QString::number(suffix) + extension);
            if (!QFile::exists(path)) {
                return path;
            }
            ++suffix;
        }
    }
}

QString Converter::getFinalOutputFile() const {
    return finalOutputFile;
}
