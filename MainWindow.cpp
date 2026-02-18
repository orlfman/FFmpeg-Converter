#include "MainWindow.h"
#include "SettingsDialog.h"
#include "av1tab.h"
#include "vp9tab.h"
#include "x265tab.h"
#include "combinetab.h"
#include "trimtab.h"
#include "presets.h"
#include "converter.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QScrollArea>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QRandomGenerator>
#include <QUuid>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrent>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QProgressBar>
#include <QDialogButtonBox>
#include <QStandardItemModel>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    settings = new QSettings("FFmpegConverter", "Settings", this);
    converter = new Converter(this);

    setWindowTitle("FFmpeg Converter");
    setWindowIcon(QIcon::fromTheme("ffmpeg-converter-qt"));
    resize(1280, 720);

    setupUi();
    loadSettings();

    debounceTimer = new QTimer(this);
    debounceTimer->setSingleShot(true);

    wireAllSignals();
    updateRecentMenu();
    refreshOutputName();
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::setupUi()
{
    QWidget *central = new QWidget(this);
    mainLayout = new QVBoxLayout(central);
    setCentralWidget(central);

    createFileSelectionSection();
    createOutputNameSection();
    createOutputDirSection();
    createScalingSection();
    createOptionsSection();
    createPresetSection();
    createCodecTabs();
    createInfoAndConsoleTabs();
    createConversionControls();
    createMenuBar();
}

void MainWindow::createFileSelectionSection()
{
    QHBoxLayout *selectFilesLayout = new QHBoxLayout();
    selectFilesButton = new QPushButton("Select File");
    selectedFilesBox = new QLineEdit();
    selectedFilesBox->setReadOnly(true);
    selectFilesLayout->addWidget(selectFilesButton);
    selectFilesLayout->addWidget(selectedFilesBox);
    mainLayout->addLayout(selectFilesLayout);
}

void MainWindow::createOutputNameSection()
{
    QHBoxLayout *outputNameLayout = new QHBoxLayout();
    outputNameModeBox = new QComboBox();
    outputNameModeBox->addItems({"Output Name", "Date", "Random", "UUID", "Metadata"});
    outputNameModeBox->setCurrentIndex(0);
    outputNameBox = new QLineEdit();
    overwriteCheck = new QCheckBox("Overwrite");
    overwriteCheck->setChecked(false);
    overwriteCheck->setToolTip("If checked, overwrite existing files instead of adding numbers.");

    outputNameLayout->addWidget(outputNameModeBox);
    outputNameLayout->addWidget(outputNameBox);
    outputNameLayout->addWidget(overwriteCheck);
    mainLayout->addLayout(outputNameLayout);
}

