#include "csdtitlebarbutton.h"

#include "csdtitlebar.h"

#include <QEvent>
#include <QPropertyAnimation>
#include <QStyleOption>
#include <QStylePainter>

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

double TitleBarButton::fader() const {
    return this->m_fader;
}

void TitleBarButton::setFader(double value) {
    this->m_fader = value;
    this->update();
}

QColor TitleBarButton::hoverColor() const {
    return this->m_hoverColor;
}

void TitleBarButton::setHoverColor(QColor hoverColor) {
    this->m_hoverColor = std::move(hoverColor);
}

bool TitleBarButton::keepDown() const {
    return this->m_keepDown;
}

void TitleBarButton::setKeepDown(bool keepDown) {
    this->m_keepDown = keepDown;
    this->update();
}

bool TitleBarButton::event(QEvent *event) {
    if (this->isDown()) {
        return QPushButton::event(event);
    }
    switch (event->type()) {
    case QEvent::Enter: {
        auto animation = new QPropertyAnimation(this, "fader");
        animation->setDuration(125);
        animation->setEndValue(1.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
        break;
    }
    case QEvent::Leave: {
        auto animation = new QPropertyAnimation(this, "fader");
        animation->setDuration(125);
        animation->setEndValue(0.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
        break;
    }
    default:
        break;
    }
    return QPushButton::event(event);
}

void TitleBarButton::paintEvent([[maybe_unused]] QPaintEvent *event) {
    auto stylePainter = QStylePainter(this);
    auto styleOptionButton = QStyleOptionButton();
    styleOptionButton.initFrom(this);
    styleOptionButton.features = QStyleOptionButton::None;
    styleOptionButton.text = this->text();
    styleOptionButton.icon = this->icon();
    styleOptionButton.iconSize = this->iconSize();

    const auto hoverColor = [this]() -> QColor {
        auto col = this->m_role == Role::Close ? QColor(232, 17, 35, 229)
                                               : this->m_hoverColor;
        if (!this->m_keepDown) {
            col.setAlpha(static_cast<int>(this->m_fader * col.alpha()));
        }
        if (!this->isEnabled() || this->m_role == Role::CaptionIcon) {
            col.setAlpha(0);
        }
        return col;
    }();
    const bool isHovered = styleOptionButton.state & QStyle::State_MouseOver;

    switch (this->m_role) {
    case Role::CaptionIcon: {
        break;
    }
    case Role::Minimize: {
        if (isHovered) {
            styleOptionButton.icon =
                QIcon(":/resources/chrome-minimize-dark.svg");
        }
        break;
    }
    case Role::MaximizeRestore: {
        if (isHovered) {
            if (this->window()->windowState() & Qt::WindowMaximized) {
                styleOptionButton.icon =
                    QIcon(":/resources/chrome-restore-dark.svg");
            } else {
                styleOptionButton.icon =
                    QIcon(":/resources/chrome-maximize-dark.svg");
            }
        }
        break;
    }
    case Role::Close: {
        if (isHovered) {
            styleOptionButton.icon =
                QIcon(":/resources/chrome-close-light.svg");
        }
        break;
    }
    case Role::Tool: {
        break;
    }
    }

    stylePainter.setRenderHint(QPainter::Antialiasing, false);
    stylePainter.setPen(Qt::NoPen);
    stylePainter.setBrush(QBrush(hoverColor));
    stylePainter.drawRect(styleOptionButton.rect);
    stylePainter.drawControl(QStyle::CE_PushButtonLabel, styleOptionButton);
}

} // namespace CSD
