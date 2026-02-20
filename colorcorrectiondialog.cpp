#include "colorcorrectiondialog.h"

ColorCorrectionDialog::ColorCorrectionDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Color Correction");
    setMinimumSize(740, 720);
    resize(740, 720);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    QGroupBox *basicGroup = new QGroupBox("Basic Correction");
    QVBoxLayout *basicLay = new QVBoxLayout(basicGroup);
    basicLay->setSpacing(10);

    auto createRow = [this, &basicLay](const QString &labelText, QCheckBox*& enableBox,
          QSlider*& slider, QDoubleSpinBox*& spinBox,
          int sMin, int sMax, int sDefault,
          double spMin, double spMax, double spDefault, double spStep) {

        QHBoxLayout *rowLayout = new QHBoxLayout();
        rowLayout->setSpacing(10);

        enableBox = new QCheckBox(labelText);
        enableBox->setChecked(false);
        enableBox->setMinimumWidth(135);

        slider = new QSlider(Qt::Horizontal);
        slider->setRange(sMin, sMax);
        slider->setValue(sDefault);

        spinBox = new QDoubleSpinBox();
        spinBox->setRange(spMin, spMax);
        spinBox->setDecimals(2);
        spinBox->setSingleStep(spStep);
        spinBox->setValue(spDefault);

        rowLayout->addWidget(enableBox);
        rowLayout->addWidget(slider, 1);
        rowLayout->addWidget(spinBox);

        basicLay->addLayout(rowLayout);

        // Sync slider spinbox
        connect(slider, &QSlider::valueChanged, spinBox, [spinBox](int val) {
            spinBox->setValue(val / 100.0);
        });
        connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), slider, [slider](double val) {
            slider->setValue(static_cast<int>(val * 100));
        });

        // Enable/disable
        connect(enableBox, &QCheckBox::toggled, this, [slider, spinBox](bool enabled) {
            slider->setEnabled(enabled);
            spinBox->setEnabled(enabled);
        });
    };

    createRow("Brightness", brightnessEnable, brightnessSlider, brightnessSpin, -100, 100, 0, -1.0, 1.0, 0.0, 0.01);
    createRow("Contrast",   contrastEnable,   contrastSlider,   contrastSpin,   0, 300, 100, 0.0, 3.0, 1.0, 0.01);
    createRow("Saturation", saturationEnable, saturationSlider, saturationSpin, 0, 300, 100, 0.0, 3.0, 1.0, 0.01);
    createRow("Gamma",      gammaEnable,      gammaSlider,      gammaSpin,      1, 300, 100, 0.1, 3.0, 1.0, 0.01);

    // Hue
    QHBoxLayout *hLay = new QHBoxLayout();
    hueEnable = new QCheckBox("Hue");
    hueEnable->setChecked(false);
    hueEnable->setMinimumWidth(135);
    hLay->addWidget(hueEnable);

    hueSlider = new QSlider(Qt::Horizontal); hueSlider->setRange(-180, 180); hueSlider->setValue(0);
    hueSpin   = new QDoubleSpinBox(); hueSpin->setRange(-180, 180); hueSpin->setSingleStep(1); hueSpin->setValue(0);
    hLay->addWidget(hueSlider);
    hLay->addWidget(hueSpin);
    basicLay->addLayout(hLay);

    connect(hueEnable, &QCheckBox::toggled, this, [this](bool en){
        hueSlider->setEnabled(en);
        hueSpin->setEnabled(en);
    });
    connect(hueSlider, &QSlider::valueChanged, hueSpin, [this](int v){ hueSpin->setValue(v); });
    connect(hueSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), hueSlider, [this](double v){ hueSlider->setValue(static_cast<int>(v)); });

    mainLayout->addWidget(basicGroup);

    // ====================== ADVANCED ======================
    QGroupBox *advGroup = new QGroupBox("Advanced");
    QVBoxLayout *advLay = new QVBoxLayout(advGroup);
    advLay->setSpacing(10);

    // Vibrance
    QHBoxLayout *vLay = new QHBoxLayout();
    vibranceEnable = new QCheckBox("Vibrance");
    vibranceEnable->setChecked(false);
    vibranceEnable->setMinimumWidth(135);
    vLay->addWidget(vibranceEnable);

    vibranceSlider = new QSlider(Qt::Horizontal); vibranceSlider->setRange(0, 200); vibranceSlider->setValue(100);
    vibranceSpin   = new QDoubleSpinBox(); vibranceSpin->setRange(0.0, 2.0); vibranceSpin->setValue(1.0); vibranceSpin->setSingleStep(0.01);
    vLay->addWidget(vibranceSlider);
    vLay->addWidget(vibranceSpin);
    advLay->addLayout(vLay);

    connect(vibranceEnable, &QCheckBox::toggled, this, [this](bool en){
        vibranceSlider->setEnabled(en);
        vibranceSpin->setEnabled(en);
    });
    connect(vibranceSlider, &QSlider::valueChanged, vibranceSpin, [this](int v){ vibranceSpin->setValue(v/100.0); });
    connect(vibranceSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), vibranceSlider, [this](double v){ vibranceSlider->setValue(static_cast<int>(v*100)); });

    // Temperature
    QHBoxLayout *tLay = new QHBoxLayout();
    temperatureEnable = new QCheckBox("Color Temperature (K)");
    temperatureEnable->setChecked(false);
    temperatureEnable->setMinimumWidth(135);
    tLay->addWidget(temperatureEnable);

    temperatureSlider = new QSlider(Qt::Horizontal); temperatureSlider->setRange(2500, 8500); temperatureSlider->setValue(5500);
    temperatureSpin = new QDoubleSpinBox(); temperatureSpin->setRange(2500, 8500); temperatureSpin->setSingleStep(100); temperatureSpin->setValue(5500);
    tLay->addWidget(temperatureSlider);
    tLay->addWidget(temperatureSpin);
    advLay->addLayout(tLay);

    connect(temperatureEnable, &QCheckBox::toggled, this, [this](bool en){
        temperatureSlider->setEnabled(en);
        temperatureSpin->setEnabled(en);
    });
    connect(temperatureSlider, &QSlider::valueChanged, temperatureSpin, [this](int v){ temperatureSpin->setValue(v); });
    connect(temperatureSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), temperatureSlider, [this](double v){ temperatureSlider->setValue(static_cast<int>(v)); });

    // Curves
    QHBoxLayout *curvesLay = new QHBoxLayout();
    curvesEnable = new QCheckBox("Curves Preset");
    curvesEnable->setChecked(false);
    curvesEnable->setMinimumWidth(135);
    curvesLay->addWidget(curvesEnable);

    curvesBox = new QComboBox();
    curvesBox->addItems({"None", "Cross Process", "Vintage", "Darker", "Lighter", 
                         "Linear Contrast", "Medium Contrast", "Strong Contrast", 
                         "Increase Contrast", "Negative"});
    curvesLay->addWidget(curvesBox);
    advLay->addLayout(curvesLay);

    connect(curvesEnable, &QCheckBox::toggled, curvesBox, &QComboBox::setEnabled);

    mainLayout->addWidget(advGroup);

    // Buttons
    QHBoxLayout *btnLay = new QHBoxLayout();
    resetButton = new QPushButton("Reset All to Defaults");
    okButton = new QPushButton("OK");
    cancelButton = new QPushButton("Cancel");

    btnLay->addWidget(resetButton);
    btnLay->addStretch();
    btnLay->addWidget(okButton);
    btnLay->addWidget(cancelButton);
    mainLayout->addLayout(btnLay);

    connect(resetButton, &QPushButton::clicked, this, &ColorCorrectionDialog::resetToDefaults);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    resetToDefaults();
    updateWidgetEnabledStates();
}

