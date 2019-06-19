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

bool TitleBarButton::isActive() const {
    return this->m_active;
}

void TitleBarButton::setActive(bool active) {
    this->m_active = active;
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
    const bool isActive = this->m_active;

    switch (this->m_role) {
    case Role::CaptionIcon: {
        break;
    }
    case Role::MenuBarItem: {
        if (isActive) {
            styleOptionButton.palette.setColor(QPalette::ButtonText,
                                               QColor(171, 178, 191));
        } else {
            styleOptionButton.palette.setColor(QPalette::ButtonText,
                                               Qt::darkGray);
        }
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
    case Role::Minimize: {
        [[fallthrough]];
    }
    case Role::MaximizeRestore: {
        if (isActive) {
            styleOptionButton.palette.setColor(QPalette::ButtonText,
                                               Qt::white);
        } else {
            styleOptionButton.palette.setColor(QPalette::ButtonText,
                                               Qt::darkGray);
        }
        if (isHovered) {
            styleOptionButton.palette.setColor(QPalette::ButtonText,
                                               Qt::white);
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
        if (isActive) {
            styleOptionButton.palette.setColor(QPalette::ButtonText,
                                               Qt::white);
        } else {
            styleOptionButton.palette.setColor(QPalette::ButtonText,
                                               Qt::darkGray);
        }
        if (isHovered) {
            styleOptionButton.palette.setColor(QPalette::ButtonText,
                                               Qt::white);
            buttonBrush.setColor(Qt::red);
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
