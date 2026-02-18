#include "MainWindow.h"

void MainWindow::onConversionFinished()
{
    logBox->append("All conversions done, preparing to update GUI...");

    QString normalOutput = converter->getFinalOutputFile();
    QString combineOutput = combineTab->getFinalOutputFile();

    // Show notification
    if (!normalOutput.isEmpty()) {
        showConversionNotification(normalOutput);
    } else if (!combineOutput.isEmpty() && QFile::exists(combineOutput)) {
        showConversionNotification(combineOutput);
    }

    // Reset UI controls (small delay for visual polish)
    QTimer::singleShot(100, this, [this]() {
        convertButton->setEnabled(true);
        cancelButton->setEnabled(false);
        conversionProgress->setVisible(false);
        logBox->append("Updating buttons and progress bar now...");
        logBox->append("GUI updated successfully!");
    });

    QString outputFile = normalOutput.isEmpty() ? combineOutput : normalOutput;
    QString inputInfo = infoBox->toHtml();

    if (outputFile.isEmpty()) {
        infoBox->setHtml("<h3>Input File</h3>" + inputInfo +
        "<h3>Output File</h3>No output file specified.");
        logBox->append("⚠️ No output file specified.");
        return;
    }

    [[maybe_unused]] auto _ = QtConcurrent::run([this, outputFile, inputInfo]() {
        QFileInfo fileInfo(outputFile);
        QString fileSizeStr = "N/A";
        QString fileContainer = fileInfo.suffix().isEmpty() ? "N/A" : fileInfo.suffix();

        qint64 fileSizeBytes = fileInfo.size();
        if (fileSizeBytes > 0)
            fileSizeStr = QString::number(fileSizeBytes / 1024.0 / 1024, 'f', 2) + " MB";

        QProcess ffprobe;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString svtAv1Path = settings->value("svtAv1Path", "").toString();
        if (!svtAv1Path.isEmpty()) {
            QString existing = env.value("LD_LIBRARY_PATH");
            env.insert("LD_LIBRARY_PATH", existing.isEmpty() ? svtAv1Path : existing + ":" + svtAv1Path);
        }
        ffprobe.setProcessEnvironment(env);

        QString ffmpegPath = settings->value("ffmpegPath", "/usr/bin/ffmpeg").toString();
        QString ffprobePath = ffmpegPath.isEmpty() ? "/usr/bin/ffprobe"
        : QFileInfo(ffmpegPath).absolutePath() + "/ffprobe" + QFileInfo(ffmpegPath).suffix();
        if (!QFile::exists(ffprobePath)) ffprobePath = "/usr/bin/ffprobe";

        ffprobe.start(ffprobePath, {"-v", "error", "-show_format", "-show_streams", outputFile});

        if (!ffprobe.waitForFinished(10000)) {
            QMetaObject::invokeMethod(infoBox, "setHtml", Qt::QueuedConnection,
                                      Q_ARG(QString, inputInfo + "<h3>Output File</h3>Failed to analyze output (timeout)"));
            return;
        }

        QString outputData = ffprobe.readAllStandardOutput();
        QString errorData = ffprobe.readAllStandardError();
        if (!errorData.isEmpty())
            QMetaObject::invokeMethod(logBox, "append", Qt::QueuedConnection,
                                      Q_ARG(QString, "ffprobe (output) stderr: " + errorData));

            QString resolution = "N/A", videoCodec = "N/A", frameRate = "N/A", videoBitRate = "N/A";
        QString audioCodec = "N/A", pixFmt = "N/A", durationStr = "N/A", title = "N/A";
        QString aspectRatio = "N/A", colorSpace = "N/A", pixelFormat = "N/A", audioSampleRate = "N/A";

        QStringList lines = outputData.split("\n", Qt::SkipEmptyParts);
        QList<QMap<QString, QString>> streams;
        QMap<QString, QString> formatMap;
        QMap<QString, QString> *currentMap = nullptr;

        for (const QString &line : lines) {
            if (line == "[STREAM]") {
                streams.append(QMap<QString, QString>());
                currentMap = &streams.last();
            } else if (line == "[FORMAT]") {
                currentMap = &formatMap;
            } else if (line.startsWith("[/")) {
                currentMap = nullptr;
            } else if (currentMap) {
                int eqPos = line.indexOf("=");
                if (eqPos > 0) {
                    QString key = line.left(eqPos).trimmed();
                    QString val = line.mid(eqPos + 1).trimmed();
                    (*currentMap)[key] = val;
                }
            }
        }

        // Video stream
        for (const auto &stream : streams) {
            if (stream.value("codec_type") == "video") {
                videoCodec = stream.value("codec_name", "N/A");
                resolution = stream.value("width", "N/A") + "x" + stream.value("height", "N/A");
                aspectRatio = stream.value("display_aspect_ratio", "N/A");
                frameRate = stream.value("avg_frame_rate", stream.value("r_frame_rate", "N/A"));
                if (frameRate.contains("/")) {
                    QStringList parts = frameRate.split("/");
                    if (parts.size() == 2 && parts[1].toDouble() > 0)
                        frameRate = QString::number(parts[0].toDouble() / parts[1].toDouble(), 'f', 2);
                }
                videoBitRate = stream.value("bit_rate", "N/A");
                if (videoBitRate == "N/A" || videoBitRate.toInt() <= 0)
                    videoBitRate = formatMap.value("bit_rate", "N/A");
                if (videoBitRate != "N/A")
                    videoBitRate = QString::number(videoBitRate.toInt() / 1000) + " kbps";

                pixelFormat = stream.value("pix_fmt", "N/A");
                if (pixelFormat.contains("10")) pixFmt = "10-bit";
                else if (pixelFormat.contains("12")) pixFmt = "12-bit";
                else pixFmt = "8-bit";

                colorSpace = stream.value("color_space", "N/A");
                break;
            }
        }

        // Audio stream
        for (const auto &stream : streams) {
            if (stream.value("codec_type") == "audio") {
                audioCodec = stream.value("codec_name", "N/A");
                audioSampleRate = stream.value("sample_rate", "N/A");
                if (audioSampleRate != "N/A") audioSampleRate += " Hz";
                break;
            }
        }

        double durationSec = formatMap.value("duration", "0").toDouble();
        if (durationSec > 0) {
            int hours = static_cast<int>(durationSec / 3600);
            int mins = static_cast<int>((durationSec - hours*3600) / 60);
            int secs = static_cast<int>(durationSec - hours*3600 - mins*60);
            durationStr = QString("%1:%2:%3").arg(hours,2,10,QChar('0'))
            .arg(mins,2,10,QChar('0'))
            .arg(secs,2,10,QChar('0'));
        }

        title = formatMap.value("TAG:title", fileInfo.completeBaseName());

        QString outputInfo = "<b>Video Title:</b> " + title + "<br>"
        + "<b>Resolution:</b> " + resolution + "<br>"
        + "<b>Aspect Ratio:</b> " + aspectRatio + "<br>"
        + "<b>Duration:</b> " + durationStr + "<br>"
        + "<b>Video Codec:</b> " + videoCodec + "<br>"
        + "<b>Frame Rate:</b> " + frameRate + "<br>"
        + "<b>Bit Rate:</b> " + videoBitRate + "<br>"
        + "<b>Audio Codec:</b> " + audioCodec + "<br>"
        + "<b>Audio Sample Rate:</b> " + audioSampleRate + "<br>"
        + "<b>Color Bit Depth:</b> " + pixFmt + "<br>"
        + "<b>Color Space:</b> " + colorSpace + "<br>"
        + "<b>Pixel Format:</b> " + pixelFormat + "<br>"
        + "<b>File Size:</b> " + fileSizeStr + "<br>"
        + "<b>File Container:</b> " + fileContainer + "<br>";

        // Final combined HTML
        QString finalHtml = "<h3>Input File</h3>" + inputInfo +
        "<h3>Output File</h3>" + outputInfo;

        QMetaObject::invokeMethod(infoBox, "setHtml", Qt::QueuedConnection,
                                  Q_ARG(QString, finalHtml));
    });
}
