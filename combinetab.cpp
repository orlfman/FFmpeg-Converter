#include "combinetab.h"
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QTextStream>
#include <QTemporaryFile>
#include <QDebug>
#include <QFileInfo>
#include <algorithm>
#include <cmath>

CombineTab::CombineTab(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *inputLayout = new QHBoxLayout();
    QPushButton *inputBtn = new QPushButton("Select Folder");
    inputDirEdit = new QLineEdit();
    inputDirEdit->setReadOnly(true);
    inputLayout->addWidget(inputBtn);
    inputLayout->addWidget(inputDirEdit);
    mainLayout->addLayout(inputLayout);
    QHBoxLayout *outputLayout = new QHBoxLayout();
    QPushButton *outputBtn = new QPushButton("Output Folder");
    outputDirEdit = new QLineEdit();
    outputDirEdit->setReadOnly(true);
    outputNameEdit = new QLineEdit("combined_video");
    containerCombo = new QComboBox();
    containerCombo->addItems({"mkv", "webm", "mp4"});
    containerCombo->setCurrentIndex(0);
    outputLayout->addWidget(outputBtn);
    outputLayout->addWidget(outputDirEdit);
    outputLayout->addWidget(outputNameEdit);
    outputLayout->addWidget(containerCombo);
    mainLayout->addLayout(outputLayout);
    QHBoxLayout *reencodeLayout = new QHBoxLayout();
    reencodeCheck = new QCheckBox("Re-encode");
    reencodeCheck->setToolTip(
        "If your videos use different audio / video codecs, "
        "check this and it'll automatically re-encode everything to the video codec you picked.\n "
        "Audio by default gets re-encoded to opus.\n "
        "Leave it unchecked for no encoding (though might fail if they are incompatible)."
    );
    reencodeCheck->setChecked(true);
    codecLabel = new QLabel("Target Codec:");
    targetCodecCombo = new QComboBox();
    reencodeLayout->addWidget(reencodeCheck);
    reencodeLayout->addWidget(codecLabel);
    reencodeLayout->addWidget(targetCodecCombo);
    reencodeLayout->addStretch();
    mainLayout->addLayout(reencodeLayout);
    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLabel *searchLabel = new QLabel("Search:");
    searchBox = new QLineEdit();
    searchBox->setPlaceholderText("Type to filter videos...");
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchBox);
    mainLayout->addLayout(searchLayout);
    table = new QTableWidget(0, 2);
    table->setHorizontalHeaderLabels({"Order (0=skip)", "Filename"});
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(table);
    connect(searchBox, &QLineEdit::textChanged, this, [this](const QString &text) {
        QString filter = text.toLower().trimmed();
        for (int row = 0; row < table->rowCount(); ++row) {
            QTableWidgetItem *item = table->item(row, 1);
            if (!item) continue;
            QString filename = item->text().toLower();
            bool matches = filename.contains(filter);
            table->setRowHidden(row, !matches);
        }
    });
    connect(inputBtn, &QPushButton::clicked, this, &CombineTab::selectInputDirectory);
    connect(outputBtn, &QPushButton::clicked, this, &CombineTab::selectOutputDirectory);
    connect(inputBtn, &QPushButton::clicked, this, &CombineTab::populateTable);
    connect(containerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CombineTab::smartUpdateExtension);
    connect(containerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CombineTab::updateCodecOptions);
    connect(outputNameEdit, &QLineEdit::textChanged, this, &CombineTab::smartUpdateExtension);
    connect(reencodeCheck, &QCheckBox::toggled, this, &CombineTab::onReencodeToggled);
    updateCodecOptions();
    setLayout(mainLayout);
    onReencodeToggled(reencodeCheck->isChecked());
}

void CombineTab::onReencodeToggled(bool checked)
{
    codecLabel->setVisible(checked);
    targetCodecCombo->setVisible(checked);
}

void CombineTab::updateCodecOptions()
{
    QString container = containerCombo->currentText();
    targetCodecCombo->clear();
    if (container == "webm") {
        targetCodecCombo->addItems({"VP9", "AV1"});
    } else if (container == "mp4") {
        targetCodecCombo->addItem("x265");
    } else {
        targetCodecCombo->addItems({"AV1", "VP9", "x265"});
    }
    smartUpdateExtension();
}

