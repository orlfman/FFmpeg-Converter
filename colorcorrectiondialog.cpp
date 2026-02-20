#include "colorcorrectiondialog.h"

ColorCorrectionDialog::ColorCorrectionDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Color Correction");
    setMinimumSize(740, 720);
    resize(740, 720);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *basicGroup = new QGroupBox("Basic Correction");
    QVBoxLayout *basicLay = new QVBoxLayout(basicGroup);

    // Brightness
    QHBoxLayout *bLay = new QHBoxLayout();
    bLay->addWidget(new QLabel("Brightness"));
    brightnessSlider = new QSlider(Qt::Horizontal); brightnessSlider->setRange(-100, 100); brightnessSlider->setValue(0);
    brightnessSpin = new QDoubleSpinBox(); brightnessSpin->setRange(-1.0, 1.0); brightnessSpin->setSingleStep(0.01); brightnessSpin->setValue(0.0);
    bLay->addWidget(brightnessSlider);
    bLay->addWidget(brightnessSpin);
    basicLay->addLayout(bLay);

    // Contrast
    QHBoxLayout *cLay = new QHBoxLayout();
    cLay->addWidget(new QLabel("Contrast"));
    contrastSlider = new QSlider(Qt::Horizontal); contrastSlider->setRange(0, 300); contrastSlider->setValue(100);
    contrastSpin = new QDoubleSpinBox(); contrastSpin->setRange(0.0, 3.0); contrastSpin->setSingleStep(0.01); contrastSpin->setValue(1.0);
    cLay->addWidget(contrastSlider);
    cLay->addWidget(contrastSpin);
    basicLay->addLayout(cLay);

    // Saturation
    QHBoxLayout *sLay = new QHBoxLayout();
    sLay->addWidget(new QLabel("Saturation"));
    saturationSlider = new QSlider(Qt::Horizontal); saturationSlider->setRange(0, 300); saturationSlider->setValue(100);
    saturationSpin = new QDoubleSpinBox(); saturationSpin->setRange(0.0, 3.0); saturationSpin->setSingleStep(0.01); saturationSpin->setValue(1.0);
    sLay->addWidget(saturationSlider);
    sLay->addWidget(saturationSpin);
    basicLay->addLayout(sLay);

    // Gamma
    QHBoxLayout *gLay = new QHBoxLayout();
    gLay->addWidget(new QLabel("Gamma"));
    gammaSlider = new QSlider(Qt::Horizontal); gammaSlider->setRange(1, 300); gammaSlider->setValue(100);
    gammaSpin = new QDoubleSpinBox(); gammaSpin->setRange(0.1, 3.0); gammaSpin->setSingleStep(0.01); gammaSpin->setValue(1.0);
    gLay->addWidget(gammaSlider);
    gLay->addWidget(gammaSpin);
    basicLay->addLayout(gLay);

    // Hue
    QHBoxLayout *hLay = new QHBoxLayout();
    hLay->addWidget(new QLabel("Hue"));
    hueSlider = new QSlider(Qt::Horizontal); hueSlider->setRange(-180, 180); hueSlider->setValue(0);
    hueSpin = new QDoubleSpinBox(); hueSpin->setRange(-180, 180); hueSpin->setSingleStep(1); hueSpin->setValue(0);
    hLay->addWidget(hueSlider);
    hLay->addWidget(hueSpin);
    basicLay->addLayout(hLay);

    mainLayout->addWidget(basicGroup);

    QGroupBox *advGroup = new QGroupBox("Advanced");
    QVBoxLayout *advLay = new QVBoxLayout(advGroup);

    // Vibrance
    QHBoxLayout *vLay = new QHBoxLayout();
    vLay->addWidget(new QLabel("Vibrance"));
    vibranceSlider = new QSlider(Qt::Horizontal); vibranceSlider->setRange(0, 200); vibranceSlider->setValue(100);
    vibranceSpin = new QDoubleSpinBox(); vibranceSpin->setRange(0.0, 2.0); vibranceSpin->setSingleStep(0.01); vibranceSpin->setValue(1.0);
    vLay->addWidget(vibranceSlider);
    vLay->addWidget(vibranceSpin);
    advLay->addLayout(vLay);

    // Color Temperature
    QHBoxLayout *tLay = new QHBoxLayout();
    tLay->addWidget(new QLabel("Color Temperature (K)"));
    temperatureSlider = new QSlider(Qt::Horizontal); temperatureSlider->setRange(2500, 8500); temperatureSlider->setValue(5500);
    temperatureSpin = new QDoubleSpinBox(); temperatureSpin->setRange(2500, 8500); temperatureSpin->setSingleStep(100); temperatureSpin->setValue(5500);
    tLay->addWidget(temperatureSlider);
    tLay->addWidget(temperatureSpin);
    advLay->addLayout(tLay);

    // Curves
    QHBoxLayout *curvesLay = new QHBoxLayout();
    curvesLay->addWidget(new QLabel("Curves Preset"));
    curvesBox = new QComboBox();
    curvesBox->addItems({"None", "Cross Process", "Vintage", "Darker", "Lighter", "Linear Contrast", "Medium Contrast", "Strong Contrast", "Increase Contrast", "Negative"});
    curvesLay->addWidget(curvesBox);
    advLay->addLayout(curvesLay);

    mainLayout->addWidget(advGroup);

    QHBoxLayout *btnLay = new QHBoxLayout();
    resetButton = new QPushButton("Reset All");
    okButton = new QPushButton("OK");
    cancelButton = new QPushButton("Cancel");
    btnLay->addWidget(resetButton);
    btnLay->addStretch();
    btnLay->addWidget(okButton);
    btnLay->addWidget(cancelButton);
    mainLayout->addLayout(btnLay);

    auto sync = [](QSlider* s, QDoubleSpinBox* sp) {
        connect(s, &QSlider::valueChanged, sp, [sp](int v){ sp->setValue(v/100.0); });
        connect(sp, QOverload<double>::of(&QDoubleSpinBox::valueChanged), s, [s](double v){ s->setValue(static_cast<int>(v*100)); });
    };
    sync(brightnessSlider, brightnessSpin);
    sync(contrastSlider, contrastSpin);
    sync(saturationSlider, saturationSpin);
    sync(gammaSlider, gammaSpin);
    sync(vibranceSlider, vibranceSpin);

    connect(temperatureSlider, &QSlider::valueChanged, temperatureSpin, [this](int v){ temperatureSpin->setValue(v); });
    connect(temperatureSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), temperatureSlider, [this](double v){ temperatureSlider->setValue(static_cast<int>(v)); });

    connect(resetButton, &QPushButton::clicked, this, &ColorCorrectionDialog::resetToDefaults);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void ColorCorrectionDialog::resetToDefaults()
{
    brightnessSlider->setValue(0); contrastSlider->setValue(100);
    saturationSlider->setValue(100); gammaSlider->setValue(100); hueSlider->setValue(0);
    vibranceSlider->setValue(100); temperatureSlider->setValue(5500);
    curvesBox->setCurrentIndex(0);
}

