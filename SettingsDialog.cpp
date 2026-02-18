#include "SettingsDialog.h"
#include <QFile>
#include <QFileInfo>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("⚙️ Settings");
    setMinimumWidth(500);
    resize(1280, 720);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();

    defaultCodecComboBox = new QComboBox();
    defaultCodecComboBox->addItems({"AV1", "x265", "VP9"});
    defaultCodecComboBox->setCurrentIndex(0);
    formLayout->addRow("Default Codec Tab:", defaultCodecComboBox);

    ffmpegPathButton = new QPushButton("📁 Browse...");
    ffmpegPathLineEdit = new QLineEdit();
    ffmpegPathLineEdit->setReadOnly(true);
    QHBoxLayout *ffmpegLayout = new QHBoxLayout();
    ffmpegLayout->addWidget(ffmpegPathLineEdit);
    ffmpegLayout->addWidget(ffmpegPathButton);
    formLayout->addRow("FFmpeg Path:", ffmpegLayout);

    svtAv1PathButton = new QPushButton("📁 Browse...");
    svtAv1PathLineEdit = new QLineEdit();
    svtAv1PathLineEdit->setReadOnly(true);
    svtAv1PathLineEdit->setPlaceholderText("Leave blank to use system default");
    QHBoxLayout *svtAv1Layout = new QHBoxLayout();
    svtAv1Layout->addWidget(svtAv1PathLineEdit);
    svtAv1Layout->addWidget(svtAv1PathButton);
    formLayout->addRow("SVT-AV1 Library Path:", svtAv1Layout);

    notifyOnFinishCheck = new QCheckBox("Show notification when conversion finishes");
    formLayout->addRow(notifyOnFinishCheck);

    defaultOutputDirButton = new QPushButton("📁 Browse...");
    defaultOutputDirLineEdit = new QLineEdit();
    defaultOutputDirLineEdit->setReadOnly(true);
    QHBoxLayout *outputDirLayout = new QHBoxLayout();
    outputDirLayout->addWidget(defaultOutputDirLineEdit);
    outputDirLayout->addWidget(defaultOutputDirButton);
    formLayout->addRow("Default Output Directory:", outputDirLayout);

    mainLayout->addLayout(formLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(defaultOutputDirButton, &QPushButton::clicked, this, &SettingsDialog::browseOutputDirectory);
    connect(ffmpegPathButton, &QPushButton::clicked, this, &SettingsDialog::browseFFmpegPath);
    connect(svtAv1PathButton, &QPushButton::clicked, this, &SettingsDialog::browseSvtAv1Path);

    loadSettings();
}

QString SettingsDialog::getDefaultOutputDir() const { return defaultOutputDirLineEdit->text(); }
int SettingsDialog::getDefaultCodecTab() const { return defaultCodecComboBox->currentIndex(); }
QString SettingsDialog::getFFmpegPath() const { return ffmpegPathLineEdit->text(); }
bool SettingsDialog::getNotifyOnFinish() const { return notifyOnFinishCheck->isChecked(); }
QString SettingsDialog::getSvtAv1Path() const { return svtAv1PathLineEdit->text(); }

void SettingsDialog::browseOutputDirectory()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setWindowTitle("Select Default Output Directory");
    QSettings s("FFmpegConverter", "Settings");
    QString last = s.value("lastSettingsBrowseDir", QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).toString();
    dialog.setDirectory(last);
    if (dialog.exec()) {
        QString dir = dialog.selectedFiles().first();
        defaultOutputDirLineEdit->setText(dir);
        s.setValue("lastSettingsBrowseDir", QFileInfo(dir).absolutePath());
    }
}

void SettingsDialog::browseFFmpegPath()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilters({"FFmpeg Executables (ffmpeg, ffprobe)", "All Files (*.*)"});
    dialog.setWindowTitle("Select FFmpeg Executable");

    QSettings s("FFmpegConverter", "Settings");
    QString last = s.value("lastFFmpegDir", QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).toString();
    dialog.setDirectory(last);

    if (dialog.exec()) {
        QString selectedPath = dialog.selectedFiles().first();

        QProcess testProcess;
        testProcess.start(selectedPath, QStringList() << "-version");
        if (!testProcess.waitForFinished(5000) || testProcess.exitCode() != 0) {
            QMessageBox::warning(this, "Error", "Invalid FFmpeg executable selected.");
            return;
        }

        QString versionOutput = testProcess.readAllStandardOutput().trimmed();
        if (!versionOutput.startsWith("ffmpeg version")) {
            QMessageBox::warning(this, "Error", QString("This is not FFmpeg! Output starts with: \"%1\"").arg(versionOutput.left(20)));
            return;
        }

        // Check ffprobe too
        QString ffprobePath = selectedPath;
        if (ffprobePath.contains("ffmpeg", Qt::CaseInsensitive))
            ffprobePath.replace("ffmpeg", "ffprobe", Qt::CaseInsensitive);

        testProcess.start(ffprobePath, QStringList() << "-version");
        if (!testProcess.waitForFinished(5000) || testProcess.exitCode() != 0) {
            QMessageBox::warning(this, "Error", "FFprobe not found or invalid at: " + ffprobePath);
            return;
        }

        QString ffprobeVersionOutput = testProcess.readAllStandardOutput().trimmed();
        if (!ffprobeVersionOutput.startsWith("ffprobe version")) {
            QMessageBox::warning(this, "Error", QString("This is not ffprobe! Output starts with: \"%1\"").arg(ffprobeVersionOutput.left(20)));
            return;
        }

        ffmpegPathLineEdit->setText(selectedPath);
        s.setValue("lastFFmpegDir", QFileInfo(selectedPath).absolutePath());
    }
}

void SettingsDialog::browseSvtAv1Path()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setWindowTitle("Select SVT-AV1 Library Directory");
    QSettings s("FFmpegConverter", "Settings");
    QString last = s.value("lastSvtAv1Dir", QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).toString();
    dialog.setDirectory(last);
    if (dialog.exec()) {
        QString dir = dialog.selectedFiles().first();
        svtAv1PathLineEdit->setText(dir);
        s.setValue("lastSvtAv1Dir", QFileInfo(dir).absolutePath());
    }
}

void SettingsDialog::loadSettings()
{
    QSettings s("FFmpegConverter", "Settings");
    defaultCodecComboBox->setCurrentIndex(s.value("defaultCodecTab", 0).toInt());
    QString savedFFmpeg = s.value("ffmpegPath", "/usr/bin/ffmpeg").toString();
    ffmpegPathLineEdit->setText(savedFFmpeg);
    if (savedFFmpeg.contains("ffprobe", Qt::CaseInsensitive) || !QFile::exists(savedFFmpeg)) {
        ffmpegPathLineEdit->setText("/usr/bin/ffmpeg");
        s.setValue("ffmpegPath", "/usr/bin/ffmpeg");
    }
    svtAv1PathLineEdit->setText(s.value("svtAv1Path", "").toString());
    defaultOutputDirLineEdit->setText(s.value("defaultOutputDir",
                                              QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString());
    notifyOnFinishCheck->setChecked(s.value("notifyOnFinish", true).toBool());
}