void CombineTab::smartUpdateExtension()
{
    QString currentText = outputNameEdit->text();
    QString desiredExt = containerCombo->currentText();
    QString currentExt = QFileInfo(currentText).suffix().toLower();
    if (currentText.isEmpty() || currentExt.isEmpty() || currentExt != desiredExt) {
        QString baseName = QFileInfo(currentText).completeBaseName();
        if (baseName.isEmpty()) baseName = "combined_video";
        QString newText = baseName + "." + desiredExt;
        if (newText != currentText) {
            outputNameEdit->blockSignals(true);
            outputNameEdit->setText(newText);
            outputNameEdit->blockSignals(false);
            outputNameEdit->setCursorPosition(baseName.length());
        }
    }
}

QString CombineTab::getFinalOutputFile() const
{
    if (outputDirEdit->text().isEmpty() || outputNameEdit->text().isEmpty())
        return QString();
    QString base = QFileInfo(outputNameEdit->text()).completeBaseName();
    QString ext = containerCombo->currentText();
    return QDir(outputDirEdit->text()).filePath(base + "." + ext);
}

CombineTab::StreamInfo CombineTab::probeFile(const QString& file, const QString& ffprobePath) {
    StreamInfo info;
    if (!QFile::exists(file)) return info;
    QProcess vProbe;
    vProbe.start(ffprobePath, QStringList() << "-v" << "quiet" << "-select_streams" << "v:0"
    << "-show_entries" << "stream=codec_name,width,height,r_frame_rate,pix_fmt,sample_aspect_ratio"
    << "-of" << "csv=p=0" << file);
    vProbe.waitForFinished(5000);
    QString vOut = QString(vProbe.readAllStandardOutput()).trimmed();
    QStringList vData = vOut.split(",");
    if (vData.size() >= 5) {
        info.videoCodec = vData[0];
        info.width = vData[1].toInt();
        info.height = vData[2].toInt();
        info.fps = vData[3];
        if (info.fps.contains("/")) {
            QStringList parts = info.fps.split("/");
            if (parts.size() == 2) {
                double num = parts[0].toDouble();
                double den = parts[1].toDouble();
                if (den != 0) info.fps = QString::number(num / den, 'f', 1);
            }
        }
        info.pixFmt = vData[4];
        info.sar = vData.size() > 5 ? vData[5] : "1:1";
    }
    QProcess aProbe;
    aProbe.start(ffprobePath, QStringList() << "-v" << "quiet" << "-select_streams" << "a:0"
    << "-show_entries" << "stream=codec_name,sample_rate"
    << "-of" << "csv=p=0" << file);
    aProbe.waitForFinished(5000);
    QString aOut = QString(aProbe.readAllStandardOutput()).trimmed();
    QStringList aData = aOut.split(",");
    if (aData.size() >= 2) {
        info.audioCodec = aData[0];
        info.audioSampleRate = aData[1].toInt();
        info.hasAudio = !info.audioCodec.isEmpty();
    } else {
        info.audioCodec = "none";
    }

    QProcess durProbe;
    durProbe.start(ffprobePath, QStringList() << "-v" << "quiet" << "-show_entries" << "format=duration"
    << "-of" << "csv=p=0" << file);
    durProbe.waitForFinished(5000);
    info.duration = QString(durProbe.readAllStandardOutput()).trimmed().toDouble();

    return info;
}

bool CombineTab::checkAllFilesCompatible(const QList<StreamInfo>& streams, StreamInfo& common) {
    if (streams.isEmpty()) return false;
    common = streams[0];
    for (const auto& s : streams) {
        if (s.videoCodec != common.videoCodec ||
            s.width != common.width || s.height != common.height ||
            s.fps != common.fps || s.pixFmt != common.pixFmt ||
            s.sar != common.sar || s.hasAudio != common.hasAudio ||
            (s.hasAudio && s.audioSampleRate != common.audioSampleRate)) {
            return false;
            }
    }
    return true;
}

