#include "vp9tab.h"
#include <QToolTip>
Vp9Tab::Vp9Tab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *vp9Layout = new QVBoxLayout(this);
    // Picking the container
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Container:");
        lbl->setToolTip("File type: mkv (general) or webm (web-optimized).");
        vp9ContainerBox = new QComboBox();
        vp9ContainerBox->addItems({"mkv", "webm"});
        vp9ContainerBox->setCurrentIndex(1);
        vp9ContainerBox->setToolTip("webm is optimized for VP9.");
        l->addWidget(lbl);
        l->addWidget(vp9ContainerBox);
        l->addStretch();
        vp9Layout->addLayout(l);
    }
    // Rate control options
    {
        QHBoxLayout *l = new QHBoxLayout();
        vp9EnableRCModeCheck = new QCheckBox("Custom Rate Control");
        vp9EnableRCModeCheck->setChecked(false);
        vp9EnableRCModeCheck->setToolTip("Uncheck for default CRF.");
        l->addWidget(vp9EnableRCModeCheck);
        vp9RCModeBox = new QComboBox();
        vp9RCModeBox->addItems({"CRF", "ABR", "CBR"});
        vp9RCModeBox->setCurrentIndex(0);
        vp9RCModeBox->setToolTip("CRF: Quality-based.\nABR: Average bitrate.\nCBR: Constant bitrate.");
        l->addWidget(vp9RCModeBox);
        l->addStretch();
        vp9Layout->addLayout(l);
    }
    // CRF settings
    vp9CRFConfigWidget = new QWidget();
    {
        QHBoxLayout *l = new QHBoxLayout(vp9CRFConfigWidget);
        QLabel *lbl = new QLabel("CRF Value:");
        lbl->setToolTip("Lower = better quality, larger file. Range 0-63.");
        vp9CRFSlider = new QSlider(Qt::Horizontal);
        vp9CRFSlider->setMaximumWidth(400);
        vp9CRFSlider->setRange(0, 63);
        vp9CRFSlider->setValue(28);
        QLabel *val = new QLabel("28");
        l->addWidget(lbl);
        l->addWidget(vp9CRFSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(vp9CRFSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        vp9Layout->addWidget(vp9CRFConfigWidget);
        vp9CRFConfigWidget->setVisible(false);
    }
    // Bitrate settings for ABR/CBR
    vp9BitrateConfigWidget = new QWidget();
    {
        QHBoxLayout *l = new QHBoxLayout(vp9BitrateConfigWidget);
        QLabel *lbl = new QLabel("Bitrate (kbps):");
        lbl->setToolTip("Target bitrate. Range 100-10000.");
        vp9BitrateSlider = new QSlider(Qt::Horizontal);
        vp9BitrateSlider->setMaximumWidth(400);
        vp9BitrateSlider->setRange(100, 10000);
        vp9BitrateSlider->setValue(1000);
        vp9BitrateSlider->setSingleStep(100);
        QLabel *val = new QLabel("1000");
        l->addWidget(lbl);
        l->addWidget(vp9BitrateSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(vp9BitrateSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        vp9Layout->addWidget(vp9BitrateConfigWidget);
        vp9BitrateConfigWidget->setVisible(false);
    }
    // Lookahead frames
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        vp9LookaheadCheck = new QCheckBox("Lag in Frames");
        vp9LookaheadCheck->setToolTip("Enables lookahead for better quality.");
        vp9LookaheadSlider = new QSlider(Qt::Horizontal);
        vp9LookaheadSlider->setMaximumWidth(300);
        vp9LookaheadSlider->setRange(0, 25);
        vp9LookaheadSlider->setValue(16);
        vp9LookaheadSlider->setEnabled(false);
        QLabel *val = new QLabel("16");
        l->addWidget(vp9LookaheadCheck);
        l->addWidget(vp9LookaheadSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(vp9LookaheadSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        vp9Layout->addWidget(w);
    }
    // Video filters group
    QGroupBox *filtersGroup = new QGroupBox("Video Filters");
    QVBoxLayout *filtersLayout = new QVBoxLayout(filtersGroup);
    vp9Layout->addWidget(filtersGroup);
    // Unsharpen filter
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        vp9UnsharpenCheck = new QCheckBox("Unsharpen");
        vp9UnsharpenCheck->setToolTip("Reduces sharpness for smoother look.");
        vp9UnsharpenStrengthSlider = new QSlider(Qt::Horizontal);
        vp9UnsharpenStrengthSlider->setMaximumWidth(300);
        vp9UnsharpenStrengthSlider->setRange(0, 5);
        vp9UnsharpenStrengthSlider->setValue(1);
        vp9UnsharpenStrengthSlider->setEnabled(false);
        QLabel *val = new QLabel("1");
        l->addWidget(vp9UnsharpenCheck);
        l->addWidget(vp9UnsharpenStrengthSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(vp9UnsharpenStrengthSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        filtersLayout->addWidget(w);
    }
    // Sharpen filter
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        vp9SharpenCheck = new QCheckBox("Sharpen");
        vp9SharpenCheck->setToolTip("Enhances edges.");
        vp9SharpenStrengthSlider = new QSlider(Qt::Horizontal);
        vp9SharpenStrengthSlider->setMaximumWidth(300);
        vp9SharpenStrengthSlider->setRange(0, 5);
        vp9SharpenStrengthSlider->setValue(1);
        vp9SharpenStrengthSlider->setEnabled(false);
        QLabel *val = new QLabel("1");
        l->addWidget(vp9SharpenCheck);
        l->addWidget(vp9SharpenStrengthSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(vp9SharpenStrengthSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        filtersLayout->addWidget(w);
    }
    // Blur filter
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        vp9BlurCheck = new QCheckBox("Blur");
        vp9BlurCheck->setToolTip("Softens image to reduce noise.");
        vp9BlurStrengthSlider = new QSlider(Qt::Horizontal);
        vp9BlurStrengthSlider->setMaximumWidth(300);
        vp9BlurStrengthSlider->setRange(0, 5);
        vp9BlurStrengthSlider->setValue(1);
        vp9BlurStrengthSlider->setEnabled(false);
        QLabel *val = new QLabel("1");
        l->addWidget(vp9BlurCheck);
        l->addWidget(vp9BlurStrengthSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(vp9BlurStrengthSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        filtersLayout->addWidget(w);
    }
    // Noise reduction
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        vp9NoiseReductionCheck = new QCheckBox("Noise Reduction");
        vp9NoiseReductionCheck->setToolTip("Reduces noise.");
        vp9NoiseReductionSlider = new QSlider(Qt::Horizontal);
        vp9NoiseReductionSlider->setMaximumWidth(300);
        vp9NoiseReductionSlider->setRange(0, 10);
        vp9NoiseReductionSlider->setValue(5);
        vp9NoiseReductionSlider->setEnabled(false);
        QLabel *val = new QLabel("5");
        l->addWidget(vp9NoiseReductionCheck);
        l->addWidget(vp9NoiseReductionSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(vp9NoiseReductionSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        filtersLayout->addWidget(w);
    }
    // Grain options
    QGroupBox *grainGroup = new QGroupBox("Grain Options");
    QVBoxLayout *grainLayout = new QVBoxLayout(grainGroup);
    vp9Layout->addWidget(grainGroup);
    // Grain synthesis
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        vp9GrainSynthCheck = new QCheckBox("Enable Grain Synthesis");
        vp9GrainSynthCheck->setToolTip("Adds film-like grain.");
        vp9GrainSynthLevel = new QSlider(Qt::Horizontal);
        vp9GrainSynthLevel->setMaximumWidth(200);
        vp9GrainSynthLevel->setRange(0, 50);
        vp9GrainSynthLevel->setValue(0);
        vp9GrainSynthLevel->setEnabled(false);
        QLabel *val = new QLabel("0");
        l->addWidget(vp9GrainSynthCheck);
        l->addWidget(vp9GrainSynthLevel);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(vp9GrainSynthLevel, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        grainLayout->addWidget(w);
    }
    // Advanced options group
    QGroupBox *advancedGroup = new QGroupBox("Advanced Options");
    QVBoxLayout *advancedLayout = new QVBoxLayout(advancedGroup);
    vp9Layout->addWidget(advancedGroup);
    // Two-pass
    {
        QHBoxLayout *l = new QHBoxLayout();
        vp9TwoPassCheck = new QCheckBox("Enable Two-Pass Encoding");
        vp9TwoPassCheck->setToolTip("Better quality, but twice as slow.");
        l->addWidget(vp9TwoPassCheck);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // AQ mode
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("AQ Mode:");
        lbl->setToolTip("Adaptive Quantization mode.");
        vp9AQModeBox = new QComboBox();
        vp9AQModeBox->addItems({"Automatic", "Disabled", "Variance", "Complexity"});
        vp9AQModeBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(vp9AQModeBox);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // AQ strength
    {
        QHBoxLayout *l = new QHBoxLayout();
        QWidget *w = new QWidget(); w->setMaximumWidth(400); w->setLayout(l);
        QLabel *lbl = new QLabel("AQ Strength:");
        lbl->setToolTip("Higher = better in complex areas. Range 0-15.");
        vp9AQStrengthSlider = new QSlider(Qt::Horizontal);
        vp9AQStrengthSlider->setMaximumWidth(300);
        vp9AQStrengthSlider->setRange(0, 15);
        vp9AQStrengthSlider->setValue(6);
        vp9AQStrengthSlider->setEnabled(false);
        QLabel *val = new QLabel("6");
        l->addWidget(lbl);
        l->addWidget(vp9AQStrengthSlider);
        l->addWidget(val);
        l->addStretch();
        QObject::connect(vp9AQStrengthSlider, &QSlider::valueChanged, [val](int v){ val->setText(QString::number(v)); });
        advancedLayout->addWidget(w);
    }
    // Keyframe interval
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Key Frame Interval:");
        lbl->setToolTip("Higher = smaller file, slower seeking.");
        vp9KeyIntBox = new QComboBox();
        vp9KeyIntBox->addItems({"15", "30", "60", "120", "240", "360", "480", "720", "960", "1440", "1920"});
        vp9KeyIntBox->setCurrentIndex(4);
        l->addWidget(lbl);
        l->addWidget(vp9KeyIntBox);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // CPU usage preset
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("CPU-used:");
        lbl->setToolTip("Speed preset: 0=slowest/best, 5=fastest/worst.");
        vp9CpuUsedBox = new QComboBox();
        for (int i = 0; i <= 5; ++i) vp9CpuUsedBox->addItem(QString::number(i));
        vp9CpuUsedBox->setCurrentIndex(4);
        l->addWidget(lbl);
        l->addWidget(vp9CpuUsedBox);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // Threads
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Threads:");
        lbl->setToolTip("CPU threads. Auto detects system.");
        vp9ThreadsBox = new QComboBox();
        vp9ThreadsBox->addItems({"Automatic", "1", "2", "4", "8", "12", "16"});
        vp9ThreadsBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(vp9ThreadsBox);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // Tile columns
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Tile Columns:");
        lbl->setToolTip("For parallelism. Higher = faster on multi-core.");
        vp9TileColumnsBox = new QComboBox();
        vp9TileColumnsBox->addItems({"0", "1", "2", "4", "8"});
        vp9TileColumnsBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(vp9TileColumnsBox);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // Tile rows
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Tile Rows:");
        lbl->setToolTip("For parallelism. Auto recommended.");
        vp9TileRowsBox = new QComboBox();
        vp9TileRowsBox->addItems({"Automatic", "1", "2", "4", "8"});
        vp9TileRowsBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(vp9TileRowsBox);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // Deadline/quality mode
    {
        QHBoxLayout *l = new QHBoxLayout();
        QLabel *lbl = new QLabel("Deadline:");
        lbl->setToolTip("Quality mode: best=slowest/high quality, realtime=fast/low quality.");
        vp9DeadlineBox = new QComboBox();
        vp9DeadlineBox->addItems({"good", "best", "realtime"});
        vp9DeadlineBox->setCurrentIndex(0);
        l->addWidget(lbl);
        l->addWidget(vp9DeadlineBox);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // Row multi-threading
    {
        QHBoxLayout *l = new QHBoxLayout();
        enableRowMtCheck = new QCheckBox("Enable Row Multi-Threading");
        enableRowMtCheck->setToolTip("Faster encoding on multi-core CPUs.");
        l->addWidget(enableRowMtCheck);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // Screen content optimization
    {
        QHBoxLayout *l = new QHBoxLayout();
        screenContentCheck = new QCheckBox("Screen Content Mode");
        screenContentCheck->setToolTip("Optimizes for text/graphics/screen recordings.");
        l->addWidget(screenContentCheck);
        l->addStretch();
        advancedLayout->addLayout(l);
    }
    // Audio settings
    {
        QHBoxLayout *l = new QHBoxLayout();
        vp9AudioCheck = new QCheckBox("Include Audio");
        vp9AudioCheck->setChecked(true);
        vp9AudioCheck->setToolTip("Uncheck to remove audio.");
        l->addWidget(vp9AudioCheck);
        QLabel *codecLbl = new QLabel("Audio Codec:");
        codecLbl->setToolTip("Opus or Vorbis for VP9.");
        vp9AudioCodecBox = new QComboBox();
        vp9AudioCodecBox->addItems({"opus", "vorbis"});
        vp9AudioCodecBox->setCurrentIndex(0);
        l->addWidget(codecLbl);
        l->addWidget(vp9AudioCodecBox);
        QLabel *sampleLbl = new QLabel("Sample Rate:");
        sampleLbl->setToolTip("48 kHz is standard.");
        vp9AudioSampleRateBox = new QComboBox();
        vp9AudioSampleRateBox->addItems({"8 kHz", "12 kHz", "16 kHz", "24 kHz", "48 kHz"});
        vp9AudioSampleRateBox->setCurrentIndex(4);
        l->addWidget(sampleLbl);
        l->addWidget(vp9AudioSampleRateBox);
        QLabel *bitrateLbl = new QLabel("Bitrate:");
        bitrateLbl->setToolTip("Higher = better audio quality.");
        vp9AudioBitrateBox = new QComboBox();
        vp9AudioBitrateBox->addItems({"64 kbps", "128 kbps", "192 kbps", "256 kbps", "320 kbps", "384 kbps", "448 kbps", "512 kbps"});
        vp9AudioBitrateBox->setCurrentIndex(1);
        l->addWidget(bitrateLbl);
        l->addWidget(vp9AudioBitrateBox);
        vp9VbrModeLabel = new QLabel("VBR Mode:");
        vp9VbrModeBox = new QComboBox();
        vp9VbrModeBox->addItems({"Default", "Constrained", "Off"});
        vp9VbrModeBox->setCurrentIndex(0);
        vp9VbrModeBox->setToolTip("For Opus: Constrained limits variance.");
        l->addWidget(vp9VbrModeLabel);
        l->addWidget(vp9VbrModeBox);
        vp9VorbisQualityLabel = new QLabel("Quality Level:");
        vp9VorbisQualityBox = new QComboBox();
        vp9VorbisQualityBox->addItems({"Disabled", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"});
        vp9VorbisQualityBox->setCurrentIndex(0);
        vp9VorbisQualityBox->setToolTip("For Vorbis: Higher = better (0-10).");
        l->addWidget(vp9VorbisQualityLabel);
        l->addWidget(vp9VorbisQualityBox);
        vp9Layout->addLayout(l);
    }
    // Reset to defaults button
    {
        QHBoxLayout *l = new QHBoxLayout();
        QPushButton *resetButton = new QPushButton("Reset to Defaults");
        resetButton->setToolTip("Reset all VP9 settings.");
        l->addStretch();
        l->addWidget(resetButton);
        vp9Layout->addLayout(l);
        QObject::connect(resetButton, &QPushButton::clicked, this, &Vp9Tab::resetDefaults);
    }
    // Hooking up the signals
    vp9RCModeBox->setEnabled(false);
    vp9TwoPassCheck->setEnabled(false);
    vp9TwoPassCheck->setChecked(false);
    // Toggling rate control widgets
    QObject::connect(vp9EnableRCModeCheck, &QCheckBox::toggled, [this](bool on){
        vp9RCModeBox->setEnabled(on);
        if (on) {
            QString m = vp9RCModeBox->currentText();
            vp9CRFConfigWidget->setVisible(m == "CRF");
            vp9BitrateConfigWidget->setVisible(m == "ABR" || m == "CBR");
            vp9TwoPassCheck->setEnabled(true);
        } else {
            vp9CRFConfigWidget->setVisible(false);
            vp9BitrateConfigWidget->setVisible(false);
            vp9TwoPassCheck->setEnabled(false);
            vp9TwoPassCheck->setChecked(false);
        }
    });
    QObject::connect(vp9RCModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](){
        if (vp9EnableRCModeCheck->isChecked()) {
            QString m = vp9RCModeBox->currentText();
            vp9CRFConfigWidget->setVisible(m == "CRF");
            vp9BitrateConfigWidget->setVisible(m == "ABR" || m == "CBR");
        }
    });
    // Enabling sliders for filters
    QObject::connect(vp9LookaheadCheck, &QCheckBox::toggled,
                     [this](bool on){ vp9LookaheadSlider->setEnabled(on); });
    QObject::connect(vp9UnsharpenCheck, &QCheckBox::toggled,
                     [this](bool on){ vp9UnsharpenStrengthSlider->setEnabled(on); });
    QObject::connect(vp9SharpenCheck, &QCheckBox::toggled,
                     [this](bool on){ vp9SharpenStrengthSlider->setEnabled(on); });
    QObject::connect(vp9BlurCheck, &QCheckBox::toggled,
                     [this](bool on){ vp9BlurStrengthSlider->setEnabled(on); });
    QObject::connect(vp9NoiseReductionCheck, &QCheckBox::toggled,
                     [this](bool on){ vp9NoiseReductionSlider->setEnabled(on); });
    QObject::connect(vp9GrainSynthCheck, &QCheckBox::toggled,
                     [this](bool on){ vp9GrainSynthLevel->setEnabled(on); });
    // Enabling AQ strength
    QObject::connect(vp9AQModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](){
        QString m = vp9AQModeBox->currentText();
        vp9AQStrengthSlider->setEnabled(m == "Variance" || m == "Complexity");
    });
    // Showing audio options based on codec
    QObject::connect(vp9AudioCodecBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](){
        QString c = vp9AudioCodecBox->currentText();
        vp9VbrModeLabel->setVisible(c == "opus");
        vp9VbrModeBox->setVisible(c == "opus");
        vp9VorbisQualityLabel->setVisible(c == "vorbis");
        vp9VorbisQualityBox->setVisible(c == "vorbis");
    });
    // Updating for container change
    QObject::connect(vp9ContainerBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Vp9Tab::updateAudioCodecOptions);
    updateAudioCodecOptions();
}
void Vp9Tab::updateAudioCodecOptions() {
    QString cont = vp9ContainerBox->currentText();
    QString cur = vp9AudioCodecBox->currentText();
    vp9AudioCodecBox->clear();
    vp9AudioCodecBox->addItems({"opus", "vorbis"});
    int idx = vp9AudioCodecBox->findText(cur);
    vp9AudioCodecBox->setCurrentIndex(idx != -1 ? idx : 0);
    emit vp9AudioCodecBox->currentIndexChanged(vp9AudioCodecBox->currentIndex());
}
void Vp9Tab::resetDefaults() {
    vp9ContainerBox->setCurrentIndex(1);
    vp9EnableRCModeCheck->setChecked(false);
    vp9RCModeBox->setCurrentIndex(0);
    vp9CRFSlider->setValue(28);
    vp9BitrateSlider->setValue(1000);
    vp9LookaheadCheck->setChecked(false);
    vp9LookaheadSlider->setValue(16);
    vp9UnsharpenCheck->setChecked(false);
    vp9UnsharpenStrengthSlider->setValue(1);
    vp9SharpenCheck->setChecked(false);
    vp9SharpenStrengthSlider->setValue(1);
    vp9BlurCheck->setChecked(false);
    vp9BlurStrengthSlider->setValue(1);
    vp9NoiseReductionCheck->setChecked(false);
    vp9NoiseReductionSlider->setValue(5);
    vp9GrainSynthCheck->setChecked(false);
    vp9GrainSynthLevel->setValue(0);
    vp9TwoPassCheck->setChecked(false);
    vp9AQModeBox->setCurrentIndex(0);
    vp9AQStrengthSlider->setValue(6);
    vp9KeyIntBox->setCurrentIndex(4);
    vp9CpuUsedBox->setCurrentIndex(4);
    vp9ThreadsBox->setCurrentIndex(0);
    vp9TileColumnsBox->setCurrentIndex(0);
    vp9TileRowsBox->setCurrentIndex(0);
    vp9DeadlineBox->setCurrentIndex(0);
    vp9AudioCheck->setChecked(true);
    vp9AudioCodecBox->setCurrentIndex(0);
    vp9AudioSampleRateBox->setCurrentIndex(4);
    vp9AudioBitrateBox->setCurrentIndex(1);
    vp9VbrModeBox->setCurrentIndex(0);
    vp9VorbisQualityBox->setCurrentIndex(0);
    enableRowMtCheck->setChecked(false);
    screenContentCheck->setChecked(false);
    // Refresh the UI after reset
    vp9EnableRCModeCheck->toggled(false);
    vp9AQModeBox->currentIndexChanged(0);
    updateAudioCodecOptions();
}
