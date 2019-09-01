#include "settings.h"

#include <QSettings>

#include <optional>

namespace CSD::Internal {

static CaptionButtonStyleType
toUnderlying(CaptionButtonStyle captionButtonStyle) {
    return static_cast<CaptionButtonStyleType>(captionButtonStyle);
}

static std::optional<CaptionButtonStyle>
fromUnderlying(CaptionButtonStyleType value) {
    if (toUnderlying(CaptionButtonStyle::custom) == value) {
        return CaptionButtonStyle::custom;
    }
    if (toUnderlying(CaptionButtonStyle::win) == value) {
        return CaptionButtonStyle::win;
    }
    if (toUnderlying(CaptionButtonStyle::mac) == value) {
        return CaptionButtonStyle::mac;
    }
    return std::nullopt;
}

void Settings::save(QSettings *settings) const {
    settings->beginGroup("CSDPlugin");
    settings->setValue("CaptionButtonStyle",
                       toUnderlying((this->captionButtonStyle)));
    settings->endGroup();
    settings->sync();
}

void Settings::load(QSettings *settings) {
    settings->beginGroup("CSDPlugin");
    this->captionButtonStyle =
        fromUnderlying(settings
                           ->value("CaptionButtonStyle",
                                   toUnderlying(CaptionButtonStyle::custom))
                           .toInt())
            .value_or(CaptionButtonStyle::custom);
    settings->endGroup();
}

bool Settings::equals(const Settings &other) const {
    return this->captionButtonStyle == other.captionButtonStyle;
}

bool operator==(Settings &s1, Settings &s2) {
    return s1.equals(s2);
}

bool operator!=(Settings &s1, Settings &s2) {
    return !s1.equals(s2);
}

} // namespace CSD::Internal
