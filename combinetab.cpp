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
    reencodeCheck->setChecked(false);
    codecLabel = new QLabel("Target Codec:");
    targetCodecCombo = new QComboBox();

    reencodeLayout->addWidget(reencodeCheck);
    reencodeLayout->addWidget(codecLabel);
    reencodeLayout->addWidget(targetCodecCombo);
    reencodeLayout->addStretch();
    mainLayout->addLayout(reencodeLayout);

    codecLabel->setVisible(false);
    targetCodecCombo->setVisible(false);

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

bool CombineTab::checkAllFilesCompatible(const QMap<int, QString> &orderMap, QString &videoCodec, QString &audioCodec)
{
    QSettings settings("FFmpegConverter", "Settings");
    QString ffmpegPath = settings.value("ffmpegPath", "/usr/bin/ffmpeg").toString();
    if (ffmpegPath.isEmpty()) ffmpegPath = "/usr/bin/ffmpeg";
    QString ffprobePath = ffmpegPath.replace("ffmpeg", "ffprobe", Qt::CaseInsensitive);

    bool first = true;
    for (const QString &file : orderMap.values()) {
        QProcess probe;
        probe.start(ffprobePath, QStringList() << "-v" << "error" << "-select_streams" << "v:0" << "-show_entries" << "stream=codec_name" << "-of" << "default=noprint_wrappers=1:nokey=1" << file);
        probe.waitForFinished(8000);
        QString vCodec = probe.readAllStandardOutput().trimmed();

        probe.start(ffprobePath, QStringList() << "-v" << "error" << "-select_streams" << "a:0" << "-show_entries" << "stream=codec_name" << "-of" << "default=noprint_wrappers=1:nokey=1" << file);
        probe.waitForFinished(8000);
        QString aCodec = probe.readAllStandardOutput().trimmed();
        if (aCodec.isEmpty()) aCodec = "none";

        if (first) {
            videoCodec = vCodec;
            audioCodec = aCodec;
            first = false;
        } else {
            if (vCodec != videoCodec || aCodec != audioCodec)
                return false;
        }
    }
    return true;
}

void CombineTab::createConcatListFile(const QMap<int, QString> &orderMap)
{
    concatTempFile = new QTemporaryFile(QDir::tempPath() + "/ffmpeg_converter_temp_XXXXXX.txt", this);
    if (!concatTempFile->open()) {
        QMessageBox::critical(this, "Error", "Cannot create temporary concat list");
        emit logMessage("ERROR: Could not create temporary file");
        emit conversionFinished();
        return;
    }

    QTextStream stream(concatTempFile);
    for (int key : orderMap.keys()) {
        stream << "file '" << QDir::toNativeSeparators(orderMap.value(key)) << "'\n";
    }
    stream.flush();
    concatTempFile->close();

    finalOutputFile = getFinalOutputFile();

    QSettings settings("FFmpegConverter", "Settings");
    QString ffmpegPath = settings.value("ffmpegPath", "/usr/bin/ffmpeg").toString();
    if (ffmpegPath.isEmpty()) ffmpegPath = "/usr/bin/ffmpeg";

    QStringList args;
    args << "-f" << "concat" << "-safe" << "0" << "-i" << concatTempFile->fileName();

    bool needReencode = false;
    QString commonV, commonA;

    if (reencodeCheck->isChecked()) {
        if (!checkAllFilesCompatible(orderMap, commonV, commonA)) {
            needReencode = true;
            emit logMessage("Incompatible files detected → re-encoding to " + targetCodecCombo->currentText());
        } else {
            emit logMessage("All files compatible (" + commonV + " + " + (commonA == "none" ? "no audio" : commonA) + ") → fast stream copy");
        }
    }

    if (needReencode) {
        QString selected = targetCodecCombo->currentText();
        if (selected == "AV1") {
            args << "-c:v" << "libsvtav1" << "-preset" << "8" << "-crf" << "30";
        } else if (selected == "VP9") {
            args << "-c:v" << "libvpx-vp9" << "-crf" << "31" << "-b:v" << "0";
        } else {
            args << "-c:v" << "libx265" << "-preset" << "medium" << "-crf" << "23";
        }
        args << "-c:a" << "libopus" << "-b:a" << "128k";
    } else {
        args << "-c" << "copy";
    }

    args << "-y" << finalOutputFile;

    QProcess *proc = new QProcess(this);
    emit logMessage("Starting combine...");
    emit logMessage("Command: " + ffmpegPath + " " + args.join(" "));

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc](int exitCode, QProcess::ExitStatus) {
                if (exitCode == 0) {
                    emit logMessage("Combine finished: " + finalOutputFile);
                } else {
                    emit logMessage("Combine failed (code " + QString::number(exitCode) + ")");
                }
                proc->deleteLater();
                emit conversionFinished();
            });

    connect(proc, &QProcess::readyReadStandardOutput, this, [this, proc]() {
        emit logMessage(proc->readAllStandardOutput().trimmed());
    });
    connect(proc, &QProcess::readyReadStandardError, this, [this, proc]() {
        emit logMessage(proc->readAllStandardError().trimmed());
    });

    proc->start(ffmpegPath, args);
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
    QMessageBox::information(this, "Cancelled", "Concatenation cancelled (if running).");
}
