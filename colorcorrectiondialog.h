#ifndef COLORCORRECTIONDIALOG_H
#define COLORCORRECTIONDIALOG_H

#include <QDialog>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>

class ColorCorrectionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ColorCorrectionDialog(QWidget *parent = nullptr);

    QString getFilterString() const;

private slots:
    void resetToDefaults();

private:
    QSlider *brightnessSlider = nullptr;
    QDoubleSpinBox *brightnessSpin = nullptr;

    QSlider *contrastSlider = nullptr;
    QDoubleSpinBox *contrastSpin = nullptr;

    QSlider *saturationSlider = nullptr;
    QDoubleSpinBox *saturationSpin = nullptr;

    QSlider *gammaSlider = nullptr;
    QDoubleSpinBox *gammaSpin = nullptr;

    QSlider *hueSlider = nullptr;
    QDoubleSpinBox *hueSpin = nullptr;

    QSlider *vibranceSlider = nullptr;
    QDoubleSpinBox *vibranceSpin = nullptr;

    QSlider *temperatureSlider = nullptr;
    QDoubleSpinBox *temperatureSpin = nullptr;

    QComboBox *curvesBox = nullptr;

    QPushButton *resetButton = nullptr;
    QPushButton *okButton = nullptr;
    QPushButton *cancelButton = nullptr;
};

#endif // COLORCORRECTIONDIALOG_H
