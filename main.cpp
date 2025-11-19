#include "av1tab.h"
#include "converter.h"
#include "vp9tab.h"
#include "x265tab.h"
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMovie>
#include <QMutex>
#include <QPixmap>
#include <QPointer>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSettings>
#include <QSlider>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QStringList>
#include <QTabWidget>
#include <QTextEdit>
#include <QTime>
#include <QTimer>
#include <QToolTip>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <QtConcurrent>
#include <QRandomGenerator>
#include <QUuid>
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("⚙️ Settings");
        setMinimumWidth(500);
        resize(550, 350);
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
        notifyOnFinishCheck = new QCheckBox();
        notifyOnFinishCheck->setText("Show notification when conversion finishes");
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
    QString getDefaultOutputDir() const { return defaultOutputDirLineEdit->text(); }
    int getDefaultCodecTab() const { return defaultCodecComboBox->currentIndex(); }
    QString getFFmpegPath() const { return ffmpegPathLineEdit->text(); }
    bool getNotifyOnFinish() const { return notifyOnFinishCheck->isChecked(); }
    QString getSvtAv1Path() const { return svtAv1PathLineEdit->text(); }
private slots:
    void browseOutputDirectory() {
        QFileDialog dialog(this);
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setWindowTitle("Select Default Output Directory");
        QSettings settings("FFmpegConverter", "Settings");
        QString lastBrowseDir = settings.value("lastSettingsBrowseDir",
                                               QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).toString();
                                               dialog.setDirectory(lastBrowseDir);
                                               if (dialog.exec()) {
                                                   QString selectedDir = dialog.selectedFiles().first();
                                                   defaultOutputDirLineEdit->setText(selectedDir);
                                                   settings.setValue("lastSettingsBrowseDir", QFileInfo(selectedDir).absolutePath());
                                               }
    }
    void browseFFmpegPath() {
        QFileDialog dialog(this);
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setNameFilters({"FFmpeg Executables (ffmpeg, ffprobe)", "All Files (*.*)"});
        dialog.setWindowTitle("Select FFmpeg Executable");
        QSettings settings("FFmpegConverter", "Settings");
        QString lastFFmpegDir = settings.value("lastFFmpegDir", QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).toString();
        dialog.setDirectory(lastFFmpegDir);
        if (dialog.exec()) {
            QString selectedPath = dialog.selectedFiles().first();
            QProcess testProcess;
            testProcess.start(selectedPath, QStringList() << "-version");
            if (!testProcess.waitForFinished(5000) || testProcess.exitCode() != 0) {
                QMessageBox::warning(this, "Error", "Invalid FFmpeg executable selected.");
                return;
            }
            QString ffprobePath = selectedPath.replace("ffmpeg", "ffprobe");
            testProcess.start(ffprobePath, QStringList() << "-version");
            if (!testProcess.waitForFinished(5000) || testProcess.exitCode() != 0) {
                QMessageBox::warning(this, "Error", "FFprobe not found or invalid at: " + ffprobePath);
                return;
            }
            ffmpegPathLineEdit->setText(selectedPath);
            settings.setValue("lastFFmpegDir", QFileInfo(selectedPath).absolutePath());
        }
    }
    void browseSvtAv1Path() {
        QFileDialog dialog(this);
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setWindowTitle("Select SVT-AV1 Library Directory");
        QSettings settings("FFmpegConverter", "Settings");
        QString lastSvtAv1Dir = settings.value("lastSvtAv1Dir", QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).toString();
        dialog.setDirectory(lastSvtAv1Dir);
        if (dialog.exec()) {
            QString selectedDir = dialog.selectedFiles().first();
            svtAv1PathLineEdit->setText(selectedDir);
            settings.setValue("lastSvtAv1Dir", QFileInfo(selectedDir).absolutePath());
        }
    }
