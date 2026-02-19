#include "x264tab.h"
#include <QToolTip>
#include <QTimer>

X264Tab::X264Tab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *x264Layout = new QVBoxLayout(this);
    // Container selection
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Container:");
        lbl->setToolTip("File type: mkv (general) or mp4 (compatible).");
        x264ContainerBox = new QComboBox();
        x264ContainerBox->addItems({"mkv", "mp4"});
        x264ContainerBox->setCurrentIndex(0);
        x264ContainerBox->setToolTip("mkv supports more features.");
        l->addWidget(lbl);
        l->addWidget(x264ContainerBox);
        l->addStretch();
        x264Layout->addLayout(l);
    }
    // Preset for balancing speed and quality
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Preset:");
        lbl->setToolTip("Speed vs quality.");
        x264PresetBox = new QComboBox();
        x264PresetBox->addItems({"ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow", "placebo"});
        x264PresetBox->setCurrentIndex(5);
        x264PresetBox->setToolTip("Slower = better quality, slower encode.");
        l->addWidget(lbl);
        l->addWidget(x264PresetBox);
        l->addStretch();
        x264Layout->addLayout(l);
    }
    // Rate control setup
    {
        QHBoxLayout *l = new QHBoxLayout();
        x264EnableRCModeCheck = new QCheckBox("Custom Rate Control");
        x264EnableRCModeCheck->setChecked(false);
        x264EnableRCModeCheck->setToolTip("Uncheck for default CRF.");
        l->addWidget(x264EnableRCModeCheck);
        x264RCModeBox = new QComboBox();
        x264RCModeBox->addItems({"QP", "CRF", "ABR", "CBR"});
        x264RCModeBox->setCurrentIndex(1);
        x264RCModeBox->setToolTip("QP: Fixed quant.\nCRF: Quality-based.\nABR: Avg bitrate.\nCBR: Constant bitrate.");
        l->addWidget(x264RCModeBox);
        l->addStretch();
        x264Layout->addLayout(l);
    }
    // QP settings
    x264QPConfigWidget = new QWidget();
    {
        QHBoxLayout *l = new QHBoxLayout(x264QPConfigWidget);
        QLabel *lbl = new QLabel("QP Value:");
        lbl->setToolTip("Lower = better quality. Range 0-51.");
        x264QPSlider = new QSlider(Qt::Horizontal);
        x264QPSlider->setMaximumWidth(400);
        x264QPSlider->setRange(0, 51);
        x264QPSlider->setValue(23);
        QLabel *val = new QLabel("23");
        l->addWidget(lbl);
        l->addWidget(x264QPSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x264QPSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        x264Layout->addWidget(x264QPConfigWidget);
        x264QPConfigWidget->setVisible(false);
    }
    // CRF settings
    x264CRFConfigWidget = new QWidget();
    {
        QHBoxLayout *l = new QHBoxLayout(x264CRFConfigWidget);
        QLabel *lbl = new QLabel("CRF Value:");
        lbl->setToolTip("Lower = better quality. Range 0-51.");
        x264CRFSlider = new QSlider(Qt::Horizontal);
        x264CRFSlider->setMaximumWidth(400);
        x264CRFSlider->setRange(0, 51);
        x264CRFSlider->setValue(23);
        QLabel *val = new QLabel("23");
        l->addWidget(lbl);
        l->addWidget(x264CRFSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x264CRFSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        x264Layout->addWidget(x264CRFConfigWidget);
        x264CRFConfigWidget->setVisible(false);
    }
    // ABR settings
    x264ABRConfigWidget = new QWidget();
    {
        QHBoxLayout *l = new QHBoxLayout(x264ABRConfigWidget);
        QLabel *lbl = new QLabel("ABR Bitrate (kbps):");
        lbl->setToolTip("Target avg bitrate. Range 100-10000.");
        x264ABRBitrateSlider = new QSlider(Qt::Horizontal);
        x264ABRBitrateSlider->setMaximumWidth(400);
        x264ABRBitrateSlider->setRange(100, 10000);
        x264ABRBitrateSlider->setValue(1000);
        x264ABRBitrateSlider->setSingleStep(100);
        QLabel *val = new QLabel("1000");
        x264ABRVBVCheck = new QCheckBox("Enable VBV");
        x264ABRVBVCheck->setToolTip("Constrains bitrate fluctuations.");
        l->addWidget(lbl);
        l->addWidget(x264ABRBitrateSlider);
        l->addWidget(val);
        l->addWidget(x264ABRVBVCheck);
        l->addStretch();
        QObject::connect(x264ABRBitrateSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        x264Layout->addWidget(x264ABRConfigWidget);
        x264ABRConfigWidget->setVisible(false);
    }
    // CBR settings
    x264CBRConfigWidget = new QWidget();
    {
        QHBoxLayout *l = new QHBoxLayout(x264CBRConfigWidget);
        QLabel *lbl = new QLabel("CBR Bitrate (kbps):");
        lbl->setToolTip("Constant bitrate. Range 100-10000.");
        x264CBRBitrateSlider = new QSlider(Qt::Horizontal);
        x264CBRBitrateSlider->setMaximumWidth(400);
        x264CBRBitrateSlider->setRange(100, 10000);
        x264CBRBitrateSlider->setValue(1000);
        x264CBRBitrateSlider->setSingleStep(100);
        QLabel *val = new QLabel("1000");
        l->addWidget(lbl);
        l->addWidget(x264CBRBitrateSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x264CBRBitrateSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        x264Layout->addWidget(x264CBRConfigWidget);
        x264CBRConfigWidget->setVisible(false);
    }
    // Tune for content type
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("x264 Tune:");
        lbl->setToolTip("Optimizes for specific content.");
        x264TuneBox = new QComboBox();
        x264TuneBox->addItems({"Auto", "film", "animation", "grain", "stillimage", "psnr", "ssim", "fastdecode", "zerolatency"});
        x264TuneBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(x264TuneBox);
        l->addStretch();
        x264Layout->addLayout(l);
    }
    // Profiles
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("x264 Profile:");
        lbl->setToolTip("auto / baseline / main / high10");
        x264ProfileBox = new QComboBox();
        x264ProfileBox->addItems({"auto", "baseline", "main", "high10"});
        x264ProfileBox->setCurrentIndex(2);
        l->addWidget(lbl);
        l->addWidget(x264ProfileBox);
        l->addStretch();
        x264Layout->addLayout(l);
    }
    // Encoding level
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Encoding Level:");
        lbl->setToolTip("For compatibility/resolution.");
        x264LevelBox = new QComboBox();
        x264LevelBox->addItems({"auto", "1", "2", "2.1", "3", "3.1", "4", "4.1", "5", "5.1", "5.2", "6", "6.1", "6.2"});
        x264LevelBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(x264LevelBox);
        l->addStretch();
        x264Layout->addLayout(l);
    }
    // Advanced options group (exact same as x265)
    QGroupBox *advancedGroup = new QGroupBox("Advanced Options");
    QVBoxLayout *advancedLayout = new QVBoxLayout(advancedGroup);
    x264Layout->addWidget(advancedGroup);
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
    // Deblock sliders (exact same as x265)
    deblockGroup = new QGroupBox("Deblock Filter");
    QVBoxLayout *deblockLayout = new QVBoxLayout(deblockGroup);
    QHBoxLayout *alphaLayout = new QHBoxLayout();
    deblockAlphaLabel = new QLabel("Alpha:");
    deblockAlphaLabel->setToolTip("Luma deblock strength (-6 to 0).");
    deblockAlphaSlider = new QSlider(Qt::Horizontal);
    deblockAlphaSlider->setRange(-6, 0);
    deblockAlphaSlider->setValue(-2);
    QLabel *alphaVal = new QLabel("-2");
    alphaLayout->addWidget(deblockAlphaLabel);
    alphaLayout->addWidget(deblockAlphaSlider);
    alphaLayout->addWidget(alphaVal);
    alphaLayout->addStretch();
    QObject::connect(deblockAlphaSlider, &QSlider::valueChanged, [alphaVal](int v){ alphaVal->setText(QString::number(v)); });
    deblockLayout->addLayout(alphaLayout);
    QHBoxLayout *betaLayout = new QHBoxLayout();
    deblockBetaLabel = new QLabel("Beta:");
    deblockBetaLabel->setToolTip("Chroma deblock strength (-6 to 0).");
    deblockBetaSlider = new QSlider(Qt::Horizontal);
    deblockBetaSlider->setRange(-6, 0);
    deblockBetaSlider->setValue(-2);
    QLabel *betaVal = new QLabel("-2");
    betaLayout->addWidget(deblockBetaLabel);
    betaLayout->addWidget(deblockBetaSlider);
    betaLayout->addWidget(betaVal);
    betaLayout->addStretch();
    QObject::connect(deblockBetaSlider, &QSlider::valueChanged, [betaVal](int v){ betaVal->setText(QString::number(v)); });
    deblockLayout->addLayout(betaLayout);
    advancedLayout->addWidget(deblockGroup);
    // Ref Frames
    QHBoxLayout *refLayout = new QHBoxLayout();
    QLabel *refLabel = new QLabel("Ref Frames:");
    refLabel->setToolTip("More refs = better compression (slower).");
    refFramesBox = new QComboBox();
    refFramesBox->addItems({"1", "2", "3", "4", "5"});
    refFramesBox->setCurrentIndex(3);
    refLayout->addWidget(refLabel);
    refLayout->addWidget(refFramesBox);
    refLayout->addStretch();
    advancedLayout->addLayout(refLayout);
    // WeightP
    QHBoxLayout *weightpLayout = new QHBoxLayout();
    weightpCheck = new QCheckBox("Weighted Prediction (WeightP)");
    weightpCheck->setToolTip("Improves P-frame efficiency ~10%.");
    weightpCheck->setChecked(true);
    weightpLayout->addWidget(weightpCheck);
    weightpLayout->addStretch();
    advancedLayout->addLayout(weightpLayout);
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
        x264LookaheadCheck = new QCheckBox("Lookahead Frames");
        x264LookaheadCheck->setToolTip("Higher = better quality, slower. Range 0-250.");
        x264LookaheadSlider = new QSlider(Qt::Horizontal);
        x264LookaheadSlider->setMaximumWidth(300);
        x264LookaheadSlider->setRange(0, 250);
        x264LookaheadSlider->setValue(40);
        x264LookaheadSlider->setEnabled(false);
        QLabel *val = new QLabel("40");
        l->addWidget(x264LookaheadCheck);
        l->addWidget(x264LookaheadSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x264LookaheadSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        x264Layout->addWidget(w);
    }
    // Video filters group (exact same as x265)
    QGroupBox *filtersGroup = new QGroupBox("Video Filters");
    QVBoxLayout *filtersLayout = new QVBoxLayout(filtersGroup);
    x264Layout->addWidget(filtersGroup);
    // Unsharpen, Sharpen, Blur, Noise Reduction (exact same as x265)
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        x264UnsharpenCheck = new QCheckBox("Unsharpen");
        x264UnsharpenCheck->setToolTip("Reduces sharpness.");
        x264UnsharpenStrengthSlider = new QSlider(Qt::Horizontal);
        x264UnsharpenStrengthSlider->setMaximumWidth(300);
        x264UnsharpenStrengthSlider->setRange(0, 5);
        x264UnsharpenStrengthSlider->setValue(1);
        x264UnsharpenStrengthSlider->setEnabled(false);
        QLabel *val = new QLabel("1");
        l->addWidget(x264UnsharpenCheck);
        l->addWidget(x264UnsharpenStrengthSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x264UnsharpenStrengthSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        filtersLayout->addWidget(w);
    }
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        x264SharpenCheck = new QCheckBox("Sharpen");
        x264SharpenCheck->setToolTip("Enhances edges.");
        x264SharpenStrengthSlider = new QSlider(Qt::Horizontal);
        x264SharpenStrengthSlider->setMaximumWidth(300);
        x264SharpenStrengthSlider->setRange(0, 5);
        x264SharpenStrengthSlider->setValue(1);
        x264SharpenStrengthSlider->setEnabled(false);
        QLabel *val = new QLabel("1");
        l->addWidget(x264SharpenCheck);
        l->addWidget(x264SharpenStrengthSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x264SharpenStrengthSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        filtersLayout->addWidget(w);
    }
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        x264BlurCheck = new QCheckBox("Blur");
        x264BlurCheck->setToolTip("Softens image.");
        x264BlurStrengthSlider = new QSlider(Qt::Horizontal);
        x264BlurStrengthSlider->setMaximumWidth(300);
        x264BlurStrengthSlider->setRange(0, 5);
        x264BlurStrengthSlider->setValue(1);
        x264BlurStrengthSlider->setEnabled(false);
        QLabel *val = new QLabel("1");
        l->addWidget(x264BlurCheck);
        l->addWidget(x264BlurStrengthSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x264BlurStrengthSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        filtersLayout->addWidget(w);
    }
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        x264NoiseReductionCheck = new QCheckBox("Noise Reduction");
        x264NoiseReductionCheck->setToolTip("Reduces noise.");
        x264NoiseReductionSlider = new QSlider(Qt::Horizontal);
        x264NoiseReductionSlider->setMaximumWidth(300);
        x264NoiseReductionSlider->setRange(0, 10);
        x264NoiseReductionSlider->setValue(5);
        x264NoiseReductionSlider->setEnabled(false);
        QLabel *val = new QLabel("5");
        l->addWidget(x264NoiseReductionCheck);
        l->addWidget(x264NoiseReductionSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x264NoiseReductionSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        filtersLayout->addWidget(w);
    }
    // Grain options
    QGroupBox *grainGroup = new QGroupBox("Grain Options");
    QVBoxLayout *grainLayout = new QVBoxLayout(grainGroup);
    x264Layout->addWidget(grainGroup);
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        x264GrainSynthCheck = new QCheckBox("Enable Grain Synthesis");
        x264GrainSynthCheck->setToolTip("Adds film grain.");
        x264GrainSynthLevel = new QSlider(Qt::Horizontal);
        x264GrainSynthLevel->setMaximumWidth(200);
        x264GrainSynthLevel->setRange(0, 50);
        x264GrainSynthLevel->setValue(0);
        x264GrainSynthLevel->setEnabled(false);
        QLabel *val = new QLabel("0");
        l->addWidget(x264GrainSynthCheck);
        l->addWidget(x264GrainSynthLevel);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(x264GrainSynthLevel, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        grainLayout->addWidget(w);
    }
    // AQ mode
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("AQ Mode:");
        lbl->setToolTip("Adaptive Quantization.");
        x264AQModeBox = new QComboBox();
        x264AQModeBox->addItems({"Automatic", "Disabled", "Variance", "Auto-Variance", "Auto-Variance Biased"});
        x264AQModeBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(x264AQModeBox);
        l->addStretch();
        x264Layout->addLayout(l);
    }
    // AQ strength
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);

        QLabel *lbl = new QLabel("AQ Strength:");
        lbl->setToolTip("Higher = more adaptive.\nRange 0.0–3.0 (0.1 steps)");

        x264AQStrengthSlider = new QSlider(Qt::Horizontal);
        x264AQStrengthSlider->setMaximumWidth(300);
        x264AQStrengthSlider->setRange(0, 30);
        x264AQStrengthSlider->setValue(10);
        x264AQStrengthSlider->setSingleStep(1);
        x264AQStrengthSlider->setEnabled(false);

        QLabel *val = new QLabel("1.0");

        l->addWidget(lbl);
        l->addWidget(x264AQStrengthSlider);
        l->addWidget(val);
        l->addStretch();

        QObject::connect(x264AQStrengthSlider, &QSlider::valueChanged, [val](int v) {
            val->setText(QString::number(v / 10.0, 'f', 1));
        });

        x264Layout->addWidget(w);
    }
    // Two-pass
    {
        QHBoxLayout *l = new QHBoxLayout();
        x264TwoPassCheck = new QCheckBox("Enable Two-Pass Encoding");
        x264TwoPassCheck->setToolTip("Better quality, slower.");
        l->addWidget(x264TwoPassCheck);
        l->addStretch();
        x264Layout->addLayout(l);
    }
    // Keyframe interval
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Key Frame Interval:");
        lbl->setToolTip("How often to insert keyframes (for seeking). Higher = smaller file.");
        x264KeyIntBox = new QComboBox();
        x264KeyIntBox->addItems({"Custom", "15", "30", "60", "120", "240", "360", "480", "720", "960", "1440", "1920"});
        x264KeyIntBox->setCurrentIndex(0);
        x264KeyIntBox->setCurrentText("Custom");
        l->addWidget(lbl);
        l->addWidget(x264KeyIntBox);
        l->addStretch();
        x264Layout->addLayout(l);
    }

    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget();
        w->setMaximumWidth(450);
        w->setLayout(l);
        w->setVisible(false);
        QLabel *lbl = new QLabel("Custom Keyframe Mode:");
        x264CustomKeyframeModeBox = new QComboBox();
        x264CustomKeyframeModeBox->addItems({
            "Every 5 seconds (fixed time)",
                                            "Every 5 seconds × framerate (recommended)"
        });
        x264CustomKeyframeModeBox->setCurrentIndex(1);
        l->addWidget(lbl);
        l->addWidget(x264CustomKeyframeModeBox);
        l->addStretch();
        x264Layout->addWidget(w);
        QObject::connect(x264KeyIntBox, &QComboBox::currentTextChanged, [w](const QString &text){
            w->setVisible(text == "Custom");
        });
        QTimer::singleShot(0, [w](){ w->setVisible(true); });
    }
    // Threads
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Threads:");
        lbl->setToolTip("CPU threads. Auto detects.");
        x264ThreadsBox = new QComboBox();
        x264ThreadsBox->addItems({"Automatic", "1", "2", "4", "8", "12", "16"});
        x264ThreadsBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(x264ThreadsBox);
        l->addStretch();
        x264Layout->addLayout(l);
    }
    // Frame threads
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Frame Threads:");
        lbl->setToolTip("Parallel frame encoding.");
        x264FrameThreadsBox = new QComboBox();
        x264FrameThreadsBox->addItems({"Automatic", "1", "2", "3", "4"});
        x264FrameThreadsBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(x264FrameThreadsBox);
        l->addStretch();
        x264Layout->addLayout(l);
    }
    // Audio settings (exact same as x265)
    {
        QHBoxLayout *l = new QHBoxLayout();
        x264AudioCheck = new QCheckBox("Include Audio");
        x264AudioCheck->setChecked(true);
        x264AudioCheck->setToolTip("Uncheck to remove audio.");
        l->addWidget(x264AudioCheck);
        QLabel *codecLbl = new QLabel("Audio Codec:");
        codecLbl->setToolTip("Audio format.");
        x264AudioCodecBox = new QComboBox();
        x264AudioCodecBox->addItems({"aac", "opus", "mp3", "flac", "vorbis"});
        x264AudioCodecBox->setCurrentIndex(0);
        l->addWidget(codecLbl);
        l->addWidget(x264AudioCodecBox);
        QLabel *sampleLbl = new QLabel("Sample Rate:");
        sampleLbl->setToolTip("48 kHz standard.");
        x264AudioSampleRateBox = new QComboBox();
        x264AudioSampleRateBox->addItems({"8 kHz", "12 kHz", "16 kHz", "24 kHz", "48 kHz"});
        x264AudioSampleRateBox->setCurrentIndex(4);
        l->addWidget(sampleLbl);
        l->addWidget(x264AudioSampleRateBox);
        QLabel *bitrateLbl = new QLabel("Bitrate:");
        bitrateLbl->setToolTip("Higher = better audio.");
        x264AudioBitrateBox = new QComboBox();
        x264AudioBitrateBox->addItems({"64 kbps", "128 kbps", "192 kbps", "256 kbps", "320 kbps", "384 kbps", "448 kbps", "512 kbps"});
        x264AudioBitrateBox->setCurrentIndex(1);
        l->addWidget(bitrateLbl);
        l->addWidget(x264AudioBitrateBox);
        x264VbrModeLabel = new QLabel("VBR Mode:");
        x264VbrModeBox = new QComboBox();
        x264VbrModeBox->addItems({"Default", "Constrained", "Off"});
        x264VbrModeBox->setCurrentIndex(0);
        x264VbrModeBox->setToolTip("For Opus.");
        l->addWidget(x264VbrModeLabel);
        l->addWidget(x264VbrModeBox);
        x264AacQualityLabel = new QLabel("Quality Level:");
        x264AacQualityBox = new QComboBox();
        x264AacQualityBox->addItems({"Disabled", "0.1", "0.5", "1"});
        x264AacQualityBox->setCurrentIndex(0);
        x264AacQualityBox->setToolTip("For AAC: Higher = better.");
        l->addWidget(x264AacQualityLabel);
        l->addWidget(x264AacQualityBox);
        x264Mp3VbrLabel = new QLabel("VBR:");
        x264Mp3VbrBox = new QComboBox();
        x264Mp3VbrBox->addItems({"Disabled", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"});
        x264Mp3VbrBox->setCurrentIndex(0);
        x264Mp3VbrBox->setToolTip("For MP3: 0=best.");
        l->addWidget(x264Mp3VbrLabel);
        l->addWidget(x264Mp3VbrBox);
        x264FlacCompressionLabel = new QLabel("Compression Level:");
        x264FlacCompressionBox = new QComboBox();
        x264FlacCompressionBox->addItems({"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"});
        x264FlacCompressionBox->setCurrentIndex(5);
        x264FlacCompressionBox->setToolTip("For FLAC: Higher = smaller/slower.");
        l->addWidget(x264FlacCompressionLabel);
        l->addWidget(x264FlacCompressionBox);
        x264VorbisQualityLabel = new QLabel("Quality Level:");
        x264VorbisQualityBox = new QComboBox();
        x264VorbisQualityBox->addItems({"Disabled", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"});
        x264VorbisQualityBox->setCurrentIndex(0);
        x264VorbisQualityBox->setToolTip("For Vorbis: Higher = better.");
        l->addWidget(x264VorbisQualityLabel);
        l->addWidget(x264VorbisQualityBox);
        x264Layout->addLayout(l);
    }
    // Reset button
    {
        QHBoxLayout *l = new QHBoxLayout();
        QPushButton *resetButton = new QPushButton("Reset to Defaults");
        resetButton->setToolTip("Reset all x264 settings.");
        l->addStretch();
        l->addWidget(resetButton);
        x264Layout->addLayout(l);
        QObject::connect(resetButton, &QPushButton::clicked, this, &X264Tab::resetDefaults);
    }
    // Signals and slots (exact same logic as x265)
    x264RCModeBox->setEnabled(false);
    x264TwoPassCheck->setEnabled(false);
    x264TwoPassCheck->setChecked(false);
    QObject::connect(x264EnableRCModeCheck, &QCheckBox::toggled, [this](bool on){
        x264RCModeBox->setEnabled(on);
        if (on) {
            QString m = x264RCModeBox->currentText();
            x264QPConfigWidget->setVisible(m == "QP");
            x264CRFConfigWidget->setVisible(m == "CRF");
            x264ABRConfigWidget->setVisible(m == "ABR");
            x264CBRConfigWidget->setVisible(m == "CBR");
            x264TwoPassCheck->setEnabled(m == "ABR" || m == "CBR");
        } else {
            x264QPConfigWidget->setVisible(false);
            x264CRFConfigWidget->setVisible(false);
            x264ABRConfigWidget->setVisible(false);
            x264CBRConfigWidget->setVisible(false);
            x264TwoPassCheck->setEnabled(false);
            x264TwoPassCheck->setChecked(false);
        }
    });
    QObject::connect(x264RCModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](){
        if (x264EnableRCModeCheck->isChecked()) {
            QString m = x264RCModeBox->currentText();
            x264QPConfigWidget->setVisible(m == "QP");
            x264CRFConfigWidget->setVisible(m == "CRF");
            x264ABRConfigWidget->setVisible(m == "ABR");
            x264CBRConfigWidget->setVisible(m == "CBR");
            x264TwoPassCheck->setEnabled(m == "ABR" || m == "CBR");
        }
    });
    QObject::connect(x264LookaheadCheck, &QCheckBox::toggled, [this](bool on){ x264LookaheadSlider->setEnabled(on); });
    QObject::connect(x264UnsharpenCheck, &QCheckBox::toggled, [this](bool on){ x264UnsharpenStrengthSlider->setEnabled(on); });
    QObject::connect(x264SharpenCheck, &QCheckBox::toggled, [this](bool on){ x264SharpenStrengthSlider->setEnabled(on); });
    QObject::connect(x264BlurCheck, &QCheckBox::toggled, [this](bool on){ x264BlurStrengthSlider->setEnabled(on); });
    QObject::connect(x264NoiseReductionCheck, &QCheckBox::toggled, [this](bool on){ x264NoiseReductionSlider->setEnabled(on); });
    QObject::connect(x264GrainSynthCheck, &QCheckBox::toggled, [this](bool on){ x264GrainSynthLevel->setEnabled(on); });
    QObject::connect(x264AQModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](){
        QString m = x264AQModeBox->currentText();
        x264AQStrengthSlider->setEnabled(m != "Automatic" && m != "Disabled");
    });
    QObject::connect(x264AudioCodecBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](){
        QString c = x264AudioCodecBox->currentText();
        x264VbrModeLabel->setVisible(c == "opus");
        x264VbrModeBox->setVisible(c == "opus");
        x264AacQualityLabel->setVisible(c == "aac");
        x264AacQualityBox->setVisible(c == "aac");
        x264Mp3VbrLabel->setVisible(c == "mp3");
        x264Mp3VbrBox->setVisible(c == "mp3");
        x264FlacCompressionLabel->setVisible(c == "flac");
        x264FlacCompressionBox->setVisible(c == "flac");
        x264VorbisQualityLabel->setVisible(c == "vorbis");
        x264VorbisQualityBox->setVisible(c == "vorbis");
    });
    QObject::connect(x264ContainerBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &X264Tab::updateAudioCodecOptions);
    updateAudioCodecOptions();
}

