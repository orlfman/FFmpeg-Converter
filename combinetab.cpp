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

    table = new QTableWidget(0, 2);
    table->setHorizontalHeaderLabels({"Order (0=skip)", "Filename"});
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(table);

    connect(inputBtn, &QPushButton::clicked, this, &CombineTab::selectInputDirectory);
    connect(outputBtn, &QPushButton::clicked, this, &CombineTab::selectOutputDirectory);
    connect(inputBtn, &QPushButton::clicked, this, &CombineTab::populateTable);

    connect(containerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CombineTab::smartUpdateExtension);
    connect(outputNameEdit, &QLineEdit::textChanged, this, &CombineTab::smartUpdateExtension);

    setLayout(mainLayout);
}

void CombineTab::smartUpdateExtension()
{
    QString currentText = outputNameEdit->text();
    QString desiredExt = containerCombo->currentText();
    QString currentExt = QFileInfo(currentText).suffix().toLower();

    if (currentText.isEmpty() || currentExt.isEmpty() || currentExt != desiredExt) {
        int cursorPos = outputNameEdit->cursorPosition();

        QString baseName = QFileInfo(currentText).completeBaseName();
        if (baseName.isEmpty()) {
            baseName = "combined_video";
        }

        QString newText = baseName + "." + desiredExt;

        if (newText != currentText) {
            outputNameEdit->blockSignals(true);
            outputNameEdit->setText(newText);
            outputNameEdit->blockSignals(false);

            int newCursorPos = baseName.length();
            outputNameEdit->setCursorPosition(newCursorPos);
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

void CombineTab::createConcatListFile(const QMap<int, QString> &orderMap)
{
    concatTempFile = new QTemporaryFile(QDir::tempPath() + "/ffmpeg_converter_temp_XXXXXX.txt", this);
    if (!concatTempFile->open()) {
        QMessageBox::critical(this, "Error", "Cannot create temporary concat list");
        emit logMessage("ERROR: Could not create temporary file for concat list");
        emit conversionFinished();
        return;
    }

    QTextStream stream(concatTempFile);
    for (int key : orderMap.keys()) {
        QString path = orderMap.value(key);
        stream << "file '" << QDir::toNativeSeparators(path) << "'\n";
    }
    stream.flush();
    concatTempFile->close();

    finalOutputFile = getFinalOutputFile();

    QSettings settings("FFmpegConverter", "Settings");
    QString ffmpegPath = settings.value("ffmpegPath", "/usr/bin/ffmpeg").toString();
    if (ffmpegPath.isEmpty()) ffmpegPath = "/usr/bin/ffmpeg";

    QStringList args;
    args << "-f" << "concat"
         << "-safe" << "0"
         << "-i" << concatTempFile->fileName()
         << "-c" << "copy"
         << "-y"
         << finalOutputFile;

    QProcess *proc = new QProcess(this);
    emit logMessage("Starting video combine...");
    emit logMessage("Command: " + ffmpegPath + " " + args.join(" "));

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc](int exitCode, QProcess::ExitStatus) {
        if (exitCode == 0) {
            emit logMessage("Combine finished successfully: " + finalOutputFile);
        } else {
            emit logMessage("Combine failed with code " + QString::number(exitCode));
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
    QMessageBox::information(this, "Cancelled", "Concatenation cancelled.");
}
