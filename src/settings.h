#pragma once

#include "captionbuttonstyle.h"

class QSettings;

namespace CSD::Internal {

struct Settings {
    CaptionButtonStyle captionButtonStyle = CaptionButtonStyle::custom;

    void save(QSettings *settings) const;
    void load(QSettings *settings);
    bool equals(const Settings &other) const;
};

bool operator==(Settings &s1, Settings &s2);
bool operator!=(Settings &s1, Settings &s2);

} // namespace CSD::Internal