void MainWindow::createOutputDirSection()
{
    QHBoxLayout *outputDirLayout = new QHBoxLayout();
    outputDirButton = new QPushButton("Output Directory");
    QString defaultOutputDir = settings->value("defaultOutputDir",
                                               QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
                                               outputDirBox = new QLineEdit(defaultOutputDir);
                                               outputDirLayout->addWidget(outputDirButton);
                                               outputDirLayout->addWidget(outputDirBox);
                                               mainLayout->addLayout(outputDirLayout);
}

void MainWindow::createScalingSection()
{
    // Width row
    QHBoxLayout *scaleWidthLayout = new QHBoxLayout();
    QLabel *scaleWidthLabel = new QLabel("Scale Width:");
    scaleWidthSpin = new QDoubleSpinBox(); scaleWidthSpin->setRange(0.0, 10.0); scaleWidthSpin->setSingleStep(0.05); scaleWidthSpin->setDecimals(2); scaleWidthSpin->setValue(1.0); scaleWidthSpin->setSuffix("x");
    scaleWidthSpin->setToolTip("Scale width by this factor (1.0 = original)");
    scaleWidthLayout->addWidget(scaleWidthLabel); scaleWidthLayout->addWidget(scaleWidthSpin);

    QLabel *scaleFilterLabel = new QLabel("Scale Filter:");
    scaleFilterBox = new QComboBox(); scaleFilterBox->addItems({"lanczos", "point", "bilinear", "bicubic", "spline16", "spline36"}); scaleFilterBox->setCurrentIndex(0);
    scaleFilterBox->setToolTip("lanczos: High-quality, sharp images, slower.\n"
    "point: Fast, but blocky.\n"
    "bilinear: Fast, smooth but blurry.\n"
    "bicubic: Balanced speed and quality.\n"
    "spline16: Smooth resizing.\n"
    "spline36: Smoother than spline16.");
    scaleWidthLayout->addWidget(scaleFilterLabel); scaleWidthLayout->addWidget(scaleFilterBox);

    eightBitCheck = new QCheckBox("Use 8-Bit Color"); eightBitCheck->setChecked(false);
    eightBitCheck->setToolTip("Enable 8-bit color to select a format. Standard for most displays.");
    scaleWidthLayout->addWidget(eightBitCheck);
    eightBitColorFormatBox = new QComboBox(); eightBitColorFormatBox->addItems({"8-bit 4:2:0", "8-bit 4:2:2", "8-bit 4:4:4"}); eightBitColorFormatBox->setCurrentIndex(0);
    eightBitColorFormatBox->setToolTip("Select 8-bit color format: 4:2:0, 4:2:2, or 4:4:4.");
    scaleWidthLayout->addWidget(eightBitColorFormatBox);

    tenBitCheck = new QCheckBox("Use 10-Bit Color");
    tenBitCheck->setToolTip("Enable 10-bit color to select a format. Reduces banding.");
    scaleWidthLayout->addWidget(tenBitCheck);
    colorFormatBox = new QComboBox(); colorFormatBox->addItems({"10-bit 4:2:0", "10-bit 4:2:2", "10-bit 4:4:4"}); colorFormatBox->setCurrentIndex(0); colorFormatBox->setEnabled(false);
    colorFormatBox->setToolTip("Select 10-bit color format: 4:2:0, 4:2:2, or 4:4:4.");
    scaleWidthLayout->addWidget(colorFormatBox);

    cropCheck = new QCheckBox("Crop");
    scaleWidthLayout->addWidget(cropCheck);
    detectCropButton = new QPushButton("Detect Crop"); detectCropButton->setEnabled(false);
    scaleWidthLayout->addWidget(detectCropButton);
    cropValueBox = new QLineEdit(); cropValueBox->setReadOnly(true); cropValueBox->setEnabled(false);
    cropValueBox->setToolTip("Detected crop values in format w:h:x:y.\nApplied before scaling.");
    scaleWidthLayout->addWidget(cropValueBox);
    scaleWidthLayout->addStretch();
    mainLayout->addLayout(scaleWidthLayout);

    // Height + seek/time row
    QHBoxLayout *scaleHeightLayout = new QHBoxLayout();
    QLabel *scaleHeightLabel = new QLabel("Scale Height:");
    scaleHeightSpin = new QDoubleSpinBox(); scaleHeightSpin->setRange(0.0, 10.0); scaleHeightSpin->setSingleStep(0.05); scaleHeightSpin->setDecimals(2); scaleHeightSpin->setValue(1.0); scaleHeightSpin->setSuffix("x");
    scaleHeightSpin->setToolTip("Scale height by this factor (1.0 = original)");
    scaleHeightLayout->addWidget(scaleHeightLabel); scaleHeightLayout->addWidget(scaleHeightSpin);

    QLabel *scaleRangeLabel = new QLabel("Scale Range:");
    scaleRangeBox = new QComboBox(); scaleRangeBox->addItems({"input", "limited", "full"}); scaleRangeBox->setCurrentIndex(0);
    scaleRangeBox->setToolTip("input: Use input video range.\n"
    "limited: 16-235 (TV standard).\n"
    "full: 0-255 (monitor standard).");
    scaleHeightLayout->addWidget(scaleRangeLabel); scaleHeightLayout->addWidget(scaleRangeBox);

    seekCheck = new QCheckBox("Seek");
    seekCheck->setToolTip("Enable to skip to a start time in the video.\nFormat: HH:MM:SS (e.g., 00:05:30 for 5 minutes 30 seconds).\nHours can be >23 for long videos.");
    scaleHeightLayout->addWidget(seekCheck);
    QLabel *seekLabel = new QLabel("HH:MM:SS");
    scaleHeightLayout->addWidget(seekLabel);
    seekHH = new QLineEdit("0"); seekHH->setFixedWidth(30); seekHH->setEnabled(false); seekHH->setValidator(new QIntValidator(0,99,seekHH));
    scaleHeightLayout->addWidget(seekHH); scaleHeightLayout->addWidget(new QLabel(":"));
    seekMM = new QLineEdit("0"); seekMM->setFixedWidth(30); seekMM->setEnabled(false); seekMM->setValidator(new QIntValidator(0,59,seekMM));
    scaleHeightLayout->addWidget(seekMM); scaleHeightLayout->addWidget(new QLabel(":"));
    seekSS = new QLineEdit("0"); seekSS->setFixedWidth(30); seekSS->setEnabled(false); seekSS->setValidator(new QIntValidator(0,59,seekSS));
    scaleHeightLayout->addWidget(seekSS);

    timeCheck = new QCheckBox("Time");
    timeCheck->setToolTip("Enable to limit output duration.\nFormat: HH:MM:SS (e.g., 01:00:00 for 1 hour).\nHours can be >23 for long videos.");
    scaleHeightLayout->addWidget(timeCheck);
    QLabel *timeLabel = new QLabel("HH:MM:SS");
    scaleHeightLayout->addWidget(timeLabel);
    timeHH = new QLineEdit("0"); timeHH->setFixedWidth(30); timeHH->setEnabled(false); timeHH->setValidator(new QIntValidator(0,99,timeHH));
    scaleHeightLayout->addWidget(timeHH); scaleHeightLayout->addWidget(new QLabel(":"));
    timeMM = new QLineEdit("0"); timeMM->setFixedWidth(30); timeMM->setEnabled(false); timeMM->setValidator(new QIntValidator(0,59,timeMM));
    scaleHeightLayout->addWidget(timeMM); scaleHeightLayout->addWidget(new QLabel(":"));
    timeSS = new QLineEdit("0"); timeSS->setFixedWidth(30); timeSS->setEnabled(false); timeSS->setValidator(new QIntValidator(0,59,timeSS));
    scaleHeightLayout->addWidget(timeSS);

    QLabel *frameRateLabel = new QLabel("Frame Rate:");
    scaleHeightLayout->addWidget(frameRateLabel);
    frameRateBox = new QComboBox(); frameRateBox->addItems({"Original", "24", "30", "60", "Custom"});
    frameRateBox->setToolTip("Set target frame rate.\nOriginal: Keep source rate.\nCustom: Enter custom value.");
    scaleHeightLayout->addWidget(frameRateBox);
    customFrameRateBox = new QLineEdit(); customFrameRateBox->setEnabled(false); customFrameRateBox->setValidator(new QDoubleValidator(0.1,1000.0,2));
    scaleHeightLayout->addWidget(customFrameRateBox);
    scaleHeightLayout->addStretch();
    mainLayout->addLayout(scaleHeightLayout);
}

void MainWindow::createOptionsSection()
{
    QHBoxLayout *optionsLayout = new QHBoxLayout();
    QLabel *rotateLabel = new QLabel("Rotate/Flip:");
    rotationBox = new QComboBox();
    rotationBox->addItems({"No Rotation", "90° Clockwise", "90° Counterclockwise", "180°", "Horizontal Flip", "Vertical Flip"});
    rotationBox->setCurrentIndex(0);
    rotationBox->setToolTip("Rotate or flip the video orientation. Applied before other filters like crop or scale.");
    optionsLayout->addWidget(rotateLabel);
    optionsLayout->addWidget(rotationBox);

    preserveMetadataCheck = new QCheckBox("Preserve metadata");
    preserveMetadataCheck->setToolTip("Keep original file metadata (title, author, etc).");
    optionsLayout->addWidget(preserveMetadataCheck);

    removeChaptersCheck = new QCheckBox("Remove Chapters");
    removeChaptersCheck->setToolTip("Remove chapter markers from video.");
    optionsLayout->addWidget(removeChaptersCheck);

    deinterlaceCheck = new QCheckBox("Deinterlace");
    deinterlaceCheck->setToolTip("Convert interlaced video to progressive scan.");
    optionsLayout->addWidget(deinterlaceCheck);

    deblockCheck = new QCheckBox("Deblock");
    deblockCheck->setToolTip("Removes ugly block artifacts from low bitrate or old videos");
    optionsLayout->addWidget(deblockCheck);

    normalizeAudioCheck = new QCheckBox("Normalize Audio");
    normalizeAudioCheck->setToolTip("Makes audio in videos play at the same loudness. No sudden spikes or quietness.");
    optionsLayout->addWidget(normalizeAudioCheck);

    denoiseCheck = new QCheckBox("Denoise");
    denoiseCheck->setToolTip("Fast, strong denoiser preset. Removes grain/noise");
    optionsLayout->addWidget(denoiseCheck);

    toneMapCheck = new QCheckBox("HDR to SDR");
    toneMapCheck->setToolTip("Fixes washed out look on HDR videos when playing in SDR mode");
    optionsLayout->addWidget(toneMapCheck);

    superSharpCheck = new QCheckBox("Super Sharp");
    superSharpCheck->setToolTip("Light sharpen plus detail pop, quick sharp preset");
    optionsLayout->addWidget(superSharpCheck);

    optionsLayout->addStretch();
    mainLayout->addLayout(optionsLayout);
}

void MainWindow::createPresetSection()
{
    QHBoxLayout *presetLayout = new QHBoxLayout();
    QLabel *presetLabel = new QLabel("Quality Preset:");
    presetCombo = new QComboBox();
    presetCombo->addItems({"Custom", "DVD", "Streaming", "Medium", "High", "Quality", "High Quality", "Ultra"});
    presetCombo->setCurrentIndex(0);
    presetCombo->setToolTip("One click presets for current codec");
    presetLayout->addWidget(presetLabel);
    presetLayout->addWidget(presetCombo);

    videoSpeedCheck = new QCheckBox();
    videoSpeedCheck->setToolTip("Enable video speed change");
    presetLayout->addWidget(videoSpeedCheck);

    QLabel *videoSpeedLabel = new QLabel(" Video Speed:");
    videoSpeedCombo = new QComboBox();
    videoSpeedCombo->setToolTip("Percentage change (positive = faster, negative = slower, 0% = normal)");
    presetLayout->addWidget(videoSpeedLabel);
    presetLayout->addWidget(videoSpeedCombo);

    audioSpeedCheck = new QCheckBox();
    audioSpeedCheck->setToolTip("Enable audio speed change (pitch preserved)");
    presetLayout->addWidget(audioSpeedCheck);

    QLabel *audioSpeedLabel = new QLabel(" Audio Speed:");
    audioSpeedCombo = new QComboBox();
    audioSpeedCombo->setToolTip("Percentage change (positive = faster, negative = slower, 0% = normal)");
    presetLayout->addWidget(audioSpeedLabel);
    presetLayout->addWidget(audioSpeedCombo);

    QStringList speedPercentages;
    for (int i = 100; i >= 5; i -= 5) speedPercentages << QString("%1%").arg(i);
    speedPercentages << "0%";
    for (int i = -5; i >= -100; i -= 5) speedPercentages << QString("%1%").arg(i);
    videoSpeedCombo->addItems(speedPercentages);
    audioSpeedCombo->addItems(speedPercentages);
    videoSpeedCombo->setCurrentText("0%");
    audioSpeedCombo->setCurrentText("0%");

    presetLayout->addSpacing(30);
    presetLayout->addWidget(videoSpeedCheck);
    presetLayout->addWidget(videoSpeedLabel);
    presetLayout->addWidget(videoSpeedCombo);
    presetLayout->addWidget(audioSpeedCheck);
    presetLayout->addWidget(audioSpeedLabel);
    presetLayout->addWidget(audioSpeedCombo);
    presetLayout->addStretch();
    mainLayout->addLayout(presetLayout);
}

void MainWindow::createCodecTabs()
{
    codecTabs = new QTabWidget();
    mainLayout->addWidget(codecTabs);

    // AV1
    av1Tab = new Av1Tab();
    QScrollArea *av1Scroll = new QScrollArea(); av1Scroll->setWidgetResizable(true); av1Scroll->setWidget(av1Tab);
    codecTabs->addTab(av1Scroll, "AV1");

    // x265
    x265Tab = new X265Tab();
    QScrollArea *x265Scroll = new QScrollArea(); x265Scroll->setWidgetResizable(true); x265Scroll->setWidget(x265Tab);
    codecTabs->addTab(x265Scroll, "x265");

    // VP9
    vp9Tab = new Vp9Tab();
    QScrollArea *vp9Scroll = new QScrollArea(); vp9Scroll->setWidgetResizable(true); vp9Scroll->setWidget(vp9Tab);
    codecTabs->addTab(vp9Scroll, "VP9");

    // Combine
    combineTab = new CombineTab();
    combineScroll = new QScrollArea();
    combineScroll->setWidgetResizable(true);
    combineScroll->setWidget(combineTab);
    codecTabs->addTab(combineScroll, "Combine Videos");

    // Trim
    trimTab = new TrimTab();
    trimScroll = new QScrollArea();
    trimScroll->setWidgetResizable(true);
    trimScroll->setWidget(trimTab);
    codecTabs->addTab(trimScroll, "Trim");
}

void MainWindow::createInfoAndConsoleTabs()
{
    // Information tab
    QWidget *infoTab = new QWidget();
    QVBoxLayout *infoLayout = new QVBoxLayout(infoTab);
    infoBox = new QTextEdit(); infoBox->setReadOnly(true);
    infoLayout->addWidget(infoBox);
    codecTabs->addTab(infoTab, "Information");

    // Console tab
    QWidget *consoleTab = new QWidget();
    QVBoxLayout *consoleLayout = new QVBoxLayout(consoleTab);

    QHBoxLayout *consoleInputLayout = new QHBoxLayout();
    QLabel *consoleLabel = new QLabel("Enter FFmpeg commands:");
    customCommandBox = new QLineEdit(); customCommandBox->setPlaceholderText("ex: -i input.mp4 -c:v libx264 output.mkv");
    runCommandButton = new QPushButton("Run Command");
    clearLogButton = new QPushButton("Clear Console");
    consoleInputLayout->addWidget(consoleLabel);
    consoleInputLayout->addWidget(customCommandBox);
    consoleInputLayout->addWidget(runCommandButton);
    consoleInputLayout->addWidget(clearLogButton);
    consoleInputLayout->addStretch();
    consoleLayout->addLayout(consoleInputLayout);

    logBox = new QTextEdit(); logBox->setReadOnly(true);
    consoleLayout->addWidget(logBox);
    codecTabs->addTab(consoleTab, "Console");
}

void MainWindow::createConversionControls()
{
    QHBoxLayout *conversionControlsLayout = new QHBoxLayout();
    convertButton = new QPushButton("Convert");
    cancelButton = new QPushButton("Cancel"); cancelButton->setEnabled(false);
    conversionControlsLayout->addWidget(convertButton);
    conversionControlsLayout->addWidget(cancelButton);
    conversionControlsLayout->addStretch();
    mainLayout->addLayout(conversionControlsLayout);

    conversionProgress = new QProgressBar();
    conversionProgress->setVisible(false);
    conversionProgress->setRange(0, 100);
    mainLayout->addWidget(conversionProgress);
}

void MainWindow::createMenuBar()
{
    QMenuBar *mBar = menuBar();
    QMenu *fileMenu = mBar->addMenu("&File");
    openAction = new QAction("&Open File...", this);
    openAction->setShortcut(QKeySequence::Open);
    fileMenu->addAction(openAction);

    recentMenu = fileMenu->addMenu("Recent Files");

    fileMenu->addSeparator();
    settingsAction = new QAction("&Settings...", this);
    settingsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Comma));
    fileMenu->addAction(settingsAction);

    fileMenu->addSeparator();
    QAction *exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    fileMenu->addAction(exitAction);

    connect(openAction, &QAction::triggered, this, &MainWindow::selectFile);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::openSettings);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    // View menu
    QMenu *viewMenu = mBar->addMenu("&View");
    QAction *viewInputAction = new QAction("📁 View Input File", this);
    viewInputAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    viewMenu->addAction(viewInputAction);
    QAction *viewOutputAction = new QAction("🎬 View Output File", this);
    viewOutputAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
    viewMenu->addAction(viewOutputAction);

    connect(viewInputAction, &QAction::triggered, this, [this]() {
        QString inputFile = selectedFilesBox->text().trimmed();
        if (inputFile.isEmpty() || !QFile::exists(inputFile)) {
            QMessageBox::warning(this, "Error", "No valid input file selected.");
            return;
        }
        QProcess::startDetached("xdg-open", QStringList() << inputFile);
    });
    connect(viewOutputAction, &QAction::triggered, this, [this]() {
        QString outputFile = converter->getFinalOutputFile();
        if (outputFile.isEmpty() || !QFile::exists(outputFile)) {
            QMessageBox::warning(this, "Error", "No output file available. Please convert first.");
            return;
        }
        QProcess::startDetached("xdg-open", QStringList() << outputFile);
    });

    // Help
    QMenu *helpMenu = mBar->addMenu("&Help");
    QAction *aboutAction = new QAction("&About", this);
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About FFmpeg Converter", "FFmpeg Converter v3.0.0\nBuilt with Qt 6\nFor video weebs!");
    });
    helpMenu->addAction(aboutAction);
}

