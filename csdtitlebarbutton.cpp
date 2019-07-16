#include "csdtitlebarbutton.h"

#include "csdtitlebar.h"

#include <utils/theme/theme.h>

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

void TitleBarButton::paintEvent([[maybe_unused]] QPaintEvent *event) {
    auto stylePainter = QStylePainter(this);
    auto styleOptionButton = QStyleOptionButton();
    styleOptionButton.initFrom(this);
    styleOptionButton.features = QStyleOptionButton::None;
    styleOptionButton.text = this->text();
    styleOptionButton.icon = this->icon();
    styleOptionButton.iconSize = this->iconSize();

    const auto hoverColor =
        Utils::creatorTheme()->color(Utils::Theme::FancyToolButtonHoverColor);
    const bool isHovered = styleOptionButton.state & QStyle::State_MouseOver;

    switch (this->m_role) {
    case Role::CaptionIcon: {
        break;
    }
    case Role::Minimize: {
        if (isHovered) {
            styleOptionButton.icon =
                QIcon(":/resources/chrome-minimize-dark.svg");

            stylePainter.save();
            stylePainter.setRenderHint(QPainter::Antialiasing, false);
            stylePainter.setPen(Qt::NoPen);
            stylePainter.setBrush(QBrush(QColor(hoverColor)));
            stylePainter.drawRect(styleOptionButton.rect);
            stylePainter.restore();
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

            stylePainter.save();
            stylePainter.setRenderHint(QPainter::Antialiasing, false);
            stylePainter.setPen(Qt::NoPen);
            stylePainter.setBrush(QBrush(QColor(hoverColor)));
            stylePainter.drawRect(styleOptionButton.rect);
            stylePainter.restore();
        }
        break;
    }
    case Role::Close: {
        if (isHovered) {
            styleOptionButton.icon =
                QIcon(":/resources/chrome-close-light.svg");

            stylePainter.save();
            stylePainter.setRenderHint(QPainter::Antialiasing, false);
            stylePainter.setPen(Qt::NoPen);
            stylePainter.setBrush(QBrush(QColor(232, 17, 35, 229)));
            stylePainter.drawRect(styleOptionButton.rect);
            stylePainter.restore();
        }
        break;
    }
    }

    stylePainter.drawControl(QStyle::CE_PushButtonLabel, styleOptionButton);
}

} // namespace CSD