void CombineTab::createConcatListFile(const QMap<int, QString> &orderMap)
{
    QSettings settings("FFmpegConverter", "Settings");
    QString ffmpegPath = settings.value("ffmpegPath", "/usr/bin/ffmpeg").toString();
    if (ffmpegPath.isEmpty()) ffmpegPath = "/usr/bin/ffmpeg";
    if (ffmpegPath.contains("ffprobe", Qt::CaseInsensitive)) {
        emit logMessage("⚠️ FFmpeg path is set to ffprobe—auto-fixing to ffmpeg");
        ffmpegPath = ffmpegPath.replace("ffprobe", "ffmpeg", Qt::CaseInsensitive);
    }
    QString ffprobePath = ffmpegPath.replace("ffmpeg", "ffprobe", Qt::CaseInsensitive);
    QProcess testProcess;
    testProcess.start(ffmpegPath, QStringList() << "-version");
    if (testProcess.waitForFinished(2000)) {
        QString versionOutput = testProcess.readAllStandardOutput().trimmed();
        if (!versionOutput.startsWith("ffmpeg version")) {
            emit logMessage("⚠️ Invalid FFmpeg path detected in Combine: " + ffmpegPath + " (not FFmpeg) - resetting to /usr/bin/ffmpeg");
            ffmpegPath = "/usr/bin/ffmpeg";
            ffprobePath = "/usr/bin/ffprobe";
        }
    } else {
        emit logMessage("⚠️ Could not validate FFmpeg path: " + ffmpegPath + " - resetting to /usr/bin/ffmpeg");
        ffmpegPath = "/usr/bin/ffmpeg";
        ffprobePath = "/usr/bin/ffprobe";
    }
    QList<StreamInfo> streams;
    totalDuration = 0.0;
    int fileCount = 0;
    QList<QString> sortedFiles;
    QList<int> sortedKeys = orderMap.keys();
    std::sort(sortedKeys.begin(), sortedKeys.end());
    for (int key : sortedKeys) {
        QString file = orderMap.value(key);
        sortedFiles.append(file);
        StreamInfo info = probeFile(file, ffprobePath);
        if (info.width > 0 && info.duration > 0) {
            streams.append(info);
            totalDuration += info.duration;
            fileCount++;
            emit logMessage(QString("Probed %1: %2x%3 @ %4 fps, dur %5s, audio: %6")
            .arg(QFileInfo(file).fileName())
            .arg(info.width).arg(info.height).arg(info.fps)
            .arg(info.duration, 0, 'f', 2)
            .arg(info.hasAudio ? "yes" : "no"));
        } else {
            emit logMessage(QString("⚠️ Skipped invalid %1").arg(QFileInfo(file).fileName()));
        }
    }
    QList<QString> validSortedFiles;
    QList<StreamInfo> validStreams;
    double validTotalDuration = 0.0;
    int validCount = 0;
    for (int idx = 0; idx < sortedFiles.size(); ++idx) {
        QString file = sortedFiles[idx];
        if (QFile::exists(file)) {
            validSortedFiles.append(file);
            validStreams.append(streams[idx]);
            validTotalDuration += streams[idx].duration;
            validCount++;
        } else {
            emit logMessage(QString("⚠️ Missing file after probe: %1 - skipping").arg(file));
        }
    }
    sortedFiles = validSortedFiles;
    streams = validStreams;
    totalDuration = validTotalDuration;
    if (streams.isEmpty()) {
        QMessageBox::warning(this, "Error", "No valid videos found! (All files missing or invalid)");
        emit conversionFinished();
        return;
    }
    emit logMessage(QString("✅ All %1 files verified exist").arg(validCount));
    emit logMessage(QString("Total: %1 files, ~%2s duration").arg(validCount).arg(totalDuration, 0, 'f', 1));

    StreamInfo common;
    bool compatible = checkAllFilesCompatible(streams, common);
    bool forceReencode = reencodeCheck->isChecked();
    bool useFilter = !compatible || forceReencode;
    if (useFilter) {
        emit logMessage("🔄 Incompatible streams detected → Using concat filter + re-encode");
    } else {
        emit logMessage("✅ All streams identical → Fast stream copy (demuxer)");
    }
    finalOutputFile = getFinalOutputFile();
    if (finalOutputFile.isEmpty()) {
        QMessageBox::warning(this, "Error", "Output path invalid!");
        emit conversionFinished();
        return;
    }
    QStringList args;
    args << "-y" << "-loglevel" << "warning";
    if (!useFilter) {
        concatTempFile = new QTemporaryFile(QDir::tempPath() + "/ffmpeg_converter_temp_XXXXXX.txt", this);
        if (!concatTempFile->open()) {
            QMessageBox::critical(this, "Error", "Cannot create temporary concat list");
            emit conversionFinished();
            return;
        }
        QTextStream stream(concatTempFile);
        for (const QString& file : sortedFiles) {
            stream << "file '" << QDir::toNativeSeparators(file) << "'\n";
        }
        stream.flush();
        concatTempFile->close();
        args << "-f" << "concat" << "-safe" << "0" << "-i" << concatTempFile->fileName() << "-c" << "copy" << finalOutputFile;
    } else {
        QString targetCodec = targetCodecCombo->currentText();
        QString vEncoder;
        QStringList vParams;
        if (targetCodec == "AV1") {
            vEncoder = "libsvtav1"; vParams << "-preset" << "8" << "-crf" << "30" << "-threads" << "0";
        } else if (targetCodec == "VP9") {
            vEncoder = "libvpx-vp9"; vParams << "-crf" << "31" << "-b:v" << "0" << "-threads" << "0";
        } else {
            vEncoder = "libx265"; vParams << "-preset" << "medium" << "-crf" << "23" << "-threads" << "0";
        }
        int minW = streams[0].width, minH = streams[0].height;
        for (const auto& s : streams) {
            minW = std::min(minW, s.width);
            minH = std::min(minH, s.height);
        }
        minW = (minW / 2) * 2;
        minH = (minH / 2) * 2;
        emit logMessage(QString("📏 Scaling all to common res: %1x%2").arg(minW).arg(minH));
        for (const QString& file : sortedFiles) {
            if (!QFile::exists(file)) {
                emit logMessage(QString("⚠️ File missing at encode time: %1 - aborting concat").arg(file));
                QMessageBox::warning(this, "Error", QString("File vanished during processing: %1\nPlease check if deleted/moved.").arg(file));
                emit conversionFinished();
                return;
            }
        }
        emit logMessage("✅ All files confirmed ready for encode");
        for (const QString& file : sortedFiles) {
            args << "-i" << file;
            emit logMessage(QString("📁 Adding input: %1").arg(file));
        }
        QStringList aFilters;
        int silenceIdx = streams.size();
        for (int i = 0; i < streams.size(); ++i) {
            const auto& s = streams[i];
            QString inV = QString("[%1:v]").arg(i);
            aFilters << QString("%1 scale=%2:%3:force_original_aspect_ratio=decrease,pad=%2:%3:(ow-iw)/2:(oh-ih)/2,setsar=1:1,setpts=PTS-STARTPTS,fps=30 [v%4];")
            .arg(inV).arg(minW).arg(minH).arg(i);
            if (!s.hasAudio) {
                QString silence = QString("anullsrc=channel_layout=stereo:sample_rate=48000:duration=%1").arg(s.duration, 0, 'g', 6);
                args << "-f" << "lavfi" << "-i" << silence;
                aFilters << QString("[%1:a] asetpts=PTS-STARTPTS [a%2];").arg(silenceIdx).arg(i);
                silenceIdx++;
            } else {
                QString inA = QString("[%1:a]").arg(i);
                aFilters << QString("%1 aresample=48000,asetpts=PTS-STARTPTS [a%2];").arg(inA).arg(i);
            }
        }
        QString vChain;
        for (int i = 0; i < streams.size(); ++i) vChain += QString("[v%1]").arg(i);
        vChain += QString("concat=n=%1:v=1:a=0 [v_out];").arg(streams.size());
        QString aChain = "[a0]";
        for (int i = 1; i < streams.size(); ++i) {
            aChain += QString("[a%1]").arg(i);
        }
        aChain += QString("concat=n=%1:v=0:a=1 [a_out]").arg(streams.size());
        QString filterComplex = aFilters.join("") + vChain + aChain;
        args << "-filter_complex" << filterComplex
        << "-map" << "[v_out]" << "-map" << "[a_out]"
        << "-c:v" << vEncoder;
        args.append(vParams);
        args << "-c:a" << "libopus" << "-b:a" << "128k"
        << finalOutputFile;
    }
    concatProcess = new QProcess(this);
    emit logMessage(QString("🔍 Final FFmpeg path for combine: %1").arg(ffmpegPath));
    emit logMessage(QString("🚀 Command: %1 %2").arg(ffmpegPath).arg(args.join(" ")));
    connect(concatProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus) {
                if (exitCode == 0) {
                    emit logMessage("✅ Combine finished: " + finalOutputFile);
                } else {
                    emit logMessage("❌ Combine failed (code " + QString::number(exitCode) + ")");
                }
                if (concatTempFile) {
                    concatTempFile->deleteLater();
                    concatTempFile = nullptr;
                }
                concatProcess->deleteLater();
                concatProcess = nullptr;
                emit conversionFinished();
            });
    connect(concatProcess, &QProcess::readyReadStandardOutput, this, &CombineTab::onStdoutReady);
    connect(concatProcess, &QProcess::readyReadStandardError, this, &CombineTab::onStderrReady);
    concatProcess->start(ffmpegPath, args);
}

