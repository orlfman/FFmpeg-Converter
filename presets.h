#ifndef PRESETS_H
#define PRESETS_H

#include <QComboBox>
#include <QCheckBox>
#include "av1tab.h"
#include "x265tab.h"
#include "x264tab.h"
#include "vp9tab.h"

class Presets
{
public:
    static void connectPresets(
        QComboBox* presetCombo,
        QTabWidget* codecTabs,
        Av1Tab* av1Tab,
        X265Tab* x265Tab,
        X264Tab* x264Tab,
        Vp9Tab* vp9Tab,
        QCheckBox* eightBitCheck,
        QComboBox* eightBitColorFormatBox,
        QCheckBox* tenBitCheck,
        QComboBox* colorFormatBox
    );
};

#endif // PRESETS_H
