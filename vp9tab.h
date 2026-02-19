#ifndef VP9TAB_H
#define VP9TAB_H
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QGroupBox>
#include <QPushButton>
class Vp9Tab : public QWidget {
    Q_OBJECT
public:
    explicit Vp9Tab(QWidget *parent = nullptr);
    QComboBox *vp9ContainerBox;
    QCheckBox *vp9EnableRCModeCheck;
    QComboBox *vp9RCModeBox;
    QSlider *vp9CRFSlider;
    QSlider *vp9BitrateSlider;
    QCheckBox *vp9LookaheadCheck;
    QSlider *vp9LookaheadSlider;
    QCheckBox *vp9UnsharpenCheck;
    QSlider *vp9UnsharpenStrengthSlider;
    QCheckBox *vp9SharpenCheck;
    QSlider *vp9SharpenStrengthSlider;
    QCheckBox *vp9BlurCheck;
    QSlider *vp9BlurStrengthSlider;
    QCheckBox *vp9NoiseReductionCheck;
    QSlider *vp9NoiseReductionSlider;
    QCheckBox *vp9GrainSynthCheck;
    QSlider *vp9GrainSynthLevel;
    QCheckBox *vp9TwoPassCheck;
    QComboBox *vp9AQModeBox;
    QSlider *vp9AQStrengthSlider;
    QComboBox *vp9KeyIntBox;
    QComboBox *vp9ThreadsBox;
    QComboBox *vp9TileRowsBox;
    QComboBox *vp9CpuUsedBox;
    QComboBox *vp9TileColumnsBox;
    QComboBox *vp9DeadlineBox;
    QCheckBox *vp9AudioCheck;
    QComboBox *vp9AudioCodecBox;
    QComboBox *vp9AudioSampleRateBox;
    QComboBox *vp9AudioBitrateBox;
    QComboBox *vp9VbrModeBox;
    QComboBox *vp9VorbisQualityBox;
    QCheckBox *enableRowMtCheck;
    QCheckBox *screenContentCheck;
    QCheckBox *vp9NlmeansCheck;
    QSlider *vp9NlmeansSigmaSSlider;
    QSlider *vp9NlmeansSigmaPSlider;
    QCheckBox *vp9NlmeansGpuCheck;
    QSlider *vp9NlmeansPatchSlider;
    QSlider *vp9QMaxSlider;
    QCheckBox *vp9ArnrCheck;
    QSlider *vp9ArnrStrengthSlider;
    QSlider *vp9ArnrMaxFramesSlider;
    QCheckBox *vp9TplCheck;
    QComboBox *vp9CustomKeyframeModeBox;
private:
    QWidget *vp9CRFConfigWidget;
    QWidget *vp9BitrateConfigWidget;
    QLabel *vp9VbrModeLabel;
    QLabel *vp9VorbisQualityLabel;
private slots:
    void updateAudioCodecOptions();
    void resetDefaults();
};
#endif // VP9TAB_H