void CombineTab::onStdoutReady() {
    if (!concatProcess) return;
    QByteArray data = concatProcess->readAllStandardOutput();
    QString out = QString(data).trimmed();
    if (!out.isEmpty()) emit logMessage(out);
    QRegularExpression timeRe("time=(\\d{2}:\\d{2}:\\d{2}\\.\\d{2})");
    QRegularExpressionMatch match = timeRe.match(out);
    if (match.hasMatch() && totalDuration > 0) {
        QTime t = QTime::fromString(match.captured(1), "hh:mm:ss.zz");
        double secs = t.hour() * 3600.0 + t.minute() * 60.0 + t.second() + t.msec() / 1000.0;
        int progress = static_cast<int>((secs / totalDuration) * 100.0);
        emit progressUpdated(std::min(progress, 100));
    }
}

void CombineTab::onStderrReady() {
    if (!concatProcess) return;
    QByteArray data = concatProcess->readAllStandardError();
    QString err = QString(data).trimmed();
    if (!err.isEmpty()) emit logMessage(err);
}

void CombineTab::selectInputDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select folder with videos");
    if (!dir.isEmpty()) {
        inputDirEdit->setText(dir);
        populateTable();
    }
}

void CombineTab::selectOutputDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select output folder");
    if (!dir.isEmpty()) {
        outputDirEdit->setText(dir);
    }
}