void MainWindow::loadSettings()
{
    int defaultTab = settings->value("defaultCodecTab", 0).toInt();
    codecTabs->setCurrentIndex(defaultTab);
}

void MainWindow::saveSettings()
{
    settings->setValue("defaultCodecTab", codecTabs->currentIndex());
}

void MainWindow::wireAllSignals()
{
    // File selection
    connect(selectFilesButton, &QPushButton::clicked, this, &MainWindow::selectFile);

    // Output dir
    connect(outputDirButton, &QPushButton::clicked, this, &MainWindow::browseOutputDirectory);

    // Scaling
    connect(cropCheck, &QCheckBox::toggled, this, [this](bool checked) {
        detectCropButton->setEnabled(checked);
        cropValueBox->setEnabled(checked);
        if (!checked) cropValueBox->clear();
    });
        connect(detectCropButton, &QPushButton::clicked, this, &MainWindow::detectCrop);

        connect(seekCheck, &QCheckBox::toggled, this, [this](bool checked) {
            seekHH->setEnabled(checked); seekMM->setEnabled(checked); seekSS->setEnabled(checked);
        });
        connect(timeCheck, &QCheckBox::toggled, this, [this](bool checked) {
            timeHH->setEnabled(checked); timeMM->setEnabled(checked); timeSS->setEnabled(checked);
        });
        connect(frameRateBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
            customFrameRateBox->setEnabled(frameRateBox->itemText(idx) == "Custom");
        });
        connect(eightBitCheck, &QCheckBox::toggled, this, [this](bool checked) {
            eightBitColorFormatBox->setEnabled(checked);
            if (checked) {
                tenBitCheck->setChecked(false);
                colorFormatBox->setEnabled(false);
            }
        });

        connect(tenBitCheck, &QCheckBox::toggled, this, [this](bool checked) {
            colorFormatBox->setEnabled(checked);
            if (checked) {
                eightBitCheck->setChecked(false);
                eightBitColorFormatBox->setEnabled(false);
            }
        });

        // Output name
        connect(outputNameModeBox, &QComboBox::activated, this, &MainWindow::refreshOutputName);
        connect(selectedFilesBox, &QLineEdit::textChanged, this, [this](const QString &text) {
            cachedMetadataTitle.clear();
            refreshOutputName();

            if (!text.isEmpty() && !isProcessingInfo) {
                isProcessingInfo = true;
                debounceTimer->start(500);
            }
        });

        connect(debounceTimer, &QTimer::timeout, this, [this]() {
            showInfo(selectedFilesBox->text());
            isProcessingInfo = false;
        });

        // Presets
        Presets::connectPresets(presetCombo, codecTabs, av1Tab, x265Tab, vp9Tab, eightBitCheck, eightBitColorFormatBox, tenBitCheck, colorFormatBox);

        // Force custom preset
        connect(codecTabs, &QTabWidget::currentChanged, this, [this](int) { presetCombo->setCurrentIndex(0); });

        // Trim
        connect(selectedFilesBox, &QLineEdit::textChanged, trimTab, &TrimTab::setInputFile);

        // Console
        connect(runCommandButton, &QPushButton::clicked, this, &MainWindow::runCustomCommand);
        connect(clearLogButton, &QPushButton::clicked, logBox, &QTextEdit::clear);

        // Conversion
        connect(convertButton, &QPushButton::clicked, this, &MainWindow::startConversion);
        connect(cancelButton, &QPushButton::clicked, this, [this]() {
            converter->cancel();
            combineTab->cancelConcatenation();
            convertButton->setEnabled(true);
            cancelButton->setEnabled(false);
            conversionProgress->setVisible(false);
            trimTab->restartPreviewPlayer();
        });

        // Converter signals
        connect(converter, &Converter::logMessage, logBox, &QTextEdit::append);
        connect(converter, &Converter::progressUpdated, conversionProgress, &QProgressBar::setValue);
        connect(converter, &Converter::conversionFinished, this, &MainWindow::onConversionFinished);

        connect(combineTab, &CombineTab::logMessage, logBox, &QTextEdit::append);
        connect(combineTab, &CombineTab::progressUpdated, conversionProgress, &QProgressBar::setValue);
        connect(combineTab, &CombineTab::conversionFinished, this, [this]() {
            convertButton->setEnabled(true);
            cancelButton->setEnabled(false);
            conversionProgress->setVisible(false);
            logBox->append("Concatenation finished!");
            QString finalFile = combineTab->getFinalOutputFile();
            if (!finalFile.isEmpty()) showConversionNotification(finalFile);
        });
            auto forceCustom = [this]() { presetCombo->setCurrentIndex(0); };

            for (QWidget* tab : { static_cast<QWidget*>(av1Tab),
                static_cast<QWidget*>(x265Tab),
                 static_cast<QWidget*>(vp9Tab) }) {
                for (QPushButton* btn : tab->findChildren<QPushButton*>()) {
                    if (btn && btn->text() == "Reset to Defaults") {
                        connect(btn, &QPushButton::clicked, this, forceCustom);
                        break;
                    }
                }
                 }
}

void MainWindow::detectCrop()
{
    QString inputFile = selectedFilesBox->text();
    if (inputFile.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select an input file to detect crop.");
        return;
    }
    QFileInfo fileInfo(inputFile);
    if (!fileInfo.exists() || !fileInfo.isReadable()) {
        logBox->append("⚠️ Input file does not exist or is not readable.");
        QMessageBox::warning(this, "Error", "Input file does not exist or is not readable.");
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
    QString svtAv1Path = settings->value("svtAv1Path", "").toString();
    if (!svtAv1Path.isEmpty()) {
        QString existing = env.value("LD_LIBRARY_PATH");
        env.insert("LD_LIBRARY_PATH", existing.isEmpty() ? svtAv1Path : existing + ":" + svtAv1Path);
    }
    process.setProcessEnvironment(env);

    QString ffmpegPath = settings->value("ffmpegPath", "/usr/bin/ffmpeg").toString();
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
    QRegularExpression cropRegex("crop=(\\d+:\\d+:\\d+:\\d+)");
    QString cropValue;
    QStringList lines = output.split("\n");
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
}

void MainWindow::refreshOutputName()
{
    int mode = outputNameModeBox->currentIndex();
    QString input = selectedFilesBox->text().trimmed();
    if (!input.isEmpty()) originalFilename = QFileInfo(input).baseName();

    QString name;
    if (mode == 0) {
        outputNameBox->setReadOnly(false);
        name = originalFilename.isEmpty() ? "Output" : originalFilename;
    } else {
        outputNameBox->setReadOnly(true);
        if (mode == 1) name = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
        else if (mode == 2) {
            const QString chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
            name.clear();
            for (int i = 0; i < 20; ++i) name += chars[QRandomGenerator::global()->bounded(chars.size())];
        } else if (mode == 3) name = QUuid::createUuid().toString(QUuid::WithoutBraces);
        else if (mode == 4) {
            if (cachedMetadataTitle.isEmpty()) cachedMetadataTitle = extractTitle(selectedFilesBox->text());
            name = cachedMetadataTitle.isEmpty() ? originalFilename : cachedMetadataTitle;
        }
    }
    outputNameBox->blockSignals(true);
    outputNameBox->setText(name);
    outputNameBox->blockSignals(false);
}

void MainWindow::forceCustomPreset()
{
    presetCombo->setCurrentIndex(0);
}

QString MainWindow::extractTitle(const QString &inputFile)
{
    if (inputFile.isEmpty()) return QString();
    QString ffmpegPath = settings->value("ffmpegPath", "/usr/bin/ffmpeg").toString();
    QString ffprobePath = ffmpegPath.contains("ffmpeg", Qt::CaseInsensitive) ? ffmpegPath.replace("ffmpeg", "ffprobe", Qt::CaseInsensitive) : "ffprobe";
    QProcess p;
    p.start(ffprobePath, {"-v", "quiet", "-print_format", "json", "-show_entries", "format_tags=title,TAG:title", inputFile});
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
}

void MainWindow::runCustomCommand()
{
    QString command = customCommandBox->text().trimmed();
    if (command.isEmpty()) return;

    runCommandButton->setEnabled(false);
    QProcess *process = new QProcess(this);
    QString ffmpegPath = settings->value("ffmpegPath", "/usr/bin/ffmpeg").toString();
    if (ffmpegPath.isEmpty()) ffmpegPath = "/usr/bin/ffmpeg";

    QStringList args = command.split(" ", Qt::SkipEmptyParts);
    process->start(ffmpegPath, args);

    logBox->append("Running custom command: " + ffmpegPath + " " + command);

    connect(process, &QProcess::readyReadStandardOutput, this, [process, this]() {
        logBox->append(process->readAllStandardOutput().trimmed());
    });
    connect(process, &QProcess::readyReadStandardError, this, [process, this]() {
        logBox->append(process->readAllStandardError().trimmed());
    });
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, process](int exitCode, QProcess::ExitStatus) {
        if (exitCode != 0) {
            logBox->append("❌ Custom command failed.");
        } else {
            logBox->append("✅ Custom command completed successfully.");
        }
        process->deleteLater();
        runCommandButton->setEnabled(true);
    });
}

