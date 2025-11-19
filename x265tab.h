#ifndef X265TAB_H
#define X265TAB_H
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QGroupBox>
#include <QPushButton>
class X265Tab : public QWidget {
    Q_OBJECT
public:
    explicit X265Tab(QWidget *parent = nullptr);
    QComboBox *x265ContainerBox;
    QComboBox *x265PresetBox;
    QCheckBox *x265EnableRCModeCheck;
    QComboBox *x265RCModeBox;
    QSlider *x265CBRBitrateSlider;
    QSlider *x265CRFSlider;
    QSlider *x265QPSlider;
    QSlider *x265ABRBitrateSlider;
    QCheckBox *x265ABRVBVCheck;
    QComboBox *x265TuneBox;
    QComboBox *x265LevelBox;
    QCheckBox *x265LookaheadCheck;
    QSlider *x265LookaheadSlider;
    QCheckBox *x265UnsharpenCheck;
    QSlider *x265UnsharpenStrengthSlider;
    QCheckBox *x265SharpenCheck;
    QSlider *x265SharpenStrengthSlider;
    QCheckBox *x265BlurCheck;
    QSlider *x265BlurStrengthSlider;
    QCheckBox *x265NoiseReductionCheck;
    QSlider *x265NoiseReductionSlider;
    QCheckBox *x265GrainSynthCheck;
    QSlider *x265GrainSynthLevel;
    QCheckBox *x265TwoPassCheck;
    QComboBox *x265AQModeBox;
    QSlider *x265AQStrengthSlider;
    QComboBox *x265KeyIntBox;
    QComboBox *x265ThreadsBox;
    QComboBox *x265FrameThreadsBox;
    QCheckBox *x265AudioCheck;
    QComboBox *x265AudioCodecBox;
    QComboBox *x265AudioSampleRateBox;
    QComboBox *x265AudioBitrateBox;
    QComboBox *x265VbrModeBox;
    QComboBox *x265AacQualityBox;
    QComboBox *x265Mp3VbrBox;
    QComboBox *x265FlacCompressionBox;
    QComboBox *x265VorbisQualityBox;
    QCheckBox *strongIntraCheck;
    QComboBox *rdoqLevelBox;
    QCheckBox *saoCheck;
    QComboBox *limitRefsBox;
    QCheckBox *enablePsyRdCheck;
    QCheckBox *enableCutreeCheck;
private:
    QWidget *x265CBRConfigWidget;
    QWidget *x265CRFConfigWidget;
    QWidget *x265QPConfigWidget;
    QWidget *x265ABRConfigWidget;
    QLabel *x265VbrModeLabel;
    QLabel *x265AacQualityLabel;
    QLabel *x265Mp3VbrLabel;
    QLabel *x265FlacCompressionLabel;
    QLabel *x265VorbisQualityLabel;
private slots:
    void updateAudioCodecOptions();
    void resetDefaults();
};
#endif // X265TAB_H