void CombineTab::populateTable()
{
    table->setRowCount(0);
    QString dir = inputDirEdit->text();
    if (dir.isEmpty()) return;
    QDir qdir(dir);
    qdir.setNameFilters(videoExtensions);
    qdir.setFilter(QDir::Files);
    QFileInfoList files = qdir.entryInfoList();
    table->setRowCount(files.size());
    for (int i = 0; i < files.size(); ++i) {
        const QFileInfo &fi = files.at(i);
        QSpinBox *spin = new QSpinBox();
        spin->setRange(0, 9999);
        spin->setValue(0);
        spin->setSpecialValueText(" ");
        table->setCellWidget(i, 0, spin);
        QTableWidgetItem *item = new QTableWidgetItem(fi.fileName());
        item->setData(Qt::UserRole, fi.absoluteFilePath());
        table->setItem(i, 1, item);
    }
}

void CombineTab::startConcatenation()
{
    if (inputDirEdit->text().isEmpty() || outputDirEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Select input and output folders");
        return;
    }
    if (outputNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Error", "Enter output filename");
        return;
    }
    QMap<int, QString> orderMap;
    for (int row = 0; row < table->rowCount(); ++row) {
        QSpinBox *spin = qobject_cast<QSpinBox*>(table->cellWidget(row, 0));
        QTableWidgetItem *item = table->item(row, 1);
        if (spin && item && spin->value() > 0) {
            orderMap[spin->value()] = item->data(Qt::UserRole).toString();
        }
    }
    if (orderMap.isEmpty()) {
        QMessageBox::warning(this, "Error", "No videos selected (use order > 0)");
        return;
    }
    QList<int> keys = orderMap.keys();
    std::sort(keys.begin(), keys.end());
    QMap<int, QString> sorted;
    for (int k : keys) sorted[k] = orderMap[k];
    createConcatListFile(sorted);
}

void CombineTab::cancelConcatenation()
{
    if (concatProcess && (concatProcess->state() == QProcess::Running || concatProcess->state() == QProcess::Starting)) {
        concatProcess->kill();
        concatProcess->waitForFinished(3000);
        emit logMessage("🛑 Concatenation cancelled by user");
    }
    if (concatTempFile) {
        concatTempFile->deleteLater();
        concatTempFile = nullptr;
    }
    concatProcess = nullptr;
}
