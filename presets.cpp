#include "presets.h"
#include <QObject>
void Presets::connectPresets(
    QComboBox* presetCombo,
    QTabWidget* codecTabs,
    Av1Tab* av1Tab,
    X265Tab* x265Tab,
    X264Tab* x264Tab,
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
        x265Tab->saoCheck,
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
    QList<QWidget*> x264Widgets = {
        x264Tab->x264ContainerBox, x264Tab->x264PresetBox, x264Tab->x264TuneBox,
        x264Tab->x264CRFSlider, x264Tab->x264QPSlider, x264Tab->x264ABRBitrateSlider,
        x264Tab->x264CBRBitrateSlider, x264Tab->x264ABRVBVCheck, x264Tab->x264EnableRCModeCheck,
        x264Tab->x264RCModeBox, x264Tab->x264KeyIntBox, x264Tab->x264ThreadsBox,
        x264Tab->x264FrameThreadsBox, x264Tab->x264TwoPassCheck, x264Tab->strongIntraCheck,
        x264Tab->limitRefsBox, x264Tab->rdoqLevelBox,
        x264Tab->x264LookaheadCheck, x264Tab->x264LookaheadSlider, x264Tab->x264AQModeBox,
        x264Tab->x264AQStrengthSlider, x264Tab->enablePsyRdCheck, x264Tab->enableCutreeCheck,
        x264Tab->x264LevelBox, x264Tab->x264ProfileBox, x264Tab->x264UnsharpenCheck, x264Tab->x264UnsharpenStrengthSlider,
        x264Tab->x264SharpenCheck, x264Tab->x264SharpenStrengthSlider, x264Tab->x264BlurCheck,
        x264Tab->x264BlurStrengthSlider, x264Tab->x264NoiseReductionCheck, x264Tab->x264NoiseReductionSlider,
        x264Tab->x264GrainSynthCheck, x264Tab->x264GrainSynthLevel
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
    x265Widgets.removeOne(x265Tab->x265AudioCheck);
    vp9Widgets.removeOne(vp9Tab->vp9AudioCheck);
    x264Widgets.removeOne(x264Tab->x264AudioCheck);
    av1Widgets.removeOne(av1Tab->av1AudioCheck);
    QList<QWidget*> globalWidgets = { eightBitCheck, eightBitColorFormatBox, tenBitCheck, colorFormatBox };
    auto updateLockState = [presetCombo, codecTabs, av1Widgets, x265Widgets, x264Widgets, vp9Widgets, globalWidgets]() {
        bool isPresetActive = (presetCombo->currentIndex() != 0);
        for (QWidget* w : globalWidgets) w->setEnabled(true);
        int tab = codecTabs->currentIndex();
        for (QWidget* w : av1Widgets) w->setEnabled(!isPresetActive || tab != 0);
        for (QWidget* w : x265Widgets) w->setEnabled(!isPresetActive || tab != 1);
        for (QWidget* w : x264Widgets) w->setEnabled(!isPresetActive || tab != 3);
        for (QWidget* w : vp9Widgets) w->setEnabled(!isPresetActive || tab != 2);
    };
        updateLockState();
        auto applyPreset = [presetCombo, codecTabs, av1Tab, x265Tab, x264Tab, vp9Tab, eightBitCheck, eightBitColorFormatBox, tenBitCheck, colorFormatBox, updateLockState]() {
            int p = presetCombo->currentIndex();
            if (p == 0) {
                updateLockState();
                return;
            }
            int currentTab = codecTabs->currentIndex();
            auto setCustomRecommended = [](QComboBox* keyBox, QComboBox* modeBox) {
                keyBox->setCurrentText("Custom");
                modeBox->setCurrentIndex(1);
            };
            if (currentTab == 0) { // AV1
                setCustomRecommended(av1Tab->av1KeyIntBox, av1Tab->av1CustomKeyframeModeBox);
                av1Tab->av1NlmeansCheck->setChecked(false);
                av1Tab->av1NlmeansSigmaSSlider->setValue(20);
                av1Tab->av1NlmeansSigmaPSlider->setValue(10);
                av1Tab->av1NlmeansPatchSlider->setValue(7);

                switch (p) {
                    case 1: // DVD
                        av1Tab->av1ContainerBox->setCurrentText("webm");
                        av1Tab->av1AudioCheck->setChecked(true);
                        av1Tab->av1AudioCodecBox->setCurrentText("opus");
                        av1Tab->av1AudioSampleRateBox->setCurrentText("48 KHz");
                        av1Tab->av1EnableRCModeCheck->setChecked(true);
                        av1Tab->av1RCModeBox->setCurrentText("CRF");
                        av1Tab->av1PresetBox->setCurrentText("8");
                        av1Tab->av1CRFSlider->setValue(30);
                        av1Tab->av1TuneBox->setCurrentText("Subjective SSIM (VQ)");
                        //av1Tab->av1KeyIntBox->setCurrentText("240");
                        av1Tab->av1AudioBitrateBox->setCurrentText("192 kbps");
                        av1Tab->av1VbrModeBox->setCurrentText("Default");
                        eightBitCheck->setChecked(true);
                        eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                        av1Tab->tplModelCheck->setChecked(true);
                        av1Tab->enableTfCheck->setChecked(false);
                        av1Tab->enableCdefCheck->setChecked(true);
                        av1Tab->av1LookaheadCheck->setChecked(true);
                        av1Tab->av1LookaheadSlider->setValue(40);
                        av1Tab->fastDecodeBox->setCurrentIndex(0);
                        av1Tab->av1SharpenCheck->setChecked(false);
                        av1Tab->av1TwoPassCheck->setChecked(false);
                        av1Tab->lowLatencyCheck->setChecked(false);
                        av1Tab->screenContentModeBox->setCurrentIndex(0);
                        av1Tab->superResModeBox->setCurrentIndex(0);
                        av1Tab->nativeGrainCheck->setChecked(false);
                        av1Tab->av1ThreadsBox->setCurrentText("Automatic");
                        av1Tab->av1TileRowsBox->setCurrentText("Automatic");
                        av1Tab->av1TileColumnsBox->setCurrentText("Automatic");
                        av1Tab->av1LevelBox->setCurrentText("Auto");
                        av1Tab->av1UnsharpenCheck->setChecked(false);
                        av1Tab->av1BlurCheck->setChecked(false);
                        av1Tab->av1NoiseReductionCheck->setChecked(false);
                        av1Tab->av1GrainSynthCheck->setChecked(false);
                        av1Tab->av1AQModeBox->setCurrentText("Automatic");
                        break;

                    case 2: // Streaming
                        av1Tab->av1ContainerBox->setCurrentText("webm");
                        av1Tab->av1AudioCheck->setChecked(true);
                        av1Tab->av1AudioCodecBox->setCurrentText("opus");
                        av1Tab->av1AudioSampleRateBox->setCurrentText("48 KHz");
                        av1Tab->av1EnableRCModeCheck->setChecked(true);
                        av1Tab->av1RCModeBox->setCurrentText("CRF");
                        av1Tab->av1PresetBox->setCurrentText("9");
                        av1Tab->av1CRFSlider->setValue(30);
                        av1Tab->av1TuneBox->setCurrentText("PSNR");
                        //av1Tab->av1KeyIntBox->setCurrentText("120");
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

                    case 3: // Medium
                        av1Tab->av1ContainerBox->setCurrentText("webm");
                        av1Tab->av1AudioCheck->setChecked(true);
                        av1Tab->av1AudioCodecBox->setCurrentText("opus");
                        av1Tab->av1AudioSampleRateBox->setCurrentText("48 KHz");
                        av1Tab->av1EnableRCModeCheck->setChecked(true);
                        av1Tab->av1RCModeBox->setCurrentText("CRF");
                        av1Tab->av1PresetBox->setCurrentText("10");
                        av1Tab->av1CRFSlider->setValue(37);
                        av1Tab->av1TuneBox->setCurrentText("PSNR");
                        //av1Tab->av1KeyIntBox->setCurrentText("240");
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

                    case 4: // High
                        av1Tab->av1ContainerBox->setCurrentText("webm");
                        av1Tab->av1AudioCheck->setChecked(true);
                        av1Tab->av1AudioCodecBox->setCurrentText("opus");
                        av1Tab->av1AudioSampleRateBox->setCurrentText("48 KHz");
                        av1Tab->av1EnableRCModeCheck->setChecked(true);
                        av1Tab->av1RCModeBox->setCurrentText("CRF");
                        av1Tab->av1PresetBox->setCurrentText("9");
                        av1Tab->av1CRFSlider->setValue(31);
                        av1Tab->av1TuneBox->setCurrentText("SSIM");
                        //av1Tab->av1KeyIntBox->setCurrentText("240");
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

                    case 5: // Quality
                        av1Tab->av1ContainerBox->setCurrentText("webm");
                        av1Tab->av1AudioCheck->setChecked(true);
                        av1Tab->av1AudioCodecBox->setCurrentText("opus");
                        av1Tab->av1AudioSampleRateBox->setCurrentText("48 KHz");
                        av1Tab->av1EnableRCModeCheck->setChecked(true);
                        av1Tab->av1RCModeBox->setCurrentText("CRF");
                        av1Tab->av1PresetBox->setCurrentText("8");
                        av1Tab->av1CRFSlider->setValue(27);
                        av1Tab->av1TuneBox->setCurrentText("SSIM");
                        //av1Tab->av1KeyIntBox->setCurrentText("240");
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

                    case 6: // High Quality
                        av1Tab->av1ContainerBox->setCurrentText("webm");
                        av1Tab->av1AudioCheck->setChecked(true);
                        av1Tab->av1AudioCodecBox->setCurrentText("opus");
                        av1Tab->av1AudioSampleRateBox->setCurrentText("48 KHz");
                        av1Tab->av1EnableRCModeCheck->setChecked(true);
                        av1Tab->av1RCModeBox->setCurrentText("CRF");
                        av1Tab->av1PresetBox->setCurrentText("8");
                        av1Tab->av1CRFSlider->setValue(21);
                        av1Tab->av1TuneBox->setCurrentText("Subjective SSIM (VQ)");
                        //av1Tab->av1KeyIntBox->setCurrentText("240");
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

                    case 7: // Ultra
                        av1Tab->av1ContainerBox->setCurrentText("webm");
                        av1Tab->av1AudioCheck->setChecked(true);
                        av1Tab->av1AudioCodecBox->setCurrentText("opus");
                        av1Tab->av1AudioSampleRateBox->setCurrentText("48 KHz");
                        av1Tab->av1EnableRCModeCheck->setChecked(true);
                        av1Tab->av1RCModeBox->setCurrentText("CRF");
                        av1Tab->av1PresetBox->setCurrentText("6");
                        av1Tab->av1CRFSlider->setValue(18);
                        av1Tab->av1TuneBox->setCurrentText("Subjective SSIM (VQ)");
                        //av1Tab->av1KeyIntBox->setCurrentText("240");
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
                    case 8: // Anime - Best possible quality for anime
                        av1Tab->av1ContainerBox->setCurrentText("mkv");
                        av1Tab->av1AudioCheck->setChecked(true);
                        av1Tab->av1AudioCodecBox->setCurrentText("opus");
                        av1Tab->av1AudioSampleRateBox->setCurrentText("48 KHz");
                        av1Tab->av1EnableRCModeCheck->setChecked(true);
                        av1Tab->av1RCModeBox->setCurrentText("CRF");
                        av1Tab->av1PresetBox->setCurrentText("5");
                        av1Tab->av1CRFSlider->setValue(16);
                        av1Tab->av1TuneBox->setCurrentText("Subjective SSIM (VQ)");
                        //av1Tab->av1KeyIntBox->setCurrentText("240");
                        av1Tab->av1AudioBitrateBox->setCurrentText("320 kbps");
                        av1Tab->av1VbrModeBox->setCurrentText("Constrained");
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        av1Tab->tplModelCheck->setChecked(true);
                        av1Tab->enableTfCheck->setChecked(true);
                        av1Tab->enableCdefCheck->setChecked(true);
                        av1Tab->av1LookaheadCheck->setChecked(true);
                        av1Tab->av1LookaheadSlider->setValue(100);
                        av1Tab->fastDecodeBox->setCurrentIndex(0);
                        av1Tab->av1SharpenCheck->setChecked(true);
                        av1Tab->av1SharpenStrengthSlider->setValue(8);
                        av1Tab->av1GrainSynthCheck->setChecked(true);
                        av1Tab->av1GrainSynthLevel->setValue(12);
                        av1Tab->av1TwoPassCheck->setChecked(false);
                        av1Tab->lowLatencyCheck->setChecked(false);
                        av1Tab->screenContentModeBox->setCurrentIndex(0);
                        av1Tab->superResModeBox->setCurrentIndex(0);
                        av1Tab->nativeGrainCheck->setChecked(true);
                        av1Tab->av1ThreadsBox->setCurrentText("Automatic");
                        av1Tab->av1TileRowsBox->setCurrentText("Automatic");
                        av1Tab->av1TileColumnsBox->setCurrentText("Automatic");
                        av1Tab->av1LevelBox->setCurrentText("Auto");
                        av1Tab->av1UnsharpenCheck->setChecked(false);
                        av1Tab->av1BlurCheck->setChecked(false);
                        av1Tab->av1NoiseReductionCheck->setChecked(false);
                        av1Tab->av1AQModeBox->setCurrentText("Variance");
                        av1Tab->av1AQStrengthSlider->setValue(8);
                        break;
                }
            }
            else if (currentTab == 1) { // x265
                setCustomRecommended(x265Tab->x265KeyIntBox, x265Tab->x265CustomKeyframeModeBox);
                x265Tab->x265ContainerBox->setCurrentText("mp4");
                x265Tab->x265AudioCheck->setChecked(true);
                x265Tab->x265AudioCodecBox->setCurrentText("opus");
                x265Tab->x265AudioSampleRateBox->setCurrentText("48 kHz");
                x265Tab->x265EnableRCModeCheck->setChecked(true);
                x265Tab->x265RCModeBox->setCurrentText("CRF");
                //x265Tab->x265KeyIntBox->setCurrentText("240");
                x265Tab->saoCheck->setChecked(true);
                x265Tab->deblockAlphaSlider->setValue(-2);
                x265Tab->deblockBetaSlider->setValue(-2);
                x265Tab->pmodeCheck->setChecked(false);
                x265Tab->refFramesBox->setCurrentIndex(3);
                x265Tab->weightpCheck->setChecked(true);
                x265Tab->x265AQModeBox->setCurrentText("Variance");
                x265Tab->x265VbrModeBox->setCurrentText("Default");

                switch (p) {
                    case 1: // DVD
                        x265Tab->x265PresetBox->setCurrentText("veryfast");
                        x265Tab->x265CRFSlider->setValue(32);
                        x265Tab->x265TuneBox->setCurrentText("grain");
                        x265Tab->x265AudioBitrateBox->setCurrentText("128 kbps");
                        eightBitCheck->setChecked(true);
                        tenBitCheck->setChecked(false);
                        eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                        x265Tab->x265LookaheadCheck->setChecked(true);
                        x265Tab->x265LookaheadSlider->setValue(10);
                        x265Tab->x265AQStrengthSlider->setValue(0.5);
                        x265Tab->enablePsyRdCheck->setChecked(false);
                        x265Tab->enableCutreeCheck->setChecked(false);
                        x265Tab->x265TwoPassCheck->setChecked(false);
                        x265Tab->x265SharpenCheck->setChecked(false);
                        x265Tab->refFramesBox->setCurrentIndex(3);
                        //x265Tab->x265KeyIntBox->setCurrentText("120");
                        break;

                    case 2: // Streaming
                        x265Tab->x265PresetBox->setCurrentText("veryfast");
                        x265Tab->x265CRFSlider->setValue(30);
                        x265Tab->x265TuneBox->setCurrentText("psnr");
                        x265Tab->x265AudioBitrateBox->setCurrentText("128 kbps");
                        eightBitCheck->setChecked(true);
                        tenBitCheck->setChecked(false);
                        eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                        x265Tab->x265LookaheadCheck->setChecked(true);
                        x265Tab->x265LookaheadSlider->setValue(10);
                        x265Tab->x265AQStrengthSlider->setValue(0.6);
                        x265Tab->enablePsyRdCheck->setChecked(false);
                        x265Tab->enableCutreeCheck->setChecked(false);
                        x265Tab->x265TwoPassCheck->setChecked(false);
                        x265Tab->x265SharpenCheck->setChecked(false);
                        x265Tab->refFramesBox->setCurrentIndex(2);
                        //x265Tab->x265KeyIntBox->setCurrentText("120");
                        break;

                    case 3: // Medium
                        x265Tab->x265PresetBox->setCurrentText("fast");
                        x265Tab->x265CRFSlider->setValue(25);
                        x265Tab->x265TuneBox->setCurrentText("ssim");
                        x265Tab->x265AudioBitrateBox->setCurrentText("192 kbps");
                        eightBitCheck->setChecked(true);
                        tenBitCheck->setChecked(false);
                        eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                        x265Tab->x265LookaheadCheck->setChecked(true);
                        x265Tab->x265LookaheadSlider->setValue(20);
                        x265Tab->x265AQStrengthSlider->setValue(0.8);
                        x265Tab->enablePsyRdCheck->setChecked(false);
                        x265Tab->enableCutreeCheck->setChecked(false);
                        x265Tab->x265TwoPassCheck->setChecked(false);
                        x265Tab->x265SharpenCheck->setChecked(false);
                        x265Tab->refFramesBox->setCurrentIndex(2);
                        break;

                    case 4: // High
                        x265Tab->x265PresetBox->setCurrentText("medium");
                        x265Tab->x265CRFSlider->setValue(22);
                        x265Tab->x265TuneBox->setCurrentText("ssim");
                        x265Tab->x265AudioBitrateBox->setCurrentText("256 kbps");
                        eightBitCheck->setChecked(false);
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        x265Tab->x265LookaheadCheck->setChecked(true);
                        x265Tab->x265LookaheadSlider->setValue(40);
                        x265Tab->x265AQStrengthSlider->setValue(1.0);
                        x265Tab->enablePsyRdCheck->setChecked(true);
                        x265Tab->enableCutreeCheck->setChecked(false);
                        x265Tab->x265TwoPassCheck->setChecked(false);
                        x265Tab->x265SharpenCheck->setChecked(false);
                        x265Tab->refFramesBox->setCurrentIndex(3);
                        break;

                    case 5: // Quality
                        x265Tab->x265PresetBox->setCurrentText("slow");
                        x265Tab->x265CRFSlider->setValue(19);
                        x265Tab->x265TuneBox->setCurrentText("ssim");
                        x265Tab->x265AudioBitrateBox->setCurrentText("320 kbps");
                        eightBitCheck->setChecked(false);
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        x265Tab->x265LookaheadCheck->setChecked(true);
                        x265Tab->x265LookaheadSlider->setValue(60);
                        x265Tab->x265AQStrengthSlider->setValue(1.0);
                        x265Tab->enablePsyRdCheck->setChecked(true);
                        x265Tab->enableCutreeCheck->setChecked(true);
                        x265Tab->x265TwoPassCheck->setChecked(false);
                        x265Tab->x265SharpenCheck->setChecked(false);
                        x265Tab->refFramesBox->setCurrentIndex(3);
                        break;

                    case 6: // High Quality
                        x265Tab->x265PresetBox->setCurrentText("slower");
                        x265Tab->x265CRFSlider->setValue(16);
                        x265Tab->x265TuneBox->setCurrentText("grain");
                        x265Tab->x265AudioBitrateBox->setCurrentText("384 kbps");
                        eightBitCheck->setChecked(false);
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        x265Tab->x265LookaheadCheck->setChecked(true);
                        x265Tab->x265LookaheadSlider->setValue(80);
                        x265Tab->x265AQStrengthSlider->setValue(1.2);
                        x265Tab->enablePsyRdCheck->setChecked(true);
                        x265Tab->enableCutreeCheck->setChecked(true);
                        x265Tab->x265TwoPassCheck->setChecked(false);
                        x265Tab->x265SharpenCheck->setChecked(true);
                        x265Tab->x265SharpenStrengthSlider->setValue(5);
                        x265Tab->refFramesBox->setCurrentIndex(4);
                        break;

                    case 7: // Ultra
                        x265Tab->x265PresetBox->setCurrentText("veryslow");
                        x265Tab->x265CRFSlider->setValue(14);
                        x265Tab->x265TuneBox->setCurrentText("grain");
                        x265Tab->x265AudioBitrateBox->setCurrentText("512 kbps");
                        eightBitCheck->setChecked(false);
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        x265Tab->x265LookaheadCheck->setChecked(true);
                        x265Tab->x265LookaheadSlider->setValue(120);
                        x265Tab->x265AQStrengthSlider->setValue(1.4);
                        x265Tab->enablePsyRdCheck->setChecked(true);
                        x265Tab->enableCutreeCheck->setChecked(true);
                        x265Tab->x265TwoPassCheck->setChecked(false);
                        x265Tab->x265SharpenCheck->setChecked(true);
                        x265Tab->x265SharpenStrengthSlider->setValue(8);
                        x265Tab->refFramesBox->setCurrentIndex(4);
                        break;
                    case 8: // Anime
                        x265Tab->x265PresetBox->setCurrentText("veryslow");
                        x265Tab->x265CRFSlider->setValue(15);
                        x265Tab->x265TuneBox->setCurrentText("grain");
                        x265Tab->x265AudioBitrateBox->setCurrentText("320 kbps");
                        eightBitCheck->setChecked(false);
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        x265Tab->x265LookaheadCheck->setChecked(true);
                        x265Tab->x265LookaheadSlider->setValue(120);
                        x265Tab->x265AQStrengthSlider->setValue(1.4);
                        x265Tab->enablePsyRdCheck->setChecked(true);
                        x265Tab->enableCutreeCheck->setChecked(true);
                        x265Tab->x265TwoPassCheck->setChecked(false);
                        x265Tab->x265SharpenCheck->setChecked(true);
                        x265Tab->x265SharpenStrengthSlider->setValue(8);
                        x265Tab->refFramesBox->setCurrentIndex(4);
                        break;
                }

                x265Tab->x265LevelBox->setCurrentText("auto");
                x265Tab->x265ThreadsBox->setCurrentText("Automatic");
                x265Tab->x265FrameThreadsBox->setCurrentText("Automatic");
            }
            else if (currentTab == 3) { // x264
                setCustomRecommended(x264Tab->x264KeyIntBox, x264Tab->x264CustomKeyframeModeBox);
                x264Tab->x264ContainerBox->setCurrentText("mp4");
                x264Tab->x264AudioCheck->setChecked(true);
                x264Tab->x264AudioCodecBox->setCurrentText("opus");
                x264Tab->x264AudioSampleRateBox->setCurrentText("48 kHz");
                x264Tab->x264EnableRCModeCheck->setChecked(true);
                x264Tab->x264RCModeBox->setCurrentText("CRF");
                //x264Tab->x264KeyIntBox->setCurrentText("240");
                x264Tab->deblockAlphaSlider->setValue(-2);
                x264Tab->deblockBetaSlider->setValue(-2);
                x264Tab->weightpCheck->setChecked(true);
                x264Tab->x264AQModeBox->setCurrentText("Variance");
                x264Tab->x264VbrModeBox->setCurrentText("Default");

                switch (p) {
                    case 1: // DVD - dedicated DVD quality settings
                        x264Tab->x264PresetBox->setCurrentText("medium");
                        x264Tab->x264TuneBox->setCurrentText("film");
                        x264Tab->x264CRFSlider->setValue(23);
                        x264Tab->x264AudioBitrateBox->setCurrentText("192 kbps");
                        eightBitCheck->setChecked(true);
                        tenBitCheck->setChecked(false);
                        eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                        x264Tab->x264ProfileBox->setCurrentText("main");
                        x264Tab->x264LookaheadCheck->setChecked(true);
                        x264Tab->x264LookaheadSlider->setValue(20);
                        x264Tab->x264AQStrengthSlider->setValue(0.8);
                        x264Tab->rdoqLevelBox->setCurrentIndex(1);
                        x264Tab->limitRefsBox->setCurrentIndex(1);
                        x264Tab->enablePsyRdCheck->setChecked(false);
                        x264Tab->enableCutreeCheck->setChecked(false);
                        x264Tab->x264TwoPassCheck->setChecked(false);
                        x264Tab->x264SharpenCheck->setChecked(false);
                        x264Tab->refFramesBox->setCurrentIndex(3);
                        x264Tab->strongIntraCheck->setChecked(true);
                        //x264Tab->x264KeyIntBox->setCurrentText("15");
                        break;

                    case 2: // Streaming
                        x264Tab->x264PresetBox->setCurrentText("veryfast");
                        x264Tab->x264TuneBox->setCurrentText("psnr");
                        x264Tab->x264CRFSlider->setValue(30);
                        x264Tab->x264AudioBitrateBox->setCurrentText("128 kbps");
                        eightBitCheck->setChecked(true);
                        tenBitCheck->setChecked(false);
                        eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                        x264Tab->x264ProfileBox->setCurrentText("main");
                        x264Tab->x264LookaheadCheck->setChecked(true);
                        x264Tab->x264LookaheadSlider->setValue(10);
                        x264Tab->x264AQStrengthSlider->setValue(0.6);
                        x264Tab->rdoqLevelBox->setCurrentIndex(0);
                        x264Tab->limitRefsBox->setCurrentIndex(0);
                        x264Tab->enablePsyRdCheck->setChecked(false);
                        x264Tab->enableCutreeCheck->setChecked(false);
                        x264Tab->x264TwoPassCheck->setChecked(false);
                        x264Tab->x264SharpenCheck->setChecked(false);
                        x264Tab->refFramesBox->setCurrentIndex(2);
                        x264Tab->strongIntraCheck->setChecked(true);
                        break;

                    case 3: // Medium
                        x264Tab->x264PresetBox->setCurrentText("fast");
                        x264Tab->x264TuneBox->setCurrentText("ssim");
                        x264Tab->x264CRFSlider->setValue(25);
                        x264Tab->x264AudioBitrateBox->setCurrentText("192 kbps");
                        eightBitCheck->setChecked(true);
                        tenBitCheck->setChecked(false);
                        eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                        x264Tab->x264ProfileBox->setCurrentText("main");
                        x264Tab->x264LookaheadCheck->setChecked(true);
                        x264Tab->x264LookaheadSlider->setValue(20);
                        x264Tab->x264AQStrengthSlider->setValue(0.8);
                        x264Tab->rdoqLevelBox->setCurrentIndex(1);
                        x264Tab->limitRefsBox->setCurrentIndex(1);
                        x264Tab->enablePsyRdCheck->setChecked(false);
                        x264Tab->enableCutreeCheck->setChecked(false);
                        x264Tab->x264TwoPassCheck->setChecked(false);
                        x264Tab->x264SharpenCheck->setChecked(false);
                        x264Tab->refFramesBox->setCurrentIndex(2);
                        x264Tab->strongIntraCheck->setChecked(true);
                        break;

                    case 4: // High
                        x264Tab->x264PresetBox->setCurrentText("medium");
                        x264Tab->x264TuneBox->setCurrentText("ssim");
                        x264Tab->x264CRFSlider->setValue(22);
                        x264Tab->x264AudioBitrateBox->setCurrentText("256 kbps");
                        eightBitCheck->setChecked(false);
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        x264Tab->x264ProfileBox->setCurrentText("high10");
                        x264Tab->x264LookaheadCheck->setChecked(true);
                        x264Tab->x264LookaheadSlider->setValue(40);
                        x264Tab->x264AQStrengthSlider->setValue(1.0);
                        x264Tab->rdoqLevelBox->setCurrentIndex(1);
                        x264Tab->limitRefsBox->setCurrentIndex(2);
                        x264Tab->enablePsyRdCheck->setChecked(true);
                        x264Tab->enableCutreeCheck->setChecked(false);
                        x264Tab->x264TwoPassCheck->setChecked(false);
                        x264Tab->x264SharpenCheck->setChecked(false);
                        x264Tab->refFramesBox->setCurrentIndex(3);
                        x264Tab->strongIntraCheck->setChecked(false);
                        break;

                    case 5: // Quality
                        x264Tab->x264PresetBox->setCurrentText("slow");
                        x264Tab->x264TuneBox->setCurrentText("ssim");
                        x264Tab->x264CRFSlider->setValue(19);
                        x264Tab->x264AudioBitrateBox->setCurrentText("320 kbps");
                        eightBitCheck->setChecked(false);
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        x264Tab->x264ProfileBox->setCurrentText("high10");
                        x264Tab->x264LookaheadCheck->setChecked(true);
                        x264Tab->x264LookaheadSlider->setValue(60);
                        x264Tab->x264AQStrengthSlider->setValue(1.0);
                        x264Tab->rdoqLevelBox->setCurrentIndex(1);
                        x264Tab->limitRefsBox->setCurrentIndex(3);
                        x264Tab->enablePsyRdCheck->setChecked(true);
                        x264Tab->enableCutreeCheck->setChecked(true);
                        x264Tab->x264TwoPassCheck->setChecked(false);
                        x264Tab->x264SharpenCheck->setChecked(false);
                        x264Tab->refFramesBox->setCurrentIndex(3);
                        x264Tab->strongIntraCheck->setChecked(true);
                        break;

                    case 6: // High Quality
                        x264Tab->x264PresetBox->setCurrentText("slower");
                        x264Tab->x264TuneBox->setCurrentText("film");
                        x264Tab->x264CRFSlider->setValue(16);
                        x264Tab->x264AudioBitrateBox->setCurrentText("384 kbps");
                        eightBitCheck->setChecked(false);
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        x264Tab->x264ProfileBox->setCurrentText("high10");
                        x264Tab->x264LookaheadCheck->setChecked(true);
                        x264Tab->x264LookaheadSlider->setValue(80);
                        x264Tab->x264AQStrengthSlider->setValue(1.2);
                        x264Tab->rdoqLevelBox->setCurrentIndex(2);
                        x264Tab->limitRefsBox->setCurrentIndex(3);
                        x264Tab->enablePsyRdCheck->setChecked(true);
                        x264Tab->enableCutreeCheck->setChecked(true);
                        x264Tab->x264TwoPassCheck->setChecked(false);
                        x264Tab->x264SharpenCheck->setChecked(true);
                        x264Tab->x264SharpenStrengthSlider->setValue(5);
                        x264Tab->refFramesBox->setCurrentIndex(4);
                        x264Tab->strongIntraCheck->setChecked(true);
                        break;

                    case 7: // Ultra
                        x264Tab->x264PresetBox->setCurrentText("veryslow");
                        x264Tab->x264TuneBox->setCurrentText("film");
                        x264Tab->x264CRFSlider->setValue(14);
                        x264Tab->x264AudioBitrateBox->setCurrentText("512 kbps");
                        eightBitCheck->setChecked(false);
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        x264Tab->x264ProfileBox->setCurrentText("high10");
                        x264Tab->x264LookaheadCheck->setChecked(true);
                        x264Tab->x264LookaheadSlider->setValue(120);
                        x264Tab->x264AQStrengthSlider->setValue(1.4);
                        x264Tab->rdoqLevelBox->setCurrentIndex(2);
                        x264Tab->limitRefsBox->setCurrentIndex(3);
                        x264Tab->enablePsyRdCheck->setChecked(true);
                        x264Tab->enableCutreeCheck->setChecked(true);
                        x264Tab->x264TwoPassCheck->setChecked(false);
                        x264Tab->x264SharpenCheck->setChecked(true);
                        x264Tab->x264SharpenStrengthSlider->setValue(8);
                        x264Tab->refFramesBox->setCurrentIndex(4);
                        x264Tab->strongIntraCheck->setChecked(true);
                        break;
                    case 8: // Anime
                        x264Tab->x264PresetBox->setCurrentText("veryslow");
                        x264Tab->x264TuneBox->setCurrentText("animation");
                        x264Tab->x264CRFSlider->setValue(15);
                        x264Tab->x264AudioBitrateBox->setCurrentText("320 kbps");
                        eightBitCheck->setChecked(false);
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        x264Tab->x264ProfileBox->setCurrentText("high10");
                        x264Tab->x264LookaheadCheck->setChecked(true);
                        x264Tab->x264LookaheadSlider->setValue(120);
                        x264Tab->x264AQStrengthSlider->setValue(1.4);
                        x264Tab->rdoqLevelBox->setCurrentIndex(2);
                        x264Tab->limitRefsBox->setCurrentIndex(3);
                        x264Tab->enablePsyRdCheck->setChecked(true);
                        x264Tab->enableCutreeCheck->setChecked(true);
                        x264Tab->x264TwoPassCheck->setChecked(false);
                        x264Tab->x264SharpenCheck->setChecked(true);
                        x264Tab->x264SharpenStrengthSlider->setValue(8);
                        x264Tab->refFramesBox->setCurrentIndex(4);
                        x264Tab->strongIntraCheck->setChecked(true);
                        break;
                }

                // Final common cleanup
                x264Tab->x264LevelBox->setCurrentText("auto");
                x264Tab->x264ThreadsBox->setCurrentText("Automatic");
                x264Tab->x264FrameThreadsBox->setCurrentText("Automatic");

                // Refresh RC visibility
                QString rcMode = x264Tab->x264RCModeBox->currentText();
                x264Tab->x264QPConfigWidget->setVisible(rcMode == "QP");
                x264Tab->x264CRFConfigWidget->setVisible(rcMode == "CRF");
                x264Tab->x264ABRConfigWidget->setVisible(rcMode == "ABR");
                x264Tab->x264CBRConfigWidget->setVisible(rcMode == "CBR");
                x264Tab->x264TwoPassCheck->setEnabled(rcMode == "ABR" || rcMode == "CBR");

                if (tenBitCheck->isChecked()) {
                    x264Tab->x264ProfileBox->setCurrentText("high10");
                }
            }
            else if (currentTab == 2) {
                setCustomRecommended(vp9Tab->vp9KeyIntBox, vp9Tab->vp9CustomKeyframeModeBox);
                vp9Tab->vp9ContainerBox->setCurrentText("webm");
                vp9Tab->vp9AudioCheck->setChecked(true);
                vp9Tab->vp9AudioCodecBox->setCurrentText("opus");
                vp9Tab->vp9AudioSampleRateBox->setCurrentText("48 kHz");
                vp9Tab->vp9VbrModeBox->setCurrentText("Constrained");
                vp9Tab->vp9EnableRCModeCheck->setChecked(true);
                vp9Tab->vp9RCModeBox->setCurrentText("CRF");
                vp9Tab->vp9AQModeBox->setCurrentText("Variance");
                vp9Tab->enableRowMtCheck->setChecked(true);
                vp9Tab->vp9TileColumnsBox->setCurrentText("4");
                vp9Tab->vp9TileRowsBox->setCurrentText("Automatic");
                //vp9Tab->vp9KeyIntBox->setCurrentText("240");
                vp9Tab->vp9ThreadsBox->setCurrentText("Automatic");
                vp9Tab->screenContentCheck->setChecked(false);
                vp9Tab->vp9LookaheadCheck->setChecked(true);
                vp9Tab->vp9SharpenCheck->setChecked(false);
                vp9Tab->vp9NlmeansCheck->setChecked(false);
                vp9Tab->vp9NlmeansSigmaSSlider->setValue(20);
                vp9Tab->vp9NlmeansSigmaPSlider->setValue(10);
                vp9Tab->vp9NlmeansPatchSlider->setValue(7);

                switch (p) {
                    case 1: // DVD
                        vp9Tab->vp9CpuUsedBox->setCurrentText("5");
                        vp9Tab->vp9CRFSlider->setValue(45);
                        vp9Tab->vp9AudioBitrateBox->setCurrentText("160 kbps");
                        vp9Tab->vp9LookaheadSlider->setValue(25);
                        vp9Tab->vp9TwoPassCheck->setChecked(false);
                        eightBitCheck->setChecked(true);
                        eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                        vp9Tab->vp9DeadlineBox->setCurrentText("good");
                        vp9Tab->vp9QMaxSlider->setValue(55);
                        vp9Tab->vp9ArnrCheck->setChecked(true);
                        vp9Tab->vp9ArnrStrengthSlider->setValue(4);
                        vp9Tab->vp9ArnrMaxFramesSlider->setValue(9);
                        vp9Tab->vp9TplCheck->setChecked(true);
                        break;

                    case 2: // Streaming - optimize for 4chan
                        vp9Tab->vp9CpuUsedBox->setCurrentText("6");
                        vp9Tab->vp9CRFSlider->setValue(52);
                        vp9Tab->vp9AudioBitrateBox->setCurrentText("96 kbps");
                        vp9Tab->vp9RCModeBox->setCurrentText("ABR");
                        vp9Tab->vp9BitrateSlider->setValue(1200);
                        vp9Tab->vp9LookaheadSlider->setValue(10);
                        vp9Tab->vp9TwoPassCheck->setChecked(false);
                        eightBitCheck->setChecked(true);
                        eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                        vp9Tab->vp9DeadlineBox->setCurrentText("realtime");
                        vp9Tab->vp9QMaxSlider->setValue(63);
                        vp9Tab->vp9ArnrCheck->setChecked(false);
                        vp9Tab->vp9TplCheck->setChecked(false);
                        break;

                    case 3: // Medium - Balanced everyday web quality
                        vp9Tab->vp9CpuUsedBox->setCurrentText("4");
                        vp9Tab->vp9CRFSlider->setValue(40);
                        vp9Tab->vp9AudioBitrateBox->setCurrentText("160 kbps");
                        vp9Tab->vp9LookaheadSlider->setValue(20);
                        vp9Tab->vp9TwoPassCheck->setChecked(false);
                        eightBitCheck->setChecked(true);
                        eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                        vp9Tab->vp9DeadlineBox->setCurrentText("good");
                        vp9Tab->vp9QMaxSlider->setValue(55);
                        vp9Tab->vp9ArnrCheck->setChecked(true);
                        vp9Tab->vp9ArnrStrengthSlider->setValue(4);
                        vp9Tab->vp9ArnrMaxFramesSlider->setValue(9);
                        break;

                    case 4: // High - Better quality for web
                        vp9Tab->vp9CpuUsedBox->setCurrentText("3");
                        vp9Tab->vp9CRFSlider->setValue(34);
                        vp9Tab->vp9AudioBitrateBox->setCurrentText("192 kbps");
                        vp9Tab->vp9LookaheadSlider->setValue(25);
                        vp9Tab->vp9TwoPassCheck->setChecked(false);
                        eightBitCheck->setChecked(true);
                        eightBitColorFormatBox->setCurrentText("8-bit 4:2:0");
                        vp9Tab->vp9DeadlineBox->setCurrentText("good");
                        vp9Tab->vp9QMaxSlider->setValue(52);
                        vp9Tab->vp9ArnrCheck->setChecked(true);
                        vp9Tab->vp9ArnrStrengthSlider->setValue(5);
                        vp9Tab->vp9ArnrMaxFramesSlider->setValue(12);
                        break;

                    case 5: // Quality
                        vp9Tab->vp9CpuUsedBox->setCurrentText("2");
                        vp9Tab->vp9CRFSlider->setValue(28);
                        vp9Tab->vp9AudioBitrateBox->setCurrentText("224 kbps");
                        vp9Tab->vp9LookaheadSlider->setValue(30);
                        vp9Tab->vp9TwoPassCheck->setChecked(true);
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        vp9Tab->vp9DeadlineBox->setCurrentText("best");
                        vp9Tab->vp9QMaxSlider->setValue(48);
                        vp9Tab->vp9ArnrCheck->setChecked(true);
                        vp9Tab->vp9ArnrStrengthSlider->setValue(6);
                        vp9Tab->vp9ArnrMaxFramesSlider->setValue(15);
                        vp9Tab->vp9TplCheck->setChecked(true);
                        break;

                    case 6: // High Quality
                        vp9Tab->vp9CpuUsedBox->setCurrentText("1");
                        vp9Tab->vp9CRFSlider->setValue(22);
                        vp9Tab->vp9AudioBitrateBox->setCurrentText("256 kbps");
                        vp9Tab->vp9LookaheadSlider->setValue(35);
                        vp9Tab->vp9TwoPassCheck->setChecked(true);
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        vp9Tab->vp9SharpenCheck->setChecked(true);
                        vp9Tab->vp9SharpenStrengthSlider->setValue(4);
                        vp9Tab->vp9AQStrengthSlider->setValue(8);
                        vp9Tab->vp9QMaxSlider->setValue(45);
                        vp9Tab->vp9ArnrCheck->setChecked(true);
                        vp9Tab->vp9ArnrStrengthSlider->setValue(6);
                        vp9Tab->vp9ArnrMaxFramesSlider->setValue(15);
                        vp9Tab->vp9TplCheck->setChecked(true);
                        vp9Tab->vp9DeadlineBox->setCurrentText("best");
                        break;

                    case 7: // Ultra
                        vp9Tab->vp9CpuUsedBox->setCurrentText("0");
                        vp9Tab->vp9CRFSlider->setValue(18);
                        vp9Tab->vp9AudioBitrateBox->setCurrentText("320 kbps");
                        vp9Tab->vp9LookaheadSlider->setValue(40);
                        vp9Tab->vp9TwoPassCheck->setChecked(true);
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        vp9Tab->vp9SharpenCheck->setChecked(true);
                        vp9Tab->vp9SharpenStrengthSlider->setValue(6);
                        vp9Tab->vp9TileColumnsBox->setCurrentText("8");
                        vp9Tab->vp9AQStrengthSlider->setValue(9);
                        vp9Tab->vp9QMaxSlider->setValue(40);
                        vp9Tab->vp9ArnrCheck->setChecked(true);
                        vp9Tab->vp9ArnrStrengthSlider->setValue(7);
                        vp9Tab->vp9ArnrMaxFramesSlider->setValue(15);
                        vp9Tab->vp9TplCheck->setChecked(true);
                        vp9Tab->vp9DeadlineBox->setCurrentText("best");
                        break;
                    case 8: // Anime
                        vp9Tab->vp9CpuUsedBox->setCurrentText("0");
                        vp9Tab->vp9CRFSlider->setValue(18);
                        vp9Tab->vp9AudioBitrateBox->setCurrentText("320 kbps");
                        vp9Tab->vp9LookaheadSlider->setValue(40);
                        vp9Tab->vp9TwoPassCheck->setChecked(true);
                        tenBitCheck->setChecked(true);
                        colorFormatBox->setCurrentText("10-bit 4:2:0");
                        vp9Tab->vp9SharpenCheck->setChecked(true);
                        vp9Tab->vp9SharpenStrengthSlider->setValue(7);
                        vp9Tab->vp9TileColumnsBox->setCurrentText("8");
                        vp9Tab->vp9AQStrengthSlider->setValue(9);
                        vp9Tab->vp9QMaxSlider->setValue(40);
                        vp9Tab->vp9ArnrCheck->setChecked(true);
                        vp9Tab->vp9ArnrStrengthSlider->setValue(7);
                        vp9Tab->vp9ArnrMaxFramesSlider->setValue(15);
                        vp9Tab->vp9TplCheck->setChecked(true);
                        vp9Tab->vp9DeadlineBox->setCurrentText("best");
                        break;
                }
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
