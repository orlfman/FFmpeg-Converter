#ifndef AV1TAB_H
#define AV1TAB_H
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QGroupBox>
#include <QPushButton>
class Av1Tab : public QWidget {
    Q_OBJECT
public:
    explicit Av1Tab(QWidget *parent = nullptr);
    // Public members for access from main.cpp
    QComboBox *av1ContainerBox;
    QComboBox *av1PresetBox;
    QCheckBox *av1EnableRCModeCheck;
    QComboBox *av1RCModeBox;
    QSlider *av1CRFSlider;
    QSlider *av1QPSlider;
    QSlider *av1VBRBitrateSlider;
    QCheckBox *av1VBRVBVCheck;
    QComboBox *av1TuneBox;
    QComboBox *av1LevelBox;
    QCheckBox *av1LookaheadCheck;
    QSlider *av1LookaheadSlider;
    QCheckBox *av1UnsharpenCheck;
    QSlider *av1UnsharpenStrengthSlider;
    QCheckBox *av1SharpenCheck;
    QSlider *av1SharpenStrengthSlider;
    QCheckBox *av1BlurCheck;
    QSlider *av1BlurStrengthSlider;
    QCheckBox *av1NoiseReductionCheck;
    QSlider *av1NoiseReductionSlider;
    QCheckBox *av1GrainSynthCheck;
    QSlider *av1GrainSynthLevel;
    QCheckBox *av1TwoPassCheck;
    QComboBox *av1AQModeBox;
    QSlider *av1AQStrengthSlider;
    QComboBox *av1KeyIntBox;
    QComboBox *av1ThreadsBox;
    QComboBox *av1TileRowsBox;
    QComboBox *av1TileColumnsBox;
    QCheckBox *av1AudioCheck;
    QComboBox *av1AudioCodecBox;
    QComboBox *av1AudioSampleRateBox;
    QComboBox *av1AudioBitrateBox;
    QComboBox *av1VbrModeBox;
    QComboBox *av1AacQualityBox;
    QComboBox *av1Mp3VbrBox;
    QComboBox *av1FlacCompressionBox;
    QComboBox *av1VorbisQualityBox;
    // 🔥 FIXED AV1-ONLY FEATURES
    QCheckBox *nativeGrainCheck;
    QSlider *grainStrengthSlider; // Renamed from grainDenoiseSlider for --film-grain (0-50)
    QComboBox *grainDenoiseCombo; // New: for --film-grain-denoise (0/1/2)
    QComboBox *superResModeBox;
    QSlider *superResDenomSlider; // Renamed from superResDenoiseSlider for --superres-denom (8-16)
    QCheckBox *lowLatencyCheck; // Will map to valid params in main.cpp
    QCheckBox *tplModelCheck;
    QCheckBox *enableCdefCheck;
    // New features
    QCheckBox *enableTfCheck; // New: Enable TF (temporal filtering)
    QComboBox *fastDecodeBox;
    QComboBox *screenContentModeBox;
private:
    QWidget *av1CRFConfigWidget;
    QWidget *av1QPConfigWidget;
    QWidget *av1VBRConfigWidget;
    QLabel *av1VbrModeLabel;
    QLabel *av1AacQualityLabel;
    QLabel *av1Mp3VbrLabel;
    QLabel *av1FlacCompressionLabel;
    QLabel *av1VorbisQualityLabel;
private slots:
    void updateAudioCodecOptions();
    void resetDefaults(); // New: Reset button slot
};
#endif // AV1TAB_H