void MainWindow::browseOutputDirectory()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setWindowTitle("Select Output Directory");

    QSettings s("FFmpegConverter", "Settings");
    QString lastDir = s.value("lastSettingsBrowseDir",
                              QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).toString();
                              dialog.setDirectory(lastDir);

                              if (dialog.exec()) {
                                  QString selectedDir = dialog.selectedFiles().first();
                                  outputDirBox->setText(selectedDir);
                                  s.setValue("lastSettingsBrowseDir", QFileInfo(selectedDir).absolutePath());
                              }
}

void MainWindow::selectFile()
{
    QFileDialog dialog(this);
    dialog.setWindowTitle("🎬 Select Video File");
    dialog.setNameFilters({"Video Files (*.mp4 *.mkv *.webm *.avi *.mov *.wmv *.flv *.m4v *.ts *.m2ts)", "All Files (*)"});
    QSettings rec("FFmpegConverter", "Recent");
    dialog.setDirectory(rec.value("lastVideoDir", QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)).toString());

    if (dialog.exec()) {
        QString file = dialog.selectedFiles().first();
        selectedFilesBox->setText(file);

        QStringList recent = rec.value("recentFiles").toStringList();
        recent.removeAll(file);
        recent.prepend(file);
        while (recent.size() > 15) recent.removeLast();
        rec.setValue("recentFiles", recent);
        rec.setValue("lastVideoDir", QFileInfo(file).absolutePath());

        updateRecentMenu();
        onFileSelected(file);
    }
}

void MainWindow::openSettings()
{
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        settings->setValue("defaultOutputDir", dialog.getDefaultOutputDir());
        settings->setValue("defaultCodecTab", dialog.getDefaultCodecTab());
        settings->setValue("ffmpegPath", dialog.getFFmpegPath());
        settings->setValue("notifyOnFinish", dialog.getNotifyOnFinish());
        settings->setValue("svtAv1Path", dialog.getSvtAv1Path());
        QMessageBox::information(this, "✅ Settings Saved", "✓ Default codec tab\n✓ FFmpeg path\n✓ SVT-AV1 library path\n✓ Output directory\n✓ Notifications");
    }
}

void MainWindow::updateRecentMenu()
{
    if (!recentMenu) return;
    recentMenu->clear();
    QSettings rec("FFmpegConverter", "Recent");
    QStringList recent = rec.value("recentFiles").toStringList();
    int count = 0;
    QStringList valid;
    for (const QString &file : recent) {
        if (QFile::exists(file)) {
            valid.append(file);
            QString displayName = QFileInfo(file).baseName() + " " + QFileInfo(file).suffix().toUpper();
            QAction *act = recentMenu->addAction(QString("%1. %2").arg(++count).arg(displayName));
            connect(act, &QAction::triggered, this, [this, file]() {
                selectedFilesBox->setText(file);
                onFileSelected(file);
            });
        }
    }
    rec.setValue("recentFiles", valid);
    if (count == 0) {
        recentMenu->addAction("(No recent files)")->setEnabled(false);
    }
    recentMenu->addSeparator();
    QAction *clearRecentAction = recentMenu->addAction("🗑️ Clear Recent Videos");
    connect(clearRecentAction, &QAction::triggered, this, [this]() {
        QSettings rec("FFmpegConverter", "Recent");
        rec.remove("recentFiles");
        rec.remove("lastVideoDir");
        QMessageBox::information(this, "Cleared!", "Recent videos & last folder cleared!");
        updateRecentMenu();
    });
}

void MainWindow::onFileSelected(const QString &file)
{
    showInfo(file);
    refreshOutputName();
}

void MainWindow::showInfo(const QString &inputFile)
{
    if (inputFile.isEmpty()) {
        infoBox->setText("No input file selected.");
        return;
    }

    [[maybe_unused]] auto _ = QtConcurrent::run([this, inputFile]() {
        QString fileSizeStr = "N/A";
        QString fileContainer = QFileInfo(inputFile).suffix().isEmpty() ? "N/A" : QFileInfo(inputFile).suffix();
        QFileInfo fileInfo(inputFile);
        if (!fileInfo.exists() || !fileInfo.isReadable()) {
            QMetaObject::invokeMethod(infoBox, "setText", Qt::QueuedConnection,
                                      Q_ARG(QString, fileInfo.exists() ? "Input file is not readable: " + inputFile : "Input file does not exist: " + inputFile));
            return;
        }
        qint64 fileSizeBytes = fileInfo.size();
        fileSizeStr = (fileSizeBytes > 0) ? QString::number(fileSizeBytes / 1024.0 / 1024, 'f', 2) + " MB" : "N/A";

        QProcess ffprobe;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString svtAv1Path = settings->value("svtAv1Path", "").toString();
        if (!svtAv1Path.isEmpty()) {
            QString existing = env.value("LD_LIBRARY_PATH");
            env.insert("LD_LIBRARY_PATH", existing.isEmpty() ? svtAv1Path : existing + ":" + svtAv1Path);
        }
        ffprobe.setProcessEnvironment(env);

        QString ffmpegPath = settings->value("ffmpegPath", "/usr/bin/ffmpeg").toString();
        QString ffprobePath = ffmpegPath.isEmpty() ? "/usr/bin/ffprobe" : QFileInfo(ffmpegPath).absolutePath() + "/ffprobe" + QFileInfo(ffmpegPath).suffix();
        if (!QFile::exists(ffprobePath)) ffprobePath = "/usr/bin/ffprobe";

        ffprobe.start(ffprobePath, {"-v", "error", "-show_format", "-show_streams", inputFile});
        if (!ffprobe.waitForFinished(10000)) {
            QMetaObject::invokeMethod(infoBox, "setText", Qt::QueuedConnection, Q_ARG(QString, "Failed to retrieve information (timeout)"));
            return;
        }

        QString output = ffprobe.readAllStandardOutput();
        QString error = ffprobe.readAllStandardError();
        if (!error.isEmpty()) logBox->append("ffprobe stderr: " + error);

        // Full parsing logic (identical to original)
        QString resolution = "N/A", videoCodec = "N/A", frameRate = "N/A", videoBitRate = "N/A";
        QString audioCodec = "N/A", pixFmt = "N/A", durationStr = "N/A", title = "N/A";
        QString aspectRatio = "N/A", colorSpace = "N/A", pixelFormat = "N/A", audioSampleRate = "N/A";

        QStringList lines = output.split("\n", Qt::SkipEmptyParts);
        QList<QMap<QString, QString>> streams;
        QMap<QString, QString> formatMap;
        QMap<QString, QString> *currentMap = nullptr;

        for (const QString &line : lines) {
            if (line == "[STREAM]") { streams.append(QMap<QString, QString>()); currentMap = &streams.last(); }
            else if (line == "[FORMAT]") { currentMap = &formatMap; }
            else if (line.startsWith("[/")) { currentMap = nullptr; }
            else if (currentMap) {
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
            if (selectedStreamIndex == -1) selectedStreamIndex = videoStreamIndices.last();
        }

        if (selectedStreamIndex >= 0) {
            const auto &stream = streams[selectedStreamIndex];
            videoCodec = stream.value("codec_name", "N/A");
            resolution = stream.value("width", "N/A") + "x" + stream.value("height", "N/A");
            aspectRatio = stream.value("display_aspect_ratio", "N/A");
            frameRate = stream.value("avg_frame_rate", stream.value("r_frame_rate", "N/A"));
            if (frameRate.contains("/")) {
                QStringList parts = frameRate.split("/");
                if (parts.size() == 2) frameRate = QString::number(parts[0].toDouble() / parts[1].toDouble(), 'f', 2);
            }
            videoBitRate = stream.value("bit_rate", "N/A");
            if (videoBitRate == "N/A" || videoBitRate.toInt() <= 0) videoBitRate = formatMap.value("bit_rate", "N/A");
            if (videoBitRate != "N/A") videoBitRate = QString::number(videoBitRate.toInt() / 1000) + " kbps";

            pixelFormat = stream.value("pix_fmt", "N/A");
            if (pixelFormat.contains("10")) pixFmt = "10-bit";
            else if (pixelFormat.contains("12")) pixFmt = "12-bit";
            else pixFmt = "8-bit";

            bool isTwelveBit = pixFmt.contains("12");
            bool isTenBit = pixFmt.contains("10") && !isTwelveBit;
            if (isTwelveBit) {
                QMetaObject::invokeMethod(eightBitCheck, "setChecked", Qt::QueuedConnection, Q_ARG(bool, false));
                QMetaObject::invokeMethod(tenBitCheck, "setChecked", Qt::QueuedConnection, Q_ARG(bool, false));
                QMetaObject::invokeMethod(eightBitColorFormatBox, "setEnabled", Qt::QueuedConnection, Q_ARG(bool, true));
                QMetaObject::invokeMethod(colorFormatBox, "setEnabled", Qt::QueuedConnection, Q_ARG(bool, true));
            } else if (isTenBit) {
                QMetaObject::invokeMethod(tenBitCheck, "setChecked", Qt::QueuedConnection, Q_ARG(bool, true));
                QMetaObject::invokeMethod(eightBitCheck, "setChecked", Qt::QueuedConnection, Q_ARG(bool, false));
                QMetaObject::invokeMethod(colorFormatBox, "setEnabled", Qt::QueuedConnection, Q_ARG(bool, true));
                QMetaObject::invokeMethod(eightBitColorFormatBox, "setEnabled", Qt::QueuedConnection, Q_ARG(bool, false));
            }

            colorSpace = stream.value("color_space", "N/A");
        }

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
            int mins = static_cast<int>((durationSec - hours * 3600) / 60);
            int secs = static_cast<int>(durationSec - hours * 3600 - mins * 60);
            durationStr = QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
        }
        title = formatMap.value("TAG:title", QFileInfo(inputFile).completeBaseName());

        QString infoText = "<b>Video Title:</b> " + title + "<br>"
        "<b>Resolution:</b> " + resolution + "<br>"
        "<b>Aspect Ratio:</b> " + aspectRatio + "<br>"
        "<b>Duration:</b> " + durationStr + "<br>"
        "<b>Video Codec:</b> " + videoCodec + "<br>"
        "<b>Frame Rate:</b> " + frameRate + "<br>"
        "<b>Bit Rate:</b> " + videoBitRate + "<br>"
        "<b>Audio Codec:</b> " + audioCodec + "<br>"
        "<b>Audio Sample Rate:</b> " + audioSampleRate + "<br>"
        "<b>Color Bit Depth:</b> " + pixFmt + "<br>"
        "<b>Color Space:</b> " + colorSpace + "<br>"
        "<b>Pixel Format:</b> " + pixelFormat + "<br>"
        "<b>File Size:</b> " + fileSizeStr + "<br>"
        "<b>File Container:</b> " + fileContainer + "<br>";

        QMetaObject::invokeMethod(infoBox, "setHtml", Qt::QueuedConnection, Q_ARG(QString, infoText));
    });
}