void X264Tab::updateAudioCodecOptions() {
    QString cont = x264ContainerBox->currentText();
    QString cur = x264AudioCodecBox->currentText();
    x264AudioCodecBox->clear();
    if (cont == "mp4") {
        x264AudioCodecBox->addItems({"aac", "mp3", "opus"});
    } else {
        x264AudioCodecBox->addItems({"aac", "opus", "mp3", "flac", "vorbis"});
    }
    int idx = x264AudioCodecBox->findText(cur);
    x264AudioCodecBox->setCurrentIndex(idx != -1 ? idx : 0);
    emit x264AudioCodecBox->currentIndexChanged(x264AudioCodecBox->currentIndex());
}

void X264Tab::resetDefaults() {
    x264ContainerBox->setCurrentIndex(0);
    x264PresetBox->setCurrentIndex(5);
    x264EnableRCModeCheck->setChecked(false);
    x264RCModeBox->setCurrentIndex(1);
    x264QPConfigWidget->setVisible(false);
    x264CRFConfigWidget->setVisible(false);
    x264ABRConfigWidget->setVisible(false);
    x264CBRConfigWidget->setVisible(false);
    x264QPSlider->setValue(23);
    x264CRFSlider->setValue(23);
    x264ABRBitrateSlider->setValue(1000);
    x264ABRVBVCheck->setChecked(false);
    x264CBRBitrateSlider->setValue(1000);
    x264TuneBox->setCurrentIndex(0);
    x264LevelBox->setCurrentIndex(0);
    x264ProfileBox->setCurrentIndex(2);
    strongIntraCheck->setChecked(true);
    rdoqLevelBox->setCurrentIndex(1);
    limitRefsBox->setCurrentIndex(0);
    x264LookaheadCheck->setChecked(false);
    x264LookaheadSlider->setValue(40);
    x264UnsharpenCheck->setChecked(false);
    x264UnsharpenStrengthSlider->setValue(1);
    x264SharpenCheck->setChecked(false);
    x264SharpenStrengthSlider->setValue(1);
    x264BlurCheck->setChecked(false);
    x264BlurStrengthSlider->setValue(1);
    x264NoiseReductionCheck->setChecked(false);
    x264NoiseReductionSlider->setValue(5);
    x264GrainSynthCheck->setChecked(false);
    x264GrainSynthLevel->setValue(0);
    x264TwoPassCheck->setChecked(false);
    x264AQModeBox->setCurrentIndex(0);
    x264AQStrengthSlider->setValue(10);
    x264KeyIntBox->setCurrentIndex(0);
    x264KeyIntBox->setCurrentText("Custom");
    x264ThreadsBox->setCurrentIndex(0);
    x264FrameThreadsBox->setCurrentIndex(0);
    x264AudioCheck->setChecked(true);
    x264AudioCodecBox->setCurrentIndex(0);
    x264AudioSampleRateBox->setCurrentIndex(4);
    x264AudioBitrateBox->setCurrentIndex(1);
    x264VbrModeBox->setCurrentIndex(0);
    x264AacQualityBox->setCurrentIndex(0);
    x264Mp3VbrBox->setCurrentIndex(0);
    x264FlacCompressionBox->setCurrentIndex(5);
    x264VorbisQualityBox->setCurrentIndex(0);
    enablePsyRdCheck->setChecked(false);
    enableCutreeCheck->setChecked(false);
    deblockAlphaSlider->setValue(-2);
    deblockBetaSlider->setValue(-2);
    refFramesBox->setCurrentIndex(3);
    weightpCheck->setChecked(true);
    x264EnableRCModeCheck->toggled(false);
    x264AQModeBox->currentIndexChanged(0);
    updateAudioCodecOptions();
}