QString ColorCorrectionDialog::getFilterString() const
{
    QStringList filters;

    filters << QString("eq=brightness=%1:contrast=%2:saturation=%3:gamma=%4")
    .arg(brightnessSpin->value(), 0, 'f', 2)
    .arg(contrastSpin->value(),   0, 'f', 2)
    .arg(saturationSpin->value(), 0, 'f', 2)
    .arg(gammaSpin->value(),      0, 'f', 2);

    if (!qFuzzyCompare(hueSpin->value(), 0.0))
        filters << QString("hue=h=%1").arg(hueSpin->value(), 0, 'f', 1);

    // Vibrance
    if (!qFuzzyCompare(vibranceSpin->value(), 1.0))
        filters << QString("vibrance=%1").arg(vibranceSpin->value(), 0, 'f', 2);

    // Color Temperature
    if (temperatureSlider->value() != 5500)
        filters << QString("colortemperature=temperature=%1").arg(temperatureSlider->value());

    // Curves
    QString curve = curvesBox->currentText();
    if (curve != "None") {
        QString preset;
        if (curve == "Cross Process") preset = "cross_process";
        else if (curve == "Vintage") preset = "vintage";
        else if (curve == "Darker") preset = "darker";
        else if (curve == "Lighter") preset = "lighter";
        else if (curve == "Linear Contrast") preset = "linear_contrast";
        else if (curve == "Medium Contrast") preset = "medium_contrast";
        else if (curve == "Strong Contrast") preset = "strong_contrast";
        else if (curve == "Increase Contrast") preset = "increase_contrast";
        else if (curve == "Negative") preset = "negative";

        if (!preset.isEmpty())
            filters << QString("curves=preset=%1").arg(preset);
    }

    return filters.join(",");
}
