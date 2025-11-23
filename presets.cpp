#include "presets.h"
#include <QObject>

void Presets::connectPresets(
    QComboBox* presetCombo,
    QTabWidget* codecTabs,
    Av1Tab* av1Tab,
    X265Tab* x265Tab,
    Vp9Tab* vp9Tab,
    QCheckBox* eightBitCheck,
    QComboBox* eightBitColorFormatBox,
    QCheckBox* tenBitCheck,
    QComboBox* colorFormatBox)
{
    auto applyPreset = [presetCombo, codecTabs, av1Tab, x265Tab, vp9Tab, eightBitCheck, eightBitColorFormatBox, tenBitCheck, colorFormatBox]() {
        int p = presetCombo->currentIndex();
        if (p == 0) return;  // Custom

        int currentTab = codecTabs->currentIndex();

        if (currentTab == 0) {  // AV1
            switch (p) {
            case 1: // Streaming
                av1Tab->av1ContainerBox->setCurrentText("webm");
                av1Tab->av1AudioCheck->setChecked(true);
                av1Tab->av1AudioCodecBox->setCurrentText("opus");
                av1Tab->av1AudioSampleRateBox->setCurrentText("48 KHz");
                av1Tab->av1EnableRCModeCheck->setChecked(true);
                av1Tab->av1RCModeBox->setCurrentText("CRF");
                av1Tab->av1PresetBox->setCurrentText("9");
                av1Tab->av1CRFSlider->setValue(30);
                av1Tab->av1TuneBox->setCurrentText("Subjective SSIM (VQ)");
                av1Tab->av1KeyIntBox->setCurrentText("120");
                av1Tab->av1AudioBitrateBox->setCurrentText("192 kbps");
                av1Tab->av1VbrModeBox->setCurrentText("Constrained");
                eightBitCheck->setChecked(true);
                eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                av1Tab->tplModelCheck->setChecked(true);
                av1Tab->enableTfCheck->setChecked(true);
                av1Tab->enableCdefCheck->setChecked(true);
                av1Tab->av1LookaheadCheck->setChecked(true);
                av1Tab->av1LookaheadSlider->setValue(60);
                av1Tab->fastDecodeBox->setCurrentIndex(2);
                av1Tab->av1SharpenCheck->setChecked(false);
                break;

            case 2: // Medium
                av1Tab->av1ContainerBox->setCurrentText("webm");
                av1Tab->av1AudioCheck->setChecked(true);
                av1Tab->av1AudioCodecBox->setCurrentText("opus");
                av1Tab->av1AudioSampleRateBox->setCurrentText("48 KHz");
                av1Tab->av1EnableRCModeCheck->setChecked(true);
                av1Tab->av1RCModeBox->setCurrentText("CRF");
                av1Tab->av1PresetBox->setCurrentText("10");
                av1Tab->av1CRFSlider->setValue(37);
                av1Tab->av1TuneBox->setCurrentText("PSNR");
                av1Tab->av1KeyIntBox->setCurrentText("240");
                av1Tab->av1AudioBitrateBox->setCurrentText("128 kbps");
                av1Tab->av1VbrModeBox->setCurrentText("Default");
                eightBitCheck->setChecked(true);
                eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                av1Tab->tplModelCheck->setChecked(false);
                av1Tab->enableTfCheck->setChecked(false);
                av1Tab->enableCdefCheck->setChecked(false);
                av1Tab->av1LookaheadCheck->setChecked(false);
                av1Tab->av1SharpenCheck->setChecked(false);
                av1Tab->fastDecodeBox->setCurrentIndex(0);
                break;

            case 3: // High
                av1Tab->av1ContainerBox->setCurrentText("webm");
                av1Tab->av1AudioCheck->setChecked(true);
                av1Tab->av1AudioCodecBox->setCurrentText("opus");
                av1Tab->av1AudioSampleRateBox->setCurrentText("48 KHz");
                av1Tab->av1EnableRCModeCheck->setChecked(true);
                av1Tab->av1RCModeBox->setCurrentText("CRF");
                av1Tab->av1PresetBox->setCurrentText("9");
                av1Tab->av1CRFSlider->setValue(31);
                av1Tab->av1TuneBox->setCurrentText("PSNR");
                av1Tab->av1KeyIntBox->setCurrentText("240");
                av1Tab->av1AudioBitrateBox->setCurrentText("256 kbps");
                av1Tab->av1VbrModeBox->setCurrentText("Default");
                eightBitCheck->setChecked(true);
                eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                av1Tab->tplModelCheck->setChecked(true);
                av1Tab->enableTfCheck->setChecked(true);
                av1Tab->enableCdefCheck->setChecked(false);
                av1Tab->av1LookaheadCheck->setChecked(false);
                av1Tab->av1SharpenCheck->setChecked(false);
                av1Tab->fastDecodeBox->setCurrentIndex(0);
                break;

            case 4: // Quality
                av1Tab->av1ContainerBox->setCurrentText("webm");
                av1Tab->av1AudioCheck->setChecked(true);
                av1Tab->av1AudioCodecBox->setCurrentText("opus");
                av1Tab->av1AudioSampleRateBox->setCurrentText("48 KHz");
                av1Tab->av1EnableRCModeCheck->setChecked(true);
                av1Tab->av1RCModeBox->setCurrentText("CRF");
                av1Tab->av1PresetBox->setCurrentText("8");
                av1Tab->av1CRFSlider->setValue(27);
                av1Tab->av1TuneBox->setCurrentText("SSIM");
                av1Tab->av1KeyIntBox->setCurrentText("240");
                av1Tab->av1AudioBitrateBox->setCurrentText("256 kbps");
                av1Tab->av1VbrModeBox->setCurrentText("Default");
                eightBitCheck->setChecked(true);
                eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                av1Tab->tplModelCheck->setChecked(true);
                av1Tab->enableTfCheck->setChecked(true);
                av1Tab->enableCdefCheck->setChecked(false);
                av1Tab->av1LookaheadCheck->setChecked(true);
                av1Tab->av1LookaheadSlider->setValue(40);
                av1Tab->av1SharpenCheck->setChecked(false);
                av1Tab->fastDecodeBox->setCurrentIndex(0);
                break;

            case 5: // High Quality
                av1Tab->av1ContainerBox->setCurrentText("webm");
                av1Tab->av1AudioCheck->setChecked(true);
                av1Tab->av1AudioCodecBox->setCurrentText("opus");
                av1Tab->av1AudioSampleRateBox->setCurrentText("48 KHz");
                av1Tab->av1EnableRCModeCheck->setChecked(true);
                av1Tab->av1RCModeBox->setCurrentText("CRF");
                av1Tab->av1PresetBox->setCurrentText("8");
                av1Tab->av1CRFSlider->setValue(21);
                av1Tab->av1TuneBox->setCurrentText("Subjective SSIM (VQ)");
                av1Tab->av1KeyIntBox->setCurrentText("240");
                av1Tab->av1AudioBitrateBox->setCurrentText("320 kbps");
                av1Tab->av1VbrModeBox->setCurrentText("Constrained");
                tenBitCheck->setChecked(true);
                colorFormatBox->setCurrentText("10-bit 4:2:0");
                av1Tab->tplModelCheck->setChecked(true);
                av1Tab->enableTfCheck->setChecked(true);
                av1Tab->enableCdefCheck->setChecked(false);
                av1Tab->av1LookaheadCheck->setChecked(true);
                av1Tab->av1LookaheadSlider->setValue(80);
                av1Tab->av1SharpenCheck->setChecked(true);
                av1Tab->av1SharpenStrengthSlider->setValue(5);
                av1Tab->fastDecodeBox->setCurrentIndex(0);
                break;

            case 6: // Ultra
                av1Tab->av1ContainerBox->setCurrentText("webm");
                av1Tab->av1AudioCheck->setChecked(true);
                av1Tab->av1AudioCodecBox->setCurrentText("opus");
                av1Tab->av1AudioSampleRateBox->setCurrentText("48 KHz");
                av1Tab->av1EnableRCModeCheck->setChecked(true);
                av1Tab->av1RCModeBox->setCurrentText("CRF");
                av1Tab->av1PresetBox->setCurrentText("6");
                av1Tab->av1CRFSlider->setValue(18);
                av1Tab->av1TuneBox->setCurrentText("Subjective SSIM (VQ)");
                av1Tab->av1KeyIntBox->setCurrentText("240");
                av1Tab->av1AudioBitrateBox->setCurrentText("512 kbps");
                av1Tab->av1VbrModeBox->setCurrentText("Off");
                tenBitCheck->setChecked(true);
                colorFormatBox->setCurrentText("10-bit 4:2:0");
                av1Tab->tplModelCheck->setChecked(true);
                av1Tab->enableTfCheck->setChecked(true);
                av1Tab->enableCdefCheck->setChecked(true);
                av1Tab->av1LookaheadCheck->setChecked(true);
                av1Tab->av1LookaheadSlider->setValue(120);
                av1Tab->av1SharpenCheck->setChecked(true);
                av1Tab->av1SharpenStrengthSlider->setValue(10);
                av1Tab->fastDecodeBox->setCurrentIndex(0);
                break;
            }
        }
        else if (currentTab == 1) {  // x265
            QString presetStr = (p <= 2) ? "veryfast" : (p == 3 ? "medium" : (p == 4 ? "slow" : "slower"));
            x265Tab->x265PresetBox->setCurrentText(presetStr);
            if (!x265Tab->x265EnableRCModeCheck->isChecked()) {
                x265Tab->x265EnableRCModeCheck->setChecked(true);
                x265Tab->x265RCModeBox->setCurrentText("CRF");
            }
            int crf = (p == 1) ? 35 : (p == 2 ? 30 : (p == 3 ? 25 : (p == 4 ? 20 : 16)));
            x265Tab->x265CRFSlider->setValue(crf);
        }
        else if (currentTab == 2) {  // VP9
            QString cpu = (p <= 3) ? "5" : "4";
            vp9Tab->vp9CpuUsedBox->setCurrentText(cpu);
            if (!vp9Tab->vp9EnableRCModeCheck->isChecked()) {
                vp9Tab->vp9EnableRCModeCheck->setChecked(true);
                vp9Tab->vp9RCModeBox->setCurrentText("CRF");
            }
            int crf = (p == 1) ? 50 : (p == 2 ? 42 : (p == 3 ? 34 : (p == 4 ? 26 : 20)));
            vp9Tab->vp9CRFSlider->setValue(crf);
        }
    };

    QObject::connect(presetCombo, &QComboBox::currentIndexChanged, applyPreset);
    QObject::connect(codecTabs, &QTabWidget::currentChanged, [presetCombo](int) {
        presetCombo->setCurrentIndex(0);  // Switch back to custom when switching tabs
    });
}
