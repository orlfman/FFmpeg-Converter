#include "MainWindow.h"
#include "SettingsDialog.h"
#include "av1tab.h"
#include "vp9tab.h"
#include "x265tab.h"
#include "x264tab.h"
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
#include "MainWindow_conversion.cpp"
#include "MainWindow_onConversionFinished.cpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    settings = new QSettings("FFmpegConverter", "Settings", this);
    converter = new Converter(this);

    setWindowTitle("FFmpeg Converter");
    setWindowIcon(QIcon::fromTheme("ffmpeg-converter-qt"));
    resize(1280, 720);

    setupUi();
    setAcceptDrops(true);
    wireAllSignals();
    updateRecentMenu();
    refreshOutputName();

    QTimer::singleShot(0, this, [this]() {
        loadSettings();
    });

    debounceTimer = new QTimer(this);
    debounceTimer->setSingleShot(true);
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

    // x264
    x264Tab = new X264Tab();
    QScrollArea *x264Scroll = new QScrollArea(); x264Scroll->setWidgetResizable(true); x264Scroll->setWidget(x264Tab);
    codecTabs->addTab(x264Scroll, "x264");

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
    QWidget *infoTab = new QWidget();
    QVBoxLayout *infoLayout = new QVBoxLayout(infoTab);
    infoBox = new QTextEdit(); infoBox->setReadOnly(true);
    infoLayout->addWidget(infoBox);
    codecTabs->addTab(infoTab, "Information");

    QWidget *consoleTab = new QWidget();
    QVBoxLayout *consoleLayout = new QVBoxLayout(consoleTab);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    clearLogButton = new QPushButton("Clear Console");
    buttonLayout->addWidget(clearLogButton);
    buttonLayout->addStretch();
    consoleLayout->addLayout(buttonLayout);

    logBox = new QTextEdit();
    logBox->setReadOnly(true);
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
    if (!codecTabs) return;

    int defaultTab = settings->value("defaultCodecTab", 0).toInt();

    if (defaultTab >= 0 && defaultTab < codecTabs->count()) {
        codecTabs->setCurrentIndex(defaultTab);
    } else {
        codecTabs->setCurrentIndex(0);
    }
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
        connect(rotationBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
            if (cropCheck->isChecked()) {
                detectCropButton->click();
            }
        });

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
        Presets::connectPresets(presetCombo, codecTabs, av1Tab, x265Tab, x264Tab, vp9Tab, eightBitCheck, eightBitColorFormatBox, tenBitCheck, colorFormatBox);

        // Force custom preset
        connect(codecTabs, &QTabWidget::currentChanged, this, [this](int) { presetCombo->setCurrentIndex(0); });

        // Trim
        connect(selectedFilesBox, &QLineEdit::textChanged, trimTab, &TrimTab::setInputFile);

        // Console
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
                 static_cast<QWidget*>(x264Tab),
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
        int newDefaultTab = dialog.getDefaultCodecTab();
        settings->setValue("defaultOutputDir", dialog.getDefaultOutputDir());
        settings->setValue("defaultCodecTab", newDefaultTab);
        settings->setValue("ffmpegPath", dialog.getFFmpegPath());
        settings->setValue("notifyOnFinish", dialog.getNotifyOnFinish());
        settings->setValue("svtAv1Path", dialog.getSvtAv1Path());

        settings->sync();

        QMessageBox::information(this, "✅ Settings Saved",
                                 "✓ Default codec tab\n✓ FFmpeg path\n✓ SVT-AV1 library path\n✓ Output directory\n✓ Notifications");
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

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        if (!urlList.isEmpty()) {
            QString filePath = urlList.first().toLocalFile();
            if (!filePath.isEmpty() && QFile::exists(filePath)) {
                selectedFilesBox->setText(filePath);
                onFileSelected(filePath);
                logBox->append("📥 File dropped: " + QFileInfo(filePath).fileName());
            }
        }
    }
    event->acceptProposedAction();
}
