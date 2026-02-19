#ifndef X264TAB_H
#define X264TAB_H
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QGroupBox>
#include <QPushButton>

class X264Tab : public QWidget {
    Q_OBJECT
public:
    explicit X264Tab(QWidget *parent = nullptr);

    QComboBox *x264ContainerBox;
    QComboBox *x264PresetBox;
    QCheckBox *x264EnableRCModeCheck;
    QComboBox *x264RCModeBox;
    QSlider *x264CBRBitrateSlider;
    QSlider *x264CRFSlider;
    QSlider *x264QPSlider;
    QSlider *x264ABRBitrateSlider;
    QCheckBox *x264ABRVBVCheck;
    QComboBox *x264TuneBox;
    QComboBox *x264LevelBox;
    QComboBox *x264ProfileBox;
    QCheckBox *x264LookaheadCheck;
    QSlider *x264LookaheadSlider;
    QCheckBox *x264UnsharpenCheck;
    QSlider *x264UnsharpenStrengthSlider;
    QCheckBox *x264SharpenCheck;
    QSlider *x264SharpenStrengthSlider;
    QCheckBox *x264BlurCheck;
    QSlider *x264BlurStrengthSlider;
    QCheckBox *x264NoiseReductionCheck;
    QSlider *x264NoiseReductionSlider;
    QCheckBox *x264GrainSynthCheck;
    QSlider *x264GrainSynthLevel;
    QCheckBox *x264TwoPassCheck;
    QComboBox *x264AQModeBox;
    QSlider *x264AQStrengthSlider;
    QComboBox *x264KeyIntBox;
    QComboBox *x264ThreadsBox;
    QComboBox *x264FrameThreadsBox;
    QCheckBox *x264AudioCheck;
    QComboBox *x264AudioCodecBox;
    QComboBox *x264AudioSampleRateBox;
    QComboBox *x264AudioBitrateBox;
    QComboBox *x264VbrModeBox;
    QComboBox *x264AacQualityBox;
    QComboBox *x264Mp3VbrBox;
    QComboBox *x264FlacCompressionBox;
    QComboBox *x264VorbisQualityBox;
    QCheckBox *strongIntraCheck;
    QComboBox *rdoqLevelBox;
    QComboBox *limitRefsBox;

    QCheckBox *enablePsyRdCheck;
    QCheckBox *enableCutreeCheck;

    QGroupBox *deblockGroup;
    QSlider *deblockAlphaSlider;
    QSlider *deblockBetaSlider;
    QLabel *deblockAlphaLabel;
    QLabel *deblockBetaLabel;

    QComboBox *refFramesBox;
    QCheckBox *weightpCheck;

    QComboBox *x264CustomKeyframeModeBox;
    QWidget *x264CBRConfigWidget;
    QWidget *x264CRFConfigWidget;
    QWidget *x264QPConfigWidget;
    QWidget *x264ABRConfigWidget;

private:
    QLabel *x264VbrModeLabel;
    QLabel *x264AacQualityLabel;
    QLabel *x264Mp3VbrLabel;
    QLabel *x264FlacCompressionLabel;
    QLabel *x264VorbisQualityLabel;

private slots:
    void updateAudioCodecOptions();
    void resetDefaults();
};

#endif // X264TAB_H
