#include "MainWindow.h"

void MainWindow::startConversion()
{
    logBox->clear();
    logBox->append("=== CONVERSION STARTED ===");

    bool isCombine = (codecTabs->currentWidget() == combineScroll);
    bool isTrim   = (codecTabs->currentWidget() == trimScroll);

    QString inputFile;
    if (!isCombine) {
        inputFile = selectedFilesBox->text().trimmed();
        if (inputFile.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please select an input file.");
            return;
        }
    } else {
        inputFile = "(Multiple files from Combine Videos tab)";
    }
    logBox->append("📁 Input: " + inputFile);
    logBox->append("🎛️ Scale: " + QString::number(scaleWidthSpin->value(), 'f', 2) + "x × " + QString::number(scaleHeightSpin->value(), 'f', 2) + "x");

    if (seekCheck->isChecked()) {
        bool okHH, okMM, okSS;
        int hh = seekHH->text().toInt(&okHH);
        int mm = seekMM->text().toInt(&okMM);
        int ss = seekSS->text().toInt(&okSS);
        if (!okHH || !okMM || !okSS || hh < 0 || hh > 99 || mm < 0 || mm > 59 || ss < 0 || ss > 59) {
            QMessageBox::warning(this, "Invalid Input", "Seek time invalid (HH:00-99, MM/SS:00-59).");
            return;
        }
    }
    if (timeCheck->isChecked()) {
        bool okHH, okMM, okSS;
        int hh = timeHH->text().toInt(&okHH);
        int mm = timeMM->text().toInt(&okMM);
        int ss = timeSS->text().toInt(&okSS);
        if (!okHH || !okMM || !okSS || hh < 0 || hh > 99 || mm < 0 || mm > 59 || ss < 0 || ss > 59) {
            QMessageBox::warning(this, "Invalid Input", "Duration time invalid (HH:00-99, MM/SS:00-59).");
            return;
        }
    }

    int currentTab = codecTabs->currentIndex();
    if (currentTab == 0 && av1Tab->av1EnableRCModeCheck->isChecked()) {
        if (av1Tab->av1RCModeBox->currentText() == "VBR" && av1Tab->av1AQModeBox->currentIndex() == 1) {
            QMessageBox::warning(this, "Invalid Configuration", "Adaptive Quantization cannot be disabled when using VBR rate control for AV1.");
            return;
        }
    } else if (currentTab == 1 && x265Tab->x265EnableRCModeCheck->isChecked()) {
        QString rcMode = x265Tab->x265RCModeBox->currentText();
        if ((rcMode == "ABR" || rcMode == "CBR") && x265Tab->x265AQModeBox->currentIndex() == 1) {
            QMessageBox::warning(this, "Invalid Configuration", "Adaptive Quantization cannot be disabled when using ABR or CBR rate control for x265.");
            return;
        }
    } else if (currentTab == 2 && vp9Tab->vp9EnableRCModeCheck->isChecked()) {
        QString rcMode = vp9Tab->vp9RCModeBox->currentText();
        if ((rcMode == "ABR" || rcMode == "CBR") && vp9Tab->vp9AQModeBox->currentIndex() == 1) {
            QMessageBox::warning(this, "Invalid Configuration", "Adaptive Quantization cannot be disabled when using ABR or CBR rate control for VP9.");
            return;
        }
    }

    QString outputDir = outputDirBox->text();
    QString baseName = outputNameBox->text().isEmpty() ? "Output" : outputNameBox->text();
    QString extension = ".mkv";
    bool twoPass = false;
    QString codecStr = "copy";

    if (!isCombine && !isTrim) {
        if (currentTab == 0) {
            extension = "." + av1Tab->av1ContainerBox->currentText();
            twoPass = av1Tab->av1TwoPassCheck->isChecked();
            codecStr = "av1";
        } else if (currentTab == 1) {
            extension = "." + x265Tab->x265ContainerBox->currentText();
            twoPass = x265Tab->x265TwoPassCheck->isChecked();
            if (x265Tab->x265EnableRCModeCheck->isChecked() && x265Tab->x265RCModeBox->currentText() == "CRF") twoPass = false;
            else if (!x265Tab->x265EnableRCModeCheck->isChecked()) twoPass = false;
            codecStr = "x265";
        } else if (currentTab == 2) {
            extension = "." + vp9Tab->vp9ContainerBox->currentText();
            twoPass = vp9Tab->vp9TwoPassCheck->isChecked();
            codecStr = "vp9";
        } else if (currentTab == 3) { // x264
        extension = "." + x264Tab->x264ContainerBox->currentText();
        twoPass = x264Tab->x264TwoPassCheck->isChecked();
        if (x264Tab->x264EnableRCModeCheck->isChecked() && x264Tab->x264RCModeBox->currentText() == "CRF") twoPass = false;
        else if (!x264Tab->x264EnableRCModeCheck->isChecked()) twoPass = false;
        codecStr = "x264";
    }
    }

    QStringList args;
    if (preserveMetadataCheck->isChecked()) args << "-map_metadata" << "0";
    if (removeChaptersCheck->isChecked()) args << "-map_chapters" << "-1";

    QStringList videoFilters;
    QStringList audioFilters;

    // Rotation
    QString rotationFilter;
    QString rotation = rotationBox->currentText();
    if (rotation == "90° Clockwise") rotationFilter = "transpose=1";
    else if (rotation == "90° Counterclockwise") rotationFilter = "transpose=2";
    else if (rotation == "180°") rotationFilter = "transpose=1,transpose=1";
    else if (rotation == "Horizontal Flip") rotationFilter = "hflip";
    else if (rotation == "Vertical Flip") rotationFilter = "vflip";
    if (!rotationFilter.isEmpty()) videoFilters << rotationFilter;

    // Crop
    if (cropCheck->isChecked() && !cropValueBox->text().isEmpty() && cropValueBox->text() != "Not detected") {
        QString cropValue = cropValueBox->text();
        if (cropValue.startsWith("crop=")) cropValue = cropValue.mid(5);
        videoFilters << "crop=" + cropValue;
    }

    if (deinterlaceCheck->isChecked()) videoFilters << "yadif";
    if (deblockCheck->isChecked()) videoFilters << "deblock";
    if (denoiseCheck->isChecked()) videoFilters << "hqdn3d=4:3:6:4.5";
    if (superSharpCheck->isChecked()) videoFilters << "unsharp=5:5:0.8:3:3:0.4";
    if (toneMapCheck->isChecked()) {
        QString desatValue;

        if (toneMapModeBox->currentText() == "Custom") {
            desatValue = toneMapDesatBox->currentText();
        } else if (toneMapModeBox->currentText() == "Anime Optimized") {
            desatValue = "0.35";
        } else {
            desatValue = "0.00";
        }

        videoFilters << QString("zscale=t=linear:npl=100,format=gbrpf32le,zscale=p=bt709,tonemap=hable:desat=%1,zscale=t=bt709:m=bt709:r=tv")
        .arg(desatValue);
    }

    // Tab-specific filters
    if (currentTab == 0) { // AV1 filters
        if (av1Tab->av1UnsharpenCheck->isChecked()) videoFilters << QString("unsharp=5:5:%1:5:5:0.0").arg(av1Tab->av1UnsharpenStrengthSlider->value() / 10.0);
        if (av1Tab->av1SharpenCheck->isChecked()) videoFilters << QString("unsharp=3:3:%1:3:3:0.0").arg(av1Tab->av1SharpenStrengthSlider->value() / 10.0);
        if (av1Tab->av1BlurCheck->isChecked()) videoFilters << QString("smartblur=%1:0.5:0").arg(av1Tab->av1BlurStrengthSlider->value() / 10.0);
        if (av1Tab->av1NoiseReductionCheck->isChecked()) videoFilters << QString("hqdn3d=luma_spatial=%1:chroma_spatial=%1").arg(av1Tab->av1NoiseReductionSlider->value());
        if (av1Tab->av1GrainSynthCheck->isChecked()) videoFilters << QString("noise=alls=%1:allf=t").arg(av1Tab->av1GrainSynthLevel->value());
        if (av1Tab->av1NlmeansCheck->isChecked()) {
            int s = av1Tab->av1NlmeansSigmaSSlider->value();
            int p = av1Tab->av1NlmeansSigmaPSlider->value();
            int patch = av1Tab->av1NlmeansPatchSlider->value();
            QString filterName = av1Tab->av1NlmeansGpuCheck->isChecked() ? "knlmeans" : "nlmeans";
            QString patchStr = (filterName == "knlmeans") ? ":patch=" + QString::number(patch) : "";
            videoFilters << QString("%1=s=%2:p=%3%4").arg(filterName).arg(s).arg(p).arg(patchStr);
        }
    } else if (currentTab == 1) { // x265
        if (x265Tab->x265UnsharpenCheck->isChecked()) videoFilters << QString("unsharp=5:5:%1:5:5:0.0").arg(x265Tab->x265UnsharpenStrengthSlider->value() / 10.0);
        if (x265Tab->x265SharpenCheck->isChecked()) videoFilters << QString("unsharp=3:3:%1:3:3:0.0").arg(x265Tab->x265SharpenStrengthSlider->value() / 10.0);
        if (x265Tab->x265BlurCheck->isChecked()) videoFilters << QString("smartblur=%1:0.5:0").arg(x265Tab->x265BlurStrengthSlider->value() / 10.0);
        if (x265Tab->x265NoiseReductionCheck->isChecked()) videoFilters << QString("hqdn3d=luma_spatial=%1:chroma_spatial=%1").arg(x265Tab->x265NoiseReductionSlider->value());
        if (x265Tab->x265GrainSynthCheck->isChecked()) videoFilters << QString("noise=alls=%1:allf=t").arg(x265Tab->x265GrainSynthLevel->value());
    } else if (currentTab == 2) { // VP9
        if (vp9Tab->vp9UnsharpenCheck->isChecked()) videoFilters << QString("unsharp=5:5:%1:5:5:0.0").arg(vp9Tab->vp9UnsharpenStrengthSlider->value() / 10.0);
        if (vp9Tab->vp9SharpenCheck->isChecked()) videoFilters << QString("unsharp=3:3:%1:3:3:0.0").arg(vp9Tab->vp9SharpenStrengthSlider->value() / 10.0);
        if (vp9Tab->vp9BlurCheck->isChecked()) videoFilters << QString("smartblur=%1:0.5:0").arg(vp9Tab->vp9BlurStrengthSlider->value() / 10.0);
        if (vp9Tab->vp9NoiseReductionCheck->isChecked()) videoFilters << QString("hqdn3d=luma_spatial=%1:chroma_spatial=%1").arg(vp9Tab->vp9NoiseReductionSlider->value());
        if (vp9Tab->vp9GrainSynthCheck->isChecked()) videoFilters << QString("noise=alls=%1:allf=t").arg(vp9Tab->vp9GrainSynthLevel->value());
        if (vp9Tab->vp9NlmeansCheck->isChecked()) {
            int s = vp9Tab->vp9NlmeansSigmaSSlider->value();
            int p = vp9Tab->vp9NlmeansSigmaPSlider->value();
            int patch = vp9Tab->vp9NlmeansPatchSlider->value();
            QString filterName = vp9Tab->vp9NlmeansGpuCheck->isChecked() ? "knlmeans" : "nlmeans";
            QString patchStr = (filterName == "knlmeans") ? ":patch=" + QString::number(patch) : "";
            videoFilters << QString("%1=s=%2:p=%3%4").arg(filterName).arg(s).arg(p).arg(patchStr);
        }
    }

    // Scale
    double sw = scaleWidthSpin->value();
    double sh = scaleHeightSpin->value();
    QString filterName = scaleFilterBox->currentText();
    if (!qFuzzyCompare(sw, 1.0) || !qFuzzyCompare(sh, 1.0)) {
        QString w = qFuzzyCompare(sw, 1.0) ? "iw" : QString("trunc(iw*%1/2)*2").arg(sw);
        QString h = qFuzzyCompare(sh, 1.0) ? "ih" : QString("trunc(ih*%1/2)*2").arg(sh);
        if (filterName == "spline16" || filterName == "spline36") {
            videoFilters << QString("zscale=w=%1:h=%2:filter=%3").arg(w, h, filterName);
        } else {
            videoFilters << QString("scale=w=%1:h=%2:flags=%3").arg(w, h, filterName.toLower());
        }
        if (scaleRangeBox->currentText() != "input") {
            videoFilters << QString("zscale=range=%1").arg(scaleRangeBox->currentText().toLower());
        }
    }

    // Frame rate
    if (frameRateBox->currentText() != "Original") {
        QString fpsValue = (frameRateBox->currentText() == "Custom") ? customFrameRateBox->text() : frameRateBox->currentText();
        videoFilters << "fps=" + fpsValue;
    }

    // Pixel format
    QString pixFmt;
    if (tenBitCheck->isChecked()) {
        QString f = colorFormatBox->currentText();
        pixFmt = (f == "10-bit 4:2:0") ? "yuv420p10le" : (f == "10-bit 4:2:2") ? "yuv422p10le" : "yuv444p10le";
    } else {
        QString f = eightBitColorFormatBox->currentText();
        pixFmt = (f == "8-bit 4:2:0") ? "yuv420p" : (f == "8-bit 4:2:2") ? "yuv422p" : "yuv444p";
    }
    videoFilters << "format=" + pixFmt;

    // Audio normalize
    if (normalizeAudioCheck->isChecked()) {
        audioFilters << "loudnorm=I=-23:TP=-1.5:LRA=11";
    }

    // Speed
    auto getPercentChange = [](const QString &str) -> double {
        if (str == "0%") return 0.0;
        return str.chopped(1).toDouble();
    };
    double videoPercent = videoSpeedCheck->isChecked() ? getPercentChange(videoSpeedCombo->currentText()) : 0.0;
    double audioPercent = audioSpeedCheck->isChecked() ? getPercentChange(audioSpeedCombo->currentText()) : 0.0;
    double videoMultiplier = 1.0 + videoPercent / 100.0;
    double audioMultiplier = 1.0 + audioPercent / 100.0;

    QString seekTimeStr = "";
    if (seekCheck->isChecked()) {
        int hh = seekHH->text().toInt();
        int mm = seekMM->text().toInt();
        int ss = seekSS->text().toInt();
        seekTimeStr = QString("%1:%2:%3").arg(hh, 2, 10, QChar('0')).arg(mm, 2, 10, QChar('0')).arg(ss, 2, 10, QChar('0'));
    }

    QString outputTimeStr = "";
    if (timeCheck->isChecked()) {
        int hh = timeHH->text().toInt();
        int mm = timeMM->text().toInt();
        int ss = timeSS->text().toInt();
        outputTimeStr = QString("%1:%2:%3").arg(hh, 2, 10, QChar('0')).arg(mm, 2, 10, QChar('0')).arg(ss, 2, 10, QChar('0'));
    }

    if (!qFuzzyCompare(videoMultiplier, 1.0)) {
        double ptsFactor = 1.0 / videoMultiplier;
        videoFilters << QString("setpts=%1*PTS").arg(ptsFactor, 0, 'g', 12);
    }
    if (!qFuzzyCompare(audioMultiplier, 1.0)) {
        auto buildAtempoChain = [](double m) -> QString {
            if (qFuzzyCompare(m, 1.0)) return "";
            QStringList p;
            double t = m;
            if (m > 1.0) {
                while (t > 2.0) { p << "atempo=2.0"; t /= 2.0; }
            } else if (m < 1.0) {
                while (t < 0.5) { p << "atempo=0.5"; t /= 0.5; }
            }
            if (!qFuzzyCompare(t, 1.0)) p << QString("atempo=%1").arg(t, 0, 'g', 12);
            return p.join(",");
        };
        QString chain = buildAtempoChain(audioMultiplier);
        if (!chain.isEmpty()) audioFilters.prepend(chain);
    }

    if (!videoFilters.isEmpty()) {
        QString chain = videoFilters.join(",");
        args << "-vf" << chain;
    }
    if (!audioFilters.isEmpty()) {
        QString chain = audioFilters.join(",");
        args << "-af" << chain;
    }

    // Codec specific args
    if (currentTab == 0) { // AV1
        args << "-c:v" << "libsvtav1";
        args << "-preset" << av1Tab->av1PresetBox->currentText();

        QStringList svtParams;
        QString tune = av1Tab->av1TuneBox->currentText();
        if (tune != "Auto") {
            int tuneVal = 0;
            if (tune == "Subjective SSIM (VQ)") tuneVal = 0;
            else if (tune == "PSNR") tuneVal = 1;
            else if (tune == "SSIM") tuneVal = 2;
            else if (tune == "VMAF") tuneVal = 6;
            else if (tune == "VMAF Neg") tuneVal = 7;
            svtParams << "tune=" + QString::number(tuneVal);
        }
        if (av1Tab->nativeGrainCheck->isChecked()) {
            int strength = av1Tab->grainStrengthSlider->value();
            if (strength > 0) svtParams << "film-grain=" + QString::number(strength);
            int denoise = av1Tab->grainDenoiseCombo->currentIndex();
            svtParams << "film-grain-denoise=" + QString::number(denoise);
        }
        int superResMode = av1Tab->superResModeBox->currentIndex();
        if (superResMode > 0) {
            svtParams << "superres-mode=" + QString::number(superResMode);
            svtParams << "superres-denom=" + QString::number(av1Tab->superResDenomSlider->value());
        }
        int fdLevel = av1Tab->fastDecodeBox->currentIndex();
        if (fdLevel > 0) svtParams << "fast-decode=" + QString::number(fdLevel);
        if (av1Tab->lowLatencyCheck->isChecked()) {
            svtParams << "irefresh-type=1";
            svtParams << "lookahead=0";
        }
        //if (av1Tab->tplModelCheck->isChecked()) svtParams << "enable-tpl-la=1";
        if (av1Tab->tplModelCheck->isChecked()) svtParams << "tpl-la=1";
        svtParams << "enable-cdef=" + QString(av1Tab->enableCdefCheck->isChecked() ? "1" : "0");
        if (av1Tab->av1LookaheadCheck->isChecked()) svtParams << "lookahead=" + QString::number(av1Tab->av1LookaheadSlider->value());
        QString aqModeStr = av1Tab->av1AQModeBox->currentText();
        if (aqModeStr != "Automatic") {
            int aqMode = (aqModeStr == "Disabled") ? 0 : (aqModeStr == "Variance") ? 1 : (aqModeStr == "Complexity") ? 2 : 0;
            svtParams << "aq-mode=" + QString::number(aqMode);
            if (aqModeStr == "Variance") {
                svtParams << "enable-variance-boost=1";
                svtParams << "variance-boost-strength=" + QString::number(av1Tab->av1AQStrengthSlider->value());
            }
        }
        if (av1Tab->av1EnableRCModeCheck->isChecked()) {
            QString mode = av1Tab->av1RCModeBox->currentText();
            if (mode == "QP") svtParams << "qp=" + QString::number(av1Tab->av1QPSlider->value());
            else if (mode == "CRF") svtParams << "crf=" + QString::number(av1Tab->av1CRFSlider->value());
            else if (mode == "VBR") {
                int br = av1Tab->av1VBRBitrateSlider->value();
                args << "-b:v" << QString::number(br) + "k";
                if (av1Tab->av1VBRVBVCheck->isChecked()) {
                    args << "-maxrate" << QString::number(br) + "k";
                    args << "-bufsize" << QString::number(2 * br) + "k";
                }
            }
        } else {
            svtParams << "crf=35";
        }
        if (av1Tab->enableTfCheck->isChecked()) svtParams << "enable-tf=1";
        svtParams << "scm=" + QString::number(av1Tab->screenContentModeBox->currentIndex());

        QString threads = av1Tab->av1ThreadsBox->currentText();
        if (threads != "Automatic") svtParams << "lp=" + threads;

        QString tileRows = av1Tab->av1TileRowsBox->currentText();
        if (tileRows != "Automatic") {
            int log2Rows = (tileRows == "1") ? 0 : (tileRows == "2") ? 1 : (tileRows == "4") ? 2 : 3;
            svtParams << "tile-rows=" + QString::number(log2Rows);
        }
        QString tileColumns = av1Tab->av1TileColumnsBox->currentText();
        if (tileColumns != "Automatic") {
            int log2Cols = (tileColumns == "1") ? 0 : (tileColumns == "2") ? 1 : (tileColumns == "4") ? 2 : 3;
            svtParams << "tile-columns=" + QString::number(log2Cols);
        }

        if (!svtParams.isEmpty()) {
            args << "-svtav1-params" << svtParams.join(":");
        }
        if (av1Tab->av1LevelBox->currentText() != "Auto")
            args << "-level" << av1Tab->av1LevelBox->currentText();

        QString keyText = av1Tab->av1KeyIntBox->currentText();
        if (keyText == "Custom") {
            int mode = av1Tab->av1CustomKeyframeModeBox->currentIndex();
            if (mode == 0) {
                args << "-force_key_frames" << "expr:gte(t,n_forced*5)";
                logBox->append("🔑 Keyframes: Every 5 seconds (fixed time)");
            } else {
                double fps = 0.0;
                QString fpsSetting = frameRateBox->currentText();
                if (fpsSetting == "Custom" && !customFrameRateBox->text().isEmpty()) {
                    fps = customFrameRateBox->text().toDouble();
                } else if (fpsSetting != "Original" && fpsSetting != "Auto") {
                    fps = fpsSetting.toDouble();
                } else {
                    QProcess probe;
                    probe.start("ffprobe", QStringList()
                    << "-v" << "quiet"
                    << "-select_streams" << "v:0"
                    << "-show_entries" << "stream=r_frame_rate,stream=avg_frame_rate"
                    << "-of" << "csv=p=0"
                    << inputFile);
                    probe.waitForFinished(3000);
                    QString raw = probe.readAllStandardOutput().trimmed();
                    logBox->append("🔍 ffprobe raw output: [" + raw + "]");
                    QStringList lines = raw.split('\n', Qt::SkipEmptyParts);
                    for (const QString &line : lines) {
                        QString clean = line.trimmed().remove('[').remove(']').remove(',').remove(' ').simplified();
                        if (clean.contains('/')) {
                            QStringList parts = clean.split('/');
                            if (parts.size() == 2 && parts[1].toDouble() > 0) {
                                fps = parts[0].toDouble() / parts[1].toDouble();
                                if (fps > 5.0) break;
                            }
                        } else if (!clean.isEmpty()) {
                            fps = clean.toDouble();
                            if (fps > 5.0) break;
                        }
                    }
                }
                if (fps < 5.0 || qIsInf(fps) || qIsNaN(fps)) {
                    logBox->append("⚠️ Could not detect framerate. Using safe default -g 240");
                    args << "-g" << "240";
                } else {
                    int gop = qRound(5.0 * fps);
                    if (gop < 10) gop = 240;
                    args << "-g" << QString::number(gop);
                    logBox->append(QString("🔑 Keyframes: Every 5 seconds × framerate (%1 fps → -g %2)")
                    .arg(fps, 0, 'f', 3).arg(gop));
                }
            }
        } else {
            args << "-g" << keyText;
        }
    }
    else if (currentTab == 1) { // x265
        args << "-c:v" << "libx265";
        args << "-preset" << x265Tab->x265PresetBox->currentText();
        if (x265Tab->x265TuneBox->currentText() != "Auto") args << "-tune" << x265Tab->x265TuneBox->currentText();
        if (x265Tab->x265LevelBox->currentText() != "auto") args << "-level" << x265Tab->x265LevelBox->currentText();

        QStringList x265Params;
        x265Params << "deblock=" + QString::number(x265Tab->deblockAlphaSlider->value()) + ":" + QString::number(x265Tab->deblockBetaSlider->value());
        if (x265Tab->pmodeCheck->isChecked()) x265Params << "pmode=1";
        x265Params << "ref=" + x265Tab->refFramesBox->currentText();
        if (x265Tab->weightpCheck->isChecked()) x265Params << "weightp=1";
        if (x265Tab->strongIntraCheck->isChecked()) x265Params << "strong-intra-smoothing=1";
        if (x265Tab->rdoqLevelBox->currentIndex() > 1) x265Params << "rdoq-level=" + QString::number(x265Tab->rdoqLevelBox->currentIndex());
        if (x265Tab->saoCheck->isChecked()) x265Params << "sao=1";
        if (x265Tab->limitRefsBox->currentIndex() > 0) x265Params << "limit-refs=" + QString::number(x265Tab->limitRefsBox->currentIndex());
        if (x265Tab->x265LookaheadCheck->isChecked()) x265Params << "rc-lookahead=" + QString::number(x265Tab->x265LookaheadSlider->value());

        QString aqModeStr = x265Tab->x265AQModeBox->currentText();
        if (aqModeStr != "Automatic") {
            int aq = (aqModeStr == "Disabled") ? 0 : (aqModeStr == "Variance") ? 1 : (aqModeStr == "Auto-Variance") ? 2 : 3;
            args << "-aq-mode" << QString::number(aq);
        }
        x265Params << "aq-strength=" + QString::number(x265Tab->x265AQStrengthSlider->value() / 10.0);
        if (x265Tab->enablePsyRdCheck->isChecked()) x265Params << "psy-rd=1.0";
        if (x265Tab->enableCutreeCheck->isChecked()) x265Params << "cutree=1";

        if (!x265Params.isEmpty()) args << "-x265-params" << x265Params.join(":");

        QString keyText = x265Tab->x265KeyIntBox->currentText();
        if (keyText == "Custom") {
            int mode = x265Tab->x265CustomKeyframeModeBox->currentIndex();
            if (mode == 0) {
                args << "-force_key_frames" << "expr:gte(t,n_forced*5)";
                logBox->append("🔑 Keyframes: Every 5 seconds (fixed time)");
            } else {
                double fps = 0.0;
                QString fpsSetting = frameRateBox->currentText();
                if (fpsSetting == "Custom" && !customFrameRateBox->text().isEmpty()) {
                    fps = customFrameRateBox->text().toDouble();
                } else if (fpsSetting != "Original" && fpsSetting != "Auto") {
                    fps = fpsSetting.toDouble();
                } else {
                    QProcess probe;
                    probe.start("ffprobe", QStringList()
                    << "-v" << "quiet"
                    << "-select_streams" << "v:0"
                    << "-show_entries" << "stream=r_frame_rate,stream=avg_frame_rate"
                    << "-of" << "csv=p=0"
                    << inputFile);
                    probe.waitForFinished(3000);
                    QString raw = probe.readAllStandardOutput().trimmed();
                    logBox->append("🔍 ffprobe raw output: [" + raw + "]");
                    QStringList lines = raw.split('\n', Qt::SkipEmptyParts);
                    for (const QString &line : lines) {
                        QString clean = line.trimmed().remove('[').remove(']').remove(',').remove(' ').simplified();
                        if (clean.contains('/')) {
                            QStringList parts = clean.split('/');
                            if (parts.size() == 2 && parts[1].toDouble() > 0) {
                                fps = parts[0].toDouble() / parts[1].toDouble();
                                if (fps > 5.0) break;
                            }
                        } else if (!clean.isEmpty()) {
                            fps = clean.toDouble();
                            if (fps > 5.0) break;
                        }
                    }
                }
                if (fps < 5.0 || qIsInf(fps) || qIsNaN(fps)) {
                    logBox->append("⚠️ Could not detect framerate. Using safe default -g 240");
                    args << "-g" << "240";
                } else {
                    int gop = qRound(5.0 * fps);
                    if (gop < 10) gop = 240;
                    args << "-g" << QString::number(gop);
                    logBox->append(QString("🔑 Keyframes: Every 5 seconds × framerate (%1 fps → -g %2)")
                    .arg(fps, 0, 'f', 3).arg(gop));
                }
            }
        } else {
            args << "-g" << keyText;
        }
        if (x265Tab->x265ThreadsBox->currentText() != "Automatic") args << "-threads" << x265Tab->x265ThreadsBox->currentText();
        if (x265Tab->x265FrameThreadsBox->currentText() != "Automatic") args << "-frame-threads" << x265Tab->x265FrameThreadsBox->currentText();

        if (x265Tab->x265EnableRCModeCheck->isChecked()) {
            QString mode = x265Tab->x265RCModeBox->currentText();
            if (mode == "QP") args << "-qp" << QString::number(x265Tab->x265QPSlider->value());
            else if (mode == "CRF") args << "-crf" << QString::number(x265Tab->x265CRFSlider->value());
            else if (mode == "ABR") {
                int br = x265Tab->x265ABRBitrateSlider->value();
                args << "-b:v" << QString::number(br) + "k";
                if (x265Tab->x265ABRVBVCheck->isChecked()) {
                    args << "-maxrate" << QString::number(br) + "k";
                    args << "-bufsize" << QString::number(2 * br) + "k";
                }
            } else if (mode == "CBR") {
                int br = x265Tab->x265CBRBitrateSlider->value();
                args << "-b:v" << QString::number(br) + "k";
                args << "-maxrate" << QString::number(br) + "k";
                args << "-bufsize" << QString::number(br) + "k";
            }
        } else {
            args << "-crf" << "23";
        }
    }
    else if (currentTab == 2) { // VP9
        args << "-c:v" << "libvpx-vp9";
        args << "-cpu-used" << vp9Tab->vp9CpuUsedBox->currentText();
        args << "-deadline" << vp9Tab->vp9DeadlineBox->currentText().toLower();
        if (vp9Tab->vp9LookaheadCheck->isChecked()) args << "-lag-in-frames" << QString::number(vp9Tab->vp9LookaheadSlider->value());

        QString aqModeStr = vp9Tab->vp9AQModeBox->currentText();
        if (aqModeStr != "Automatic") {
            int aq = (aqModeStr == "Disabled") ? 0 : (aqModeStr == "Variance") ? 1 : 2;
            args << "-aq-mode" << QString::number(aq);
        }
        if (vp9Tab->vp9ArnrCheck->isChecked()) {
            args << "-arnr-strength" << QString::number(vp9Tab->vp9ArnrStrengthSlider->value());
            args << "-arnr-maxframes" << QString::number(vp9Tab->vp9ArnrMaxFramesSlider->value());
        } else {
            args << "-arnr-strength" << QString::number(vp9Tab->vp9AQStrengthSlider->value());
        }
        if (vp9Tab->vp9TplCheck->isChecked()) args << "-auto-alt-ref" << "1";
        if (vp9Tab->enableRowMtCheck->isChecked()) args << "-row-mt" << "1";
        if (vp9Tab->screenContentCheck->isChecked()) args << "-tune-content" << "screen";

        QString keyText = vp9Tab->vp9KeyIntBox->currentText();
        if (keyText == "Custom") {
            int mode = vp9Tab->vp9CustomKeyframeModeBox->currentIndex();
            if (mode == 0) {
                args << "-force_key_frames" << "expr:gte(t,n_forced*5)";
                logBox->append("🔑 Keyframes: Every 5 seconds (fixed time)");
            } else {
                double fps = 0.0;
                QString fpsSetting = frameRateBox->currentText();
                if (fpsSetting == "Custom" && !customFrameRateBox->text().isEmpty()) {
                    fps = customFrameRateBox->text().toDouble();
                } else if (fpsSetting != "Original" && fpsSetting != "Auto") {
                    fps = fpsSetting.toDouble();
                } else {
                    QProcess probe;
                    probe.start("ffprobe", QStringList()
                    << "-v" << "quiet"
                    << "-select_streams" << "v:0"
                    << "-show_entries" << "stream=r_frame_rate,stream=avg_frame_rate"
                    << "-of" << "csv=p=0"
                    << inputFile);
                    probe.waitForFinished(3000);
                    QString raw = probe.readAllStandardOutput().trimmed();
                    logBox->append("🔍 ffprobe raw output: [" + raw + "]");
                    QStringList lines = raw.split('\n', Qt::SkipEmptyParts);
                    for (const QString &line : lines) {
                        QString clean = line.trimmed().remove('[').remove(']').remove(',').remove(' ').simplified();
                        if (clean.contains('/')) {
                            QStringList parts = clean.split('/');
                            if (parts.size() == 2 && parts[1].toDouble() > 0) {
                                fps = parts[0].toDouble() / parts[1].toDouble();
                                if (fps > 5.0) break;
                            }
                        } else if (!clean.isEmpty()) {
                            fps = clean.toDouble();
                            if (fps > 5.0) break;
                        }
                    }
                }
                if (fps < 5.0 || qIsInf(fps) || qIsNaN(fps)) {
                    logBox->append("⚠️ Could not detect framerate. Using safe default -g 240");
                    args << "-g" << "240";
                } else {
                    int gop = qRound(5.0 * fps);
                    if (gop < 10) gop = 240;
                    args << "-g" << QString::number(gop);
                    logBox->append(QString("🔑 Keyframes: Every 5 seconds × framerate (%1 fps → -g %2)")
                    .arg(fps, 0, 'f', 3).arg(gop));
                }
            }
        } else {
            args << "-g" << keyText;
        }
        if (vp9Tab->vp9ThreadsBox->currentText() != "Automatic") args << "-threads" << vp9Tab->vp9ThreadsBox->currentText();

        QString tileColumns = vp9Tab->vp9TileColumnsBox->currentText();
        int log2Cols = (tileColumns == "0" || tileColumns == "1") ? 0 : (tileColumns == "2") ? 1 : (tileColumns == "4") ? 2 : 3;
        args << "-tile-columns" << QString::number(log2Cols);

        QString tileRows = vp9Tab->vp9TileRowsBox->currentText();
        if (tileRows != "Automatic") {
            int log2Rows = (tileRows == "1") ? 0 : (tileRows == "2") ? 1 : (tileRows == "4") ? 2 : 3;
            args << "-row-mt" << "1";
            args << "-tile-rows" << QString::number(log2Rows);
        }

        args << "-qmax" << QString::number(vp9Tab->vp9QMaxSlider->value());

        if (vp9Tab->vp9EnableRCModeCheck->isChecked()) {
            QString mode = vp9Tab->vp9RCModeBox->currentText();
            if (mode == "CRF") {
                args << "-crf" << QString::number(vp9Tab->vp9CRFSlider->value());
                args << "-b:v" << "0";
            } else {
                int br = vp9Tab->vp9BitrateSlider->value();
                args << "-b:v" << QString::number(br) + "k";
                if (mode == "CBR") {
                    args << "-maxrate" << QString::number(br) + "k";
                    args << "-bufsize" << QString::number(br) + "k";
                }
            }
        } else {
            args << "-crf" << "31";
            args << "-b:v" << "0";
        }
    }
    else if (currentTab == 3) { // x264
        args << "-c:v" << "libx264";
        args << "-preset" << x264Tab->x264PresetBox->currentText();
        if (x264Tab->x264TuneBox->currentText() != "Auto") args << "-tune" << x264Tab->x264TuneBox->currentText();
        QString profile = x264Tab->x264ProfileBox->currentText();
        if (tenBitCheck->isChecked() || profile == "auto") {
            profile = "high10";
        }
        if (tenBitCheck->isChecked() && (profile == "baseline" || profile == "main")) {
            profile = "high10";
            logBox->append("⚠️ 10-bit enabled → forced high10 profile (baseline/main not supported)");
        }
        args << "-profile:v" << profile;

        if (tenBitCheck->isChecked()) {
            logBox->append("✅ Using high10 profile (10-bit color)");
        }

        if (x264Tab->x264LevelBox->currentText() != "auto") args << "-level" << x264Tab->x264LevelBox->currentText();

        QStringList x264Params;
        x264Params << "deblock=" + QString::number(x264Tab->deblockAlphaSlider->value()) + ":" + QString::number(x264Tab->deblockBetaSlider->value());
        if (x264Tab->pmodeCheck->isChecked()) x264Params << "pmode=1";
        x264Params << "ref=" + x264Tab->refFramesBox->currentText();
        if (x264Tab->weightpCheck->isChecked()) x264Params << "weightp=1";
        if (x264Tab->strongIntraCheck->isChecked()) x264Params << "strong-intra-smoothing=1";
        if (x264Tab->rdoqLevelBox->currentIndex() > 1) x264Params << "rdoq-level=" + QString::number(x264Tab->rdoqLevelBox->currentIndex());
        if (x264Tab->saoCheck->isChecked()) x264Params << "sao=1";
        if (x264Tab->limitRefsBox->currentIndex() > 0) x264Params << "limit-refs=" + QString::number(x264Tab->limitRefsBox->currentIndex());
        if (x264Tab->x264LookaheadCheck->isChecked()) x264Params << "rc-lookahead=" + QString::number(x264Tab->x264LookaheadSlider->value());
        QString aqModeStr = x264Tab->x264AQModeBox->currentText();
        if (aqModeStr != "Automatic") {
            int aq = (aqModeStr == "Disabled") ? 0 : (aqModeStr == "Variance") ? 1 : (aqModeStr == "Auto-Variance") ? 2 : 3;
            args << "-aq-mode" << QString::number(aq);
        }
        x264Params << "aq-strength=" + QString::number(x264Tab->x264AQStrengthSlider->value() / 10.0);
        if (x264Tab->enablePsyRdCheck->isChecked()) x264Params << "psy-rd=1.0";
        if (x264Tab->enableCutreeCheck->isChecked()) x264Params << "cutree=1";
        if (!x264Params.isEmpty()) args << "-x264-params" << x264Params.join(":");
        QString keyText = x264Tab->x264KeyIntBox->currentText();
        if (keyText == "Custom") {
            int mode = x264Tab->x264CustomKeyframeModeBox->currentIndex();
            if (mode == 0) {
                args << "-force_key_frames" << "expr:gte(t,n_forced*5)";
                logBox->append("🔑 Keyframes: Every 5 seconds (fixed time)");
            } else {
                double fps = 0.0;
                QString fpsSetting = frameRateBox->currentText();
                if (fpsSetting == "Custom" && !customFrameRateBox->text().isEmpty()) {
                    fps = customFrameRateBox->text().toDouble();
                } else if (fpsSetting != "Original" && fpsSetting != "Auto") {
                    fps = fpsSetting.toDouble();
                } else {
                    QProcess probe;
                    probe.start("ffprobe", QStringList()
                    << "-v" << "quiet"
                    << "-select_streams" << "v:0"
                    << "-show_entries" << "stream=r_frame_rate,stream=avg_frame_rate"
                    << "-of" << "csv=p=0"
                    << inputFile);
                    probe.waitForFinished(3000);
                    QString raw = probe.readAllStandardOutput().trimmed();
                    logBox->append("🔍 ffprobe raw output: [" + raw + "]");
                    QStringList lines = raw.split('\n', Qt::SkipEmptyParts);
                    for (const QString &line : lines) {
                        QString clean = line.trimmed().remove('[').remove(']').remove(',').remove(' ').simplified();
                        if (clean.contains('/')) {
                            QStringList parts = clean.split('/');
                            if (parts.size() == 2 && parts[1].toDouble() > 0) {
                                fps = parts[0].toDouble() / parts[1].toDouble();
                                if (fps > 5.0) break;
                            }
                        } else if (!clean.isEmpty()) {
                            fps = clean.toDouble();
                            if (fps > 5.0) break;
                        }
                    }
                }
                if (fps < 5.0 || qIsInf(fps) || qIsNaN(fps)) {
                    logBox->append("⚠️ Could not detect framerate. Using safe default -g 240");
                    args << "-g" << "240";
                } else {
                    int gop = qRound(5.0 * fps);
                    if (gop < 10) gop = 240;
                    args << "-g" << QString::number(gop);
                    logBox->append(QString("🔑 Keyframes: Every 5 seconds × framerate (%1 fps → -g %2)")
                    .arg(fps, 0, 'f', 3).arg(gop));
                }
            }
        } else {
            args << "-g" << keyText;
        }
        if (x264Tab->x264ThreadsBox->currentText() != "Automatic") args << "-threads" << x264Tab->x264ThreadsBox->currentText();
        if (x264Tab->x264EnableRCModeCheck->isChecked()) {
            QString mode = x264Tab->x264RCModeBox->currentText();
            if (mode == "QP") args << "-qp" << QString::number(x264Tab->x264QPSlider->value());
            else if (mode == "CRF") args << "-crf" << QString::number(x264Tab->x264CRFSlider->value());
            else if (mode == "ABR") {
                int br = x264Tab->x264ABRBitrateSlider->value();
                args << "-b:v" << QString::number(br) + "k";
                if (x264Tab->x264ABRVBVCheck->isChecked()) {
                    args << "-maxrate" << QString::number(br) + "k";
                    args << "-bufsize" << QString::number(2 * br) + "k";
                }
            } else if (mode == "CBR") {
                int br = x264Tab->x264CBRBitrateSlider->value();
                args << "-b:v" << QString::number(br) + "k";
                args << "-maxrate" << QString::number(br) + "k";
                args << "-bufsize" << QString::number(br) + "k";
            }
        } else {
            args << "-crf" << "23";
        }
    }

    // Audio
    if (!isCombine && !isTrim && currentTab <= 3) {
        QCheckBox *audioCheck = nullptr;
        QComboBox *audioCodecBox = nullptr;
        QComboBox *audioSampleRateBox = nullptr;
        QComboBox *audioBitrateBox = nullptr;
        QComboBox *vbrModeBox = nullptr;
        QComboBox *aacQualityBox = nullptr;
        QComboBox *mp3VbrBox = nullptr;
        QComboBox *flacCompressionBox = nullptr;
        QComboBox *vorbisQualityBox = nullptr;

        if (currentTab == 0) {
            audioCheck = av1Tab->av1AudioCheck;
            audioCodecBox = av1Tab->av1AudioCodecBox;
            audioSampleRateBox = av1Tab->av1AudioSampleRateBox;
            audioBitrateBox = av1Tab->av1AudioBitrateBox;
            vbrModeBox = av1Tab->av1VbrModeBox;
            aacQualityBox = av1Tab->av1AacQualityBox;
            mp3VbrBox = av1Tab->av1Mp3VbrBox;
            flacCompressionBox = av1Tab->av1FlacCompressionBox;
            vorbisQualityBox = av1Tab->av1VorbisQualityBox;
        } else if (currentTab == 1) {
            audioCheck = x265Tab->x265AudioCheck;
            audioCodecBox = x265Tab->x265AudioCodecBox;
            audioSampleRateBox = x265Tab->x265AudioSampleRateBox;
            audioBitrateBox = x265Tab->x265AudioBitrateBox;
            vbrModeBox = x265Tab->x265VbrModeBox;
            aacQualityBox = x265Tab->x265AacQualityBox;
            mp3VbrBox = x265Tab->x265Mp3VbrBox;
            flacCompressionBox = x265Tab->x265FlacCompressionBox;
            vorbisQualityBox = x265Tab->x265VorbisQualityBox;
        } else if (currentTab == 2) {
            audioCheck = vp9Tab->vp9AudioCheck;
            audioCodecBox = vp9Tab->vp9AudioCodecBox;
            audioSampleRateBox = vp9Tab->vp9AudioSampleRateBox;
            audioBitrateBox = vp9Tab->vp9AudioBitrateBox;
            vbrModeBox = vp9Tab->vp9VbrModeBox;
            vorbisQualityBox = vp9Tab->vp9VorbisQualityBox;
        } else if (currentTab == 3) {
            audioCheck = x264Tab->x264AudioCheck;
            audioCodecBox = x264Tab->x264AudioCodecBox;
            audioSampleRateBox = x264Tab->x264AudioSampleRateBox;
            audioBitrateBox = x264Tab->x264AudioBitrateBox;
            vbrModeBox = x264Tab->x264VbrModeBox;
            aacQualityBox = x264Tab->x264AacQualityBox;
            mp3VbrBox = x264Tab->x264Mp3VbrBox;
            flacCompressionBox = x264Tab->x264FlacCompressionBox;
            vorbisQualityBox = x264Tab->x264VorbisQualityBox;
    }

        if (audioCheck && audioCheck->isChecked()) {
            QString audioCodecStr = audioCodecBox->currentText();
            QString encoder = (audioCodecStr == "opus") ? "libopus" :
            (audioCodecStr == "vorbis") ? "libvorbis" :
            (audioCodecStr == "mp3") ? "libmp3lame" :
            (audioCodecStr == "aac") ? "aac" :
            (audioCodecStr == "flac") ? "flac" : audioCodecStr;

            args << "-c:a" << encoder;
            auto getSampleRateInHz = [](const QString& sampleRateStr) -> QString {
                QString numericPart = sampleRateStr.split(" ").first();
                bool ok; double kHz = numericPart.toDouble(&ok);
                return ok ? QString::number(static_cast<int>(kHz * 1000)) : "48000";
            };
            args << "-ar" << getSampleRateInHz(audioSampleRateBox->currentText());

            auto getBitrateValue = [](const QString& bitrateStr) -> QString {
                QString numericPart = bitrateStr.split(" ").first();
                return numericPart.isEmpty() ? "128" : numericPart;
            };
            args << "-b:a" << getBitrateValue(audioBitrateBox->currentText()) + "k";

            if (audioCodecStr == "opus" && vbrModeBox) {
                QString vbr = vbrModeBox->currentText();
                if (vbr == "Constrained") args << "-vbr" << "constrained";
                else if (vbr == "Off") args << "-vbr" << "off";
            } else if (audioCodecStr == "aac" && aacQualityBox) {
                QString quality = aacQualityBox->currentText();
                if (quality != "Disabled") args << "-q:a" << quality;
            } else if (audioCodecStr == "mp3" && mp3VbrBox) {
                QString vbr = mp3VbrBox->currentText();
                if (vbr != "Disabled") args << "-qscale:a" << vbr;
            } else if (audioCodecStr == "flac" && flacCompressionBox) {
                args << "-compression_level" << flacCompressionBox->currentText();
            } else if (audioCodecStr == "vorbis" && vorbisQualityBox) {
                QString quality = vorbisQualityBox->currentText();
                if (quality != "Disabled") args << "-q:a" << quality;
            }
        } else {
            args << "-an";
        }
    }

    QString ffmpegPath = settings->value("ffmpegPath", "/usr/bin/ffmpeg").toString();
    if (ffmpegPath.isEmpty()) ffmpegPath = "/usr/bin/ffmpeg";

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString svtAv1Path = settings->value("svtAv1Path", "").toString();
    if (!svtAv1Path.isEmpty()) {
        QString existing = env.value("LD_LIBRARY_PATH");
        env.insert("LD_LIBRARY_PATH", existing.isEmpty() ? svtAv1Path : existing + ":" + svtAv1Path);
    }

    convertButton->setEnabled(false);
    cancelButton->setEnabled(true);
    conversionProgress->setVisible(true);
    conversionProgress->setValue(0);

    if (isCombine) {
        combineTab->startConcatenation();
        return;
    }

    if (isTrim) {
        logBox->clear();
        logBox->append("=== LOSSLESS TRIM STARTED (PURE STREAM COPY) ===");
        logBox->append("All filters, scaling, speed changes, and codec settings are ignored.");

        QString inputFileTrim = selectedFilesBox->text().trimmed();
        trimTab->stopPreviewPlayer();

        auto segList = trimTab->getSegments();
        if (segList.isEmpty()) {
            QMessageBox::warning(this, "Error", "Add at least one segment in the Trim tab.");
            convertButton->setEnabled(true);
            cancelButton->setEnabled(false);
            conversionProgress->setVisible(false);
            return;
        }

        bool individual = trimTab->isIndividualSegments();
        QString originalExt = QFileInfo(inputFileTrim).suffix().toLower();
        if (originalExt.isEmpty()) originalExt = "mkv";
        QString containerExt = "." + originalExt;

        QString outputDirTrim = outputDirBox->text();
        QString baseNameTrim = outputNameBox->text().isEmpty() ? "Output" : outputNameBox->text();

        QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) +
        "/ffmpeg_converter_trim_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
        QDir().mkpath(tempDir);

        QStringList tempFiles;
        for (int i = 0; i < segList.size(); ++i) {
            tempFiles << tempDir + QString("/seg_%1%2").arg(i + 1, 3, 10, QChar('0')).arg(containerExt);
        }

        QVector<double> segDurations;
        double totalDur = 0.0;
        for (const auto& seg : segList) {
            double d = (seg.second - seg.first) / 1000.0;
            segDurations << d;
            totalDur += d;
        }

        auto formatTimeLocal = [](qint64 ms) -> QString {
            if (ms <= 0) return "00:00:00.00";
            qint64 secs = ms / 1000;
            int h = secs / 3600;
            int m = (secs % 3600) / 60;
            int s = secs % 60;
            int cs = (ms % 1000) / 10;
            return QString("%1:%2:%3.%4")
            .arg(h, 2, 10, QChar('0'))
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0'))
            .arg(cs, 2, 10, QChar('0'));
        };

        struct TrimState {
            int currentSeg = 0;
            double completedDur = 0.0;
        };
        auto state = std::make_shared<TrimState>();

        std::function<void()> encodeNextSegment;
        encodeNextSegment = [this, state, segList, segDurations, tempFiles, tempDir, inputFileTrim, outputDirTrim, baseNameTrim, containerExt, individual, env, ffmpegPath, formatTimeLocal, encodeNextSegment]() mutable {
            if (state->currentSeg >= segList.size()) {
                if (individual) {
                    logBox->append("=== SAVING INDIVIDUAL SEGMENTS (NO CONCATENATION) ===");
                    QString firstSavedFile;
                    for (int i = 0; i < tempFiles.size(); ++i) {
                        QString src = tempFiles[i];
                        QString partName = baseNameTrim + "_part" + QString("%1").arg(i + 1, 3, 10, QChar('0')) + containerExt;
                        QString dest = QDir::cleanPath(outputDirTrim + "/" + partName);
                        if (QFile::exists(dest)) {
                            if (overwriteCheck->isChecked()) {
                                QFile::remove(dest);
                            } else {
                                int num = 1;
                                QString newPartName;
                                QString newDest;
                                do {
                                    newPartName = baseNameTrim + "_part" + QString("%1").arg(i + 1, 3, 10, QChar('0')) + " (" + QString::number(num) + ")";
                                    newDest = QDir::cleanPath(outputDirTrim + "/" + newPartName + containerExt);
                                    num++;
                                } while (QFile::exists(newDest));
                                dest = newDest;
                            }
                        }
                        QFile::copy(src, dest);
                        QString savedName = QFileInfo(dest).fileName();
                        logBox->append("Saved segment: " + savedName);
                        if (i == 0) firstSavedFile = dest;
                    }
                    if (!firstSavedFile.isEmpty()) showConversionNotification(firstSavedFile);
                    QDir(tempDir).removeRecursively();
                    trimTab->restartPreviewPlayer();
                    convertButton->setEnabled(true);
                    cancelButton->setEnabled(false);
                    conversionProgress->setVisible(false);
                } else {
                    QString finalFile = QDir::cleanPath(outputDirTrim + "/" + baseNameTrim + containerExt);
                    QString resolvedFinal = finalFile;
                    if (QFile::exists(resolvedFinal)) {
                        if (overwriteCheck->isChecked()) {
                            QFile::remove(resolvedFinal);
                        } else {
                            int counter = 1;
                            QFileInfo fi(resolvedFinal);
                            QString dir = fi.absolutePath();
                            QString base = fi.completeBaseName();
                            QString ext = fi.suffix().isEmpty() ? "" : "." + fi.suffix();
                            do {
                                resolvedFinal = QDir::cleanPath(dir + "/" + base + " (" + QString::number(counter) + ")" + ext);
                                counter++;
                            } while (QFile::exists(resolvedFinal));
                        }
                    }
                    QString listFile = tempDir + "/concat_list.txt";
                    QFile f(listFile);
                    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        QTextStream out(&f);
                        for (const QString& tf : tempFiles) {
                            out << "file '" << QDir::toNativeSeparators(tf) << "'\n";
                        }
                    }
                    QStringList concatArgs;
                    concatArgs << "-f" << "concat" << "-safe" << "0" << "-i" << listFile << "-c" << "copy" << resolvedFinal;

                    QProcess *concatProc = new QProcess();
                    concatProc->setProcessEnvironment(env);
                    concatProc->start(ffmpegPath, concatArgs);

                    connect(concatProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
                            [this, concatProc, resolvedFinal, tempDir, concatArgs](int, QProcess::ExitStatus) {
                                if (concatProc->exitCode() == 0) {
                                    logBox->append("✅ Trim complete: " + QFileInfo(resolvedFinal).fileName());
                                    showConversionNotification(resolvedFinal);
                                } else {
                                    logBox->append("❌ Concatenation failed.");
                                }
                                QDir(tempDir).removeRecursively();
                                trimTab->restartPreviewPlayer();
                                convertButton->setEnabled(true);
                                cancelButton->setEnabled(false);
                                conversionProgress->setVisible(false);
                                concatProc->deleteLater();
                            });
                }
                return;
            }

            const auto& seg = segList[state->currentSeg];
            double thisDur = segDurations[state->currentSeg];

            QStringList segArgs;
            segArgs << "-c" << "copy" << "-map" << "0" << "-avoid_negative_ts" << "make_zero";
            if (preserveMetadataCheck->isChecked()) segArgs << "-map_metadata" << "0";
            if (removeChaptersCheck->isChecked()) segArgs << "-map_chapters" << "-1";
            segArgs << "-ss" << QString::number(seg.first / 1000.0, 'f', 6);
            segArgs << "-t" << QString::number(thisDur, 'f', 6);

            logBox->append(QString("Encoding segment %1/%2 (%3 → %4)")
            .arg(state->currentSeg + 1).arg(segList.size())
            .arg(formatTimeLocal(seg.first)).arg(formatTimeLocal(seg.second)));

            state->currentSeg++;
            QString tempOut = tempFiles[state->currentSeg - 1];

            QString baseTemp = QFileInfo(tempOut).completeBaseName();
            converter->startConversion(inputFileTrim, QFileInfo(tempOut).absolutePath(), baseTemp, segArgs, false, containerExt, "copy", ffmpegPath, env, true, "", "", 1.0);
        };

        connect(converter, &Converter::progressUpdated, this, [this, state, totalDur, segDurations](int value) {
            if (state->currentSeg <= 0 || segDurations.isEmpty()) return;
            double segProgress = (value / 100.0) * segDurations[state->currentSeg - 1];
            int overall = static_cast<int>((state->completedDur + segProgress) / totalDur * 100.0);
            conversionProgress->setValue(overall);
        });

        connect(converter, &Converter::conversionFinished, this, [this, state, segDurations, encodeNextSegment]() mutable {
            if (state->currentSeg > 0) state->completedDur += segDurations[state->currentSeg - 1];
            QTimer::singleShot(6000, encodeNextSegment); // small delay for stability
        });

        state->currentSeg = 0;
        state->completedDur = 0.0;
        conversionProgress->setValue(0);
        encodeNextSegment();
        return;
    }

    converter->startConversion(inputFile, outputDir, baseName, args, twoPass, extension, codecStr, ffmpegPath, env, overwriteCheck->isChecked(), seekTimeStr, outputTimeStr, videoMultiplier);
}
