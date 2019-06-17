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
    if (this->m_active) {
        styleOptionButton.palette.setColor(QPalette::ButtonText, Qt::white);
    } else {
        styleOptionButton.palette.setColor(QPalette::ButtonText, Qt::gray);
    }

    bool hovered = styleOptionButton.state & QStyle::State_MouseOver;
    if (hovered && !(this->m_role == Role::CaptionIcon)) {
        QBrush brush = styleOptionButton.palette.brush(QPalette::Button);
        if (this->m_role == Role::Close) {
            brush.setColor(Qt::red);
            if (!this->m_active) {
                styleOptionButton.palette.setColor(QPalette::ButtonText,
                                                   Qt::white);
            }
        } else if (this->isActive()) {
            QColor brushColor = Qt::lightGray;
            brushColor.setAlpha(50);
            brush.setColor(brushColor);
        } else {
            styleOptionButton.palette.setColor(QPalette::ButtonText,
                                               Qt::white);
        }
        qDrawShadePanel(&stylePainter,
                        styleOptionButton.rect,
                        styleOptionButton.palette,
                        false,
                        0,
                        &brush);
    }
    stylePainter.drawControl(QStyle::CE_PushButtonLabel, styleOptionButton);
}

} // namespace CSD