void MainWindow::startConversion()
{
    logBox->clear();
    logBox->append("=== CONVERSION STARTED ===");

    bool isCombine = (codecTabs->currentWidget() == combineScroll);
    bool isTrim   = (codecTabs->currentWidget() == trimScroll);

    QString inputFile;
    if (!isCombine) {
        inputFile = selectedFilesBox->text().trimmed();
        if (inputFile.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please select an input file.");
            return;
        }
    } else {
        inputFile = "(Multiple files from Combine Videos tab)";
    }
    logBox->append("📁 Input: " + inputFile);
    logBox->append("🎛️ Scale: " + QString::number(scaleWidthSpin->value(), 'f', 2) + "x × " + QString::number(scaleHeightSpin->value(), 'f', 2) + "x");

    if (seekCheck->isChecked()) {
        bool okHH, okMM, okSS;
        int hh = seekHH->text().toInt(&okHH);
        int mm = seekMM->text().toInt(&okMM);
        int ss = seekSS->text().toInt(&okSS);
        if (!okHH || !okMM || !okSS || hh < 0 || hh > 99 || mm < 0 || mm > 59 || ss < 0 || ss > 59) {
            QMessageBox::warning(this, "Invalid Input", "Seek time invalid (HH:00-99, MM/SS:00-59).");
            return;
        }
    }
    if (timeCheck->isChecked()) {
        bool okHH, okMM, okSS;
        int hh = timeHH->text().toInt(&okHH);
        int mm = timeMM->text().toInt(&okMM);
        int ss = timeSS->text().toInt(&okSS);
        if (!okHH || !okMM || !okSS || hh < 0 || hh > 99 || mm < 0 || mm > 59 || ss < 0 || ss > 59) {
            QMessageBox::warning(this, "Invalid Input", "Duration time invalid (HH:00-99, MM/SS:00-59).");
            return;
        }
    }

    int currentTab = codecTabs->currentIndex();
    if (currentTab == 0 && av1Tab->av1EnableRCModeCheck->isChecked()) {
        if (av1Tab->av1RCModeBox->currentText() == "VBR" && av1Tab->av1AQModeBox->currentIndex() == 1) {
            QMessageBox::warning(this, "Invalid Configuration", "Adaptive Quantization cannot be disabled when using VBR rate control for AV1.");
            return;
        }
    } else if (currentTab == 1 && x265Tab->x265EnableRCModeCheck->isChecked()) {
        QString rcMode = x265Tab->x265RCModeBox->currentText();
        if ((rcMode == "ABR" || rcMode == "CBR") && x265Tab->x265AQModeBox->currentIndex() == 1) {
            QMessageBox::warning(this, "Invalid Configuration", "Adaptive Quantization cannot be disabled when using ABR or CBR rate control for x265.");
            return;
        }
    } else if (currentTab == 2 && vp9Tab->vp9EnableRCModeCheck->isChecked()) {
        QString rcMode = vp9Tab->vp9RCModeBox->currentText();
        if ((rcMode == "ABR" || rcMode == "CBR") && vp9Tab->vp9AQModeBox->currentIndex() == 1) {
            QMessageBox::warning(this, "Invalid Configuration", "Adaptive Quantization cannot be disabled when using ABR or CBR rate control for VP9.");
            return;
        }
    }

    QString outputDir = outputDirBox->text();
    QString baseName = outputNameBox->text().isEmpty() ? "Output" : outputNameBox->text();
    QString extension = ".mkv";
    bool twoPass = false;
    QString codecStr = "copy";

    if (!isCombine && !isTrim) {
        if (currentTab == 0) {
            extension = "." + av1Tab->av1ContainerBox->currentText();
            twoPass = av1Tab->av1TwoPassCheck->isChecked();
            codecStr = "av1";
        } else if (currentTab == 1) {
            extension = "." + x265Tab->x265ContainerBox->currentText();
            twoPass = x265Tab->x265TwoPassCheck->isChecked();
            if (x265Tab->x265EnableRCModeCheck->isChecked() && x265Tab->x265RCModeBox->currentText() == "CRF") twoPass = false;
            else if (!x265Tab->x265EnableRCModeCheck->isChecked()) twoPass = false;
            codecStr = "x265";
        } else if (currentTab == 2) {
            extension = "." + vp9Tab->vp9ContainerBox->currentText();
            twoPass = vp9Tab->vp9TwoPassCheck->isChecked();
            codecStr = "vp9";
        }
    }

    QStringList args;
    if (preserveMetadataCheck->isChecked()) args << "-map_metadata" << "0";
    if (removeChaptersCheck->isChecked()) args << "-map_chapters" << "-1";

    QStringList videoFilters;
    QStringList audioFilters;

    // Rotation
    QString rotationFilter;
    QString rotation = rotationBox->currentText();
    if (rotation == "90° Clockwise") rotationFilter = "transpose=1";
    else if (rotation == "90° Counterclockwise") rotationFilter = "transpose=2";
    else if (rotation == "180°") rotationFilter = "transpose=1,transpose=1";
    else if (rotation == "Horizontal Flip") rotationFilter = "hflip";
    else if (rotation == "Vertical Flip") rotationFilter = "vflip";
    if (!rotationFilter.isEmpty()) videoFilters << rotationFilter;

    // Crop
    if (cropCheck->isChecked() && !cropValueBox->text().isEmpty() && cropValueBox->text() != "Not detected") {
        QString cropValue = cropValueBox->text();
        if (cropValue.startsWith("crop=")) cropValue = cropValue.mid(5);
        videoFilters << "crop=" + cropValue;
    }

    if (deinterlaceCheck->isChecked()) videoFilters << "yadif";
    if (deblockCheck->isChecked()) videoFilters << "deblock";
    if (denoiseCheck->isChecked()) videoFilters << "hqdn3d=4:3:6:4.5";
    if (superSharpCheck->isChecked()) videoFilters << "unsharp=5:5:0.8:3:3:0.4";
    if (toneMapCheck->isChecked()) {
        videoFilters << "zscale=t=linear:npl=100,format=gbrpf32le,zscale=p=bt709,tonemap=tonemap=hable:desat=0,zscale=t=bt709:m=bt709:r=tv";
    }

    // Tab-specific filters
    if (currentTab == 0) { // AV1 filters
        if (av1Tab->av1UnsharpenCheck->isChecked()) videoFilters << QString("unsharp=5:5:%1:5:5:0.0").arg(av1Tab->av1UnsharpenStrengthSlider->value() / 10.0);
        if (av1Tab->av1SharpenCheck->isChecked()) videoFilters << QString("unsharp=3:3:%1:3:3:0.0").arg(av1Tab->av1SharpenStrengthSlider->value() / 10.0);
        if (av1Tab->av1BlurCheck->isChecked()) videoFilters << QString("smartblur=%1:0.5:0").arg(av1Tab->av1BlurStrengthSlider->value() / 10.0);
        if (av1Tab->av1NoiseReductionCheck->isChecked()) videoFilters << QString("hqdn3d=luma_spatial=%1:chroma_spatial=%1").arg(av1Tab->av1NoiseReductionSlider->value());
        if (av1Tab->av1GrainSynthCheck->isChecked()) videoFilters << QString("noise=alls=%1:allf=t").arg(av1Tab->av1GrainSynthLevel->value());
        if (av1Tab->av1NlmeansCheck->isChecked()) {
            int s = av1Tab->av1NlmeansSigmaSSlider->value();
            int p = av1Tab->av1NlmeansSigmaPSlider->value();
            int patch = av1Tab->av1NlmeansPatchSlider->value();
            QString filterName = av1Tab->av1NlmeansGpuCheck->isChecked() ? "knlmeans" : "nlmeans";
            QString patchStr = (filterName == "knlmeans") ? ":patch=" + QString::number(patch) : "";
            videoFilters << QString("%1=s=%2:p=%3%4").arg(filterName).arg(s).arg(p).arg(patchStr);
        }
    } else if (currentTab == 1) { // x265
        if (x265Tab->x265UnsharpenCheck->isChecked()) videoFilters << QString("unsharp=5:5:%1:5:5:0.0").arg(x265Tab->x265UnsharpenStrengthSlider->value() / 10.0);
        if (x265Tab->x265SharpenCheck->isChecked()) videoFilters << QString("unsharp=3:3:%1:3:3:0.0").arg(x265Tab->x265SharpenStrengthSlider->value() / 10.0);
        if (x265Tab->x265BlurCheck->isChecked()) videoFilters << QString("smartblur=%1:0.5:0").arg(x265Tab->x265BlurStrengthSlider->value() / 10.0);
        if (x265Tab->x265NoiseReductionCheck->isChecked()) videoFilters << QString("hqdn3d=luma_spatial=%1:chroma_spatial=%1").arg(x265Tab->x265NoiseReductionSlider->value());
        if (x265Tab->x265GrainSynthCheck->isChecked()) videoFilters << QString("noise=alls=%1:allf=t").arg(x265Tab->x265GrainSynthLevel->value());
    } else if (currentTab == 2) { // VP9
        if (vp9Tab->vp9UnsharpenCheck->isChecked()) videoFilters << QString("unsharp=5:5:%1:5:5:0.0").arg(vp9Tab->vp9UnsharpenStrengthSlider->value() / 10.0);
        if (vp9Tab->vp9SharpenCheck->isChecked()) videoFilters << QString("unsharp=3:3:%1:3:3:0.0").arg(vp9Tab->vp9SharpenStrengthSlider->value() / 10.0);
        if (vp9Tab->vp9BlurCheck->isChecked()) videoFilters << QString("smartblur=%1:0.5:0").arg(vp9Tab->vp9BlurStrengthSlider->value() / 10.0);
        if (vp9Tab->vp9NoiseReductionCheck->isChecked()) videoFilters << QString("hqdn3d=luma_spatial=%1:chroma_spatial=%1").arg(vp9Tab->vp9NoiseReductionSlider->value());
        if (vp9Tab->vp9GrainSynthCheck->isChecked()) videoFilters << QString("noise=alls=%1:allf=t").arg(vp9Tab->vp9GrainSynthLevel->value());
        if (vp9Tab->vp9NlmeansCheck->isChecked()) {
            int s = vp9Tab->vp9NlmeansSigmaSSlider->value();
            int p = vp9Tab->vp9NlmeansSigmaPSlider->value();
            int patch = vp9Tab->vp9NlmeansPatchSlider->value();
            QString filterName = vp9Tab->vp9NlmeansGpuCheck->isChecked() ? "knlmeans" : "nlmeans";
            QString patchStr = (filterName == "knlmeans") ? ":patch=" + QString::number(patch) : "";
            videoFilters << QString("%1=s=%2:p=%3%4").arg(filterName).arg(s).arg(p).arg(patchStr);
        }
    }

    // Scale
    double sw = scaleWidthSpin->value();
    double sh = scaleHeightSpin->value();
    QString filterName = scaleFilterBox->currentText();
    if (!qFuzzyCompare(sw, 1.0) || !qFuzzyCompare(sh, 1.0)) {
        QString w = qFuzzyCompare(sw, 1.0) ? "iw" : QString("trunc(iw*%1/2)*2").arg(sw);
        QString h = qFuzzyCompare(sh, 1.0) ? "ih" : QString("trunc(ih*%1/2)*2").arg(sh);
        if (filterName == "spline16" || filterName == "spline36") {
            videoFilters << QString("zscale=w=%1:h=%2:filter=%3").arg(w, h, filterName);
        } else {
            videoFilters << QString("scale=w=%1:h=%2:flags=%3").arg(w, h, filterName.toLower());
        }
        if (scaleRangeBox->currentText() != "input") {
            videoFilters << QString("zscale=range=%1").arg(scaleRangeBox->currentText().toLower());
        }
    }

    // Frame rate
    if (frameRateBox->currentText() != "Original") {
        QString fpsValue = (frameRateBox->currentText() == "Custom") ? customFrameRateBox->text() : frameRateBox->currentText();
        videoFilters << "fps=" + fpsValue;
    }

    // Pixel format
    QString pixFmt;
    if (tenBitCheck->isChecked()) {
        QString f = colorFormatBox->currentText();
        pixFmt = (f == "10-bit 4:2:0") ? "yuv420p10le" : (f == "10-bit 4:2:2") ? "yuv422p10le" : "yuv444p10le";
    } else {
        QString f = eightBitColorFormatBox->currentText();
        pixFmt = (f == "8-bit 4:2:0") ? "yuv420p" : (f == "8-bit 4:2:2") ? "yuv422p" : "yuv444p";
    }
    videoFilters << "format=" + pixFmt;

    // Audio normalize
    if (normalizeAudioCheck->isChecked()) {
        audioFilters << "loudnorm=I=-23:TP=-1.5:LRA=11";
    }

    // Speed
    auto getPercentChange = [](const QString &str) -> double {
        if (str == "0%") return 0.0;
        return str.chopped(1).toDouble();
    };
    double videoPercent = videoSpeedCheck->isChecked() ? getPercentChange(videoSpeedCombo->currentText()) : 0.0;
    double audioPercent = audioSpeedCheck->isChecked() ? getPercentChange(audioSpeedCombo->currentText()) : 0.0;
    double videoMultiplier = 1.0 + videoPercent / 100.0;
    double audioMultiplier = 1.0 + audioPercent / 100.0;

    QString seekTimeStr = "";
    if (seekCheck->isChecked()) {
        int hh = seekHH->text().toInt();
        int mm = seekMM->text().toInt();
        int ss = seekSS->text().toInt();
        seekTimeStr = QString("%1:%2:%3").arg(hh, 2, 10, QChar('0')).arg(mm, 2, 10, QChar('0')).arg(ss, 2, 10, QChar('0'));
    }

    QString outputTimeStr = "";
    if (timeCheck->isChecked()) {
        int hh = timeHH->text().toInt();
        int mm = timeMM->text().toInt();
        int ss = timeSS->text().toInt();
        outputTimeStr = QString("%1:%2:%3").arg(hh, 2, 10, QChar('0')).arg(mm, 2, 10, QChar('0')).arg(ss, 2, 10, QChar('0'));
    }

    if (!qFuzzyCompare(videoMultiplier, 1.0)) {
        double ptsFactor = 1.0 / videoMultiplier;
        videoFilters << QString("setpts=%1*PTS").arg(ptsFactor, 0, 'g', 12);
    }
    if (!qFuzzyCompare(audioMultiplier, 1.0)) {
        auto buildAtempoChain = [](double m) -> QString {
            if (qFuzzyCompare(m, 1.0)) return "";
            QStringList p;
            double t = m;
            if (m > 1.0) {
                while (t > 2.0) { p << "atempo=2.0"; t /= 2.0; }
            } else if (m < 1.0) {
                while (t < 0.5) { p << "atempo=0.5"; t /= 0.5; }
            }
            if (!qFuzzyCompare(t, 1.0)) p << QString("atempo=%1").arg(t, 0, 'g', 12);
            return p.join(",");
        };
        QString chain = buildAtempoChain(audioMultiplier);
        if (!chain.isEmpty()) audioFilters.prepend(chain);
    }

    if (!videoFilters.isEmpty()) {
        QString chain = videoFilters.join(",");
        args << "-vf" << chain;
    }
    if (!audioFilters.isEmpty()) {
        QString chain = audioFilters.join(",");
        args << "-af" << chain;
    }

    // Codec specific args
    if (currentTab == 0) { // AV1
        args << "-c:v" << "libsvtav1";
        args << "-preset" << av1Tab->av1PresetBox->currentText();

        QStringList svtParams;
        QString tune = av1Tab->av1TuneBox->currentText();
        if (tune != "Auto") {
            int tuneVal = 0;
            if (tune == "Subjective SSIM (VQ)") tuneVal = 0;
            else if (tune == "PSNR") tuneVal = 1;
            else if (tune == "SSIM") tuneVal = 2;
            else if (tune == "VMAF") tuneVal = 6;
            else if (tune == "VMAF Neg") tuneVal = 7;
            svtParams << "tune=" + QString::number(tuneVal);
        }
        if (av1Tab->nativeGrainCheck->isChecked()) {
            int strength = av1Tab->grainStrengthSlider->value();
            if (strength > 0) svtParams << "film-grain=" + QString::number(strength);
            int denoise = av1Tab->grainDenoiseCombo->currentIndex();
            svtParams << "film-grain-denoise=" + QString::number(denoise);
        }
        int superResMode = av1Tab->superResModeBox->currentIndex();
        if (superResMode > 0) {
            svtParams << "superres-mode=" + QString::number(superResMode);
            svtParams << "superres-denom=" + QString::number(av1Tab->superResDenomSlider->value());
        }
        int fdLevel = av1Tab->fastDecodeBox->currentIndex();
        if (fdLevel > 0) svtParams << "fast-decode=" + QString::number(fdLevel);
        if (av1Tab->lowLatencyCheck->isChecked()) {
            svtParams << "irefresh-type=1";
            svtParams << "lookahead=0";
        }
        if (av1Tab->tplModelCheck->isChecked()) svtParams << "enable-tpl-la=1";
        svtParams << "enable-cdef=" + QString(av1Tab->enableCdefCheck->isChecked() ? "1" : "0");
        if (av1Tab->av1LookaheadCheck->isChecked()) svtParams << "lookahead=" + QString::number(av1Tab->av1LookaheadSlider->value());
        QString aqModeStr = av1Tab->av1AQModeBox->currentText();
        if (aqModeStr != "Automatic") {
            int aqMode = (aqModeStr == "Disabled") ? 0 : (aqModeStr == "Variance") ? 1 : (aqModeStr == "Complexity") ? 2 : 0;
            svtParams << "aq-mode=" + QString::number(aqMode);
            if (aqModeStr == "Variance") {
                svtParams << "enable-variance-boost=1";
                svtParams << "variance-boost-strength=" + QString::number(av1Tab->av1AQStrengthSlider->value());
            }
        }
        if (av1Tab->av1EnableRCModeCheck->isChecked()) {
            QString mode = av1Tab->av1RCModeBox->currentText();
            if (mode == "QP") svtParams << "qp=" + QString::number(av1Tab->av1QPSlider->value());
            else if (mode == "CRF") svtParams << "crf=" + QString::number(av1Tab->av1CRFSlider->value());
            else if (mode == "VBR") {
                int br = av1Tab->av1VBRBitrateSlider->value();
                args << "-b:v" << QString::number(br) + "k";
                if (av1Tab->av1VBRVBVCheck->isChecked()) {
                    args << "-maxrate" << QString::number(br) + "k";
                    args << "-bufsize" << QString::number(2 * br) + "k";
                }
            }
        } else {
            svtParams << "crf=35";
        }
        if (av1Tab->enableTfCheck->isChecked()) svtParams << "enable-tf=1";
        svtParams << "scm=" + QString::number(av1Tab->screenContentModeBox->currentIndex());

        QString threads = av1Tab->av1ThreadsBox->currentText();
        if (threads != "Automatic") svtParams << "lp=" + threads;

        QString tileRows = av1Tab->av1TileRowsBox->currentText();
        if (tileRows != "Automatic") {
            int log2Rows = (tileRows == "1") ? 0 : (tileRows == "2") ? 1 : (tileRows == "4") ? 2 : 3;
            svtParams << "tile-rows=" + QString::number(log2Rows);
        }
        QString tileColumns = av1Tab->av1TileColumnsBox->currentText();
        if (tileColumns != "Automatic") {
            int log2Cols = (tileColumns == "1") ? 0 : (tileColumns == "2") ? 1 : (tileColumns == "4") ? 2 : 3;
            svtParams << "tile-columns=" + QString::number(log2Cols);
        }

        if (!svtParams.isEmpty()) {
            args << "-svtav1-params" << svtParams.join(":");
        }

        if (av1Tab->av1LevelBox->currentText() != "Auto") args << "-level" << av1Tab->av1LevelBox->currentText();
        args << "-g" << av1Tab->av1KeyIntBox->currentText();
    }
    else if (currentTab == 1) { // x265
        args << "-c:v" << "libx265";
        args << "-preset" << x265Tab->x265PresetBox->currentText();
        if (x265Tab->x265TuneBox->currentText() != "Auto") args << "-tune" << x265Tab->x265TuneBox->currentText();
        if (x265Tab->x265LevelBox->currentText() != "auto") args << "-level" << x265Tab->x265LevelBox->currentText();

        QStringList x265Params;
        x265Params << "deblock=" + QString::number(x265Tab->deblockAlphaSlider->value()) + ":" + QString::number(x265Tab->deblockBetaSlider->value());
        if (x265Tab->pmodeCheck->isChecked()) x265Params << "pmode=1";
        x265Params << "ref=" + x265Tab->refFramesBox->currentText();
        if (x265Tab->weightpCheck->isChecked()) x265Params << "weightp=1";
        if (x265Tab->strongIntraCheck->isChecked()) x265Params << "strong-intra-smoothing=1";
        if (x265Tab->rdoqLevelBox->currentIndex() > 1) x265Params << "rdoq-level=" + QString::number(x265Tab->rdoqLevelBox->currentIndex());
        if (x265Tab->saoCheck->isChecked()) x265Params << "sao=1";
        if (x265Tab->limitRefsBox->currentIndex() > 0) x265Params << "limit-refs=" + QString::number(x265Tab->limitRefsBox->currentIndex());
        if (x265Tab->x265LookaheadCheck->isChecked()) x265Params << "rc-lookahead=" + QString::number(x265Tab->x265LookaheadSlider->value());

        QString aqModeStr = x265Tab->x265AQModeBox->currentText();
        if (aqModeStr != "Automatic") {
            int aq = (aqModeStr == "Disabled") ? 0 : (aqModeStr == "Variance") ? 1 : (aqModeStr == "Auto-Variance") ? 2 : 3;
            args << "-aq-mode" << QString::number(aq);
        }
        x265Params << "aq-strength=" + QString::number(x265Tab->x265AQStrengthSlider->value() / 10.0);
        if (x265Tab->enablePsyRdCheck->isChecked()) x265Params << "psy-rd=1.0";
        if (x265Tab->enableCutreeCheck->isChecked()) x265Params << "cutree=1";

        if (!x265Params.isEmpty()) args << "-x265-params" << x265Params.join(":");
        args << "-g" << x265Tab->x265KeyIntBox->currentText();
        if (x265Tab->x265ThreadsBox->currentText() != "Automatic") args << "-threads" << x265Tab->x265ThreadsBox->currentText();
        if (x265Tab->x265FrameThreadsBox->currentText() != "Automatic") args << "-frame-threads" << x265Tab->x265FrameThreadsBox->currentText();

        if (x265Tab->x265EnableRCModeCheck->isChecked()) {
            QString mode = x265Tab->x265RCModeBox->currentText();
            if (mode == "QP") args << "-qp" << QString::number(x265Tab->x265QPSlider->value());
            else if (mode == "CRF") args << "-crf" << QString::number(x265Tab->x265CRFSlider->value());
            else if (mode == "ABR") {
                int br = x265Tab->x265ABRBitrateSlider->value();
                args << "-b:v" << QString::number(br) + "k";
                if (x265Tab->x265ABRVBVCheck->isChecked()) {
                    args << "-maxrate" << QString::number(br) + "k";
                    args << "-bufsize" << QString::number(2 * br) + "k";
                }
            } else if (mode == "CBR") {
                int br = x265Tab->x265CBRBitrateSlider->value();
                args << "-b:v" << QString::number(br) + "k";
                args << "-maxrate" << QString::number(br) + "k";
                args << "-bufsize" << QString::number(br) + "k";
            }
        } else {
            args << "-crf" << "23";
        }
    }
    else if (currentTab == 2) { // VP9
        args << "-c:v" << "libvpx-vp9";
        args << "-cpu-used" << vp9Tab->vp9CpuUsedBox->currentText();
        args << "-deadline" << vp9Tab->vp9DeadlineBox->currentText().toLower();
        if (vp9Tab->vp9LookaheadCheck->isChecked()) args << "-lag-in-frames" << QString::number(vp9Tab->vp9LookaheadSlider->value());

        QString aqModeStr = vp9Tab->vp9AQModeBox->currentText();
        if (aqModeStr != "Automatic") {
            int aq = (aqModeStr == "Disabled") ? 0 : (aqModeStr == "Variance") ? 1 : 2;
            args << "-aq-mode" << QString::number(aq);
        }
        if (vp9Tab->vp9ArnrCheck->isChecked()) {
            args << "-arnr-strength" << QString::number(vp9Tab->vp9ArnrStrengthSlider->value());
            args << "-arnr-maxframes" << QString::number(vp9Tab->vp9ArnrMaxFramesSlider->value());
        } else {
            args << "-arnr-strength" << QString::number(vp9Tab->vp9AQStrengthSlider->value());
        }
        if (vp9Tab->vp9TplCheck->isChecked()) args << "-auto-alt-ref" << "1";
        if (vp9Tab->enableRowMtCheck->isChecked()) args << "-row-mt" << "1";
        if (vp9Tab->screenContentCheck->isChecked()) args << "-tune-content" << "screen";

        args << "-g" << vp9Tab->vp9KeyIntBox->currentText();
        if (vp9Tab->vp9ThreadsBox->currentText() != "Automatic") args << "-threads" << vp9Tab->vp9ThreadsBox->currentText();

        QString tileColumns = vp9Tab->vp9TileColumnsBox->currentText();
        int log2Cols = (tileColumns == "0" || tileColumns == "1") ? 0 : (tileColumns == "2") ? 1 : (tileColumns == "4") ? 2 : 3;
        args << "-tile-columns" << QString::number(log2Cols);

        QString tileRows = vp9Tab->vp9TileRowsBox->currentText();
        if (tileRows != "Automatic") {
            int log2Rows = (tileRows == "1") ? 0 : (tileRows == "2") ? 1 : (tileRows == "4") ? 2 : 3;
            args << "-row-mt" << "1";
            args << "-tile-rows" << QString::number(log2Rows);
        }

        args << "-qmax" << QString::number(vp9Tab->vp9QMaxSlider->value());

        if (vp9Tab->vp9EnableRCModeCheck->isChecked()) {
            QString mode = vp9Tab->vp9RCModeBox->currentText();
            if (mode == "CRF") {
                args << "-crf" << QString::number(vp9Tab->vp9CRFSlider->value());
                args << "-b:v" << "0";
            } else {
                int br = vp9Tab->vp9BitrateSlider->value();
                args << "-b:v" << QString::number(br) + "k";
                if (mode == "CBR") {
                    args << "-maxrate" << QString::number(br) + "k";
                    args << "-bufsize" << QString::number(br) + "k";
                }
            }
        } else {
            args << "-crf" << "31";
            args << "-b:v" << "0";
        }
    }

    // Audio
    if (!isCombine && !isTrim && currentTab <= 2) {
        QCheckBox *audioCheck = nullptr;
        QComboBox *audioCodecBox = nullptr;
        QComboBox *audioSampleRateBox = nullptr;
        QComboBox *audioBitrateBox = nullptr;
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

        if (audioCheck && audioCheck->isChecked()) {
            QString audioCodecStr = audioCodecBox->currentText();
            QString encoder = (audioCodecStr == "opus") ? "libopus" :
            (audioCodecStr == "vorbis") ? "libvorbis" :
            (audioCodecStr == "mp3") ? "libmp3lame" :
            (audioCodecStr == "aac") ? "aac" :
            (audioCodecStr == "flac") ? "flac" : audioCodecStr;

            args << "-c:a" << encoder;
            auto getSampleRateInHz = [](const QString& sampleRateStr) -> QString {
                QString numericPart = sampleRateStr.split(" ").first();
                bool ok; double kHz = numericPart.toDouble(&ok);
                return ok ? QString::number(static_cast<int>(kHz * 1000)) : "48000";
            };
            args << "-ar" << getSampleRateInHz(audioSampleRateBox->currentText());

            auto getBitrateValue = [](const QString& bitrateStr) -> QString {
                QString numericPart = bitrateStr.split(" ").first();
                return numericPart.isEmpty() ? "128" : numericPart;
            };
            args << "-b:a" << getBitrateValue(audioBitrateBox->currentText()) + "k";

            if (audioCodecStr == "opus" && vbrModeBox) {
                QString vbr = vbrModeBox->currentText();
                if (vbr == "Constrained") args << "-vbr" << "constrained";
                else if (vbr == "Off") args << "-vbr" << "off";
            } else if (audioCodecStr == "aac" && aacQualityBox) {
                QString quality = aacQualityBox->currentText();
                if (quality != "Disabled") args << "-q:a" << quality;
            } else if (audioCodecStr == "mp3" && mp3VbrBox) {
                QString vbr = mp3VbrBox->currentText();
                if (vbr != "Disabled") args << "-qscale:a" << vbr;
            } else if (audioCodecStr == "flac" && flacCompressionBox) {
                args << "-compression_level" << flacCompressionBox->currentText();
            } else if (audioCodecStr == "vorbis" && vorbisQualityBox) {
                QString quality = vorbisQualityBox->currentText();
                if (quality != "Disabled") args << "-q:a" << quality;
            }
        } else {
            args << "-an";
        }
    }

    QString ffmpegPath = settings->value("ffmpegPath", "/usr/bin/ffmpeg").toString();
    if (ffmpegPath.isEmpty()) ffmpegPath = "/usr/bin/ffmpeg";

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString svtAv1Path = settings->value("svtAv1Path", "").toString();
    if (!svtAv1Path.isEmpty()) {
        QString existing = env.value("LD_LIBRARY_PATH");
        env.insert("LD_LIBRARY_PATH", existing.isEmpty() ? svtAv1Path : existing + ":" + svtAv1Path);
    }

    convertButton->setEnabled(false);
    cancelButton->setEnabled(true);
    conversionProgress->setVisible(true);
    conversionProgress->setValue(0);

    if (isCombine) {
        combineTab->startConcatenation();
        return;
    }

    if (isTrim) {
        logBox->clear();
        logBox->append("=== LOSSLESS TRIM STARTED (PURE STREAM COPY) ===");
        logBox->append("All filters, scaling, speed changes, and codec settings are ignored.");

        QString inputFileTrim = selectedFilesBox->text().trimmed();
        trimTab->stopPreviewPlayer();

        auto segList = trimTab->getSegments();
        if (segList.isEmpty()) {
            QMessageBox::warning(this, "Error", "Add at least one segment in the Trim tab.");
            convertButton->setEnabled(true);
            cancelButton->setEnabled(false);
            conversionProgress->setVisible(false);
            return;
        }

        bool individual = trimTab->isIndividualSegments();
        QString originalExt = QFileInfo(inputFileTrim).suffix().toLower();
        if (originalExt.isEmpty()) originalExt = "mkv";
        QString containerExt = "." + originalExt;

        QString outputDirTrim = outputDirBox->text();
        QString baseNameTrim = outputNameBox->text().isEmpty() ? "Output" : outputNameBox->text();

        QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) +
        "/ffmpeg_converter_trim_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
        QDir().mkpath(tempDir);

        QStringList tempFiles;
        for (int i = 0; i < segList.size(); ++i) {
            tempFiles << tempDir + QString("/seg_%1%2").arg(i + 1, 3, 10, QChar('0')).arg(containerExt);
        }

        QVector<double> segDurations;
        double totalDur = 0.0;
        for (const auto& seg : segList) {
            double d = (seg.second - seg.first) / 1000.0;
            segDurations << d;
            totalDur += d;
        }

        auto formatTimeLocal = [](qint64 ms) -> QString {
            if (ms <= 0) return "00:00:00.00";
            qint64 secs = ms / 1000;
            int h = secs / 3600;
            int m = (secs % 3600) / 60;
            int s = secs % 60;
            int cs = (ms % 1000) / 10;
            return QString("%1:%2:%3.%4")
            .arg(h, 2, 10, QChar('0'))
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0'))
            .arg(cs, 2, 10, QChar('0'));
        };

        struct TrimState {
            int currentSeg = 0;
            double completedDur = 0.0;
        };
        auto state = std::make_shared<TrimState>();

        std::function<void()> encodeNextSegment;
        encodeNextSegment = [this, state, segList, segDurations, tempFiles, tempDir, inputFileTrim, outputDirTrim, baseNameTrim, containerExt, individual, env, ffmpegPath, formatTimeLocal, encodeNextSegment]() mutable {
            if (state->currentSeg >= segList.size()) {
                if (individual) {
                    logBox->append("=== SAVING INDIVIDUAL SEGMENTS (NO CONCATENATION) ===");
                    QString firstSavedFile;
                    for (int i = 0; i < tempFiles.size(); ++i) {
                        QString src = tempFiles[i];
                        QString partName = baseNameTrim + "_part" + QString("%1").arg(i + 1, 3, 10, QChar('0')) + containerExt;
                        QString dest = QDir::cleanPath(outputDirTrim + "/" + partName);
                        if (QFile::exists(dest)) {
                            if (overwriteCheck->isChecked()) {
                                QFile::remove(dest);
                            } else {
                                int num = 1;
                                QString newPartName;
                                QString newDest;
                                do {
                                    newPartName = baseNameTrim + "_part" + QString("%1").arg(i + 1, 3, 10, QChar('0')) + " (" + QString::number(num) + ")";
                                    newDest = QDir::cleanPath(outputDirTrim + "/" + newPartName + containerExt);
                                    num++;
                                } while (QFile::exists(newDest));
                                dest = newDest;
                            }
                        }
                        QFile::copy(src, dest);
                        QString savedName = QFileInfo(dest).fileName();
                        logBox->append("Saved segment: " + savedName);
                        if (i == 0) firstSavedFile = dest;
                    }
                    if (!firstSavedFile.isEmpty()) showConversionNotification(firstSavedFile);
                    QDir(tempDir).removeRecursively();
                    trimTab->restartPreviewPlayer();
                    convertButton->setEnabled(true);
                    cancelButton->setEnabled(false);
                    conversionProgress->setVisible(false);
                } else {
                    QString finalFile = QDir::cleanPath(outputDirTrim + "/" + baseNameTrim + containerExt);
                    QString resolvedFinal = finalFile;
                    if (QFile::exists(resolvedFinal)) {
                        if (overwriteCheck->isChecked()) {
                            QFile::remove(resolvedFinal);
                        } else {
                            int counter = 1;
                            QFileInfo fi(resolvedFinal);
                            QString dir = fi.absolutePath();
                            QString base = fi.completeBaseName();
                            QString ext = fi.suffix().isEmpty() ? "" : "." + fi.suffix();
                            do {
                                resolvedFinal = QDir::cleanPath(dir + "/" + base + " (" + QString::number(counter) + ")" + ext);
                                counter++;
                            } while (QFile::exists(resolvedFinal));
                        }
                    }
                    QString listFile = tempDir + "/concat_list.txt";
                    QFile f(listFile);
                    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        QTextStream out(&f);
                        for (const QString& tf : tempFiles) {
                            out << "file '" << QDir::toNativeSeparators(tf) << "'\n";
                        }
                    }
                    QStringList concatArgs;
                    concatArgs << "-f" << "concat" << "-safe" << "0" << "-i" << listFile << "-c" << "copy" << resolvedFinal;

                    QProcess *concatProc = new QProcess();
                    concatProc->setProcessEnvironment(env);
                    concatProc->start(ffmpegPath, concatArgs);

                    connect(concatProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
                            [this, concatProc, resolvedFinal, tempDir, concatArgs](int, QProcess::ExitStatus) {
                                if (concatProc->exitCode() == 0) {
                                    logBox->append("✅ Trim complete: " + QFileInfo(resolvedFinal).fileName());
                                    showConversionNotification(resolvedFinal);
                                } else {
                                    logBox->append("❌ Concatenation failed.");
                                }
                                QDir(tempDir).removeRecursively();
                                trimTab->restartPreviewPlayer();
                                convertButton->setEnabled(true);
                                cancelButton->setEnabled(false);
                                conversionProgress->setVisible(false);
                                concatProc->deleteLater();
                            });
                }
                return;
            }

            const auto& seg = segList[state->currentSeg];
            double thisDur = segDurations[state->currentSeg];

            QStringList segArgs;
            segArgs << "-c" << "copy" << "-map" << "0" << "-avoid_negative_ts" << "make_zero";
            if (preserveMetadataCheck->isChecked()) segArgs << "-map_metadata" << "0";
            if (removeChaptersCheck->isChecked()) segArgs << "-map_chapters" << "-1";
            segArgs << "-ss" << QString::number(seg.first / 1000.0, 'f', 6);
            segArgs << "-t" << QString::number(thisDur, 'f', 6);

            logBox->append(QString("Encoding segment %1/%2 (%3 → %4)")
            .arg(state->currentSeg + 1).arg(segList.size())
            .arg(formatTimeLocal(seg.first)).arg(formatTimeLocal(seg.second)));

            state->currentSeg++;
            QString tempOut = tempFiles[state->currentSeg - 1];

            QString baseTemp = QFileInfo(tempOut).completeBaseName();
            converter->startConversion(inputFileTrim, QFileInfo(tempOut).absolutePath(), baseTemp, segArgs, false, containerExt, "copy", ffmpegPath, env, true, "", "", 1.0);
        };

        connect(converter, &Converter::progressUpdated, this, [this, state, totalDur, segDurations](int value) {
            if (state->currentSeg <= 0 || segDurations.isEmpty()) return;
            double segProgress = (value / 100.0) * segDurations[state->currentSeg - 1];
            int overall = static_cast<int>((state->completedDur + segProgress) / totalDur * 100.0);
            conversionProgress->setValue(overall);
        });

        connect(converter, &Converter::conversionFinished, this, [this, state, segDurations, encodeNextSegment]() mutable {
            if (state->currentSeg > 0) state->completedDur += segDurations[state->currentSeg - 1];
            QTimer::singleShot(6000, encodeNextSegment); // small delay for stability
        });

        state->currentSeg = 0;
        state->completedDur = 0.0;
        conversionProgress->setValue(0);
        encodeNextSegment();
        return;
    }

    converter->startConversion(inputFile, outputDir, baseName, args, twoPass, extension, codecStr, ffmpegPath, env, overwriteCheck->isChecked(), seekTimeStr, outputTimeStr, videoMultiplier);
}

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

    // ====================== BUILD INPUT + OUTPUT INFO ======================
    QString outputFile = normalOutput.isEmpty() ? combineOutput : normalOutput;
    QString inputInfo = infoBox->toHtml();   // preserve what was already shown for input

    if (outputFile.isEmpty()) {
        infoBox->setHtml("<h3>Input File</h3>" + inputInfo +
        "<h3>Output File</h3>No output file specified.");
        logBox->append("⚠️ No output file specified.");
        return;
    }

    // Run ffprobe on the output file in background (non-blocking)
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

            // === Full parsing (exact same logic as input analysis) ===
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

void MainWindow::showConversionNotification(const QString &outputFile)
{
    QSettings s("FFmpegConverter", "Settings");
    if (!s.value("notifyOnFinish", true).toBool()) return;

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
        QMessageBox::information(this, title, message);
    }
}
