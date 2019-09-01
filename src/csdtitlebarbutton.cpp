#include "csdtitlebarbutton.h"

#include "csdtitlebar.h"

#include <utils/icon.h>
#include <utils/stylehelper.h>

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
    auto *titleBar = static_cast<TitleBar *>(this->parent());

    auto stylePainter = QStylePainter(this);
    auto styleOptionButton = QStyleOptionButton();
    styleOptionButton.initFrom(this);
    styleOptionButton.features = QStyleOptionButton::None;
    styleOptionButton.text = this->text();
    styleOptionButton.icon =
        this->m_role == Role::Tool ? QIcon() : this->icon();
    styleOptionButton.iconSize = this->iconSize();

    const auto hoverColor = [titleBar, this]() -> QColor {
        auto col = this->m_role == Role::Close ? QColor(232, 17, 35, 229)
                                               : this->m_hoverColor;
        if (!this->m_keepDown) {
            col.setAlpha(static_cast<int>(this->m_fader * col.alpha()));
        }

        bool isMacCaptionStyle =
            titleBar->captionButtonStyle() == CaptionButtonStyle::mac;
        bool isMacCaptionButton =
            (isMacCaptionStyle && this->m_role == Role::Minimize) ||
            (isMacCaptionStyle && this->m_role == Role::MaximizeRestore) ||
            (isMacCaptionStyle && this->m_role == Role::Close);

        if (!this->isEnabled() || this->m_role == Role::CaptionIcon ||
            isMacCaptionButton) {
            col.setAlpha(0);
        }

        return col;
    }();

    // On mac style, all caption buttons get the 'hovered' style if any of them
    // is hovered - this mimics real macOS
    const bool isHovered =
        (styleOptionButton.state & QStyle::State_MouseOver) ||
        (titleBar->captionButtonStyle() == CaptionButtonStyle::mac &&
         titleBar->isCaptionButtonHovered());

    const auto iconPaths =
        Internal::captionIconPathsForState(titleBar->isActive(),
                                           titleBar->isMaximized(),
                                           isHovered,
                                           this->isDown(),
                                           titleBar->captionButtonStyle());

    switch (this->m_role) {
    case Role::CaptionIcon: {
        break;
    }
    case Role::Minimize: {
        if (isHovered) {
            styleOptionButton.icon = QIcon(iconPaths[0].toString());
        }
        break;
    }
    case Role::MaximizeRestore: {
        if (isHovered) {
            styleOptionButton.icon = QIcon(iconPaths[1].toString());
        }
        break;
    }
    case Role::Close: {
        if (isHovered) {
            styleOptionButton.icon = QIcon(iconPaths[2].toString());
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

    if (this->m_role == Role::Tool) {
        const QIcon::Mode iconMode =
            this->isEnabled()
                ? ((this->m_keepDown) ? QIcon::Active : QIcon::Normal)
                : QIcon::Disabled;
        QRect iconRect(0, 0, this->width() - 12, this->height() - 12);
        iconRect.moveCenter(this->rect().center());
        Utils::StyleHelper::drawIconWithShadow(
            this->icon(), iconRect, &stylePainter, iconMode);

        if (this->m_keepDown) {
            stylePainter.setOpacity(1.0);
            QRect accentRect = this->rect();
            accentRect.setHeight(1);
            stylePainter.fillRect(
                accentRect,
                Utils::creatorTheme()->color(Utils::Theme::IconsBaseColor));
        }
    }
}

void TitleBarButton::enterEvent(QEvent *event) {
    QPushButton::enterEvent(event);
    static_cast<TitleBar *>(this->parent())->triggerCaptionRepaint();
}

void TitleBarButton::leaveEvent(QEvent *event) {
    QPushButton::leaveEvent(event);
    static_cast<TitleBar *>(this->parent())->triggerCaptionRepaint();
}

} // namespace CSD