private:
    void loadSettings() {
        QSettings settings("FFmpegConverter", "Settings");
        int savedTab = settings.value("defaultCodecTab", 0).toInt();
        defaultCodecComboBox->setCurrentIndex(savedTab);
        QString savedFFmpeg = settings.value("ffmpegPath", "/usr/bin/ffmpeg").toString();
        ffmpegPathLineEdit->setText(savedFFmpeg);
        QString savedSvtAv1 = settings.value("svtAv1Path", "").toString();
        svtAv1PathLineEdit->setText(savedSvtAv1);
        QString savedDir = settings.value("defaultOutputDir",
                                          QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
                                          defaultOutputDirLineEdit->setText(savedDir);
                                          notifyOnFinishCheck->setChecked(settings.value("notifyOnFinish", true).toBool());
    }
    QComboBox *defaultCodecComboBox = nullptr;
    QPushButton *ffmpegPathButton = nullptr;
    QLineEdit *ffmpegPathLineEdit = nullptr;
    QPushButton *svtAv1PathButton = nullptr;
    QLineEdit *svtAv1PathLineEdit = nullptr;
    QPushButton *defaultOutputDirButton = nullptr;
    QLineEdit *defaultOutputDirLineEdit = nullptr;
    QCheckBox *notifyOnFinishCheck = nullptr;
};
// Function to show a notification when the conversion finishes
void showConversionNotification(const QString& outputFile, QWidget* parent) {
    QSettings settings("FFmpegConverter", "Settings");
    if (!settings.value("notifyOnFinish", true).toBool()) return;
    QString title = "🎉 FFmpeg Converter - Done!";
    QString message = QString("Output saved:\n%1").arg(QFileInfo(outputFile).fileName());
    QProcess notifyProcess;
    notifyProcess.start("notify-send", QStringList()
    << "--urgency=normal"
    << title
    << message
    << "--icon=video-x-generic");
    notifyProcess.waitForFinished(2000);
    if (notifyProcess.exitCode() != 0) {
        QMessageBox::information(parent, title, message);
    }
}
#include "main.moc"
int main(int argc, char *argv[]) {
    Q_UNUSED(argc); // Just suppressing those unused parameter warnings
    Q_UNUSED(argv);
    QApplication app(argc, argv);
    QMainWindow window;
    window.setWindowTitle("FFmpeg Converter");
    window.resize(800, 600);
    QWidget *centralWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    window.setCentralWidget(centralWidget);
    QHBoxLayout *selectFilesLayout = new QHBoxLayout();
    QPushButton *selectFilesButton = new QPushButton("Select File");
    QLineEdit *selectedFilesBox = new QLineEdit();
    selectedFilesBox->setReadOnly(true);
    selectFilesLayout->addWidget(selectFilesButton);
    selectFilesLayout->addWidget(selectedFilesBox);
    mainLayout->addLayout(selectFilesLayout);
    // This part handles output naming, including pulling titles from metadata
    QHBoxLayout *outputNameLayout = new QHBoxLayout();
    QComboBox *outputNameModeBox = new QComboBox();
    outputNameModeBox->addItem("Output Name"); // 0
    outputNameModeBox->addItem("Date"); // 1
    outputNameModeBox->addItem("Random"); // 2
    outputNameModeBox->addItem("UUID"); // 3
    outputNameModeBox->addItem("Metadata"); // 4
    outputNameModeBox->setCurrentIndex(0);
    QLineEdit *outputNameBox = new QLineEdit();
    QCheckBox *overwriteCheck = new QCheckBox("Overwrite");
    overwriteCheck->setChecked(false);
    overwriteCheck->setToolTip("If checked, overwrite existing files instead of adding numbers.");
    QString originalFilename;
    QString cachedMetadataTitle;
    // Here's the QSettings instance that everything else uses
    QSettings settings("FFmpegConverter", "Settings");
    auto extractTitle = [selectedFilesBox, &settings]() -> QString {
        QString input = selectedFilesBox->text().trimmed();
        if (input.isEmpty()) return QString();
        QString ffmpegPath = settings.value("ffmpegPath", "/usr/bin/ffmpeg").toString();
        QString ffprobePath = ffmpegPath.isEmpty() ? "ffprobe" : ffmpegPath.replace("ffmpeg", "ffprobe", Qt::CaseInsensitive);
        QProcess p;
        p.start(ffprobePath, {"-v", "quiet", "-print_format", "json", "-show_entries", "format_tags=title,TAG:title", input});
        p.waitForFinished(8000);
        if (p.exitCode() != 0) return QString();
        QJsonDocument doc = QJsonDocument::fromJson(p.readAllStandardOutput());
        if (doc.isNull() || !doc.object().contains("format")) return QString();
        QJsonObject fmt = doc.object()["format"].toObject();
        QJsonObject tags = fmt["tags"].toObject();
        QString title = tags["title"].toString();
        if (title.isEmpty()) title = tags["TAG:title"].toString();
        if (title.isEmpty()) title = fmt["title"].toString();
        title.remove(QRegularExpression("[<>:\"|?*\\\\/]"));
        return title.trimmed();
    };
    auto refreshName = [outputNameBox, outputNameModeBox, selectedFilesBox, &originalFilename, &cachedMetadataTitle, &extractTitle]() {
        int mode = outputNameModeBox->currentIndex();
        QString input = selectedFilesBox->text().trimmed();
        if (!input.isEmpty()) {
            originalFilename = QFileInfo(input).baseName();
        }
        QString name;
        if (mode == 0) {
            outputNameBox->setReadOnly(false);
            name = originalFilename.isEmpty() ? "Output" : originalFilename;
        }
        else {
            outputNameBox->setReadOnly(true);
            if (mode == 1) name = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
            else if (mode == 2) {
                const QString chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
                QString rand;
                for (int i = 0; i < 20; ++i) rand += chars[QRandomGenerator::global()->bounded(chars.size())];
                name = rand;
            }
            else if (mode == 3) name = QUuid::createUuid().toString(QUuid::WithoutBraces);
            else if (mode == 4) {
                if (cachedMetadataTitle.isEmpty()) cachedMetadataTitle = extractTitle();
                name = cachedMetadataTitle.isEmpty() ? originalFilename : cachedMetadataTitle;
            }
        }
        outputNameBox->blockSignals(true);
        outputNameBox->setText(name);
        outputNameBox->blockSignals(false);
    };
    refreshName();
    QObject::connect(selectedFilesBox, &QLineEdit::textChanged, [&cachedMetadataTitle, refreshName]() {
        cachedMetadataTitle.clear();
        refreshName();
    });
    QObject::connect(outputNameModeBox, &QComboBox::activated, [outputNameModeBox, &cachedMetadataTitle, refreshName]() {
        if (outputNameModeBox->currentIndex() == 4) cachedMetadataTitle.clear();
        refreshName();
    });
    QObject::connect(outputNameBox, &QLineEdit::textEdited, [](const QString&) { /* preserve user text */ });
    outputNameLayout->addWidget(outputNameModeBox);
    outputNameLayout->addWidget(outputNameBox);
    outputNameLayout->addWidget(overwriteCheck);
    mainLayout->addLayout(outputNameLayout);
    QHBoxLayout *outputDirLayout = new QHBoxLayout();
    QPushButton *outputDirButton = new QPushButton("Output Directory");
    QString defaultOutputDir = settings.value("defaultOutputDir",
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
    QLineEdit *outputDirBox = new QLineEdit(defaultOutputDir);
    outputDirLayout->addWidget(outputDirButton);
    outputDirLayout->addWidget(outputDirBox);
    mainLayout->addLayout(outputDirLayout);
    QHBoxLayout *scaleWidthLayout = new QHBoxLayout();
    QLabel *scaleWidthLabel = new QLabel("Scale Width:");
    QDoubleSpinBox *scaleWidthSpin = new QDoubleSpinBox();
    scaleWidthSpin->setRange(0.0, 10.0);
    scaleWidthSpin->setSingleStep(0.05);
    scaleWidthSpin->setDecimals(2);
    scaleWidthSpin->setValue(1.0);
    scaleWidthSpin->setSuffix("x");
    scaleWidthSpin->setToolTip("Scale width by this factor (1.0 = original)");
    scaleWidthLayout->addWidget(scaleWidthLabel);
    scaleWidthLayout->addWidget(scaleWidthSpin);
    QLabel *scaleFilterLabel = new QLabel("Scale Filter:");
    QComboBox *scaleFilterBox = new QComboBox();
    scaleFilterBox->addItems({"lanczos", "point", "bilinear", "bicubic", "spline16", "spline36"});
    scaleFilterBox->setCurrentIndex(0);
    scaleFilterBox->setToolTip("lanczos: High-quality, sharp images, slower.\n"
    "point: Fast, but blocky.\n"
    "bilinear: Fast, smooth but blurry.\n"
    "bicubic: Balanced speed and quality.\n"
    "spline16: Smooth resizing.\n"
    "spline36: Smoother than spline16.");
    scaleWidthLayout->addWidget(scaleFilterLabel);
    scaleWidthLayout->addWidget(scaleFilterBox);
    QCheckBox *eightBitCheck = new QCheckBox("Use 8-Bit Color");
    eightBitCheck->setChecked(true);
    eightBitCheck->setToolTip("Enable 8-bit color to select a format. Standard for most displays.");
    scaleWidthLayout->addWidget(eightBitCheck);
    QComboBox *eightBitColorFormatBox = new QComboBox();
    eightBitColorFormatBox->addItems({"8-bit 4:2:0", "8-bit 4:2:2", "8-bit 4:4:4"});
    eightBitColorFormatBox->setCurrentIndex(0);
    eightBitColorFormatBox->setToolTip("Select 8-bit color format: 4:2:0, 4:2:2, or 4:4:4.");
    scaleWidthLayout->addWidget(eightBitColorFormatBox);
    QCheckBox *tenBitCheck = new QCheckBox("Use 10-Bit Color");
    tenBitCheck->setToolTip("Enable 10-bit color to select a format. Reduces banding.");
    scaleWidthLayout->addWidget(tenBitCheck);
    QComboBox *colorFormatBox = new QComboBox();
    colorFormatBox->addItems({"10-bit 4:2:0", "10-bit 4:2:2", "10-bit 4:4:4"});
    colorFormatBox->setCurrentIndex(0);
    colorFormatBox->setEnabled(false);
    colorFormatBox->setToolTip("Select 10-bit color format: 4:2:0, 4:2:2, or 4:4:4.");
    scaleWidthLayout->addWidget(colorFormatBox);
    QCheckBox *cropCheck = new QCheckBox("Crop");
    scaleWidthLayout->addWidget(cropCheck);
    QPushButton *detectCropButton = new QPushButton("Detect Crop");
    detectCropButton->setEnabled(false);
    scaleWidthLayout->addWidget(detectCropButton);
    QLineEdit *cropValueBox = new QLineEdit();
    cropValueBox->setToolTip("Detected crop values in format w:h:x:y.\nApplied before scaling.");
    cropValueBox->setReadOnly(true);
    cropValueBox->setEnabled(false);
    scaleWidthLayout->addWidget(cropValueBox);
    scaleWidthLayout->addStretch();
    mainLayout->addLayout(scaleWidthLayout);
    QHBoxLayout *scaleHeightLayout = new QHBoxLayout();
    QLabel *scaleHeightLabel = new QLabel("Scale Height:");
    QDoubleSpinBox *scaleHeightSpin = new QDoubleSpinBox();
    scaleHeightSpin->setRange(0.0, 10.0);
    scaleHeightSpin->setSingleStep(0.05);
    scaleHeightSpin->setDecimals(2);
    scaleHeightSpin->setValue(1.0);
    scaleHeightSpin->setSuffix("x");
    scaleHeightSpin->setToolTip("Scale height by this factor (1.0 = original)");
    scaleHeightLayout->addWidget(scaleHeightLabel);
    scaleHeightLayout->addWidget(scaleHeightSpin);
    QLabel *scaleRangeLabel = new QLabel("Scale Range:");
    QComboBox *scaleRangeBox = new QComboBox();
    scaleRangeBox->addItems({"input", "limited", "full"});
    scaleRangeBox->setCurrentIndex(0);
    scaleRangeBox->setToolTip("input: Use input video range.\n"
    "limited: 16-235 (TV standard).\n"
    "full: 0-255 (monitor standard).");
    scaleHeightLayout->addWidget(scaleRangeLabel);
    scaleHeightLayout->addWidget(scaleRangeBox);
    QCheckBox *seekCheck = new QCheckBox("Seek");
    seekCheck->setToolTip("Enable to skip to a start time in the video.\nFormat: HH:MM:SS (e.g., 00:05:30 for 5 minutes 30 seconds).\nHours can be >23 for long videos.");
    scaleHeightLayout->addWidget(seekCheck);
    QLabel *seekLabel = new QLabel("HH:MM:SS");
    scaleHeightLayout->addWidget(seekLabel);
    QLineEdit *seekHH = new QLineEdit("0");
    seekHH->setFixedWidth(30);
    seekHH->setEnabled(false);
    seekHH->setValidator(new QIntValidator(0, 99, seekHH));
    scaleHeightLayout->addWidget(seekHH);
    QLabel *colon1 = new QLabel(":");
    scaleHeightLayout->addWidget(colon1);
    QLineEdit *seekMM = new QLineEdit("0");
    seekMM->setFixedWidth(30);
    seekMM->setEnabled(false);
    seekMM->setValidator(new QIntValidator(0, 59, seekMM));
    scaleHeightLayout->addWidget(seekMM);
    QLabel *colon2 = new QLabel(":");
    scaleHeightLayout->addWidget(colon2);
    QLineEdit *seekSS = new QLineEdit("0");
    seekSS->setFixedWidth(30);
    seekSS->setEnabled(false);
    seekSS->setValidator(new QIntValidator(0, 59, seekSS));
    scaleHeightLayout->addWidget(seekSS);
    QCheckBox *timeCheck = new QCheckBox("Time");
    timeCheck->setToolTip("Enable to limit output duration.\nFormat: HH:MM:SS (e.g., 01:00:00 for 1 hour).\nHours can be >23 for long videos.");
    scaleHeightLayout->addWidget(timeCheck);
    QLabel *timeLabel = new QLabel("HH:MM:SS");
    scaleHeightLayout->addWidget(timeLabel);
    QLineEdit *timeHH = new QLineEdit("0");
    timeHH->setFixedWidth(30);
    timeHH->setEnabled(false);
    timeHH->setValidator(new QIntValidator(0, 99, timeHH));
    scaleHeightLayout->addWidget(timeHH);
    QLabel *colon3 = new QLabel(":");
    scaleHeightLayout->addWidget(colon3);
    QLineEdit *timeMM = new QLineEdit("0");
    timeMM->setFixedWidth(30);
    timeMM->setEnabled(false);
    timeMM->setValidator(new QIntValidator(0, 59, timeMM));
    scaleHeightLayout->addWidget(timeMM);
    QLabel *colon4 = new QLabel(":");
    scaleHeightLayout->addWidget(colon4);
    QLineEdit *timeSS = new QLineEdit("0");
    timeSS->setFixedWidth(30);
    timeSS->setEnabled(false);
    timeSS->setValidator(new QIntValidator(0, 59, timeSS));
    scaleHeightLayout->addWidget(timeSS);
    QLabel *frameRateLabel = new QLabel("Frame Rate:");
    scaleHeightLayout->addWidget(frameRateLabel);
    QComboBox *frameRateBox = new QComboBox();
    frameRateBox->addItems({"Original", "24", "30", "60", "Custom"});
    frameRateBox->setCurrentIndex(0);
    frameRateBox->setToolTip("Set target frame rate.\nOriginal: Keep source rate.\nCustom: Enter custom value.");
    scaleHeightLayout->addWidget(frameRateBox);
    QLineEdit *customFrameRateBox = new QLineEdit();
    customFrameRateBox->setEnabled(false);
    customFrameRateBox->setValidator(new QDoubleValidator(0.1, 1000.0, 2, customFrameRateBox));
    scaleHeightLayout->addWidget(customFrameRateBox);
    scaleHeightLayout->addStretch();
    mainLayout->addLayout(scaleHeightLayout);
    QHBoxLayout *optionsLayout = new QHBoxLayout();
    QLabel *rotateLabel = new QLabel("Rotate/Flip:");
    optionsLayout->addWidget(rotateLabel);
    QComboBox *rotationBox = new QComboBox();
    rotationBox->addItems({"No Rotation", "90° Clockwise", "90° Counterclockwise", "180°", "Horizontal Flip", "Vertical Flip"});
    rotationBox->setCurrentIndex(0);
    rotationBox->setToolTip("Rotate or flip the video orientation. Applied before other filters like crop or scale.");
    optionsLayout->addWidget(rotationBox);
    QCheckBox *preserveMetadataCheck = new QCheckBox("Preserve metadata");
    preserveMetadataCheck->setToolTip("Keep original file metadata (title, author, etc).");
    optionsLayout->addWidget(preserveMetadataCheck);
    QCheckBox *removeChaptersCheck = new QCheckBox("Remove Chapters");
    removeChaptersCheck->setToolTip("Remove chapter markers from video.");
    optionsLayout->addWidget(removeChaptersCheck);
    QCheckBox *deinterlaceCheck = new QCheckBox("Deinterlace");
    deinterlaceCheck->setToolTip("Convert interlaced video to progressive scan.");
    optionsLayout->addWidget(deinterlaceCheck);
    optionsLayout->addStretch();
    mainLayout->addLayout(optionsLayout);
    QTabWidget *codecTabs = new QTabWidget();
    mainLayout->addWidget(codecTabs);
    Av1Tab *av1Tab = new Av1Tab();
    QScrollArea *av1Scroll = new QScrollArea();
    av1Scroll->setWidgetResizable(true);
    av1Scroll->setWidget(av1Tab);
    codecTabs->addTab(av1Scroll, "AV1");
    X265Tab *x265Tab = new X265Tab();
    QScrollArea *x265Scroll = new QScrollArea();
    x265Scroll->setWidgetResizable(true);
    x265Scroll->setWidget(x265Tab);
    codecTabs->addTab(x265Scroll, "x265");
    Vp9Tab *vp9Tab = new Vp9Tab();
    QScrollArea *vp9Scroll = new QScrollArea();
    vp9Scroll->setWidgetResizable(true);
    vp9Scroll->setWidget(vp9Tab);
    codecTabs->addTab(vp9Scroll, "VP9");
    // Loading the default codec tab from settings
    QSettings tabSettings("FFmpegConverter", "Settings");
    int defaultTab = tabSettings.value("defaultCodecTab", 0).toInt();
    codecTabs->setCurrentIndex(defaultTab);
    QWidget *infoTab = new QWidget();
    QVBoxLayout *infoLayout = new QVBoxLayout(infoTab);
    QTextEdit *infoBox = new QTextEdit();
    infoBox->setReadOnly(true);
    infoLayout->addWidget(infoBox);
    codecTabs->addTab(infoTab, "Information");
    QWidget *consoleTab = new QWidget();
    QVBoxLayout *consoleLayout = new QVBoxLayout(consoleTab);
    codecTabs->addTab(consoleTab, "Console");
    QHBoxLayout *consoleInputLayout = new QHBoxLayout();
    QLabel *consoleLabel = new QLabel("Enter FFmpeg commands:");
    QLineEdit *customCommandBox = new QLineEdit();
    customCommandBox->setPlaceholderText("ex: -i input.mp4 -c:v libx264 output.mkv");
    QPushButton *runCommandButton = new QPushButton("Run Command");
    QPushButton *clearLogButton = new QPushButton("Clear Console");
    consoleInputLayout->addWidget(consoleLabel);
    consoleInputLayout->addWidget(customCommandBox);
    consoleInputLayout->addWidget(runCommandButton);
    consoleInputLayout->addWidget(clearLogButton);
    consoleInputLayout->addStretch();
    consoleLayout->addLayout(consoleInputLayout);
    QTextEdit *logBox = new QTextEdit();
    logBox->setReadOnly(true);
    consoleLayout->addWidget(logBox);
    QHBoxLayout *conversionControlsLayout = new QHBoxLayout();
    QPushButton *convertButton = new QPushButton("Convert");
    QPushButton *cancelButton = new QPushButton("Cancel");
    cancelButton->setEnabled(false);
    conversionControlsLayout->addWidget(convertButton);
    conversionControlsLayout->addWidget(cancelButton);
    conversionControlsLayout->addStretch();
    mainLayout->addLayout(conversionControlsLayout);
    QProgressBar *conversionProgress = new QProgressBar();
    Converter *converter = new Converter(&window);
    conversionProgress->setVisible(false);
    conversionProgress->setRange(0, 100);
    mainLayout->addWidget(conversionProgress);
    auto getSampleRateInHz = [](const QString& sampleRateStr) -> QString {
        QString numericPart = sampleRateStr.split(" ").first();
        bool ok;
        double kHz = numericPart.toDouble(&ok);
        return ok ? QString::number(static_cast<int>(kHz * 1000)) : "48000";
    };
    auto getBitrateValue = [](const QString& bitrateStr) -> QString {
        QString numericPart = bitrateStr.split(" ").first();
        return numericPart.isEmpty() ? "128" : numericPart;
    };
    auto updateInfo = [&converter, infoBox, &settings, selectedFilesBox, logBox, tenBitCheck, eightBitCheck, colorFormatBox, eightBitColorFormatBox](const QString &inputFile) {
        if (inputFile.isEmpty()) {
            infoBox->setText("No input file selected.");
            return;
        }
        static_cast<void>(QtConcurrent::run([inputFile, infoBox, logBox, &settings, tenBitCheck, eightBitCheck, colorFormatBox, eightBitColorFormatBox]() {
            QFileInfo fileInfo(inputFile);
            if (!fileInfo.exists()) {
                QMetaObject::invokeMethod(infoBox, "setText", Qt::QueuedConnection, Q_ARG(QString, "Input file does not exist: " + inputFile));
                return;
            }
            if (!fileInfo.isReadable()) {
                QMetaObject::invokeMethod(infoBox, "setText", Qt::QueuedConnection, Q_ARG(QString, "Input file is not readable: " + inputFile));
                return;
            }
            QProcess ffprobe;
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            QString svtAv1Path = settings.value("svtAv1Path", "").toString();
            if (!svtAv1Path.isEmpty()) {
                QString existing = env.value("LD_LIBRARY_PATH");
                env.insert("LD_LIBRARY_PATH", existing.isEmpty() ? svtAv1Path : existing + ":" + svtAv1Path);
            }
            ffprobe.setProcessEnvironment(env);
            QString ffmpegPath = settings.value("ffmpegPath", "/usr/bin/ffmpeg").toString();
            QString ffprobePath;
            if (ffmpegPath.isEmpty()) {
                ffprobePath = "/usr/bin/ffprobe";
            } else {
                QFileInfo ffmpegInfo(ffmpegPath);
                QString probeCandidate = ffmpegInfo.absolutePath() + "/ffprobe" + ffmpegInfo.suffix();
                if (QFile::exists(probeCandidate)) {
                    ffprobePath = probeCandidate;
                } else {
                    ffprobePath = "/usr/bin/ffprobe";
                    QMetaObject::invokeMethod(logBox, "append", Qt::QueuedConnection, Q_ARG(QString, "⚠️ ffprobe not found near custom ffmpeg (" + ffmpegPath + "), using system: " + ffprobePath));
                }
            }
            QStringList args = {"-v", "error", "-show_format", "-show_streams", inputFile};
            QString ffprobeCommand = ffprobePath + " " + args.join(" ");
            QMetaObject::invokeMethod(logBox, "append", Qt::QueuedConnection, Q_ARG(QString, "Executing ffprobe: " + ffprobeCommand));
            ffprobe.start(ffprobePath, args);
            if (!ffprobe.waitForFinished(10000)) {
                QString error = ffprobe.readAllStandardError();
                QMetaObject::invokeMethod(logBox, "append", Qt::QueuedConnection, Q_ARG(QString, "ffprobe failed: " + error));
                QMetaObject::invokeMethod(infoBox, "setText", Qt::QueuedConnection, Q_ARG(QString, "Failed to retrieve information: " + error));
                return;
            }
            QString output = ffprobe.readAllStandardOutput();
            QString error = ffprobe.readAllStandardError();
            if (!error.isEmpty()) {
                QMetaObject::invokeMethod(logBox, "append", Qt::QueuedConnection, Q_ARG(QString, "ffprobe stderr: " + error));
            }
            if (output.isEmpty()) {
                QMetaObject::invokeMethod(infoBox, "setText", Qt::QueuedConnection, Q_ARG(QString, "Unable to retrieve information: " + error));
                return;
            }
            QString resolution = "N/A", videoCodec = "N/A", frameRate = "N/A", videoBitRate = "N/A";
            QString audioCodec = "N/A", pixFmt = "N/A", durationStr = "N/A", fileContainer = "N/A", title = "N/A";
            QString aspectRatio = "N/A", colorSpace = "N/A", pixelFormat = "N/A", audioSampleRate = "N/A";
            qint64 fileSizeBytes = QFileInfo(inputFile).size();
            QString fileSizeStr = (fileSizeBytes > 0) ? QString::number(fileSizeBytes / 1024.0 / 1024, 'f', 2) + " MB" : "N/A";
            fileContainer = QFileInfo(inputFile).suffix();
            if (fileContainer.isEmpty()) {
                fileContainer = "N/A";
            }
            QStringList lines = output.split("\n", Qt::SkipEmptyParts);
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
            QStringList preferredCodecs = {"h264", "hevc", "vp9", "av1", "vp8"};
            QStringList videoStreamCodecs;
            QList<int> videoStreamIndices;
            for (int i = 0; i < streams.size(); ++i) {
                const auto &stream = streams[i];
                if (stream.value("codec_type") == "video" && stream.value("disposition:attached_pic", "0") == "0") {
                    videoStreamCodecs.append(stream.value("codec_name", "N/A"));
                    videoStreamIndices.append(i);
                }
            }
            int selectedStreamIndex = -1;
            if (!videoStreamCodecs.isEmpty()) {
                for (int i = 0; i < videoStreamCodecs.size(); ++i) {
                    if (preferredCodecs.contains(videoStreamCodecs[i])) {
                        selectedStreamIndex = videoStreamIndices[i];
                        break;
                    }
                }
                if (selectedStreamIndex == -1) {
                    selectedStreamIndex = videoStreamIndices.last();
                }
            }
            if (selectedStreamIndex >= 0) {
                const auto &stream = streams[selectedStreamIndex];
                videoCodec = stream.value("codec_name", "N/A");
                resolution = stream.value("width", "N/A") + "x" + stream.value("height", "N/A");
                aspectRatio = stream.value("display_aspect_ratio", "N/A");
                frameRate = stream.value("avg_frame_rate", stream.value("r_frame_rate", "N/A"));
                if (frameRate.contains("/")) {
                    QStringList parts = frameRate.split("/");
                    if (parts.size() == 2) {
                        double num = parts[0].toDouble();
                        double den = parts[1].toDouble();
                        if (den != 0 && num != 0) {
                            frameRate = QString::number(num / den, 'f', 2);
                        } else {
                            frameRate = "N/A";
                        }
                    } else {
                        frameRate = "N/A";
                    }
                }
                videoBitRate = stream.value("bit_rate", "N/A");
                if (videoBitRate == "N/A" || videoBitRate.toInt() <= 0) {
                    videoBitRate = formatMap.value("bit_rate", "N/A");
                }
                if (videoBitRate != "N/A") {
                    videoBitRate = QString::number(videoBitRate.toInt() / 1000) + " kbps";
                }
                pixelFormat = stream.value("pix_fmt", "N/A");
                if (pixelFormat.contains("10")) pixFmt = "10-bit";
                else if (pixelFormat.contains("12")) pixFmt = "12-bit";
                else pixFmt = "8-bit";
                // Automatically set the 8/10-bit checkboxes based on the input video
                bool isTenBit = (pixFmt == "10-bit");
                QMetaObject::invokeMethod(tenBitCheck, "setChecked", Qt::QueuedConnection, Q_ARG(bool, isTenBit));
                QMetaObject::invokeMethod(eightBitCheck, "setChecked", Qt::QueuedConnection, Q_ARG(bool, !isTenBit));
                QMetaObject::invokeMethod(colorFormatBox, "setEnabled", Qt::QueuedConnection, Q_ARG(bool, isTenBit));
                QMetaObject::invokeMethod(eightBitColorFormatBox, "setEnabled", Qt::QueuedConnection, Q_ARG(bool, !isTenBit));
                QMetaObject::invokeMethod(logBox, "append", Qt::QueuedConnection, Q_ARG(QString, "🔍 Auto-detected " + pixFmt + " input, setting UI accordingly."));
                colorSpace = stream.value("color_space", "N/A");
            }
            for (const auto &stream : streams) {
                if (stream.value("codec_type") == "audio") {
                    audioCodec = stream.value("codec_name", "N/A");
                    audioSampleRate = stream.value("sample_rate", "N/A");
                    if (audioSampleRate != "N/A") {
                        audioSampleRate += " Hz";
                    }
                    break;
                }
            }
            double durationSec = formatMap.value("duration", "0").toDouble();
            if (durationSec > 0) {
                int hours = static_cast<int>(durationSec / 3600);
                double remaining = fmod(durationSec, 3600.0);
                int mins = static_cast<int>(remaining / 60);
                int secs = static_cast<int>(fmod(remaining, 60.0));
                durationStr = QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
            }
            title = formatMap.value("TAG:title", "N/A");
            if (title == "N/A" || title.isEmpty()) {
                title = QFileInfo(inputFile).completeBaseName();
            }
            QString infoText = "<b>Video Title:</b> " + title + "<br>"
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
            QMetaObject::invokeMethod(infoBox, "setHtml", Qt::QueuedConnection, Q_ARG(QString, infoText));
        }));
    };
    // Setting up the menu bar, all good for Qt6
    QMenuBar *menuBar = window.menuBar();
    QMenu *fileMenu = menuBar->addMenu("&File");
    // Sweet file browser with thumbnails
    auto *openAction = new QAction("&Open File...", &window);
    openAction->setShortcut(QKeySequence::Open);
    fileMenu->addAction(openAction);
    // Recent files submenu that updates itself
    QMenu *recentMenu = fileMenu->addMenu("Recent Files");
    std::function<void()> updateRecentMenu = [&]() {
        recentMenu->clear();
        QSettings settings("FFmpegConverter", "Recent");
        QStringList recent = settings.value("recentFiles").toStringList();
        int count = 0;
        QStringList validRecent;
        for (const QString& file : recent) {
            if (QFile::exists(file)) {
                validRecent.append(file);
                QString displayName = QFileInfo(file).baseName() + " " + QFileInfo(file).suffix().toUpper();
                QAction *act = recentMenu->addAction(QString("%1. %2").arg(++count).arg(displayName));
                QObject::connect(act, &QAction::triggered, [file, selectedFilesBox, outputNameBox, &updateInfo]() {
                    selectedFilesBox->setText(file);
                    outputNameBox->setText(QFileInfo(file).baseName());
                    updateInfo(file);
                });
            }
        }
        settings.setValue("recentFiles", validRecent);
        if (count == 0) {
            recentMenu->addAction("(No recent files)")->setEnabled(false);
        }
        recentMenu->addSeparator();
        QAction *clearRecentAction = recentMenu->addAction("🗑️ Clear Recent Videos");
        QObject::connect(clearRecentAction, &QAction::triggered, [&window, &updateRecentMenu]() {
            QSettings settings("FFmpegConverter", "Recent");
            settings.remove("recentFiles");
            settings.remove("lastVideoDir");
            QMessageBox::information(&window, "Cleared!", "Recent videos & last folder cleared!");
            updateRecentMenu();
        });
    };
    updateRecentMenu();
    fileMenu->addSeparator();
    // Settings dialog trigger
    auto *settingsAction = new QAction("&Settings...", &window);
    settingsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Comma));
    fileMenu->addAction(settingsAction);
    QObject::connect(settingsAction, &QAction::triggered, [&window]() {
        SettingsDialog dialog(&window);
        if (dialog.exec() == QDialog::Accepted) {
            QSettings settings("FFmpegConverter", "Settings");
            settings.setValue("defaultOutputDir", dialog.getDefaultOutputDir());
            settings.setValue("defaultCodecTab", dialog.getDefaultCodecTab());
            settings.setValue("ffmpegPath", dialog.getFFmpegPath());
            settings.setValue("notifyOnFinish", dialog.getNotifyOnFinish());
            settings.setValue("svtAv1Path", dialog.getSvtAv1Path());
            QMessageBox::information(&window, "✅ Settings Saved",
                                     "✓ Default codec tab\n✓ FFmpeg path\n✓ SVT-AV1 library path\n✓ Output directory\n✓ Notifications");
        }
    });
    QObject::connect(openAction, &QAction::triggered, [&]() {
        QFileDialog dialog(&window);
        dialog.setWindowTitle("🎬 Select Video File");
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setNameFilters({
            "Video Files (*.mp4 *.mkv *.webm *.avi *.mov *.wmv *.flv *.m4v *.ts *.m2ts)",
                              "All Files (*)"
        });
        QSettings settings("FFmpegConverter", "Recent");
        dialog.setDirectory(settings.value("lastVideoDir", QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)).toString());
        if (dialog.exec()) {
            QString file = dialog.selectedFiles().first();
            selectedFilesBox->setText(file);
            QFileInfo fileInfo(file);
            outputNameBox->setText(fileInfo.baseName());
            QStringList recent = settings.value("recentFiles").toStringList();
            recent.removeAll(file);
            recent.prepend(file);
            while (recent.size() > 15) recent.removeLast();
            settings.setValue("recentFiles", recent);
            settings.setValue("lastVideoDir", QFileInfo(file).absolutePath());
            updateRecentMenu();
        }
    });
    fileMenu->addSeparator();
    auto *exitAction = new QAction("E&xit", &window);
    exitAction->setShortcut(QKeySequence::Quit);
    QObject::connect(exitAction, &QAction::triggered, &window, &QMainWindow::close);
    fileMenu->addAction(exitAction);
    QMenu *viewMenu = menuBar->addMenu("&View");
    // Open input file in default viewer
    auto *viewInputAction = new QAction("📁 View Input File", &window);
    viewInputAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    viewMenu->addAction(viewInputAction);
    // Open output file in default viewer
    auto *viewOutputAction = new QAction("🎬 View Output File", &window);
    viewOutputAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
    viewMenu->addAction(viewOutputAction);
    QObject::connect(viewInputAction, &QAction::triggered, [selectedFilesBox]() {
        QString inputFile = selectedFilesBox->text().trimmed();
        if (inputFile.isEmpty() || !QFile::exists(inputFile)) {
            QMessageBox::warning(nullptr, "Error", "No valid input file selected.");
            return;
        }
        QProcess::startDetached("xdg-open", QStringList() << inputFile);
    });
    QObject::connect(viewOutputAction, &QAction::triggered, [converter]() {
        QString outputFile = converter->getFinalOutputFile();
        if (outputFile.isEmpty() || !QFile::exists(outputFile)) {
            QMessageBox::warning(nullptr, "Error", "No output file available. Please convert first.");
            return;
        }
        QProcess::startDetached("xdg-open", QStringList() << outputFile);
    });
    QMenu *helpMenu = menuBar->addMenu("&Help");
    auto *aboutAction = new QAction("&About", &window);
    QObject::connect(aboutAction, &QAction::triggered, [&window]() {
        QMessageBox::about(&window, "About FFmpeg Converter",
                           "FFmpeg Converter v1.0\nBuilt with Qt 6\nFor video enthusiasts!");
    });
    helpMenu->addAction(aboutAction);
    // Connecting all signals down here so everything's defined first
    QObject::connect(selectFilesButton, &QPushButton::clicked, [&]() {
        QFileDialog dialog(&window);
        dialog.setWindowTitle("🎬 Select Video File - Thumbnail Preview");
        dialog.setNameFilters({
            "Video Files (*.mp4 *.mkv *.webm *.avi *.mov *.wmv *.flv *.m4v *.ts *.m2ts)",
                              "All Files (*)"
        });
        QSettings settings("FFmpegConverter", "Recent");
        dialog.setDirectory(settings.value("lastVideoDir", QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)).toString());
        if (dialog.exec()) {
            QString file = dialog.selectedFiles().first();
            selectedFilesBox->setText(file);
            QFileInfo fileInfo(file);
            outputNameBox->setText(fileInfo.baseName());
            QStringList recent = settings.value("recentFiles").toStringList();
            recent.removeAll(file);
            recent.prepend(file);
            while (recent.size() > 15) recent.removeLast();
            settings.setValue("recentFiles", recent);
            settings.setValue("lastVideoDir", QFileInfo(file).absolutePath());
            updateRecentMenu();
        }
    });
    QObject::connect(outputDirButton, &QPushButton::clicked, [&]() {
        QFileDialog dialog(&window);
        dialog.setFileMode(QFileDialog::Directory);
        if (dialog.exec()) {
            outputDirBox->setText(dialog.selectedFiles().first());
        }
    });
    QObject::connect(cropCheck, &QCheckBox::toggled, [detectCropButton, cropValueBox](bool checked) {
        detectCropButton->setEnabled(checked);
        cropValueBox->setEnabled(checked);
        if (!checked) {
            cropValueBox->clear();
        }
    });
    QObject::connect(detectCropButton, &QPushButton::clicked, [selectedFilesBox, cropValueBox, logBox, &settings, rotationBox]() {
        QString inputFile = selectedFilesBox->text();
        if (inputFile.isEmpty()) {
            QMessageBox::warning(nullptr, "Error", "Please select an input file to detect crop.");
            return;
        }
        QFileInfo fileInfo(inputFile);
        if (!fileInfo.exists() || !fileInfo.isReadable()) {
            QString errorMsg = !fileInfo.exists() ? "does not exist" : "is not readable";
            logBox->append("⚠️ Input file " + errorMsg + ": " + inputFile);
            QMessageBox::warning(nullptr, "Error", "Input file " + errorMsg + ": " + inputFile);
            return;
        }
        QString rotationFilter;
        QString rotation = rotationBox->currentText();
        if (rotation == "90° Clockwise") rotationFilter = "transpose=1";
        else if (rotation == "90° Counterclockwise") rotationFilter = "transpose=2";
        else if (rotation == "180°") rotationFilter = "transpose=1,transpose=1";
        else if (rotation == "Horizontal Flip") rotationFilter = "hflip";
        else if (rotation == "Vertical Flip") rotationFilter = "vflip";
        QString vf = "cropdetect=0.1:2:0";
        if (!rotationFilter.isEmpty()) vf = rotationFilter + "," + vf;
        QProcess process;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString svtAv1Path = settings.value("svtAv1Path", "").toString();
        if (!svtAv1Path.isEmpty()) {
            QString existing = env.value("LD_LIBRARY_PATH");
            env.insert("LD_LIBRARY_PATH", existing.isEmpty() ? svtAv1Path : existing + ":" + svtAv1Path);
            logBox->append("📚 Using custom SVT-AV1 path: " + svtAv1Path);
        }
        process.setProcessEnvironment(env);
        QString ffmpegPath = settings.value("ffmpegPath", "/usr/bin/ffmpeg").toString();
        QString effectiveFFmpeg = ffmpegPath.isEmpty() ? "/usr/bin/ffmpeg" : ffmpegPath;
        QStringList args = {"-i", inputFile, "-vf", vf, "-f", "null", "-t", "10", "-"};
        logBox->append("Executing crop detect: " + effectiveFFmpeg + " " + args.join(" "));
        process.start(effectiveFFmpeg, args);
        if (!process.waitForFinished(30000)) {
            logBox->append("⚠️ Crop detect timed out.");
            cropValueBox->setText("Not detected");
            return;
        }
        if (process.exitCode() != 0) {
            QString error = process.readAllStandardError();
            logBox->append("⚠️ Crop detect failed: " + error.trimmed());
            cropValueBox->setText("Not detected");
            return;
        }
        QString output = process.readAllStandardError();
        QStringList lines = output.split("\n");
        QStringList filtered;
        for (const QString& line : lines) {
            if (line.contains("[Parsed_cropdetect")) {
                filtered.append(line);
            }
        }
        if (!filtered.isEmpty()) {
            logBox->append("Crop detect output (filtered): " + filtered.last());
        }
        QRegularExpression cropRegex("crop=(\\d+:\\d+:\\d+:\\d+)");
        QString cropValue;
        for (int i = lines.size() - 1; i >= 0; --i) {
            QRegularExpressionMatch match = cropRegex.match(lines[i]);
            if (match.hasMatch()) {
                cropValue = match.captured(1);
                break;
            }
        }
        if (cropValue.isEmpty()) {
            logBox->append("⚠️ No crop value found.");
            cropValueBox->setText("Not detected");
        } else {
            logBox->append("✅ Detected crop: " + cropValue);
            cropValueBox->setText("crop=" + cropValue);
        }
    });
    QObject::connect(rotationBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [detectCropButton, cropCheck](int) {
        if (cropCheck->isChecked()) {
            detectCropButton->click();
        }
    });
    QObject::connect(seekCheck, &QCheckBox::toggled, [seekHH, seekMM, seekSS](bool checked) {
        seekHH->setEnabled(checked);
        seekMM->setEnabled(checked);
        seekSS->setEnabled(checked);
    });
    QObject::connect(timeCheck, &QCheckBox::toggled, [timeHH, timeMM, timeSS](bool checked) {
        timeHH->setEnabled(checked);
        timeMM->setEnabled(checked);
        timeSS->setEnabled(checked);
    });
    QObject::connect(frameRateBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [frameRateBox, customFrameRateBox](int index) {
        customFrameRateBox->setEnabled(frameRateBox->itemText(index) == "Custom");
    });
    QObject::connect(eightBitCheck, &QCheckBox::toggled, [eightBitColorFormatBox, tenBitCheck, colorFormatBox](bool checked) {
        eightBitColorFormatBox->setEnabled(checked);
        if (checked) {
            tenBitCheck->setChecked(false);
            colorFormatBox->setEnabled(false);
        }
    });
    QObject::connect(tenBitCheck, &QCheckBox::toggled, [colorFormatBox, eightBitCheck, eightBitColorFormatBox](bool checked) {
        colorFormatBox->setEnabled(checked);
        if (checked) {
            eightBitCheck->setChecked(false);
            eightBitColorFormatBox->setEnabled(false);
        }
    });
    QObject::connect(runCommandButton, &QPushButton::clicked, [customCommandBox, logBox, runCommandButton, &settings]() {
        QString command = customCommandBox->text();
        if (!command.isEmpty()) {
            runCommandButton->setEnabled(false);
            QProcess* process = new QProcess();
            QString ffmpegPath = settings.value("ffmpegPath", "/usr/bin/ffmpeg").toString();
            process->start(ffmpegPath, command.split(" "));
            logBox->append("Running command: " + ffmpegPath + " " + command);
            QObject::connect(process, &QProcess::readyReadStandardOutput, [process, logBox]() {
                logBox->append(process->readAllStandardOutput());
            });
            QObject::connect(process, &QProcess::readyReadStandardError, [process, logBox]() {
                logBox->append(process->readAllStandardError());
            });
            QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [process, logBox, runCommandButton]() {
                if (process->exitCode() != 0) {
                    logBox->append("Command failed. Check the output for details.");
                } else {
                    logBox->append("Command completed successfully.");
                }
                process->deleteLater();
                runCommandButton->setEnabled(true);
            });
        }
    });
    QObject::connect(clearLogButton, &QPushButton::clicked, [logBox]() {
        logBox->clear();
    });
    QObject::connect(convertButton, &QPushButton::clicked, [converter, convertButton, cancelButton, selectedFilesBox, outputDirBox, outputNameBox, scaleWidthSpin, scaleHeightSpin, scaleFilterBox, scaleRangeBox, eightBitCheck, eightBitColorFormatBox, tenBitCheck, colorFormatBox, cropCheck, cropValueBox, seekCheck, seekHH, seekMM, seekSS, timeCheck, timeHH, timeMM, timeSS, frameRateBox, customFrameRateBox, preserveMetadataCheck, removeChaptersCheck, deinterlaceCheck, rotationBox, av1Tab, x265Tab, vp9Tab, logBox, conversionProgress, codecTabs, getSampleRateInHz, getBitrateValue, &updateRecentMenu, &settings, overwriteCheck]() {
        logBox->clear();
        if (seekCheck->isChecked()) {
            bool okHH, okMM, okSS;
            int hh = seekHH->text().toInt(&okHH);
            int mm = seekMM->text().toInt(&okMM);
            int ss = seekSS->text().toInt(&okSS);
            if (!okHH || !okMM || !okSS || hh < 0 || hh > 99 || mm < 0 || mm > 59 || ss < 0 || ss > 59) {
                QMessageBox::warning(nullptr, "Invalid Input", "Seek time invalid (HH:00-99, MM/SS:00-59).");
                return;
            }
        }
        if (timeCheck->isChecked()) {
            bool okHH, okMM, okSS;
            int hh = timeHH->text().toInt(&okHH);
            int mm = timeMM->text().toInt(&okMM);
            int ss = timeSS->text().toInt(&okSS);
            if (!okHH || !okMM || !okSS || hh < 0 || hh > 99 || mm < 0 || mm > 59 || ss < 0 || ss > 59) {
                QMessageBox::warning(nullptr, "Invalid Input", "Duration time invalid (HH:00-99, MM/SS:00-59).");
                return;
            }
        }
        logBox->append("=== CONVERSION STARTED ===");
        QString inputFile = selectedFilesBox->text();
        if (inputFile.isEmpty()) {
            QMessageBox::warning(nullptr, "Error", "Please select an input file.");
            return;
        }
        logBox->append("📁 Input: " + inputFile);
        logBox->append("🎛️ Scale: " + QString::number(scaleWidthSpin->value(), 'f', 2) + "x × " + QString::number(scaleHeightSpin->value(), 'f', 2) + "x");
        if (!qFuzzyCompare(scaleWidthSpin->value(), 1.0) || !qFuzzyCompare(scaleHeightSpin->value(), 1.0)) {
            logBox->append("🔧 Scale Filter: " + scaleFilterBox->currentText());
        } else {
            logBox->append("🔧 Scale Filter: None (1:1)");
        }
        logBox->append("🌈 Range: " + scaleRangeBox->currentText());
        logBox->append("🎨 Pixel Format: " + (tenBitCheck->isChecked() ? colorFormatBox->currentText() : eightBitColorFormatBox->currentText()));
        int currentTab = codecTabs->currentIndex();
        logBox->append("🔀 Codec: " + QString(currentTab == 0 ? "AV1" : currentTab == 1 ? "x265" : "VP9"));
        if (currentTab == 0 && av1Tab->av1EnableRCModeCheck->isChecked()) {
            QString rcMode = av1Tab->av1RCModeBox->currentText();
            int aqModeIndex = av1Tab->av1AQModeBox->currentIndex();
            if (rcMode == "VBR" && aqModeIndex == 1) {
                QMessageBox::warning(nullptr, "Invalid Configuration",
                                     "Adaptive Quantization cannot be disabled when using VBR rate control for AV1. "
                                     "Please select 'Automatic', 'Variance', or 'Complexity' AQ mode.");
                return;
            }
        } else if (currentTab == 1 && x265Tab->x265EnableRCModeCheck->isChecked()) {
            QString rcMode = x265Tab->x265RCModeBox->currentText();
            int aqModeIndex = x265Tab->x265AQModeBox->currentIndex();
            if ((rcMode == "ABR" || rcMode == "CBR") && aqModeIndex == 1) {
                QMessageBox::warning(nullptr, "Invalid Configuration",
                                     "Adaptive Quantization cannot be disabled when using ABR or CBR rate control for x265. "
                                     "Please select 'Automatic', 'Variance', 'Auto-Variance', or 'Auto-Variance Biased' AQ mode.");
                return;
            }
        } else if (currentTab == 2 && vp9Tab->vp9EnableRCModeCheck->isChecked()) {
            QString rcMode = vp9Tab->vp9RCModeBox->currentText();
            int aqModeIndex = vp9Tab->vp9AQModeBox->currentIndex();
            if ((rcMode == "ABR" || rcMode == "CBR") && aqModeIndex == 1) {
                QMessageBox::warning(nullptr, "Invalid Configuration",
                                     "Adaptive Quantization cannot be disabled when using ABR or CBR rate control for VP9. "
                                     "Please select 'Automatic', 'Variance', or 'Complexity' AQ mode.");
                return;
            }
        }
        QString outputDir = outputDirBox->text();
        QString baseName = outputNameBox->text().isEmpty() ? "Output" : outputNameBox->text();
        QString extension;
        bool twoPass;
        QString codecStr;
        if (currentTab == 0) {
            extension = "." + av1Tab->av1ContainerBox->currentText();
            twoPass = av1Tab->av1TwoPassCheck->isChecked();
            codecStr = "av1";
        } else if (currentTab == 1) {
            extension = "." + x265Tab->x265ContainerBox->currentText();
            twoPass = x265Tab->x265TwoPassCheck->isChecked();
            codecStr = "x265";
        } else if (currentTab == 2) {
            extension = "." + vp9Tab->vp9ContainerBox->currentText();
            twoPass = vp9Tab->vp9TwoPassCheck->isChecked();
            codecStr = "vp9";
        }
        QStringList args;
        if (seekCheck->isChecked()) {
            bool okHH, okMM, okSS;
            int hh = seekHH->text().toInt(&okHH);
            int mm = seekMM->text().toInt(&okMM);
            int ss = seekSS->text().toInt(&okSS);
            if (okHH && okMM && okSS) {
                args << "-ss" << QString("%1:%2:%3").arg(hh, 2, 10, QChar('0')).arg(mm, 2, 10, QChar('0')).arg(ss, 2, 10, QChar('0'));
            }
        }
        if (timeCheck->isChecked()) {
            bool okHH, okMM, okSS;
            int hh = timeHH->text().toInt(&okHH);
            int mm = timeMM->text().toInt(&okMM);
            int ss = timeSS->text().toInt(&okSS);
            if (okHH && okMM && okSS) {
                args << "-t" << QString("%1:%2:%3").arg(hh, 2, 10, QChar('0')).arg(mm, 2, 10, QChar('0')).arg(ss, 2, 10, QChar('0'));
            }
        }
        if (preserveMetadataCheck->isChecked()) args << "-map_metadata" << "0";
        if (removeChaptersCheck->isChecked()) args << "-map_chapters" << "-1";
        QStringList filters;
        QString rotationFilter;
        QString rotation = rotationBox->currentText();
        if (rotation == "90° Clockwise") {
            rotationFilter = "transpose=1";
        } else if (rotation == "90° Counterclockwise") {
            rotationFilter = "transpose=2";
        } else if (rotation == "180°") {
            rotationFilter = "transpose=1,transpose=1";
        } else if (rotation == "Horizontal Flip") {
            rotationFilter = "hflip";
        } else if (rotation == "Vertical Flip") {
            rotationFilter = "vflip";
        }
        if (!rotationFilter.isEmpty()) {
            filters << rotationFilter;
        }
        if (cropCheck->isChecked() && !cropValueBox->text().isEmpty() && cropValueBox->text() != "Not detected") {
            QString cropValue = cropValueBox->text();
            if (cropValue.startsWith("crop=")) {
                cropValue = cropValue.mid(5);
            }
            filters << "crop=" + cropValue;
        }
        if (deinterlaceCheck->isChecked()) filters << "yadif";
        double scaleWidthValue = scaleWidthSpin->value();
        double scaleHeightValue = scaleHeightSpin->value();
        QString selectedFilter = scaleFilterBox->currentText();
        QString scaleRange = scaleRangeBox->currentText();
        QString widthExpr = (qFuzzyCompare(scaleWidthValue, 1.0)) ? "iw" : QString("trunc(iw*%1/2)*2").arg(scaleWidthValue);
        QString heightExpr = (qFuzzyCompare(scaleHeightValue, 1.0)) ? "ih" : QString("trunc(ih*%1/2)*2").arg(scaleHeightValue);
        if (!qFuzzyCompare(scaleWidthValue, 1.0) || !qFuzzyCompare(scaleHeightValue, 1.0)) {
            filters << QString("zscale=w=%1:h=%2:filter=%3:r=%4").arg(widthExpr, heightExpr, selectedFilter, scaleRange);
        }
        QString fps = frameRateBox->currentText();
        if (fps != "Original") {
            QString fpsValue = (fps == "Custom") ? customFrameRateBox->text() : fps;
            bool ok;
            double fpsNum = fpsValue.toDouble(&ok);
            if (ok && fpsNum > 0) filters << "fps=" + QString::number(fpsNum);
        }
        if (currentTab == 0) {
            if (av1Tab->av1UnsharpenCheck->isChecked()) {
                double strength = av1Tab->av1UnsharpenStrengthSlider->value() / 10.0;
                filters << QString("unsharp=5:5:%1:5:5:0.0").arg(strength);
            }
            if (av1Tab->av1SharpenCheck->isChecked()) {
                double strength = av1Tab->av1SharpenStrengthSlider->value() / 10.0;
                filters << QString("unsharp=3:3:%1:3:3:0.0").arg(strength);
            }
            if (av1Tab->av1BlurCheck->isChecked()) {
                double strength = av1Tab->av1BlurStrengthSlider->value() / 10.0;
                filters << QString("smartblur=%1:0.5:0").arg(strength);
            }
            if (av1Tab->av1NoiseReductionCheck->isChecked()) {
                double strength = av1Tab->av1NoiseReductionSlider->value();
                filters << QString("hqdn3d=luma_spatial=%1:chroma_spatial=%1").arg(strength);
            }
            if (av1Tab->av1GrainSynthCheck->isChecked()) {
                int level = av1Tab->av1GrainSynthLevel->value();
                filters << QString("noise=alls=%1:allf=t").arg(level);
            }
        } else if (currentTab == 1) {
            if (x265Tab->x265UnsharpenCheck->isChecked()) {
                double strength = x265Tab->x265UnsharpenStrengthSlider->value() / 10.0;
                filters << QString("unsharp=5:5:%1:5:5:0.0").arg(strength);
            }
            if (x265Tab->x265SharpenCheck->isChecked()) {
                double strength = x265Tab->x265SharpenStrengthSlider->value() / 10.0;
                filters << QString("unsharp=3:3:%1:3:3:0.0").arg(strength);
            }
            if (x265Tab->x265BlurCheck->isChecked()) {
                double strength = x265Tab->x265BlurStrengthSlider->value() / 10.0;
                filters << QString("smartblur=%1:0.5:0").arg(strength);
            }
            if (x265Tab->x265NoiseReductionCheck->isChecked()) {
                double strength = x265Tab->x265NoiseReductionSlider->value();
                filters << QString("hqdn3d=luma_spatial=%1:chroma_spatial=%1").arg(strength);
            }
            if (x265Tab->x265GrainSynthCheck->isChecked()) {
                int level = x265Tab->x265GrainSynthLevel->value();
                filters << QString("noise=alls=%1:allf=t").arg(level);
            }
        } else if (currentTab == 2) {
            if (vp9Tab->vp9UnsharpenCheck->isChecked()) {
                double strength = vp9Tab->vp9UnsharpenStrengthSlider->value() / 10.0;
                filters << QString("unsharp=5:5:%1:5:5:0.0").arg(strength);
            }
            if (vp9Tab->vp9SharpenCheck->isChecked()) {
                double strength = vp9Tab->vp9SharpenStrengthSlider->value() / 10.0;
                filters << QString("unsharp=3:3:%1:3:3:0.0").arg(strength);
            }
            if (vp9Tab->vp9BlurCheck->isChecked()) {
                double strength = vp9Tab->vp9BlurStrengthSlider->value() / 10.0;
                filters << QString("smartblur=%1:0.5:0").arg(strength);
            }
            if (vp9Tab->vp9NoiseReductionCheck->isChecked()) {
                double strength = vp9Tab->vp9NoiseReductionSlider->value();
                filters << QString("hqdn3d=luma_spatial=%1:chroma_spatial=%1").arg(strength);
            }
            if (vp9Tab->vp9GrainSynthCheck->isChecked()) {
                int level = vp9Tab->vp9GrainSynthLevel->value();
                filters << QString("noise=alls=%1:allf=t").arg(level);
            }
        }
        QString pixFmt = "yuv420p";
        if (eightBitCheck->isChecked()) {
            QString format = eightBitColorFormatBox->currentText();
            pixFmt = (format == "8-bit 4:2:0") ? "yuv420p" : (format == "8-bit 4:2:2") ? "yuv422p" : "yuv444p";
        } else if (tenBitCheck->isChecked()) {
            QString format = colorFormatBox->currentText();
            pixFmt = (format == "10-bit 4:2:0") ? "yuv420p10le" : (format == "10-bit 4:2:2") ? "yuv422p10le" : "yuv444p10le";
            if (!filters.contains("format=" + pixFmt)) {
                filters << "format=" + pixFmt;
            }
        }
        QString filterChain = filters.join(",");
        if (!filterChain.isEmpty()) {
            logBox->append("🛠️ Video filters: " + filterChain);
            args << "-vf" << filterChain;
        } else {
            logBox->append("🛠️ Video filters: None");
        }
        if (currentTab == 0) {
            args << "-c:v" << "libsvtav1";
            args << "-preset" << av1Tab->av1PresetBox->currentText();
            QStringList svtParams;
            QString tune = av1Tab->av1TuneBox->currentText();
            if (tune != "Auto") {
                int tuneVal;
                if (tune == "Subjective SSIM (VQ)")      tuneVal = 0;
                else if (tune == "PSNR")                 tuneVal = 1;
                else if (tune == "SSIM")                 tuneVal = 2;
                else if (tune == "VMAF")                 tuneVal = 6;
                else if (tune == "VMAF Neg")             tuneVal = 7;
                else                                     tuneVal = 0;  // fallback

                svtParams << "tune=" + QString::number(tuneVal);
            }
            if (av1Tab->nativeGrainCheck->isChecked()) {
                int strength = av1Tab->grainStrengthSlider->value();
                if (strength > 0) {
                    svtParams << "film-grain=" + QString::number(strength);
                }
                int denoise = av1Tab->grainDenoiseCombo->currentIndex();
                svtParams << "film-grain-denoise=" + QString::number(denoise);
            }
            int superResMode = av1Tab->superResModeBox->currentIndex();
            if (superResMode > 0) {
                svtParams << "superres-mode=" + QString::number(superResMode);
                int denom = av1Tab->superResDenomSlider->value();
                svtParams << "superres-denom=" + QString::number(denom);
            }
            int fdLevel = av1Tab->fastDecodeBox->currentIndex();
            if (fdLevel > 0) svtParams << "fast-decode=" + QString::number(fdLevel);
            if (av1Tab->lowLatencyCheck->isChecked()) {
                svtParams << "irefresh-type=1";
                svtParams << "lookahead=0";
            }
            if (av1Tab->tplModelCheck->isChecked()) {
                svtParams << "enable-tpl-la=1";
            }
            svtParams << "enable-cdef=" + QString(av1Tab->enableCdefCheck->isChecked() ? "1" : "0");
            if (av1Tab->av1LookaheadCheck->isChecked()) {
                svtParams << "lookahead=" + QString::number(av1Tab->av1LookaheadSlider->value());
            }
            QString aqModeStr = av1Tab->av1AQModeBox->currentText();
            if (aqModeStr != "Automatic") {
                int aqMode = (aqModeStr == "Disabled") ? 0 : (aqModeStr == "Variance") ? 1 : (aqModeStr == "Complexity") ? 2 : 0;
                svtParams << "aq-mode=" + QString::number(aqMode);
                if (aqModeStr == "Variance") {
                    svtParams << "enable-variance-boost=1";
                    svtParams << "variance-boost-strength=" + QString::number(av1Tab->av1AQStrengthSlider->value());
                } else if (aqModeStr == "Complexity") {
                    int qpMin = 1 + (av1Tab->av1AQStrengthSlider->value() - 1) * 5;
                    svtParams << "qp-min=" + QString::number(qpMin);
                    svtParams << "qp-max=63";
                }
            }
            if (av1Tab->av1EnableRCModeCheck->isChecked()) {
                QString mode = av1Tab->av1RCModeBox->currentText();
                if (mode == "QP") {
                    svtParams << "qp=" + QString::number(av1Tab->av1QPSlider->value());
                } else if (mode == "CRF") {
                    svtParams << "crf=" + QString::number(av1Tab->av1CRFSlider->value());
                } else if (mode == "VBR") {
                    args << "-b:v" << QString::number(av1Tab->av1VBRBitrateSlider->value()) + "k";
                    if (av1Tab->av1VBRVBVCheck->isChecked()) {
                        int bitrate = av1Tab->av1VBRBitrateSlider->value();
                        args << "-maxrate" << QString::number(bitrate) + "k";
                        args << "-bufsize" << QString::number(2 * bitrate) + "k";
                    }
                }
            } else {
                svtParams << "crf=35";
            }
            if (av1Tab->enableTfCheck->isChecked()) {
                svtParams << "enable-tf=1";
            }
            int scm = av1Tab->screenContentModeBox->currentIndex();
            svtParams << "scm=" + QString::number(scm);
            if (!svtParams.isEmpty()) {
                args << "-svtav1-params" << svtParams.join(":");
            }
            QString level = av1Tab->av1LevelBox->currentText();
            if (level != "Auto") {
                args << "-level" << level;
            }
            args << "-g" << av1Tab->av1KeyIntBox->currentText();
            QString threads = av1Tab->av1ThreadsBox->currentText();
            if (threads != "Automatic") {
                svtParams << "lp=" + threads;
                if (!svtParams.isEmpty()) {
                    args << "-svtav1-params" << svtParams.join(":");
                }
            }
            QString tileRows = av1Tab->av1TileRowsBox->currentText();
            if (tileRows != "Automatic") {
                int log2Rows = (tileRows == "1") ? 0 : (tileRows == "2") ? 1 : (tileRows == "4") ? 2 : (tileRows == "8") ? 3 : 0;
                svtParams << "tile-rows=" + QString::number(log2Rows);
                if (!svtParams.isEmpty()) {
                    args << "-svtav1-params" << svtParams.join(":");
                }
            }
            QString tileColumns = av1Tab->av1TileColumnsBox->currentText();
            if (tileColumns != "Automatic") {
                int log2Cols = (tileColumns == "1") ? 0 : (tileColumns == "2") ? 1 : (tileColumns == "4") ? 2 : (tileColumns == "8") ? 3 : 0;
                svtParams << "tile-columns=" + QString::number(log2Cols);
                if (!svtParams.isEmpty()) {
                    args << "-svtav1-params" << svtParams.join(":");
                }
            }
        } else if (currentTab == 1) {
            args << "-c:v" << "libx265";
            args << "-preset" << x265Tab->x265PresetBox->currentText();
            QString tune = x265Tab->x265TuneBox->currentText();
            if (tune != "Auto") {
                args << "-tune" << tune;
            }
            QString level = x265Tab->x265LevelBox->currentText();
            if (level != "auto") {
                args << "-level" << level;
            }
            QStringList x265Params;
            if (x265Tab->strongIntraCheck->isChecked()) {
                x265Params << "strong-intra-smoothing=1";
            }
            int rdoqLevel = x265Tab->rdoqLevelBox->currentIndex();
            if (rdoqLevel > 1) {
                x265Params << "rdoq-level=" + QString::number(rdoqLevel);
            }
            if (x265Tab->saoCheck->isChecked()) {
                x265Params << "sao=1";
            }
            int limitRefs = x265Tab->limitRefsBox->currentIndex();
            if (limitRefs > 0) {
                x265Params << "limit-refs=" + QString::number(limitRefs);
            }
            if (x265Tab->x265LookaheadCheck->isChecked()) {
                x265Params << "rc-lookahead=" + QString::number(x265Tab->x265LookaheadSlider->value());
            }
            QString aqModeStr = x265Tab->x265AQModeBox->currentText();
            if (aqModeStr != "Automatic") {
                int aqMode = (aqModeStr == "Disabled") ? 0 :
                (aqModeStr == "Variance") ? 1 :
                (aqModeStr == "Auto-Variance") ? 2 :
                (aqModeStr == "Auto-Variance Biased") ? 3 : 0;
                args << "-aq-mode" << QString::number(aqMode);
            }
            x265Params << "aq-strength=" + QString::number(x265Tab->x265AQStrengthSlider->value());
            if (x265Tab->enablePsyRdCheck->isChecked()) {
                x265Params << "psy-rd=1.0";
            }
            if (x265Tab->enableCutreeCheck->isChecked()) {
                x265Params << "cutree=1";
            }
            if (!x265Params.isEmpty()) {
                args << "-x265-params" << x265Params.join(":");
            }
            args << "-g" << x265Tab->x265KeyIntBox->currentText();
            QString threads = x265Tab->x265ThreadsBox->currentText();
            if (threads != "Automatic") {
                args << "-threads" << threads;
            }
            QString frameThreads = x265Tab->x265FrameThreadsBox->currentText();
            if (frameThreads != "Automatic") {
                args << "-frame-threads" << frameThreads;
            }
            if (x265Tab->x265EnableRCModeCheck->isChecked()) {
                QString mode = x265Tab->x265RCModeBox->currentText();
                if (mode == "QP") {
                    args << "-qp" << QString::number(x265Tab->x265QPSlider->value());
                } else if (mode == "CRF") {
                    args << "-crf" << QString::number(x265Tab->x265CRFSlider->value());
                } else if (mode == "ABR") {
                    args << "-b:v" << QString::number(x265Tab->x265ABRBitrateSlider->value()) + "k";
                    if (x265Tab->x265ABRVBVCheck->isChecked()) {
                        int bitrate = x265Tab->x265ABRBitrateSlider->value();
                        args << "-maxrate" << QString::number(bitrate) + "k";
                        args << "-bufsize" << QString::number(2 * bitrate) + "k";
                    }
                } else if (mode == "CBR") {
                    int bitrate = x265Tab->x265CBRBitrateSlider->value();
                    args << "-b:v" << QString::number(bitrate) + "k";
                    args << "-maxrate" << QString::number(bitrate) + "k";
                    args << "-bufsize" << QString::number(bitrate) + "k";
                }
            } else {
                args << "-crf" << "23";
            }
        } else if (currentTab == 2) {
            args << "-c:v" << "libvpx-vp9";
            args << "-cpu-used" << vp9Tab->vp9CpuUsedBox->currentText();
            QString deadline = vp9Tab->vp9DeadlineBox->currentText();
            args << "-deadline" << deadline.toLower();
            if (vp9Tab->vp9LookaheadCheck->isChecked()) {
                args << "-lag-in-frames" << QString::number(vp9Tab->vp9LookaheadSlider->value());
            }
            QString aqModeStr = vp9Tab->vp9AQModeBox->currentText();
            if (aqModeStr != "Automatic") {
                int aqMode = (aqModeStr == "Disabled") ? 0 : (aqModeStr == "Variance") ? 1 : (aqModeStr == "Complexity") ? 2 : 0;
                args << "-aq-mode" << QString::number(aqMode);
            }
            args << "-arnr-strength" << QString::number(vp9Tab->vp9AQStrengthSlider->value());
            if (vp9Tab->enableRowMtCheck->isChecked()) {
                args << "-row-mt" << "1";
            }
            if (vp9Tab->screenContentCheck->isChecked()) {
                args << "-tune-content" << "screen";
            }
            args << "-g" << vp9Tab->vp9KeyIntBox->currentText();
            QString threads = vp9Tab->vp9ThreadsBox->currentText();
            if (threads != "Automatic") {
                args << "-threads" << threads;
            }
            QString tileColumns = vp9Tab->vp9TileColumnsBox->currentText();
            int log2Cols = (tileColumns == "0") ? 0 : (tileColumns == "1") ? 0 : (tileColumns == "2") ? 1 : (tileColumns == "4") ? 2 : (tileColumns == "8") ? 3 : 0;
            args << "-tile-columns" << QString::number(log2Cols);
            QString tileRows = vp9Tab->vp9TileRowsBox->currentText();
            if (tileRows != "Automatic") {
                int log2Rows = (tileRows == "1") ? 0 : (tileRows == "2") ? 1 : (tileRows == "4") ? 2 : (tileRows == "8") ? 3 : 0;
                args << "-row-mt" << "1";
                args << "-tile-rows" << QString::number(log2Rows);
            }
            if (vp9Tab->vp9EnableRCModeCheck->isChecked()) {
                QString mode = vp9Tab->vp9RCModeBox->currentText();
                if (mode == "CRF") {
                    args << "-crf" << QString::number(vp9Tab->vp9CRFSlider->value());
                    args << "-b:v" << "0";
                } else if (mode == "ABR" || mode == "CBR") {
                    int bitrate = vp9Tab->vp9BitrateSlider->value();
                    args << "-b:v" << QString::number(bitrate) + "k";
                    if (mode == "CBR") {
                        args << "-maxrate" << QString::number(bitrate) + "k";
                        args << "-bufsize" << QString::number(bitrate) + "k";
                    }
                }
            } else {
                args << "-crf" << "31";
                args << "-b:v" << "0";
            }
        }
        QCheckBox *audioCheck;
        QComboBox *audioCodecBox;
        QComboBox *audioSampleRateBox;
        QComboBox *audioBitrateBox;
        QComboBox *vbrModeBox = nullptr;
        QComboBox *aacQualityBox = nullptr;
        QComboBox *mp3VbrBox = nullptr;
        QComboBox *flacCompressionBox = nullptr;
        QComboBox *vorbisQualityBox = nullptr;
        if (currentTab == 0) {
            audioCheck = av1Tab->av1AudioCheck;
            audioCodecBox = av1Tab->av1AudioCodecBox;
            audioSampleRateBox = av1Tab->av1AudioSampleRateBox;
            audioBitrateBox = av1Tab->av1AudioBitrateBox;
            vbrModeBox = av1Tab->av1VbrModeBox;
            aacQualityBox = av1Tab->av1AacQualityBox;
            mp3VbrBox = av1Tab->av1Mp3VbrBox;
            flacCompressionBox = av1Tab->av1FlacCompressionBox;
            vorbisQualityBox = av1Tab->av1VorbisQualityBox;
        } else if (currentTab == 1) {
            audioCheck = x265Tab->x265AudioCheck;
            audioCodecBox = x265Tab->x265AudioCodecBox;
            audioSampleRateBox = x265Tab->x265AudioSampleRateBox;
            audioBitrateBox = x265Tab->x265AudioBitrateBox;
            vbrModeBox = x265Tab->x265VbrModeBox;
            aacQualityBox = x265Tab->x265AacQualityBox;
            mp3VbrBox = x265Tab->x265Mp3VbrBox;
            flacCompressionBox = x265Tab->x265FlacCompressionBox;
            vorbisQualityBox = x265Tab->x265VorbisQualityBox;
        } else if (currentTab == 2) {
            audioCheck = vp9Tab->vp9AudioCheck;
            audioCodecBox = vp9Tab->vp9AudioCodecBox;
            audioSampleRateBox = vp9Tab->vp9AudioSampleRateBox;
            audioBitrateBox = vp9Tab->vp9AudioBitrateBox;
            vbrModeBox = vp9Tab->vp9VbrModeBox;
            vorbisQualityBox = vp9Tab->vp9VorbisQualityBox;
        }
        if (audioCheck->isChecked()) {
            QString audioCodecStr = audioCodecBox->currentText();
            QString encoder;
            if (audioCodecStr == "opus") {
                encoder = "libopus";
            } else if (audioCodecStr == "vorbis") {
                encoder = "libvorbis";
            } else if (audioCodecStr == "mp3") {
                encoder = "libmp3lame";
            } else if (audioCodecStr == "aac") {
                encoder = "aac";
            } else if (audioCodecStr == "flac") {
                encoder = "flac";
            } else {
                encoder = audioCodecStr;
            }
            args << "-c:a" << encoder;
            args << "-ar" << getSampleRateInHz(audioSampleRateBox->currentText());
            args << "-b:a" << getBitrateValue(audioBitrateBox->currentText()) + "k";
            if (audioCodecStr == "opus" && vbrModeBox) {
                QString vbr = vbrModeBox->currentText();
                if (vbr == "Constrained") {
                    args << "-vbr" << "constrained";
                } else if (vbr == "Off") {
                    args << "-vbr" << "off";
                }
            } else if (audioCodecStr == "aac" && aacQualityBox) {
                QString quality = aacQualityBox->currentText();
                if (quality != "Disabled") {
                    args << "-q:a" << quality;
                }
            } else if (audioCodecStr == "mp3" && mp3VbrBox) {
                QString vbr = mp3VbrBox->currentText();
                if (vbr != "Disabled") {
                    args << "-qscale:a" << vbr;
                }
            } else if (audioCodecStr == "flac" && flacCompressionBox) {
                args << "-compression_level" << flacCompressionBox->currentText();
            } else if (audioCodecStr == "vorbis" && vorbisQualityBox) {
                QString quality = vorbisQualityBox->currentText();
                if (quality != "Disabled") {
                    args << "-q:a" << quality;
                }
            }
        } else {
            args << "-an";
        }
        QString ffmpegPath = settings.value("ffmpegPath", "").toString();
        if (ffmpegPath.isEmpty()) {
            ffmpegPath = "/usr/bin/ffmpeg";
            logBox->append("⚠️ Warning: FFmpeg path is empty in settings, falling back to: " + ffmpegPath);
        }
        logBox->append("🔍 Debug: ffmpegPath used: " + ffmpegPath);
        logBox->append("📽️ Using FFmpeg path: " + ffmpegPath);
        QString fullCommand = ffmpegPath + " -i \"" + inputFile + "\" " + args.join(" ") + " \"" + QDir::cleanPath(outputDir + "/" + baseName + extension) + "\"";
        logBox->append("\n🔧 FULL FFmpeg COMMAND:");
        logBox->append(fullCommand);
        logBox->append("────────────────────────────────────────────────────────────────────────────────");
        convertButton->setEnabled(false);
        cancelButton->setEnabled(true);
        conversionProgress->setVisible(true);
        conversionProgress->setRange(0, 100);
        conversionProgress->setValue(0);
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString svtAv1Path = settings.value("svtAv1Path", "").toString();
        if (!svtAv1Path.isEmpty()) {
            env.insert("LD_LIBRARY_PATH", svtAv1Path);
            logBox->append("📚 Using custom SVT-AV1 library path: " + svtAv1Path);
        }
        converter->startConversion(inputFile, outputDir, baseName, args, twoPass, extension, codecStr, ffmpegPath, env, overwriteCheck->isChecked());
    });
    QObject::connect(cancelButton, &QPushButton::clicked, converter, &Converter::cancel);
    QObject::connect(converter, &Converter::logMessage, logBox, &QTextEdit::append);
    QObject::connect(converter, &Converter::progressUpdated, conversionProgress, &QProgressBar::setValue);
    QObject::connect(converter, &Converter::conversionFinished, [convertButton, cancelButton, conversionProgress, logBox, &window, converter, infoBox, selectedFilesBox]() {
        logBox->append("All conversions done, preparing to update GUI...");
        QString outputFile = converter->getFinalOutputFile();
        if (!outputFile.isEmpty()) {
            showConversionNotification(outputFile, &window);
        }
        QTimer::singleShot(100, [convertButton, cancelButton, conversionProgress, logBox]() {
            logBox->append("Updating buttons and progress bar now...");
            convertButton->setEnabled(true);
            cancelButton->setEnabled(false);
            conversionProgress->setVisible(false);
            logBox->append("GUI updated successfully!");
        });
        QString inputInfo = infoBox->toHtml();
        QString outputInfo = "";
        if (outputFile.isEmpty()) {
            outputInfo = "No output file specified.";
            logBox->append("⚠️ No output file specified.");
        } else {
            QFileInfo fileInfo(outputFile);
            if (!fileInfo.exists()) {
                outputInfo = "Output file does not exist: " + outputFile;
                logBox->append("⚠️ Output file does not exist: " + outputFile);
            } else if (!fileInfo.isReadable()) {
                outputInfo = "Output file is not readable: " + outputFile;
                logBox->append("⚠️ Output file is not readable: " + outputFile);
            } else {
                QProcess ffprobe;
                QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
                QSettings settings("FFmpegConverter", "Settings");
                QString svtAv1Path = settings.value("svtAv1Path", "").toString();
                if (!svtAv1Path.isEmpty()) {
                    QString existing = env.value("LD_LIBRARY_PATH");
                    env.insert("LD_LIBRARY_PATH", existing.isEmpty() ? svtAv1Path : existing + ":" + svtAv1Path);
                }
                ffprobe.setProcessEnvironment(env);
                QString ffmpegPath = settings.value("ffmpegPath", "/usr/bin/ffmpeg").toString();
                QString ffprobePath = ffmpegPath.isEmpty() ? "/usr/bin/ffprobe" : ffmpegPath.replace("ffmpeg", "ffprobe");
                QStringList args = {"-v", "error", "-show_format", "-show_streams", outputFile};
                QString ffprobeCommand = ffprobePath + " " + args.join(" ");
                logBox->append("Executing ffprobe for output: " + ffprobeCommand);
                ffprobe.start(ffprobePath, args);
                if (!ffprobe.waitForFinished(10000)) {
                    QString error = ffprobe.readAllStandardError();
                    logBox->append("⚠️ ffprobe failed for output: " + error);
                    outputInfo = "Failed to retrieve output file information: " + error;
                } else {
                    QString ffprobeOutput = ffprobe.readAllStandardOutput();
                    QString error = ffprobe.readAllStandardError();
                    if (!error.isEmpty()) {
                        logBox->append("ffprobe stderr for output: " + error);
                    }
                    if (ffprobeOutput.isEmpty()) {
                        logBox->append("⚠️ ffprobe produced no output for: " + outputFile);
                        outputInfo = "Unable to retrieve output file information.";
                    } else {
                        QString resolution = "N/A", videoCodec = "N/A", frameRate = "N/A", videoBitRate = "N/A";
                        QString audioCodec = "N/A", pixFmt = "N/A", durationStr = "N/A", fileContainer = "N/A", title = "N/A";
                        QString aspectRatio = "N/A", colorSpace = "N/A", pixelFormat = "N/A", audioSampleRate = "N/A";
                        qint64 fileSizeBytes = fileInfo.size();
                        QString fileSizeStr = (fileSizeBytes > 0) ? QString::number(fileSizeBytes / 1024.0 / 1024, 'f', 2) + " MB" : "N/A";
                        fileContainer = fileInfo.suffix();
                        if (fileContainer.isEmpty()) {
                            fileContainer = "N/A";
                        }
                        QStringList lines = ffprobeOutput.split("\n", Qt::SkipEmptyParts);
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
                        for (const auto &stream : streams) {
                            if (stream.value("codec_type") == "video") {
                                videoCodec = stream.value("codec_name", "N/A");
                                resolution = stream.value("width", "N/A") + "x" + stream.value("height", "N/A");
                                aspectRatio = stream.value("display_aspect_ratio", "N/A");
                                frameRate = stream.value("avg_frame_rate", stream.value("r_frame_rate", "N/A"));
                                if (frameRate.contains("/")) {
                                    QStringList parts = frameRate.split("/");
                                    if (parts.size() == 2) {
                                        double num = parts[0].toDouble();
                                        double den = parts[1].toDouble();
                                        if (den != 0 && num != 0) {
                                            frameRate = QString::number(num / den, 'f', 2);
                                        } else {
                                            frameRate = "N/A";
                                        }
                                    } else {
                                        frameRate = "N/A";
                                    }
                                }
                                videoBitRate = stream.value("bit_rate", "N/A");
                                if (videoBitRate == "N/A" || videoBitRate.toInt() <= 0) {
                                    videoBitRate = formatMap.value("bit_rate", "N/A");
                                }
                                if (videoBitRate != "N/A") {
                                    videoBitRate = QString::number(videoBitRate.toInt() / 1000) + " kbps";
                                }
                                pixelFormat = stream.value("pix_fmt", "N/A");
                                if (pixelFormat.contains("10")) pixFmt = "10-bit";
                                else if (pixelFormat.contains("12")) pixFmt = "12-bit";
                                else pixFmt = "8-bit";
                                colorSpace = stream.value("color_space", "N/A");
                                break;
                            }
                        }
                        for (const auto &stream : streams) {
                            if (stream.value("codec_type") == "audio") {
                                audioCodec = stream.value("codec_name", "N/A");
                                audioSampleRate = stream.value("sample_rate", "N/A");
                                if (audioSampleRate != "N/A") {
                                    audioSampleRate += " Hz";
                                }
                                break;
                            }
                        }
                        double durationSec = formatMap.value("duration", "0").toDouble();
                        if (durationSec > 0) {
                            int hours = static_cast<int>(durationSec / 3600);
                            double remaining = fmod(durationSec, 3600.0);
                            int mins = static_cast<int>(remaining / 60);
                            int secs = static_cast<int>(fmod(remaining, 60.0));
                            durationStr = QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
                        }
                        title = formatMap.value("TAG:title", "N/A");
                        if (title == "N/A" || title.isEmpty()) {
                            title = QFileInfo(outputFile).completeBaseName();
                        }
                        outputInfo = "<b>Video Title:</b> " + title + "<br>"
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
                    }
                }
            }
        }
        infoBox->setHtml("<h3>Input File</h3>" + inputInfo + "<h3>Output File</h3>" + outputInfo);
    });
    QTimer* debounceTimer = new QTimer(&window);
    debounceTimer->setSingleShot(true);
    static bool isProcessing = false;
    QObject::connect(selectedFilesBox, &QLineEdit::textChanged, [&debounceTimer, &updateInfo, logBox](const QString &text) {
        logBox->append("🔍 textChanged triggered with: " + text);
        if (!text.isEmpty() && !isProcessing) {
            isProcessing = true;
            debounceTimer->start(500);
        }
    });
    QObject::connect(debounceTimer, &QTimer::timeout, [selectedFilesBox, &updateInfo]() {
        updateInfo(selectedFilesBox->text());
        isProcessing = false;
    });
    window.show();
    return app.exec();
}
