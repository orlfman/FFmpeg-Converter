#include "av1tab.h"
#include <QToolTip>
Av1Tab::Av1Tab(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *av1Layout = new QVBoxLayout(this);
    // Picking the output container
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Container:");
        lbl->setToolTip("File type: mkv (general) or webm (web-optimized).");
        av1ContainerBox = new QComboBox();
        av1ContainerBox->addItems({"mkv", "webm"});
        av1ContainerBox->setCurrentIndex(0);
        av1ContainerBox->setToolTip("Choose mkv for most uses, webm for web playback.");
        l->addWidget(lbl);
        l->addWidget(av1ContainerBox);
        l->addStretch();
        av1Layout->addLayout(l);
    }
    // Choosing the preset for speed vs quality
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Preset:");
        lbl->setToolTip("Speed vs quality tradeoff.");
        av1PresetBox = new QComboBox();
        for (int i = 0; i <= 13; ++i) av1PresetBox->addItem(QString::number(i));
        av1PresetBox->setCurrentIndex(8);
        av1PresetBox->setToolTip(
            "Lower number = better quality but slower encoding.\n"
            "Higher number = faster but lower quality.\n"
            "Default 8 is balanced for most videos.");
        l->addWidget(lbl);
        l->addWidget(av1PresetBox);
        l->addStretch();
        av1Layout->addLayout(l);
    }
    // Setting up rate control options
    {
        QHBoxLayout *l = new QHBoxLayout();
        av1EnableRCModeCheck = new QCheckBox("Custom Rate Control");
        av1EnableRCModeCheck->setChecked(false);
        av1EnableRCModeCheck->setToolTip("Uncheck for default CRF (good for most uses).");
        av1RCModeBox = new QComboBox();
        av1RCModeBox->addItems({"QP", "CRF", "VBR"});
        av1RCModeBox->setCurrentIndex(1);
        av1RCModeBox->setToolTip(
            "QP: Fixed quality level (simple).\n"
            "CRF: Balances quality and size automatically.\n"
            "VBR: Targets a specific bitrate (for streaming).");
        l->addWidget(av1EnableRCModeCheck);
        l->addWidget(av1RCModeBox);
        l->addStretch();
        av1Layout->addLayout(l);
    }
    // CRF settings, shows when CRF is picked
    av1CRFConfigWidget = new QWidget();
    {
        QHBoxLayout *l = new QHBoxLayout(av1CRFConfigWidget);
        QLabel *lbl = new QLabel("CRF Value:");
        lbl->setToolTip("Lower = better quality, larger file. Range 0-63.");
        av1CRFSlider = new QSlider(Qt::Horizontal);
        av1CRFSlider->setMaximumWidth(400);
        av1CRFSlider->setRange(0, 63);
        av1CRFSlider->setValue(28);
        QLabel *val = new QLabel("28");
        l->addWidget(lbl);
        l->addWidget(av1CRFSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(av1CRFSlider, &QSlider::valueChanged,
                         [val](int v){ val->setText(QString::number(v)); });
        av1Layout->addWidget(av1CRFConfigWidget);
        av1CRFConfigWidget->setVisible(false);
    }
    // QP settings, similar to CRF
    av1QPConfigWidget = new QWidget();
    {
        QHBoxLayout *l = new QHBoxLayout(av1QPConfigWidget);
        QLabel *lbl = new QLabel("QP Value:");
        lbl->setToolTip("Lower = better quality, larger file. Range 0-63.");
        av1QPSlider = new QSlider(Qt::Horizontal);
        av1QPSlider->setMaximumWidth(400);
        av1QPSlider->setRange(0, 63);
        av1QPSlider->setValue(28);
        QLabel *val = new QLabel("28");
        l->addWidget(lbl);
        l->addWidget(av1QPSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(av1QPSlider, &QSlider::valueChanged,
                         [val](int v){ val->setText(QString::number(v)); });
        av1Layout->addWidget(av1QPConfigWidget);
        av1QPConfigWidget->setVisible(false);
    }
    // VBR settings for bitrate targeting
    av1VBRConfigWidget = new QWidget();
    {
        QHBoxLayout *l = new QHBoxLayout(av1VBRConfigWidget);
        QLabel *lbl = new QLabel("VBR Bitrate (kbps):");
        lbl->setToolTip("Target average bitrate. Higher = better quality, larger file. Range 100-10000.");
        av1VBRBitrateSlider = new QSlider(Qt::Horizontal);
        av1VBRBitrateSlider->setMaximumWidth(400);
        av1VBRBitrateSlider->setRange(100, 10000);
        av1VBRBitrateSlider->setSingleStep(100);
        av1VBRBitrateSlider->setValue(1000);
        QLabel *val = new QLabel("1000");
        av1VBRVBVCheck = new QCheckBox("Enable VBV");
        av1VBRVBVCheck->setToolTip("Limits bitrate spikes for smoother streaming.");
        l->addWidget(lbl);
        l->addWidget(av1VBRBitrateSlider);
        l->addWidget(val);
        l->addWidget(av1VBRVBVCheck);
        l->addStretch();
        QObject::connect(av1VBRBitrateSlider, &QSlider::valueChanged,
                         [val](int v){ val->setText(QString::number(v)); });
        av1Layout->addWidget(av1VBRConfigWidget);
        av1VBRConfigWidget->setVisible(false);
    }
    // Tune for specific metrics
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("AV1 Tune:");
        lbl->setToolTip("Optimizes for specific quality metrics (PSNR for sharpness, VQ for default, etc. Note, VMAF(neg) is made for netflix and only works in certain versions of svt builds).");
        av1TuneBox = new QComboBox();
        av1TuneBox->addItems({"Auto", "Subjective SSIM (VQ)", "PSNR", "SSIM", "VMAF", "VMAF Neg"});
        av1TuneBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(av1TuneBox);
        l->addStretch();
        av1Layout->addLayout(l);
    }
    // Encoding level for compatibility
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Encoding Level:");
        lbl->setToolTip("AV1 compatibility level. Auto works for most players.");
        av1LevelBox = new QComboBox();
        av1LevelBox->addItems({"Auto","2.0","2.1","3.0","3.1","4.0","4.1",
            "5.0","5.1","5.2","5.3","6.0","6.1","6.1","6.2","6.3"});
        av1LevelBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(av1LevelBox);
        l->addStretch();
        av1Layout->addLayout(l);
    }
    // Lookahead for better compression
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget();
        w->setMaximumWidth(400);
        w->setLayout(l);
        av1LookaheadCheck = new QCheckBox("Lookahead Frames");
        av1LookaheadCheck->setToolTip("Looks ahead for better compression. Higher = slower but better. Range 0-120.");
        av1LookaheadSlider = new QSlider(Qt::Horizontal);
        av1LookaheadSlider->setMaximumWidth(300);
        av1LookaheadSlider->setRange(0, 120);
        av1LookaheadSlider->setValue(35);
        av1LookaheadSlider->setEnabled(false);
        QLabel *val = new QLabel("35");
        l->addWidget(av1LookaheadCheck);
        l->addWidget(av1LookaheadSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(av1LookaheadSlider, &QSlider::valueChanged,
                         [val](int v){ val->setText(QString::number(v)); });
        av1Layout->addWidget(w);
    }
    // Grouping video filters
    QGroupBox *filtersGroup = new QGroupBox("Video Filters");
    QVBoxLayout *filtersLayout = new QVBoxLayout(filtersGroup);
    av1Layout->addWidget(filtersGroup);
    // Unsharpen filter
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        av1UnsharpenCheck = new QCheckBox("Unsharpen");
        av1UnsharpenCheck->setToolTip("Reduces sharpness slightly for smoother look.");
        av1UnsharpenStrengthSlider = new QSlider(Qt::Horizontal);
        av1UnsharpenStrengthSlider->setMaximumWidth(300);
        av1UnsharpenStrengthSlider->setRange(0, 50);
        av1UnsharpenStrengthSlider->setValue(10);
        av1UnsharpenStrengthSlider->setEnabled(false);
        QLabel *val = new QLabel("1.0");
        l->addWidget(av1UnsharpenCheck);
        l->addWidget(av1UnsharpenStrengthSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(av1UnsharpenStrengthSlider, &QSlider::valueChanged,
                         [val](int v){ val->setText(QString::number(v/10.0,'f',1)); });
        filtersLayout->addWidget(w);
    }
    // Sharpen filter
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        av1SharpenCheck = new QCheckBox("Sharpen");
        av1SharpenCheck->setToolTip("Increases edge sharpness.");
        av1SharpenStrengthSlider = new QSlider(Qt::Horizontal);
        av1SharpenStrengthSlider->setMaximumWidth(300);
        av1SharpenStrengthSlider->setRange(0, 50);
        av1SharpenStrengthSlider->setValue(10);
        av1SharpenStrengthSlider->setEnabled(false);
        QLabel *val = new QLabel("1.0");
        l->addWidget(av1SharpenCheck);
        l->addWidget(av1SharpenStrengthSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(av1SharpenStrengthSlider, &QSlider::valueChanged,
                         [val](int v){ val->setText(QString::number(v/10.0,'f',1)); });
        filtersLayout->addWidget(w);
    }
    // Blur filter
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        av1BlurCheck = new QCheckBox("Blur");
        av1BlurCheck->setToolTip("Softens the image to reduce noise.");
        av1BlurStrengthSlider = new QSlider(Qt::Horizontal);
        av1BlurStrengthSlider->setMaximumWidth(300);
        av1BlurStrengthSlider->setRange(0, 50);
        av1BlurStrengthSlider->setValue(10);
        av1BlurStrengthSlider->setEnabled(false);
        QLabel *val = new QLabel("1.0");
        l->addWidget(av1BlurCheck);
        l->addWidget(av1BlurStrengthSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(av1BlurStrengthSlider, &QSlider::valueChanged,
                         [val](int v){ val->setText(QString::number(v/10.0,'f',1)); });
        filtersLayout->addWidget(w);
    }
    // Noise reduction
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        av1NoiseReductionCheck = new QCheckBox("Noise Reduction");
        av1NoiseReductionCheck->setToolTip("Reduces video noise (grainy artifacts).");
        av1NoiseReductionSlider = new QSlider(Qt::Horizontal);
        av1NoiseReductionSlider->setMaximumWidth(300);
        av1NoiseReductionSlider->setRange(0, 10);
        av1NoiseReductionSlider->setValue(5);
        av1NoiseReductionSlider->setEnabled(false);
        QLabel *val = new QLabel("5");
        l->addWidget(av1NoiseReductionCheck);
        l->addWidget(av1NoiseReductionSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(av1NoiseReductionSlider, &QSlider::valueChanged,
                         [val](int v){ val->setText(QString::number(v)); });
        filtersLayout->addWidget(w);
    }
    // NLMeans Denoise filter
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        av1NlmeansCheck = new QCheckBox("NLMeans Denoise");
        av1NlmeansCheck->setToolTip("Strong non-local means denoiser for heavy grain/noise.");
        l->addWidget(av1NlmeansCheck);
        l->addStretch();
        filtersLayout->addWidget(w);
    }
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        QLabel *sLbl = new QLabel("Sigma S:");
        sLbl->setToolTip("Spatial strength (0-100, higher=more denoising).");
        av1NlmeansSigmaSSlider = new QSlider(Qt::Horizontal);
        av1NlmeansSigmaSSlider->setMaximumWidth(200);
        av1NlmeansSigmaSSlider->setRange(0, 100);
        av1NlmeansSigmaSSlider->setValue(20);
        av1NlmeansSigmaSSlider->setEnabled(false);
        QLabel *sVal = new QLabel("20");
        l->addWidget(sLbl);
        l->addWidget(av1NlmeansSigmaSSlider);
        l->addWidget(sVal);
        l->addStretch();
        QObject::connect(av1NlmeansSigmaSSlider, &QSlider::valueChanged, [sVal](int v){ sVal->setText(QString::number(v)); });
        filtersLayout->addWidget(w);
    }
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        QLabel *pLbl = new QLabel("Sigma P:");
        pLbl->setToolTip("Temporal strength (0-100, higher=more across frames).");
        av1NlmeansSigmaPSlider = new QSlider(Qt::Horizontal);
        av1NlmeansSigmaPSlider->setMaximumWidth(200);
        av1NlmeansSigmaPSlider->setRange(0, 100);
        av1NlmeansSigmaPSlider->setValue(10);
        av1NlmeansSigmaPSlider->setEnabled(false);
        QLabel *pVal = new QLabel("10");
        l->addWidget(pLbl);
        l->addWidget(av1NlmeansSigmaPSlider);
        l->addWidget(pVal);
        l->addStretch();
        QObject::connect(av1NlmeansSigmaPSlider, &QSlider::valueChanged, [pVal](int v){ pVal->setText(QString::number(v)); });
        filtersLayout->addWidget(w);
    }
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        av1NlmeansGpuCheck = new QCheckBox("GPU Mode (KNLMeansCL)");
        av1NlmeansGpuCheck->setToolTip("OpenCL-accelerated (AMD/Intel GPUs; fallback to CPU if unsupported).");
        av1NlmeansGpuCheck->setEnabled(false); // Only if NLMeans checked
        l->addWidget(av1NlmeansGpuCheck);
        l->addStretch();
        filtersLayout->addWidget(w);
    }
    // Grain options group
    QGroupBox *grainGroup = new QGroupBox("Grain Options");
    QVBoxLayout *grainLayout = new QVBoxLayout(grainGroup);
    av1Layout->addWidget(grainGroup);
    // FFmpeg grain synthesis
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        av1GrainSynthCheck = new QCheckBox("FFmpeg Grain Synthesis");
        av1GrainSynthCheck->setToolTip("Adds synthetic grain using FFmpeg filter (for film-like look). Range 0-50.");
        av1GrainSynthLevel = new QSlider(Qt::Horizontal);
        av1GrainSynthLevel->setMaximumWidth(200);
        av1GrainSynthLevel->setRange(0, 50);
        av1GrainSynthLevel->setValue(0);
        av1GrainSynthLevel->setEnabled(false);
        QLabel *val = new QLabel("0");
        l->addWidget(av1GrainSynthCheck);
        l->addWidget(av1GrainSynthLevel);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(av1GrainSynthLevel, &QSlider::valueChanged,
                         [val](int v){ val->setText(QString::number(v)); });
        grainLayout->addWidget(w);
    }
    // Native AV1 film grain
    {
        QHBoxLayout *l = new QHBoxLayout();
        nativeGrainCheck = new QCheckBox("Native Film Grain Synthesis");
        nativeGrainCheck->setToolTip("Adds film-like grain directly in AV1 (better for compression). Strength 0-50.");
        grainStrengthSlider = new QSlider(Qt::Horizontal);
        grainStrengthSlider->setMaximumWidth(300);
        grainStrengthSlider->setRange(0, 50);
        grainStrengthSlider->setValue(0);
        grainStrengthSlider->setEnabled(false);
        QLabel *strengthVal = new QLabel("0");
        l->addWidget(nativeGrainCheck);
        l->addWidget(grainStrengthSlider);
        l->addWidget(strengthVal);
        l->addStretch();
        QObject::connect(grainStrengthSlider, &QSlider::valueChanged,
                         [strengthVal](int v){ strengthVal->setText(QString::number(v)); });
        grainLayout->addLayout(l);
    }
    // Grain denoise level
    {
        QHBoxLayout *denoiseRow = new QHBoxLayout();
        denoiseRow->setContentsMargins(20, 0, 0, 0);
        QLabel *denoiseLbl = new QLabel("Grain Denoise:");
        denoiseLbl->setToolTip("0 = Add grain on top of existing noise, 1 = Denoise first, then add clean synthetic grain");
        grainDenoiseCombo = new QComboBox();
        grainDenoiseCombo->addItems({"0 (Off)", "1 (On - Denoise + Add Grain)"});
        grainDenoiseCombo->setCurrentIndex(0);
        grainDenoiseCombo->setEnabled(false);
        denoiseRow->addWidget(denoiseLbl);
        denoiseRow->addWidget(grainDenoiseCombo);
        denoiseRow->addStretch();
        grainLayout->addLayout(denoiseRow);
    }
    // Advanced options group
    QGroupBox *advancedGroup = new QGroupBox("Advanced Options");
    QVBoxLayout *advancedLayout = new QVBoxLayout(advancedGroup);
    av1Layout->addWidget(advancedGroup);
    // CDEF filter
    {
        QHBoxLayout *l = new QHBoxLayout();
        enableCdefCheck = new QCheckBox("Enable CDEF");
        enableCdefCheck->setChecked(true);
        enableCdefCheck->setToolTip("Constrained Directional Enhancement Filter for deringing. Disable for faster encoding.");
        l->addWidget(enableCdefCheck);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // Super-res mode
    {
        QHBoxLayout *comboRow = new QHBoxLayout();
        QLabel *lbl = new QLabel("Super-Res Mode:");
        lbl->setToolTip("Upscales video for better quality at low bitrates. 0=disabled.");
        superResModeBox = new QComboBox();
        superResModeBox->addItems({"Disabled","1 (Fastest)","2 (Fast)","3 (All)","4 (Random)"});
        superResModeBox->setCurrentIndex(0);
        comboRow->addWidget(lbl);
        comboRow->addWidget(superResModeBox);
        comboRow->addStretch();
        advancedLayout->addLayout(comboRow);
        QHBoxLayout *sliderRow = new QHBoxLayout();
        sliderRow->setContentsMargins(20, 0, 0, 0);
        superResDenomSlider = new QSlider(Qt::Horizontal);
        superResDenomSlider->setMaximumWidth(300);
        superResDenomSlider->setRange(8, 16);
        superResDenomSlider->setValue(8);
        superResDenomSlider->setEnabled(false);
        superResDenomSlider->setToolTip("Denominator for super-res (8-16). Higher = more upscaling.");
        QLabel *val = new QLabel("8");
        sliderRow->addWidget(superResDenomSlider);
        sliderRow->addWidget(val);
        sliderRow->addStretch();
        advancedLayout->addLayout(sliderRow);
        QObject::connect(superResModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                         [this](int i){
                             superResDenomSlider->setEnabled(i > 0);
                         });
        QObject::connect(superResDenomSlider, &QSlider::valueChanged,
                         [val](int v){ val->setText(QString::number(v)); });
    }
    // Fast decode levels
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *fdLbl = new QLabel("Fast Decode:");
        fdLbl->setToolTip("Speeds up playback decoding. Level 2 is fastest but may reduce quality.");
        fastDecodeBox = new QComboBox();
        fastDecodeBox->addItems({"Disabled", "Level 1", "Level 2"});
        fastDecodeBox->setCurrentIndex(0);
        l->addWidget(fdLbl);
        l->addWidget(fastDecodeBox);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // Low latency mode
    {
        QHBoxLayout *l = new QHBoxLayout();
        lowLatencyCheck = new QCheckBox("Low Latency Mode");
        lowLatencyCheck->setToolTip("For live streaming: reduces delay.");
        l->addWidget(lowLatencyCheck);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // Spatio-temporal prediction
    {
        QHBoxLayout *l = new QHBoxLayout();
        tplModelCheck = new QCheckBox("Spatio-Temporal Prediction");
        tplModelCheck->setToolTip("Improves quality by 3-5% at cost of speed.");
        l->addWidget(tplModelCheck);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // Temporal filtering
    {
        QHBoxLayout *l = new QHBoxLayout();
        enableTfCheck = new QCheckBox("Enable Temporal Filtering");
        enableTfCheck->setToolTip("Smooths motion for better compression (good for noisy videos).");
        l->addWidget(enableTfCheck);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // Screen content mode
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *scmLbl = new QLabel("Screen Content Mode:");
        scmLbl->setToolTip("Optimizes for screen recordings/graphics. Auto detects if needed.");
        screenContentModeBox = new QComboBox();
        screenContentModeBox->addItems({"Disabled", "Forced", "Auto-Detect"});
        screenContentModeBox->setCurrentIndex(0);
        l->addWidget(scmLbl);
        l->addWidget(screenContentModeBox);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // AQ mode
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("AQ Mode:");
        lbl->setToolTip("Adaptive Quantization: Adjusts quality per frame.");
        av1AQModeBox = new QComboBox();
        av1AQModeBox->addItems({"Automatic","Disabled","Variance","Complexity"});
        av1AQModeBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(av1AQModeBox);
        l->addStretch();
        av1Layout->addLayout(l);
    }
    // AQ strength
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        QLabel *lbl = new QLabel("AQ Strength:");
        lbl->setToolTip("Variance Boost Strength (1–4). Only used when AQ Mode = Variance. Ignored in Complexity mode");
        av1AQStrengthSlider = new QSlider(Qt::Horizontal);
        av1AQStrengthSlider->setMaximumWidth(300);
        av1AQStrengthSlider->setRange(1, 4);
        av1AQStrengthSlider->setValue(2);
        av1AQStrengthSlider->setEnabled(false);
        QLabel *val = new QLabel("2");
        l->addWidget(lbl);
        l->addWidget(av1AQStrengthSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(av1AQStrengthSlider, &QSlider::valueChanged,
                         [val](int v){ val->setText(QString::number(v)); });
        av1Layout->addWidget(w);
    }
    QObject::connect(av1AQModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     [this](int index) {
                         bool isComplexity = (index == 3);
                         av1AQStrengthSlider->setEnabled(!isComplexity);
                     });
    // Two-pass encoding
    {
        QHBoxLayout *l = new QHBoxLayout();
        av1TwoPassCheck = new QCheckBox("Enable Two-Pass Encoding");
        av1TwoPassCheck->setToolTip("Runs twice for optimal quality (slower). Good for VBR.");
        l->addWidget(av1TwoPassCheck);
        l->addStretch();
        av1Layout->addLayout(l);
    }
    // Keyframe interval
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Key Frame Interval:");
        lbl->setToolTip("How often to insert keyframes (for seeking). Higher = smaller file.");
        av1KeyIntBox = new QComboBox();
        av1KeyIntBox->addItems({"15","30","60","120","240","360","480","720","960","1440","1920"});
        av1KeyIntBox->setCurrentIndex(4);
        l->addWidget(lbl);
        l->addWidget(av1KeyIntBox);
        l->addStretch();
        av1Layout->addLayout(l);
    }
    // Threads for encoding
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Threads:");
        lbl->setToolTip("Number of CPU threads. Auto detects your system. Too high may crash on weak PCs.");
        av1ThreadsBox = new QComboBox();
        av1ThreadsBox->addItems({"Automatic","1","2","4","8","12","16"});
        av1ThreadsBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(av1ThreadsBox);
        l->addStretch();
        av1Layout->addLayout(l);
    }
    // Tile rows
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Tile Rows:");
        lbl->setToolTip("Splits frame for parallel encoding. Auto is best for most.");
        av1TileRowsBox = new QComboBox();
        av1TileRowsBox->addItems({"Automatic","1","2","4","8"});
        av1TileRowsBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(av1TileRowsBox);
        l->addStretch();
        av1Layout->addLayout(l);
    }
    // Tile columns
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Tile Columns:");
        lbl->setToolTip("Splits frame horizontally. Auto is recommended.");
        av1TileColumnsBox = new QComboBox();
        av1TileColumnsBox->addItems({"Automatic","1","2","4","8"});
        av1TileColumnsBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(av1TileColumnsBox);
        l->addStretch();
        av1Layout->addLayout(l);
    }
    // Audio settings
    {
        QHBoxLayout *l = new QHBoxLayout();
        av1AudioCheck = new QCheckBox("Include Audio");
        av1AudioCheck->setChecked(true);
        av1AudioCheck->setToolTip("Uncheck to remove audio from output.");
        l->addWidget(av1AudioCheck);
        QLabel *cLbl = new QLabel("Audio Codec:");
        cLbl->setToolTip("Format for audio compression.");
        av1AudioCodecBox = new QComboBox();
        av1AudioCodecBox->addItems({"opus","aac","mp3","flac","vorbis"});
        av1AudioCodecBox->setCurrentIndex(0);
        l->addWidget(cLbl);
        l->addWidget(av1AudioCodecBox);
        QLabel *sLbl = new QLabel("Sample Rate:");
        sLbl->setToolTip("Audio quality/speed. 48 kHz is standard.");
        av1AudioSampleRateBox = new QComboBox();
        av1AudioSampleRateBox->addItems({"8 kHz","12 kHz","16 kHz","24 kHz","48 kHz"});
        av1AudioSampleRateBox->setCurrentIndex(4);
        l->addWidget(sLbl);
        l->addWidget(av1AudioSampleRateBox);
        QLabel *bLbl = new QLabel("Bitrate:");
        bLbl->setToolTip("Audio quality. Higher = better but larger file.");
        av1AudioBitrateBox = new QComboBox();
        av1AudioBitrateBox->addItems({"64 kbps","128 kbps","192 kbps","256 kbps",
            "320 kbps","384 kbps","448 kbps","512 kbps"});
        av1AudioBitrateBox->setCurrentIndex(1);
        l->addWidget(bLbl);
        l->addWidget(av1AudioBitrateBox);
        // Extra options for specific codecs
        av1VbrModeLabel = new QLabel("VBR Mode:");
        av1VbrModeBox = new QComboBox();
        av1VbrModeBox->addItems({"Default","Constrained","Off"});
        av1VbrModeBox->setCurrentIndex(0);
        av1VbrModeBox->setToolTip("Variable bitrate mode for Opus.");
        l->addWidget(av1VbrModeLabel);
        l->addWidget(av1VbrModeBox);
        av1AacQualityLabel = new QLabel("AAC Quality:");
        av1AacQualityBox = new QComboBox();
        av1AacQualityBox->addItems({"Disabled","0.1","0.5","1"});
        av1AacQualityBox->setCurrentIndex(0);
        av1AacQualityBox->setToolTip("Quality scale for AAC.");
        l->addWidget(av1AacQualityLabel);
        l->addWidget(av1AacQualityBox);
        av1Mp3VbrLabel = new QLabel("MP3 VBR:");
        av1Mp3VbrBox = new QComboBox();
        av1Mp3VbrBox->addItems({"Disabled","0","1","2","3","4","5","6","7","8","9"});
        av1Mp3VbrBox->setCurrentIndex(0);
        av1Mp3VbrBox->setToolTip("VBR quality for MP3 (0=best).");
        l->addWidget(av1Mp3VbrLabel);
        l->addWidget(av1Mp3VbrBox);
        av1FlacCompressionLabel = new QLabel("FLAC Comp:");
        av1FlacCompressionBox = new QComboBox();
        av1FlacCompressionBox->addItems({"0","1","2","3","4","5","6","7","8","9","10","11","12"});
        av1FlacCompressionBox->setCurrentIndex(5);
        av1FlacCompressionBox->setToolTip("Compression level for FLAC (higher=slower/smaller, range 0-12).");
        l->addWidget(av1FlacCompressionLabel);
        l->addWidget(av1FlacCompressionBox);
        av1VorbisQualityLabel = new QLabel("Vorbis Q:");
        av1VorbisQualityBox = new QComboBox();
        av1VorbisQualityBox->addItems({"Disabled","0","1","2","3","4","5","6","7","8","9","10"});
        av1VorbisQualityBox->setCurrentIndex(0);
        av1VorbisQualityBox->setToolTip("Quality scale for Vorbis (higher=better, range 0-10).");
        l->addWidget(av1VorbisQualityLabel);
        l->addWidget(av1VorbisQualityBox);
        av1Layout->addLayout(l);
    }
    // Reset button to defaults
    {
        QHBoxLayout *l = new QHBoxLayout();
        QPushButton *resetButton = new QPushButton("Reset to Defaults");
        resetButton->setToolTip("Reset all AV1 settings to starting values.");
        l->addStretch();
        l->addWidget(resetButton);
        av1Layout->addLayout(l);
        QObject::connect(resetButton, &QPushButton::clicked, this, &Av1Tab::resetDefaults);
    }
    // Hooking up signals and slots
    av1RCModeBox->setEnabled(false);
    av1TwoPassCheck->setEnabled(false);
    av1TwoPassCheck->setChecked(false);
    // Toggling rate control widgets
    QObject::connect(av1EnableRCModeCheck, &QCheckBox::toggled, [this](bool on){
        av1RCModeBox->setEnabled(on);
        if (on) {
            QString m = av1RCModeBox->currentText();
            av1CRFConfigWidget->setVisible(m=="CRF");
            av1QPConfigWidget->setVisible(m=="QP");
            av1VBRConfigWidget->setVisible(m=="VBR");
            av1TwoPassCheck->setEnabled(m=="VBR");
        } else {
            av1CRFConfigWidget->setVisible(false);
            av1QPConfigWidget->setVisible(false);
            av1VBRConfigWidget->setVisible(false);
            av1TwoPassCheck->setEnabled(false);
            av1TwoPassCheck->setChecked(false);
        }
    });
    QObject::connect(av1RCModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](){
        if (av1EnableRCModeCheck->isChecked()) {
            QString m = av1RCModeBox->currentText();
            av1CRFConfigWidget->setVisible(m=="CRF");
            av1QPConfigWidget->setVisible(m=="QP");
            av1VBRConfigWidget->setVisible(m=="VBR");
            av1TwoPassCheck->setEnabled(m=="VBR");
        }
    });
    // Enabling sliders when checkboxes are ticked
    QObject::connect(av1LookaheadCheck, &QCheckBox::toggled,
                     [this](bool on){ av1LookaheadSlider->setEnabled(on); });
    QObject::connect(av1UnsharpenCheck, &QCheckBox::toggled,
                     [this](bool on){ av1UnsharpenStrengthSlider->setEnabled(on); });
    QObject::connect(av1SharpenCheck, &QCheckBox::toggled,
                     [this](bool on){ av1SharpenStrengthSlider->setEnabled(on); });
    QObject::connect(av1BlurCheck, &QCheckBox::toggled,
                     [this](bool on){ av1BlurStrengthSlider->setEnabled(on); });
    QObject::connect(av1NoiseReductionCheck, &QCheckBox::toggled,
                     [this](bool on){ av1NoiseReductionSlider->setEnabled(on); });
    QObject::connect(av1NlmeansCheck, &QCheckBox::toggled, [this](bool on){
        av1NlmeansSigmaSSlider->setEnabled(on);
        av1NlmeansSigmaPSlider->setEnabled(on);
    });
    QObject::connect(av1NlmeansCheck, &QCheckBox::toggled, [this](bool on){
        av1NlmeansSigmaSSlider->setEnabled(on);
        av1NlmeansSigmaPSlider->setEnabled(on);
        av1NlmeansGpuCheck->setEnabled(on);
    });
    QObject::connect(av1GrainSynthCheck, &QCheckBox::toggled,
                     [this](bool on){ av1GrainSynthLevel->setEnabled(on); });
    QObject::connect(nativeGrainCheck, &QCheckBox::toggled,
                     [this](bool on){
                         grainStrengthSlider->setEnabled(on);
                         grainDenoiseCombo->setEnabled(on);
                     });
    // Enabling AQ strength based on mode
    QObject::connect(av1AQModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](){
        QString m = av1AQModeBox->currentText();
        av1AQStrengthSlider->setEnabled(m == "Variance");
    });
    // Showing codec-specific audio options
    QObject::connect(av1AudioCodecBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](){
        QString c = av1AudioCodecBox->currentText();
        av1VbrModeLabel->setVisible(c=="opus");
        av1VbrModeBox->setVisible(c=="opus");
        av1AacQualityLabel->setVisible(c=="aac");
        av1AacQualityBox->setVisible(c=="aac");
        av1Mp3VbrLabel->setVisible(c=="mp3");
        av1Mp3VbrBox->setVisible(c=="mp3");
        av1FlacCompressionLabel->setVisible(c=="flac");
        av1FlacCompressionBox->setVisible(c=="flac");
        av1VorbisQualityLabel->setVisible(c=="vorbis");
        av1VorbisQualityBox->setVisible(c=="vorbis");
    });
    // Updating audio codecs based on container
    QObject::connect(av1ContainerBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     this, &Av1Tab::updateAudioCodecOptions);
    // Making sure low latency and lookahead don't both run
    QObject::connect(lowLatencyCheck, &QCheckBox::toggled, [this](bool on){
        if (on && av1LookaheadCheck->isChecked()) {
            av1LookaheadCheck->setChecked(false);
            av1LookaheadSlider->setEnabled(false);
            QToolTip::showText(lowLatencyCheck->mapToGlobal(QPoint(0,0)), "Low latency disables lookahead");
        }
    });
    QObject::connect(av1LookaheadCheck, &QCheckBox::toggled, [this](bool on){
        if (on && lowLatencyCheck->isChecked()) {
            lowLatencyCheck->setChecked(false);
            QToolTip::showText(av1LookaheadCheck->mapToGlobal(QPoint(0,0)), "Lookahead disabled for low latency");
        }
    });
    // Starting audio setup
    updateAudioCodecOptions();
    av1AudioCodecBox->currentIndexChanged(av1AudioCodecBox->currentIndex());
}
// Updating audio choices based on container
void Av1Tab::updateAudioCodecOptions()
{
    QString cont = av1ContainerBox->currentText();
    QString cur = av1AudioCodecBox->currentText();
    av1AudioCodecBox->clear();
    if (cont == "webm")
        av1AudioCodecBox->addItems({"opus","vorbis"});
    else
        av1AudioCodecBox->addItems({"opus","aac","mp3","flac","vorbis"});
    int idx = av1AudioCodecBox->findText(cur);
    av1AudioCodecBox->setCurrentIndex(idx != -1 ? idx : 0);
}
// Reset everything to defaults
void Av1Tab::resetDefaults() {
    av1PresetBox->setCurrentIndex(8);
    av1EnableRCModeCheck->setChecked(false);
    av1RCModeBox->setCurrentIndex(1);
    av1CRFSlider->setValue(28);
    av1QPSlider->setValue(28);
    av1VBRBitrateSlider->setValue(1000);
    av1VBRVBVCheck->setChecked(false);
    av1TuneBox->setCurrentIndex(0);
    av1LevelBox->setCurrentIndex(0);
    av1LookaheadCheck->setChecked(false);
    av1LookaheadSlider->setValue(35);
    av1UnsharpenCheck->setChecked(false);
    av1UnsharpenStrengthSlider->setValue(10);
    av1SharpenCheck->setChecked(false);
    av1SharpenStrengthSlider->setValue(10);
    av1BlurCheck->setChecked(false);
    av1BlurStrengthSlider->setValue(10);
    av1NoiseReductionCheck->setChecked(false);
    av1NoiseReductionSlider->setValue(5);
    av1NlmeansCheck->setChecked(false);
    av1NlmeansSigmaSSlider->setValue(20);
    av1NlmeansSigmaPSlider->setValue(10);
    av1NlmeansGpuCheck->setChecked(false);
    av1GrainSynthCheck->setChecked(false);
    av1GrainSynthLevel->setValue(0);
    nativeGrainCheck->setChecked(false);
    grainStrengthSlider->setValue(0);
    grainDenoiseCombo->setCurrentIndex(0);
    superResModeBox->setCurrentIndex(0);
    superResDenomSlider->setValue(8);
    screenContentModeBox->setCurrentIndex(0);
    lowLatencyCheck->setChecked(false);
    tplModelCheck->setChecked(false);
    enableCdefCheck->setChecked(true);
    screenContentModeBox->setCurrentIndex(0);
    fastDecodeBox->setCurrentIndex(0);
    enableTfCheck->setChecked(false);
    av1AQModeBox->setCurrentIndex(0);
    av1AQStrengthSlider->setValue(2);
    av1TwoPassCheck->setChecked(false);
    av1KeyIntBox->setCurrentIndex(4);
    av1ThreadsBox->setCurrentIndex(0);
    av1TileRowsBox->setCurrentIndex(0);
    av1TileColumnsBox->setCurrentIndex(0);
    av1AudioCheck->setChecked(true);
    av1AudioCodecBox->setCurrentIndex(0);
    av1AudioSampleRateBox->setCurrentIndex(4);
    av1AudioBitrateBox->setCurrentIndex(1);
    av1VbrModeBox->setCurrentIndex(0);
    av1AacQualityBox->setCurrentIndex(0);
    av1Mp3VbrBox->setCurrentIndex(0);
    av1FlacCompressionBox->setCurrentIndex(5);
    av1VorbisQualityBox->setCurrentIndex(0);
    // Update the UI after reset
    av1EnableRCModeCheck->toggled(false);
    av1AQModeBox->currentIndexChanged(0);
    av1ContainerBox->currentIndexChanged(av1ContainerBox->currentIndex());
}
