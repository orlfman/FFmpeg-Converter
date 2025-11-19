#include "x265tab.h"
#include <QToolTip>
X265Tab::X265Tab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *x265Layout = new QVBoxLayout(this);
    // Container selection
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Container:");
        lbl->setToolTip("File type: mkv (general) or mp4 (compatible).");
        x265ContainerBox = new QComboBox();
        x265ContainerBox->addItems({"mkv", "mp4"});
        x265ContainerBox->setCurrentIndex(0);
        x265ContainerBox->setToolTip("mkv supports more features.");
        l->addWidget(lbl);
        l->addWidget(x265ContainerBox);
        l->addStretch();
        x265Layout->addLayout(l);
    }
    // Preset for balancing speed and quality
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Preset:");
        lbl->setToolTip("Speed vs quality.");
        x265PresetBox = new QComboBox();
        x265PresetBox->addItems({"ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow", "placebo"});
        x265PresetBox->setCurrentIndex(5);
        x265PresetBox->setToolTip("Slower = better quality, slower encode.");
        l->addWidget(lbl);
        l->addWidget(x265PresetBox);
        l->addStretch();
        x265Layout->addLayout(l);
    }
    // Rate control setup
    {
        QHBoxLayout *l = new QHBoxLayout();
        x265EnableRCModeCheck = new QCheckBox("Custom Rate Control");
        x265EnableRCModeCheck->setChecked(false);
        x265EnableRCModeCheck->setToolTip("Uncheck for default CRF.");
        l->addWidget(x265EnableRCModeCheck);
        x265RCModeBox = new QComboBox();
        x265RCModeBox->addItems({"QP", "CRF", "ABR", "CBR"});
        x265RCModeBox->setCurrentIndex(1);
        x265RCModeBox->setToolTip("QP: Fixed quant.\nCRF: Quality-based.\nABR: Avg bitrate.\nCBR: Constant bitrate.");
        l->addWidget(x265RCModeBox);
        l->addStretch();
        x265Layout->addLayout(l);
    }
    // QP settings, shows when QP selected
    x265QPConfigWidget = new QWidget();
    {
        QHBoxLayout *l = new QHBoxLayout(x265QPConfigWidget);
        QLabel *lbl = new QLabel("QP Value:");
        lbl->setToolTip("Lower = better quality. Range 0-51.");
        x265QPSlider = new QSlider(Qt::Horizontal);
        x265QPSlider->setMaximumWidth(400);
        x265QPSlider->setRange(0, 51);
        x265QPSlider->setValue(28);
        QLabel *val = new QLabel("28");
        l->addWidget(lbl);
        l->addWidget(x265QPSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x265QPSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        x265Layout->addWidget(x265QPConfigWidget);
        x265QPConfigWidget->setVisible(false);
    }
    // CRF settings
    x265CRFConfigWidget = new QWidget();
    {
        QHBoxLayout *l = new QHBoxLayout(x265CRFConfigWidget);
        QLabel *lbl = new QLabel("CRF Value:");
        lbl->setToolTip("Lower = better quality. Range 0-51.");
        x265CRFSlider = new QSlider(Qt::Horizontal);
        x265CRFSlider->setMaximumWidth(400);
        x265CRFSlider->setRange(0, 51);
        x265CRFSlider->setValue(23);
        QLabel *val = new QLabel("23");
        l->addWidget(lbl);
        l->addWidget(x265CRFSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x265CRFSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        x265Layout->addWidget(x265CRFConfigWidget);
        x265CRFConfigWidget->setVisible(false);
    }
    // ABR settings
    x265ABRConfigWidget = new QWidget();
    {
        QHBoxLayout *l = new QHBoxLayout(x265ABRConfigWidget);
        QLabel *lbl = new QLabel("ABR Bitrate (kbps):");
        lbl->setToolTip("Target avg bitrate. Range 100-10000.");
        x265ABRBitrateSlider = new QSlider(Qt::Horizontal);
        x265ABRBitrateSlider->setMaximumWidth(400);
        x265ABRBitrateSlider->setRange(100, 10000);
        x265ABRBitrateSlider->setValue(1000);
        x265ABRBitrateSlider->setSingleStep(100);
        QLabel *val = new QLabel("1000");
        x265ABRVBVCheck = new QCheckBox("Enable VBV");
        x265ABRVBVCheck->setToolTip("Constrains bitrate fluctuations.");
        l->addWidget(lbl);
        l->addWidget(x265ABRBitrateSlider);
        l->addWidget(val);
        l->addWidget(x265ABRVBVCheck);
        l->addStretch();
        QObject::connect(x265ABRBitrateSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        x265Layout->addWidget(x265ABRConfigWidget);
        x265ABRConfigWidget->setVisible(false);
    }
    // CBR settings
    x265CBRConfigWidget = new QWidget();
    {
        QHBoxLayout *l = new QHBoxLayout(x265CBRConfigWidget);
        QLabel *lbl = new QLabel("CBR Bitrate (kbps):");
        lbl->setToolTip("Constant bitrate. Range 100-10000.");
        x265CBRBitrateSlider = new QSlider(Qt::Horizontal);
        x265CBRBitrateSlider->setMaximumWidth(400);
        x265CBRBitrateSlider->setRange(100, 10000);
        x265CBRBitrateSlider->setValue(1000);
        x265CBRBitrateSlider->setSingleStep(100);
        QLabel *val = new QLabel("1000");
        l->addWidget(lbl);
        l->addWidget(x265CBRBitrateSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x265CBRBitrateSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        x265Layout->addWidget(x265CBRConfigWidget);
        x265CBRConfigWidget->setVisible(false);
    }
    // Tune for content type
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("x265 Tune:");
        lbl->setToolTip("Optimizes for specific content.");
        x265TuneBox = new QComboBox();
        x265TuneBox->addItems({"Auto", "psnr", "ssim", "grain", "zerolatency", "fastdecode", "animation"});
        x265TuneBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(x265TuneBox);
        l->addStretch();
        x265Layout->addLayout(l);
    }
    // Encoding level
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Encoding Level:");
        lbl->setToolTip("For compatibility/resolution.");
        x265LevelBox = new QComboBox();
        x265LevelBox->addItems({"auto", "1", "2", "2.1", "3", "3.1", "4", "4.1", "5", "5.1", "5.2", "6", "6.1", "6.2"});
        x265LevelBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(x265LevelBox);
        l->addStretch();
        x265Layout->addLayout(l);
    }
    // Advanced options group
    QGroupBox *advancedGroup = new QGroupBox("Advanced Options");
    QVBoxLayout *advancedLayout = new QVBoxLayout(advancedGroup);
    x265Layout->addWidget(advancedGroup);
    // Strong intra smoothing
    {
        QHBoxLayout *l = new QHBoxLayout();
        strongIntraCheck = new QCheckBox("Strong Intra Smoothing");
        strongIntraCheck->setChecked(true);
        strongIntraCheck->setToolTip("Improves dark/complex scenes.");
        l->addWidget(strongIntraCheck);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // RDOQ level
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("RDOQ Level:");
        lbl->setToolTip("Rate-Distortion Quantization.");
        rdoqLevelBox = new QComboBox();
        rdoqLevelBox->addItems({"0 (Fastest)", "1 (Default)", "2 (Best)"});
        rdoqLevelBox->setCurrentIndex(1);
        l->addWidget(lbl);
        l->addWidget(rdoqLevelBox);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // SAO filter
    {
        QHBoxLayout *l = new QHBoxLayout();
        saoCheck = new QCheckBox("SAO Filtering");
        saoCheck->setChecked(true);
        saoCheck->setToolTip("Reduces artifacts.");
        l->addWidget(saoCheck);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // Limit references
    QHBoxLayout *limitRefsLayout = new QHBoxLayout();
    QLabel *limitRefsLabel = new QLabel("Limit Refs:");
    limitRefsLabel->setToolTip("Limits reference frames for faster encoding.");
    limitRefsBox = new QComboBox();
    limitRefsBox->addItems({"0 (Full)", "1 (Medium)", "2 (Fast)", "3 (Fastest)"});
    limitRefsBox->setCurrentIndex(0);
    limitRefsLayout->addWidget(limitRefsLabel);
    limitRefsLayout->addWidget(limitRefsBox);
    limitRefsLayout->addStretch();
    advancedLayout->addLayout(limitRefsLayout);
    // Psy-RD
    {
        QHBoxLayout *l = new QHBoxLayout();
        enablePsyRdCheck = new QCheckBox("Enable Psy-RD");
        enablePsyRdCheck->setToolTip("Psychovisual optimizations for better subjective quality.");
        l->addWidget(enablePsyRdCheck);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // Cutree
    {
        QHBoxLayout *l = new QHBoxLayout();
        enableCutreeCheck = new QCheckBox("Enable Cutree");
        enableCutreeCheck->setToolTip("Complexity-based rate control for better efficiency.");
        l->addWidget(enableCutreeCheck);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // Lookahead frames
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        x265LookaheadCheck = new QCheckBox("Lookahead Frames");
        x265LookaheadCheck->setToolTip("Higher = better quality, slower. Range 0-250.");
        x265LookaheadSlider = new QSlider(Qt::Horizontal);
        x265LookaheadSlider->setMaximumWidth(300);
        x265LookaheadSlider->setRange(0, 250);
        x265LookaheadSlider->setValue(40);
        x265LookaheadSlider->setEnabled(false);
        QLabel *val = new QLabel("40");
        l->addWidget(x265LookaheadCheck);
        l->addWidget(x265LookaheadSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x265LookaheadSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        x265Layout->addWidget(w);
    }
    // Video filters group
    QGroupBox *filtersGroup = new QGroupBox("Video Filters");
    QVBoxLayout *filtersLayout = new QVBoxLayout(filtersGroup);
    x265Layout->addWidget(filtersGroup);
    // Unsharpen
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        x265UnsharpenCheck = new QCheckBox("Unsharpen");
        x265UnsharpenCheck->setToolTip("Reduces sharpness.");
        x265UnsharpenStrengthSlider = new QSlider(Qt::Horizontal);
        x265UnsharpenStrengthSlider->setMaximumWidth(300);
        x265UnsharpenStrengthSlider->setRange(0, 5);
        x265UnsharpenStrengthSlider->setValue(1);
        x265UnsharpenStrengthSlider->setEnabled(false);
        QLabel *val = new QLabel("1");
        l->addWidget(x265UnsharpenCheck);
        l->addWidget(x265UnsharpenStrengthSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x265UnsharpenStrengthSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        filtersLayout->addWidget(w);
    }
    // Sharpen
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        x265SharpenCheck = new QCheckBox("Sharpen");
        x265SharpenCheck->setToolTip("Enhances edges.");
        x265SharpenStrengthSlider = new QSlider(Qt::Horizontal);
        x265SharpenStrengthSlider->setMaximumWidth(300);
        x265SharpenStrengthSlider->setRange(0, 5);
        x265SharpenStrengthSlider->setValue(1);
        x265SharpenStrengthSlider->setEnabled(false);
        QLabel *val = new QLabel("1");
        l->addWidget(x265SharpenCheck);
        l->addWidget(x265SharpenStrengthSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x265SharpenStrengthSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        filtersLayout->addWidget(w);
    }
    // Blur
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        x265BlurCheck = new QCheckBox("Blur");
        x265BlurCheck->setToolTip("Softens image.");
        x265BlurStrengthSlider = new QSlider(Qt::Horizontal);
        x265BlurStrengthSlider->setMaximumWidth(300);
        x265BlurStrengthSlider->setRange(0, 5);
        x265BlurStrengthSlider->setValue(1);
        x265BlurStrengthSlider->setEnabled(false);
        QLabel *val = new QLabel("1");
        l->addWidget(x265BlurCheck);
        l->addWidget(x265BlurStrengthSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x265BlurStrengthSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        filtersLayout->addWidget(w);
    }
    // Noise reduction
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        x265NoiseReductionCheck = new QCheckBox("Noise Reduction");
        x265NoiseReductionCheck->setToolTip("Reduces noise.");
        x265NoiseReductionSlider = new QSlider(Qt::Horizontal);
        x265NoiseReductionSlider->setMaximumWidth(300);
        x265NoiseReductionSlider->setRange(0, 10);
        x265NoiseReductionSlider->setValue(5);
        x265NoiseReductionSlider->setEnabled(false);
        QLabel *val = new QLabel("5");
        l->addWidget(x265NoiseReductionCheck);
        l->addWidget(x265NoiseReductionSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x265NoiseReductionSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        filtersLayout->addWidget(w);
    }
    // Grain options
    QGroupBox *grainGroup = new QGroupBox("Grain Options");
    QVBoxLayout *grainLayout = new QVBoxLayout(grainGroup);
    x265Layout->addWidget(grainGroup);
    // Grain synthesis
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        x265GrainSynthCheck = new QCheckBox("Enable Grain Synthesis");
        x265GrainSynthCheck->setToolTip("Adds film grain.");
        x265GrainSynthLevel = new QSlider(Qt::Horizontal);
        x265GrainSynthLevel->setMaximumWidth(200);
        x265GrainSynthLevel->setRange(0, 50);
        x265GrainSynthLevel->setValue(0);
        x265GrainSynthLevel->setEnabled(false);
        QLabel *val = new QLabel("0");
        l->addWidget(x265GrainSynthCheck);
        l->addWidget(x265GrainSynthLevel);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x265GrainSynthLevel, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        grainLayout->addWidget(w);
    }
    // AQ mode
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("AQ Mode:");
        lbl->setToolTip("Adaptive Quantization.");
        x265AQModeBox = new QComboBox();
        x265AQModeBox->addItems({"Automatic", "Disabled", "Variance", "Auto-Variance", "Auto-Variance Biased"});
        x265AQModeBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(x265AQModeBox);
        l->addStretch();
        x265Layout->addLayout(l);
    }
    // AQ strength
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        QLabel *lbl = new QLabel("AQ Strength:");
        lbl->setToolTip("Higher = more adaptive. Range 0-3.");
        x265AQStrengthSlider = new QSlider(Qt::Horizontal);
        x265AQStrengthSlider->setMaximumWidth(300);
        x265AQStrengthSlider->setRange(0, 3);
        x265AQStrengthSlider->setValue(1);
        x265AQStrengthSlider->setEnabled(false);
        QLabel *val = new QLabel("1");
        l->addWidget(lbl);
        l->addWidget(x265AQStrengthSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x265AQStrengthSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        x265Layout->addWidget(w);
    }
    // Two-pass
    {
        QHBoxLayout *l = new QHBoxLayout();
        x265TwoPassCheck = new QCheckBox("Enable Two-Pass Encoding");
        x265TwoPassCheck->setToolTip("Better quality, slower.");
        l->addWidget(x265TwoPassCheck);
        l->addStretch();
        x265Layout->addLayout(l);
    }
    // Keyframe interval
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Key Frame Interval:");
        lbl->setToolTip("Higher = smaller file.");
        x265KeyIntBox = new QComboBox();
        x265KeyIntBox->addItems({"15", "30", "60", "120", "240", "360", "480", "720", "960", "1440", "1920"});
        x265KeyIntBox->setCurrentIndex(4);
        l->addWidget(lbl);
        l->addWidget(x265KeyIntBox);
        l->addStretch();
        x265Layout->addLayout(l);
    }
    // Threads
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Threads:");
        lbl->setToolTip("CPU threads. Auto detects.");
        x265ThreadsBox = new QComboBox();
        x265ThreadsBox->addItems({"Automatic", "1", "2", "4", "8", "12", "16"});
        x265ThreadsBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(x265ThreadsBox);
        l->addStretch();
        x265Layout->addLayout(l);
    }
    // Frame threads
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Frame Threads:");
        lbl->setToolTip("Parallel frame encoding.");
        x265FrameThreadsBox = new QComboBox();
        x265FrameThreadsBox->addItems({"Automatic", "1", "2", "3", "4"});
        x265FrameThreadsBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(x265FrameThreadsBox);
        l->addStretch();
        x265Layout->addLayout(l);
    }
    // Audio settings
    {
        QHBoxLayout *l = new QHBoxLayout();
        x265AudioCheck = new QCheckBox("Include Audio");
        x265AudioCheck->setChecked(true);
        x265AudioCheck->setToolTip("Uncheck to remove audio.");
        l->addWidget(x265AudioCheck);
        QLabel *codecLbl = new QLabel("Audio Codec:");
        codecLbl->setToolTip("Audio format.");
        x265AudioCodecBox = new QComboBox();
        x265AudioCodecBox->addItems({"aac", "opus", "mp3", "flac", "vorbis"});
        x265AudioCodecBox->setCurrentIndex(0);
        l->addWidget(codecLbl);
        l->addWidget(x265AudioCodecBox);
        QLabel *sampleLbl = new QLabel("Sample Rate:");
        sampleLbl->setToolTip("48 kHz standard.");
        x265AudioSampleRateBox = new QComboBox();
        x265AudioSampleRateBox->addItems({"8 kHz", "12 kHz", "16 kHz", "24 kHz", "48 kHz"});
        x265AudioSampleRateBox->setCurrentIndex(4);
        l->addWidget(sampleLbl);
        l->addWidget(x265AudioSampleRateBox);
        QLabel *bitrateLbl = new QLabel("Bitrate:");
        bitrateLbl->setToolTip("Higher = better audio.");
        x265AudioBitrateBox = new QComboBox();
        x265AudioBitrateBox->addItems({"64 kbps", "128 kbps", "192 kbps", "256 kbps", "320 kbps", "384 kbps", "448 kbps", "512 kbps"});
        x265AudioBitrateBox->setCurrentIndex(1);
        l->addWidget(bitrateLbl);
        l->addWidget(x265AudioBitrateBox);
        x265VbrModeLabel = new QLabel("VBR Mode:");
        x265VbrModeBox = new QComboBox();
        x265VbrModeBox->addItems({"Default", "Constrained", "Off"});
        x265VbrModeBox->setCurrentIndex(0);
        x265VbrModeBox->setToolTip("For Opus.");
        l->addWidget(x265VbrModeLabel);
        l->addWidget(x265VbrModeBox);
        x265AacQualityLabel = new QLabel("Quality Level:");
        x265AacQualityBox = new QComboBox();
        x265AacQualityBox->addItems({"Disabled", "0.1", "0.5", "1"});
        x265AacQualityBox->setCurrentIndex(0);
        x265AacQualityBox->setToolTip("For AAC: Higher = better.");
        l->addWidget(x265AacQualityLabel);
        l->addWidget(x265AacQualityBox);
        x265Mp3VbrLabel = new QLabel("VBR:");
        x265Mp3VbrBox = new QComboBox();
        x265Mp3VbrBox->addItems({"Disabled", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"});
        x265Mp3VbrBox->setCurrentIndex(0);
        x265Mp3VbrBox->setToolTip("For MP3: 0=best.");
        l->addWidget(x265Mp3VbrLabel);
        l->addWidget(x265Mp3VbrBox);
        x265FlacCompressionLabel = new QLabel("Compression Level:");
        x265FlacCompressionBox = new QComboBox();
        x265FlacCompressionBox->addItems({"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"});
        x265FlacCompressionBox->setCurrentIndex(5);
        x265FlacCompressionBox->setToolTip("For FLAC: Higher = smaller/slower.");
        l->addWidget(x265FlacCompressionLabel);
        l->addWidget(x265FlacCompressionBox);
        x265VorbisQualityLabel = new QLabel("Quality Level:");
        x265VorbisQualityBox = new QComboBox();
        x265VorbisQualityBox->addItems({"Disabled", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"});
        x265VorbisQualityBox->setCurrentIndex(0);
        x265VorbisQualityBox->setToolTip("For Vorbis: Higher = better.");
        l->addWidget(x265VorbisQualityLabel);
        l->addWidget(x265VorbisQualityBox);
        x265Layout->addLayout(l);
    }
    // Reset button
    {
        QHBoxLayout *l = new QHBoxLayout();
        QPushButton *resetButton = new QPushButton("Reset to Defaults");
        resetButton->setToolTip("Reset all x265 settings.");
        l->addStretch();
        l->addWidget(resetButton);
        x265Layout->addLayout(l);
        QObject::connect(resetButton, &QPushButton::clicked, this, &X265Tab::resetDefaults);
    }
    // Signals and slots
    x265RCModeBox->setEnabled(false);
    x265TwoPassCheck->setEnabled(false);
    x265TwoPassCheck->setChecked(false);
    // Toggling rate control widgets
    QObject::connect(x265EnableRCModeCheck, &QCheckBox::toggled, [this](bool on){
        x265RCModeBox->setEnabled(on);
        if (on) {
            QString m = x265RCModeBox->currentText();
            x265QPConfigWidget->setVisible(m == "QP");
            x265CRFConfigWidget->setVisible(m == "CRF");
            x265ABRConfigWidget->setVisible(m == "ABR");
            x265CBRConfigWidget->setVisible(m == "CBR");
            x265TwoPassCheck->setEnabled(m == "ABR" || m == "CBR");
        } else {
            x265QPConfigWidget->setVisible(false);
            x265CRFConfigWidget->setVisible(false);
            x265ABRConfigWidget->setVisible(false);
            x265CBRConfigWidget->setVisible(false);
            x265TwoPassCheck->setEnabled(false);
            x265TwoPassCheck->setChecked(false);
        }
    });
    QObject::connect(x265RCModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](){
        if (x265EnableRCModeCheck->isChecked()) {
            QString m = x265RCModeBox->currentText();
            x265QPConfigWidget->setVisible(m == "QP");
            x265CRFConfigWidget->setVisible(m == "CRF");
            x265ABRConfigWidget->setVisible(m == "ABR");
            x265CBRConfigWidget->setVisible(m == "CBR");
            x265TwoPassCheck->setEnabled(m == "ABR" || m == "CBR");
        }
    });
    // Enabling filter sliders
    QObject::connect(x265LookaheadCheck, &QCheckBox::toggled,
                     [this](bool on){ x265LookaheadSlider->setEnabled(on); });
    QObject::connect(x265UnsharpenCheck, &QCheckBox::toggled,
                     [this](bool on){ x265UnsharpenStrengthSlider->setEnabled(on); });
    QObject::connect(x265SharpenCheck, &QCheckBox::toggled,
                     [this](bool on){ x265SharpenStrengthSlider->setEnabled(on); });
    QObject::connect(x265BlurCheck, &QCheckBox::toggled,
                     [this](bool on){ x265BlurStrengthSlider->setEnabled(on); });
    QObject::connect(x265NoiseReductionCheck, &QCheckBox::toggled,
                     [this](bool on){ x265NoiseReductionSlider->setEnabled(on); });
    QObject::connect(x265GrainSynthCheck, &QCheckBox::toggled,
                     [this](bool on){ x265GrainSynthLevel->setEnabled(on); });
    // AQ strength enable
    QObject::connect(x265AQModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](){
        QString m = x265AQModeBox->currentText();
        x265AQStrengthSlider->setEnabled(m != "Automatic" && m != "Disabled");
    });
    // Showing codec-specific audio options
    QObject::connect(x265AudioCodecBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](){
        QString c = x265AudioCodecBox->currentText();
        x265VbrModeLabel->setVisible(c == "opus");
        x265VbrModeBox->setVisible(c == "opus");
        x265AacQualityLabel->setVisible(c == "aac");
        x265AacQualityBox->setVisible(c == "aac");
        x265Mp3VbrLabel->setVisible(c == "mp3");
        x265Mp3VbrBox->setVisible(c == "mp3");
        x265FlacCompressionLabel->setVisible(c == "flac");
        x265FlacCompressionBox->setVisible(c == "flac");
        x265VorbisQualityLabel->setVisible(c == "vorbis");
        x265VorbisQualityBox->setVisible(c == "vorbis");
    });
    // Updating for container change
    QObject::connect(x265ContainerBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &X265Tab::updateAudioCodecOptions);
    updateAudioCodecOptions();
}
void X265Tab::updateAudioCodecOptions() {
    QString cont = x265ContainerBox->currentText();
    QString cur = x265AudioCodecBox->currentText();
    x265AudioCodecBox->clear();
    if (cont == "mp4") {
        x265AudioCodecBox->addItems({"aac", "mp3", "opus"});
    } else {
        x265AudioCodecBox->addItems({"aac", "opus", "mp3", "flac", "vorbis"});
    }
    int idx = x265AudioCodecBox->findText(cur);
    x265AudioCodecBox->setCurrentIndex(idx != -1 ? idx : 0);
    emit x265AudioCodecBox->currentIndexChanged(x265AudioCodecBox->currentIndex());
}
void X265Tab::resetDefaults() {
    x265ContainerBox->setCurrentIndex(0);
    x265PresetBox->setCurrentIndex(5);
    x265EnableRCModeCheck->setChecked(false);
    x265RCModeBox->setCurrentIndex(1);
    x265QPConfigWidget->setVisible(false);
    x265CRFConfigWidget->setVisible(false);
    x265ABRConfigWidget->setVisible(false);
    x265CBRConfigWidget->setVisible(false);
    x265QPSlider->setValue(28);
    x265CRFSlider->setValue(23);
    x265ABRBitrateSlider->setValue(1000);
    x265ABRVBVCheck->setChecked(false);
    x265CBRBitrateSlider->setValue(1000);
    x265TuneBox->setCurrentIndex(0);
    x265LevelBox->setCurrentIndex(0);
    strongIntraCheck->setChecked(true);
    rdoqLevelBox->setCurrentIndex(1);
    saoCheck->setChecked(true);
    limitRefsBox->setCurrentIndex(0);
    x265LookaheadCheck->setChecked(false);
    x265LookaheadSlider->setValue(40);
    x265UnsharpenCheck->setChecked(false);
    x265UnsharpenStrengthSlider->setValue(1);
    x265SharpenCheck->setChecked(false);
    x265SharpenStrengthSlider->setValue(1);
    x265BlurCheck->setChecked(false);
    x265BlurStrengthSlider->setValue(1);
    x265NoiseReductionCheck->setChecked(false);
    x265NoiseReductionSlider->setValue(5);
    x265GrainSynthCheck->setChecked(false);
    x265GrainSynthLevel->setValue(0);
    x265TwoPassCheck->setChecked(false);
    x265AQModeBox->setCurrentIndex(0);
    x265AQStrengthSlider->setValue(1);
    x265KeyIntBox->setCurrentIndex(4);
    x265ThreadsBox->setCurrentIndex(0);
    x265FrameThreadsBox->setCurrentIndex(0);
    x265AudioCheck->setChecked(true);
    x265AudioCodecBox->setCurrentIndex(0);
    x265AudioSampleRateBox->setCurrentIndex(4);
    x265AudioBitrateBox->setCurrentIndex(1);
    x265VbrModeBox->setCurrentIndex(0);
    x265AacQualityBox->setCurrentIndex(0);
    x265Mp3VbrBox->setCurrentIndex(0);
    x265FlacCompressionBox->setCurrentIndex(5);
    x265VorbisQualityBox->setCurrentIndex(0);
    enablePsyRdCheck->setChecked(false);
    enableCutreeCheck->setChecked(false);
    // Refresh UI
    x265EnableRCModeCheck->toggled(false);
    x265AQModeBox->currentIndexChanged(0);
    updateAudioCodecOptions();
}