void ColorCorrectionDialog::resetToDefaults()
{
    brightnessEnable->setChecked(false);
    contrastEnable->setChecked(false);
    saturationEnable->setChecked(false);
    gammaEnable->setChecked(false);
    hueEnable->setChecked(false);

    vibranceEnable->setChecked(false);
    temperatureEnable->setChecked(false);
    curvesEnable->setChecked(false);

    brightnessSlider->setValue(0);   brightnessSpin->setValue(0.0);
    contrastSlider->setValue(100);   contrastSpin->setValue(1.0);
    saturationSlider->setValue(100); saturationSpin->setValue(1.0);
    gammaSlider->setValue(100);      gammaSpin->setValue(1.0);
    hueSlider->setValue(0);          hueSpin->setValue(0);

    vibranceSlider->setValue(100);   vibranceSpin->setValue(1.0);
    temperatureSlider->setValue(5500); temperatureSpin->setValue(5500);
    curvesBox->setCurrentIndex(0);

    updateWidgetEnabledStates();
}

QString ColorCorrectionDialog::getFilterString() const
{
    QStringList filters;

    // EQ FILTER
    QStringList eqParams;
    if (brightnessEnable->isChecked())
        eqParams << QString("brightness=%1").arg(brightnessSpin->value(), 0, 'f', 2);
    if (contrastEnable->isChecked())
        eqParams << QString("contrast=%1").arg(contrastSpin->value(), 0, 'f', 2);
    if (saturationEnable->isChecked())
        eqParams << QString("saturation=%1").arg(saturationSpin->value(), 0, 'f', 2);
    if (gammaEnable->isChecked())
        eqParams << QString("gamma=%1").arg(gammaSpin->value(), 0, 'f', 2);

    if (!eqParams.isEmpty())
        filters << "eq=" + eqParams.join(":");

    // HUE HUE HUE HEW
    if (hueEnable->isChecked() && !qFuzzyCompare(hueSpin->value(), 0.0))
        filters << QString("hue=h=%1").arg(hueSpin->value(), 0, 'f', 1);

    // VIBRANCE oh MY that BULGE
    if (vibranceEnable->isChecked() && !qFuzzyCompare(vibranceSpin->value(), 1.0))
        filters << QString("vibrance=%1").arg(vibranceSpin->value(), 0, 'f', 2);

    // COLOR temp
    if (temperatureEnable->isChecked() && temperatureSlider->value() != 5500)
        filters << QString("colortemperature=temperature=%1").arg(temperatureSlider->value());

    // thick CURVES
    if (curvesEnable->isChecked()) {
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
    }

    return filters.join(",");
}

void ColorCorrectionDialog::updateWidgetEnabledStates()
{
    brightnessSlider->setEnabled(brightnessEnable->isChecked());
    brightnessSpin->setEnabled(brightnessEnable->isChecked());

    contrastSlider->setEnabled(contrastEnable->isChecked());
    contrastSpin->setEnabled(contrastEnable->isChecked());

    saturationSlider->setEnabled(saturationEnable->isChecked());
    saturationSpin->setEnabled(saturationEnable->isChecked());

    gammaSlider->setEnabled(gammaEnable->isChecked());
    gammaSpin->setEnabled(gammaEnable->isChecked());

    hueSlider->setEnabled(hueEnable->isChecked());
    hueSpin->setEnabled(hueEnable->isChecked());

    vibranceSlider->setEnabled(vibranceEnable->isChecked());
    vibranceSpin->setEnabled(vibranceEnable->isChecked());

    temperatureSlider->setEnabled(temperatureEnable->isChecked());
    temperatureSpin->setEnabled(temperatureEnable->isChecked());

    curvesBox->setEnabled(curvesEnable->isChecked());
}
