#include "colorcorrectiondialog.h"

ColorCorrectionDialog::ColorCorrectionDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Color Correction");
    setMinimumWidth(520);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Brightness
    QGroupBox *brightGroup = new QGroupBox("Brightness");
    QHBoxLayout *brightLay = new QHBoxLayout(brightGroup);
    brightnessSlider = new QSlider(Qt::Horizontal); brightnessSlider->setRange(-100, 100); brightnessSlider->setValue(0);
    brightnessSpin = new QDoubleSpinBox(); brightnessSpin->setRange(-1.0, 1.0); brightnessSpin->setSingleStep(0.01); brightnessSpin->setValue(0.0);
    brightLay->addWidget(brightnessSlider);
    brightLay->addWidget(brightnessSpin);
    mainLayout->addWidget(brightGroup);

    // Contrast
    QGroupBox *contGroup = new QGroupBox("Contrast");
    QHBoxLayout *contLay = new QHBoxLayout(contGroup);
    contrastSlider = new QSlider(Qt::Horizontal); contrastSlider->setRange(0, 300); contrastSlider->setValue(100);
    contrastSpin = new QDoubleSpinBox(); contrastSpin->setRange(0.0, 3.0); contrastSpin->setSingleStep(0.01); contrastSpin->setValue(1.0);
    contLay->addWidget(contrastSlider);
    contLay->addWidget(contrastSpin);
    mainLayout->addWidget(contGroup);

    // Saturation
    QGroupBox *satGroup = new QGroupBox("Saturation");
    QHBoxLayout *satLay = new QHBoxLayout(satGroup);
    saturationSlider = new QSlider(Qt::Horizontal); saturationSlider->setRange(0, 300); saturationSlider->setValue(100);
    saturationSpin = new QDoubleSpinBox(); saturationSpin->setRange(0.0, 3.0); saturationSpin->setSingleStep(0.01); saturationSpin->setValue(1.0);
    satLay->addWidget(saturationSlider);
    satLay->addWidget(saturationSpin);
    mainLayout->addWidget(satGroup);

    // Gamma
    QGroupBox *gammaGroup = new QGroupBox("Gamma");
    QHBoxLayout *gammaLay = new QHBoxLayout(gammaGroup);
    gammaSlider = new QSlider(Qt::Horizontal); gammaSlider->setRange(1, 300); gammaSlider->setValue(100);
    gammaSpin = new QDoubleSpinBox(); gammaSpin->setRange(0.1, 3.0); gammaSpin->setSingleStep(0.01); gammaSpin->setValue(1.0);
    gammaLay->addWidget(gammaSlider);
    gammaLay->addWidget(gammaSpin);
    mainLayout->addWidget(gammaGroup);

    // Hue
    QGroupBox *hueGroup = new QGroupBox("Hue (degrees)");
    QHBoxLayout *hueLay = new QHBoxLayout(hueGroup);
    hueSlider = new QSlider(Qt::Horizontal); hueSlider->setRange(-180, 180); hueSlider->setValue(0);
    hueSpin = new QDoubleSpinBox(); hueSpin->setRange(-180.0, 180.0); hueSpin->setSingleStep(1.0); hueSpin->setValue(0.0);
    hueLay->addWidget(hueSlider);
    hueLay->addWidget(hueSpin);
    mainLayout->addWidget(hueGroup);

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    resetButton = new QPushButton("Reset to Defaults");
    okButton = new QPushButton("OK");
    cancelButton = new QPushButton("Cancel");
    btnLayout->addWidget(resetButton);
    btnLayout->addStretch();
    btnLayout->addWidget(okButton);
    btnLayout->addWidget(cancelButton);
    mainLayout->addLayout(btnLayout);

    // Connections
    connect(brightnessSlider, &QSlider::valueChanged, this, [this](int v){ brightnessSpin->setValue(v/100.0); });
    connect(brightnessSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double v){ brightnessSlider->setValue(static_cast<int>(v*100)); });

    connect(contrastSlider, &QSlider::valueChanged, this, [this](int v){ contrastSpin->setValue(v/100.0); });
    connect(contrastSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double v){ contrastSlider->setValue(static_cast<int>(v*100)); });

    connect(saturationSlider, &QSlider::valueChanged, this, [this](int v){ saturationSpin->setValue(v/100.0); });
    connect(saturationSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double v){ saturationSlider->setValue(static_cast<int>(v*100)); });

    connect(gammaSlider, &QSlider::valueChanged, this, [this](int v){ gammaSpin->setValue(v/100.0); });
    connect(gammaSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double v){ gammaSlider->setValue(static_cast<int>(v*100)); });

    connect(hueSlider, &QSlider::valueChanged, this, [this](int v){ hueSpin->setValue(v); });
    connect(hueSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double v){ hueSlider->setValue(static_cast<int>(v)); });

    connect(resetButton, &QPushButton::clicked, this, &ColorCorrectionDialog::resetToDefaults);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void ColorCorrectionDialog::resetToDefaults()
{
    brightnessSlider->setValue(0); contrastSlider->setValue(100);
    saturationSlider->setValue(100); gammaSlider->setValue(100); hueSlider->setValue(0);
}

QString ColorCorrectionDialog::getFilterString() const
{
    double b = brightnessSpin->value();
    double c = contrastSpin->value();
    double s = saturationSpin->value();
    double g = gammaSpin->value();
    double h = hueSpin->value();

    QString eq = QString("eq=brightness=%1:contrast=%2:saturation=%3:gamma=%4")
    .arg(b, 0, 'f', 2)
    .arg(c, 0, 'f', 2)
    .arg(s, 0, 'f', 2)
    .arg(g, 0, 'f', 2);

    if (!qFuzzyCompare(h, 0.0)) {
        return eq + QString(",hue=h=%1").arg(h, 0, 'f', 1);
    }
    return eq;
}
