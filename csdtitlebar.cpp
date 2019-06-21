#include "csdtitlebar.h"

#include "csdtitlebarbutton.h"

#include <coreplugin/coreicons.h>
#include <coreplugin/icore.h>

#ifdef _WIN32
#include <Windows.h>
#include <dwmapi.h>
#endif

#include <QBoxLayout>
#include <QEvent>
#include <QMenuBar>
#include <QPainter>
#include <QStyleOption>

namespace CSD {

TitleBar::TitleBar(QWidget *parent) : QWidget(parent) {
    this->setObjectName("TitleBar");
    this->setMinimumSize(QSize(0, 30));
    this->setMaximumSize(QSize(QWIDGETSIZE_MAX, 30));
    this->m_activeColor = [this]() -> QColor {
#ifdef _WIN32
        auto maybeColor = this->readDWMColorizationColor();
        return maybeColor.value_or(Qt::black);
#else
        Q_UNUSED(this)
        return Qt::black;
#endif
    }();

    this->m_horizontalLayout = new QHBoxLayout(this);
    this->m_horizontalLayout->setSpacing(0);
    this->m_horizontalLayout->setObjectName("HorizontalLayout");
    this->m_horizontalLayout->setContentsMargins(0, 0, 0, 0);

    this->m_leftMargin = new QWidget(this);
    this->m_leftMargin->setObjectName("LeftMargin");
    this->m_leftMargin->setMinimumSize(QSize(5, 0));
    this->m_leftMargin->setMaximumSize(QSize(5, QWIDGETSIZE_MAX));
    this->m_horizontalLayout->addWidget(this->m_leftMargin);

    this->m_buttonCaptionIcon =
        new TitleBarButton(TitleBarButton::CaptionIcon, this);
    this->m_buttonCaptionIcon->setObjectName("ButtonCaptionIcon");
    this->m_buttonCaptionIcon->setMinimumSize(QSize(30, 30));
    this->m_buttonCaptionIcon->setMaximumSize(QSize(30, 30));
    this->m_buttonCaptionIcon->setFocusPolicy(Qt::NoFocus);
#ifdef _WIN32
    int icon_size = ::GetSystemMetrics(SM_CXSMICON);
#else
    int icon_size = 16;
#endif
    this->m_buttonCaptionIcon->setIconSize(QSize(icon_size, icon_size));
    this->m_buttonCaptionIcon->setIcon(Core::Icons::QTCREATORLOGO_BIG.icon());
    this->m_horizontalLayout->addWidget(this->m_buttonCaptionIcon);

    this->m_menuBar = Core::ICore::mainWindow()->menuBar();
    this->m_horizontalLayout->addWidget(this->m_menuBar);
    this->m_menuBar->setFixedHeight(30);

    auto *emptySpace = new QWidget(this);
    this->m_horizontalLayout->addWidget(emptySpace, 1);

    this->m_buttonMinimize =
        new TitleBarButton(TitleBarButton::Minimize, this);
    this->m_buttonMinimize->setObjectName("ButtonMinimize");
    this->m_buttonMinimize->setMinimumSize(QSize(46, 30));
    this->m_buttonMinimize->setMaximumSize(QSize(46, 30));
    this->m_buttonMinimize->setFocusPolicy(Qt::NoFocus);
    this->m_buttonMinimize->setText("―");
    this->m_horizontalLayout->addWidget(this->m_buttonMinimize);
    connect(this->m_buttonMinimize, &QPushButton::clicked, this, [this]() {
        emit this->minimizeClicked();
    });

    this->m_buttonMaximizeRestore =
        new TitleBarButton(TitleBarButton::MaximizeRestore, this);
    this->m_buttonMaximizeRestore->setObjectName("ButtonMaximizeRestore");
    this->m_buttonMaximizeRestore->setMinimumSize(QSize(46, 30));
    this->m_buttonMaximizeRestore->setMaximumSize(QSize(46, 30));
    this->m_buttonMaximizeRestore->setFocusPolicy(Qt::NoFocus);
    this->m_buttonMaximizeRestore->setText("☐");
    this->m_horizontalLayout->addWidget(this->m_buttonMaximizeRestore);
    connect(this->m_buttonMaximizeRestore,
            &QPushButton::clicked,
            this,
            [this]() { emit this->maximizeRestoreClicked(); });

    this->m_buttonClose = new TitleBarButton(TitleBarButton::Close, this);
    this->m_buttonClose->setObjectName("ButtonClose");
    this->m_buttonClose->setMinimumSize(QSize(46, 30));
    this->m_buttonClose->setMaximumSize(QSize(46, 30));
    this->m_buttonClose->setFocusPolicy(Qt::NoFocus);
    this->m_buttonClose->setText("✕");
    this->m_horizontalLayout->addWidget(this->m_buttonClose);
    connect(this->m_buttonClose, &QPushButton::clicked, this, [this]() {
        emit this->closeClicked();
    });

    this->setAutoFillBackground(true);
    this->setActive(this->window()->isActiveWindow());
    this->setMaximized(static_cast<bool>(this->window()->windowState() &
                                         Qt::WindowMaximized));
}

#ifdef _WIN32
std::optional<QColor> TitleBar::readDWMColorizationColor() {
    auto handleKey = ::HKEY();
    auto regOpenResult = ::RegOpenKeyExW(HKEY_CURRENT_USER,
                                         L"SOFTWARE\\Microsoft\\Windows\\DWM",
                                         0,
                                         KEY_READ,
                                         &handleKey);
    if (regOpenResult != ERROR_SUCCESS) {
        return std::nullopt;
    }
    auto value = ::DWORD();
    auto dwordBufferSize = ::DWORD(sizeof(::DWORD));
    auto regQueryResult = ::RegQueryValueExW(handleKey,
                                             L"ColorizationColor",
                                             nullptr,
                                             nullptr,
                                             reinterpret_cast<LPBYTE>(&value),
                                             &dwordBufferSize);
    if (regQueryResult != ERROR_SUCCESS) {
        return std::nullopt;
    }
    return QColor(static_cast<QRgb>(value));
}
#endif

void TitleBar::paintEvent([[maybe_unused]] QPaintEvent *event) {
    auto styleOption = QStyleOption();
    styleOption.init(this);
    auto painter = QPainter(this);
    this->style()->drawPrimitive(
        QStyle::PE_Widget, &styleOption, &painter, this);
}

bool TitleBar::isActive() const {
    return this->m_active;
}

void TitleBar::setActive(bool active) {
    this->m_active = active;
    if (active) {
        auto palette = this->palette();
        palette.setColor(QPalette::Background, this->m_activeColor);
        this->setPalette(palette);

        this->m_buttonCaptionIcon->setActive(true);
        this->m_buttonMinimize->setActive(true);
        this->m_buttonMaximizeRestore->setActive(true);
        this->m_buttonClose->setActive(true);
    } else {
        auto palette = this->palette();
        palette.setColor(QPalette::Background, QColor(33, 37, 43));
        this->setPalette(palette);

        this->m_buttonCaptionIcon->setActive(false);
        this->m_buttonMinimize->setActive(false);
        this->m_buttonMaximizeRestore->setActive(false);
        this->m_buttonClose->setActive(false);
    }
}

bool TitleBar::isMaximized() const {
    return this->m_maximized;
}

void TitleBar::setMaximized(bool maximized) {
    this->m_maximized = maximized;
}

void TitleBar::setMinimizable(bool on) {
    this->m_buttonMinimize->setVisible(on);
}

void TitleBar::setMaximizable(bool on) {
    this->m_buttonMaximizeRestore->setVisible(on);
}

QColor TitleBar::activeColor() {
    return this->m_activeColor;
}

void TitleBar::setActiveColor(const QColor &activeColor) {
    this->m_activeColor = activeColor;
    this->update();
}

void TitleBar::onWindowStateChange(Qt::WindowStates state) {
    this->setActive(this->window()->isActiveWindow());
    this->setMaximized(static_cast<bool>(state & Qt::WindowMaximized));
}

bool TitleBar::hovered() const {
    auto cursorPos = QCursor::pos();
    bool hovered = this->rect().contains(this->mapFromGlobal(cursorPos));
    if (!hovered) {
        return false;
    }

    if (this->m_menuBar->rect().contains(
            this->m_menuBar->mapFromGlobal(cursorPos))) {
        return false;
    }

    for (const TitleBarButton *btn : this->findChildren<TitleBarButton *>()) {
        bool btnHovered = btn->rect().contains(btn->mapFromGlobal(cursorPos));
        if (btnHovered) {
            return false;
        }
    }
    return true;
}

} // namespace CSD
