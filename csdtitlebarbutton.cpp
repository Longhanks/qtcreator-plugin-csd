#include "csdtitlebarbutton.h"

#include "csdtitlebar.h"

#include <QStyle>
#include <QStyleOption>
#include <QStylePainter>
#include <Qt>
#include <qdrawutil.h>

namespace CSD {

TitleBarButton::TitleBarButton(Role role, TitleBar *parent)
    : TitleBarButton(QIcon(), QString(), role, parent) {}

TitleBarButton::TitleBarButton(const QString &text,
                               Role role,
                               TitleBar *parent)
    : TitleBarButton(QIcon(), text, role, parent) {}

TitleBarButton::TitleBarButton(const QIcon &icon,
                               const QString &text,
                               Role role,
                               TitleBar *parent)
    : QPushButton(icon, text, parent), m_role(role) {
    this->setAttribute(Qt::WidgetAttribute::WA_Hover, true);
}

void TitleBarButton::paintEvent([[maybe_unused]] QPaintEvent *event) {
    auto stylePainter = QStylePainter(this);
    auto styleOptionButton = QStyleOptionButton();
    styleOptionButton.initFrom(this);
    styleOptionButton.features = QStyleOptionButton::None;
    styleOptionButton.text = this->text();
    styleOptionButton.icon = this->icon();
    styleOptionButton.iconSize = this->iconSize();

    QBrush buttonBrush = styleOptionButton.palette.brush(QPalette::Button);
    const auto hoverColor = QColor(171, 178, 191, 75);
    const bool isHovered = styleOptionButton.state & QStyle::State_MouseOver;

    switch (this->m_role) {
    case Role::CaptionIcon: {
        break;
    }
    case Role::Minimize: {
        [[fallthrough]];
    }
    case Role::MaximizeRestore: {
        if (isHovered) {
            buttonBrush.setColor(hoverColor);
            qDrawShadePanel(&stylePainter,
                            styleOptionButton.rect,
                            styleOptionButton.palette,
                            false,
                            0,
                            &buttonBrush);
        }
        break;
    }
    case Role::Close: {
        if (isHovered) {
            buttonBrush.setColor(QColor::fromRgb(232, 17, 35, 229));
            qDrawShadePanel(&stylePainter,
                            styleOptionButton.rect,
                            styleOptionButton.palette,
                            false,
                            0,
                            &buttonBrush);
        }
        break;
    }
    }

    stylePainter.drawControl(QStyle::CE_PushButtonLabel, styleOptionButton);
}

} // namespace CSD
