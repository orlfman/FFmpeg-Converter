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
    QList<QWidget*> av1Widgets = {
        av1Tab->av1ContainerBox,
        av1Tab->av1AudioCheck,
        av1Tab->av1AudioCodecBox,
        av1Tab->av1AudioSampleRateBox,
        av1Tab->av1AudioBitrateBox,
        av1Tab->av1VbrModeBox,
        av1Tab->av1EnableRCModeCheck,
        av1Tab->av1RCModeBox,
        av1Tab->av1PresetBox,
        av1Tab->av1CRFSlider,
        av1Tab->av1QPSlider,
        av1Tab->av1VBRBitrateSlider,
        av1Tab->av1VBRVBVCheck,
        av1Tab->av1TuneBox,
        av1Tab->av1KeyIntBox,
        av1Tab->tplModelCheck,
        av1Tab->enableTfCheck,
        av1Tab->enableCdefCheck,
        av1Tab->av1LookaheadCheck,
        av1Tab->av1LookaheadSlider,
        av1Tab->fastDecodeBox,
        av1Tab->av1SharpenCheck,
        av1Tab->av1SharpenStrengthSlider,
        av1Tab->av1TwoPassCheck,
        av1Tab->lowLatencyCheck,
        av1Tab->screenContentModeBox,
        av1Tab->superResModeBox,
        av1Tab->superResDenomSlider,
        av1Tab->nativeGrainCheck,
        av1Tab->grainStrengthSlider,
        av1Tab->grainDenoiseCombo,
        av1Tab->av1ThreadsBox,
        av1Tab->av1TileRowsBox,
        av1Tab->av1TileColumnsBox,
        av1Tab->av1LevelBox,
        av1Tab->av1UnsharpenCheck,
        av1Tab->av1UnsharpenStrengthSlider,
        av1Tab->av1BlurCheck,
        av1Tab->av1BlurStrengthSlider,
        av1Tab->av1NoiseReductionCheck,
        av1Tab->av1NoiseReductionSlider,
        av1Tab->av1GrainSynthCheck,
        av1Tab->av1GrainSynthLevel,
        av1Tab->av1AQModeBox,
        av1Tab->av1AQStrengthSlider
    };

    QList<QWidget*> x265Widgets = {
        x265Tab->x265ContainerBox,
        x265Tab->x265PresetBox,
        x265Tab->x265TuneBox,
        x265Tab->x265CRFSlider,
        x265Tab->x265QPSlider,
        x265Tab->x265ABRBitrateSlider,
        x265Tab->x265CBRBitrateSlider,
        x265Tab->x265ABRVBVCheck,
        x265Tab->x265EnableRCModeCheck,
        x265Tab->x265RCModeBox,
        x265Tab->x265KeyIntBox,
        x265Tab->x265ThreadsBox,
        x265Tab->x265FrameThreadsBox,
        x265Tab->x265TwoPassCheck,
        x265Tab->strongIntraCheck,
        x265Tab->saoCheck,
        x265Tab->limitRefsBox,
        x265Tab->rdoqLevelBox,
        x265Tab->x265LookaheadCheck,
        x265Tab->x265LookaheadSlider,
        x265Tab->x265AQModeBox,
        x265Tab->x265AQStrengthSlider,
        x265Tab->enablePsyRdCheck,
        x265Tab->enableCutreeCheck,
        x265Tab->x265LevelBox,
        x265Tab->x265UnsharpenCheck,
        x265Tab->x265UnsharpenStrengthSlider,
        x265Tab->x265SharpenCheck,
        x265Tab->x265SharpenStrengthSlider,
        x265Tab->x265BlurCheck,
        x265Tab->x265BlurStrengthSlider,
        x265Tab->x265NoiseReductionCheck,
        x265Tab->x265NoiseReductionSlider,
        x265Tab->x265GrainSynthCheck,
        x265Tab->x265GrainSynthLevel
    };

    QList<QWidget*> vp9Widgets = {
        vp9Tab->vp9ContainerBox,
        vp9Tab->vp9CpuUsedBox,
        vp9Tab->vp9DeadlineBox,
        vp9Tab->vp9CRFSlider,
        vp9Tab->vp9BitrateSlider,
        vp9Tab->vp9EnableRCModeCheck,
        vp9Tab->vp9RCModeBox,
        vp9Tab->vp9KeyIntBox,
        vp9Tab->vp9ThreadsBox,
        vp9Tab->vp9TileColumnsBox,
        vp9Tab->vp9TileRowsBox,
        vp9Tab->vp9TwoPassCheck,
        vp9Tab->enableRowMtCheck,
        vp9Tab->screenContentCheck,
        vp9Tab->vp9LookaheadCheck,
        vp9Tab->vp9LookaheadSlider,
        vp9Tab->vp9AQModeBox,
        vp9Tab->vp9AQStrengthSlider,
        vp9Tab->vp9UnsharpenCheck,
        vp9Tab->vp9UnsharpenStrengthSlider,
        vp9Tab->vp9SharpenCheck,
        vp9Tab->vp9SharpenStrengthSlider,
        vp9Tab->vp9BlurCheck,
        vp9Tab->vp9BlurStrengthSlider,
        vp9Tab->vp9NoiseReductionCheck,
        vp9Tab->vp9NoiseReductionSlider,
        vp9Tab->vp9GrainSynthCheck,
        vp9Tab->vp9GrainSynthLevel
    };

    QList<QWidget*> globalWidgets = { eightBitCheck, eightBitColorFormatBox, tenBitCheck, colorFormatBox };

    auto updateLockState = [presetCombo, codecTabs, av1Widgets, x265Widgets, vp9Widgets, globalWidgets]() {
        bool isPresetActive = (presetCombo->currentIndex() != 0);

        for (QWidget* w : globalWidgets) w->setEnabled(true);
        int tab = codecTabs->currentIndex();
        for (QWidget* w : av1Widgets)   w->setEnabled(!isPresetActive || tab != 0);
        for (QWidget* w : x265Widgets)  w->setEnabled(!isPresetActive || tab != 1);
        for (QWidget* w : vp9Widgets)   w->setEnabled(!isPresetActive || tab != 2);
    };

        updateLockState();

        auto applyPreset = [presetCombo, codecTabs, av1Tab, x265Tab, vp9Tab, eightBitCheck, eightBitColorFormatBox, tenBitCheck, colorFormatBox, updateLockState]() {
            int p = presetCombo->currentIndex();
            if (p == 0) {
                updateLockState();
                return;
            }

            int currentTab = codecTabs->currentIndex();

            if (currentTab == 0) { // AV1
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
            else if (currentTab == 1) { // x265
                x265Tab->x265ContainerBox->setCurrentText("mp4");
                x265Tab->x265AudioCheck->setChecked(true);
                x265Tab->x265AudioCodecBox->setCurrentText("opus");
                x265Tab->x265AudioSampleRateBox->setCurrentText("48 KHz");
                x265Tab->x265EnableRCModeCheck->setChecked(true);
                x265Tab->x265RCModeBox->setCurrentText("CRF");
                x265Tab->x265TuneBox->setCurrentText("film");
                x265Tab->x265KeyIntBox->setCurrentText("240");
                x265Tab->saoCheck->setChecked(true);
                x265Tab->deblockAlphaSlider->setValue(-2);
                x265Tab->deblockBetaSlider->setValue(-2);
                x265Tab->pmodeCheck->setChecked(false);
                x265Tab->refFramesBox->setCurrentIndex(3);
                x265Tab->weightpCheck->setChecked(true);
                x265Tab->x265AQModeBox->setCurrentText("Variance");
                x265Tab->x265VbrModeBox->setCurrentText("Default");

                switch (p) {
                    case 1: // Streaming
                        x265Tab->x265PresetBox->setCurrentText("veryfast");
                        x265Tab->x265CRFSlider->setValue(30);
                        x265Tab->x265AudioBitrateBox->setCurrentText("128 kbps");
                        eightBitCheck->setChecked(true);
                        eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                        x265Tab->x265LookaheadCheck->setChecked(true);
                        x265Tab->x265LookaheadSlider->setValue(10);
                        x265Tab->x265AQStrengthSlider->setValue(0.6);
                        x265Tab->rdoqLevelBox->setCurrentIndex(0);
                        x265Tab->limitRefsBox->setCurrentIndex(0);
                        x265Tab->enablePsyRdCheck->setChecked(false);
                        x265Tab->enableCutreeCheck->setChecked(false);
                        x265Tab->x265TwoPassCheck->setChecked(false);
                        x265Tab->x265SharpenCheck->setChecked(false);
                        x265Tab->x265KeyIntBox->setCurrentText("120");
                        break;
                    case 2: // Medium
                        x265Tab->x265PresetBox->setCurrentText("fast");
                        x265Tab->x265CRFSlider->setValue(25);
                        x265Tab->x265AudioBitrateBox->setCurrentText("192 kbps");
                        eightBitCheck->setChecked(true);
                        eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                        x265Tab->x265LookaheadCheck->setChecked(true);
                        x265Tab->x265LookaheadSlider->setValue(20);
                        x265Tab->x265AQStrengthSlider->setValue(0.8);
                        x265Tab->rdoqLevelBox->setCurrentIndex(0);
                        x265Tab->limitRefsBox->setCurrentIndex(1);
                        x265Tab->enablePsyRdCheck->setChecked(false);
                        x265Tab->enableCutreeCheck->setChecked(false);
                        x265Tab->x265TwoPassCheck->setChecked(false);
                        x265Tab->x265SharpenCheck->setChecked(false);
                        break;
                    case 3: // High
                        x265Tab->x265PresetBox->setCurrentText("medium");
                        x265Tab->x265CRFSlider->setValue(22);
                        x265Tab->x265AudioBitrateBox->setCurrentText("256 kbps");
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        x265Tab->x265LookaheadCheck->setChecked(true);
                        x265Tab->x265LookaheadSlider->setValue(40);
                        x265Tab->x265AQStrengthSlider->setValue(1.0);
                        x265Tab->rdoqLevelBox->setCurrentIndex(1);
                        x265Tab->limitRefsBox->setCurrentIndex(2);
                        x265Tab->enablePsyRdCheck->setChecked(true);
                        x265Tab->enableCutreeCheck->setChecked(false);
                        x265Tab->x265TwoPassCheck->setChecked(false);
                        x265Tab->x265SharpenCheck->setChecked(false);
                        x265Tab->strongIntraCheck->setChecked(false);
                        break;
                    case 4: // Quality
                        x265Tab->x265PresetBox->setCurrentText("slow");
                        x265Tab->x265CRFSlider->setValue(19);
                        x265Tab->x265AudioBitrateBox->setCurrentText("320 kbps");
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        x265Tab->x265LookaheadCheck->setChecked(true);
                        x265Tab->x265LookaheadSlider->setValue(60);
                        x265Tab->x265AQStrengthSlider->setValue(1.0);
                        x265Tab->rdoqLevelBox->setCurrentIndex(1);
                        x265Tab->limitRefsBox->setCurrentIndex(3);
                        x265Tab->enablePsyRdCheck->setChecked(true);
                        x265Tab->enableCutreeCheck->setChecked(true);
                        x265Tab->x265TwoPassCheck->setChecked(false);
                        x265Tab->x265SharpenCheck->setChecked(false);
                        x265Tab->strongIntraCheck->setChecked(true);
                        x265Tab->x265EnableRCModeCheck->setChecked(true);
                        x265Tab->x265RCModeBox->setCurrentText("CRF");
                        break;
                    case 5: // High Quality
                        x265Tab->x265PresetBox->setCurrentText("slower");
                        x265Tab->x265CRFSlider->setValue(16);
                        x265Tab->x265AudioBitrateBox->setCurrentText("384 kbps");
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        x265Tab->x265LookaheadCheck->setChecked(true);
                        x265Tab->x265LookaheadSlider->setValue(80);
                        x265Tab->x265AQStrengthSlider->setValue(1.2);
                        x265Tab->rdoqLevelBox->setCurrentIndex(2);
                        x265Tab->limitRefsBox->setCurrentIndex(3);
                        x265Tab->enablePsyRdCheck->setChecked(true);
                        x265Tab->enableCutreeCheck->setChecked(true);
                        x265Tab->x265TwoPassCheck->setChecked(false);
                        x265Tab->x265SharpenCheck->setChecked(true);
                        x265Tab->x265SharpenStrengthSlider->setValue(5);
                        x265Tab->strongIntraCheck->setChecked(true);
                        x265Tab->x265EnableRCModeCheck->setChecked(true);
                        x265Tab->x265RCModeBox->setCurrentText("CRF");
                        break;
                    case 6: // Ultra
                        x265Tab->x265PresetBox->setCurrentText("veryslow");
                        x265Tab->x265CRFSlider->setValue(14);
                        x265Tab->x265AudioBitrateBox->setCurrentText("512 kbps");
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        x265Tab->x265LookaheadCheck->setChecked(true);
                        x265Tab->x265LookaheadSlider->setValue(120);
                        x265Tab->x265AQStrengthSlider->setValue(1.4);
                        x265Tab->rdoqLevelBox->setCurrentIndex(2);
                        x265Tab->limitRefsBox->setCurrentIndex(3);
                        x265Tab->enablePsyRdCheck->setChecked(true);
                        x265Tab->enableCutreeCheck->setChecked(true);
                        x265Tab->x265TwoPassCheck->setChecked(false);
                        x265Tab->x265SharpenCheck->setChecked(true);
                        x265Tab->x265SharpenStrengthSlider->setValue(8);
                        x265Tab->strongIntraCheck->setChecked(true);
                        x265Tab->x265EnableRCModeCheck->setChecked(true);
                        x265Tab->x265RCModeBox->setCurrentText("CRF");
                        break;
                }
                x265Tab->x265LevelBox->setCurrentText("auto");
                x265Tab->x265ThreadsBox->setCurrentText("Automatic");
                x265Tab->x265FrameThreadsBox->setCurrentText("Automatic");
            }
            else if (currentTab == 2) { // VP9
                QString cpu = (p <= 3) ? "5" : "4";
                vp9Tab->vp9CpuUsedBox->setCurrentText(cpu);
                if (!vp9Tab->vp9EnableRCModeCheck->isChecked()) {
                    vp9Tab->vp9EnableRCModeCheck->setChecked(true);
                    vp9Tab->vp9RCModeBox->setCurrentText("CRF");
                }
                int crf = (p == 1) ? 50 : (p == 2 ? 42 : (p == 3 ? 34 : (p == 4 ? 26 : 20)));
                vp9Tab->vp9CRFSlider->setValue(crf);
            }

            updateLockState();
        };

        QObject::connect(presetCombo, &QComboBox::currentIndexChanged, applyPreset);

        QObject::connect(codecTabs, &QTabWidget::currentChanged, [presetCombo, updateLockState](int) {
            presetCombo->setCurrentIndex(0);
            updateLockState();
        });

        updateLockState();
}
