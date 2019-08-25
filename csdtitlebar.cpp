#include "csdtitlebar.h"

#include "csdtitlebarbutton.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/coreicons.h>
#include <coreplugin/designmode.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>
#include <debugger/debuggerconstants.h>
#include <help/helpconstants.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/session.h>

#ifdef _WIN32
#include "qtwinbackports.h"

#include <Windows.h>
#include <dwmapi.h>
#endif

#include <QApplication>
#include <QBoxLayout>
#include <QEvent>
#include <QMainWindow>
#include <QMenuBar>
#include <QPainter>
#include <QStyleOption>
#include <QTimer>

#if !defined(_WIN32) && !defined(__APPLE__)
#include <QMouseEvent>
#include <QWindow>

#include <QX11Info>

#include <private/qhighdpiscaling_p.h>
#include <qpa/qplatformwindow.h>

#include <cstring>
#endif

namespace CSD {

#if !defined(_WIN32) && !defined(__APPLE__)
constexpr static const char _NET_WM_MOVERESIZE[] = "_NET_WM_MOVERESIZE";

static QWidget *titleBarTopLevelWidget(QWidget *w) {
    while (w && !w->isWindow() && w->windowType() != Qt::SubWindow) {
        w = w->parentWidget();
    }
    return w;
}
#endif

TitleBar::TitleBar(const QIcon &captionIcon, QWidget *parent)
    : QWidget(parent) {
    this->setObjectName("TitleBar");
    this->setMinimumSize(QSize(0, 30));
    this->setMaximumSize(QSize(QWIDGETSIZE_MAX, 30));
    this->m_activeColor = [this]() -> QColor {
#ifdef _WIN32
        auto maybeColor = this->readDWMColorizationColor();
        return maybeColor.value_or(QColor(40, 44, 52));
#else
        Q_UNUSED(this)
        return QColor(40, 44, 52);
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
    const auto icon = [&captionIcon, this]() -> QIcon {
        if (!captionIcon.isNull()) {
            return captionIcon;
        }
        auto globalWindowIcon = this->window()->windowIcon();
        if (!globalWindowIcon.isNull()) {
            return globalWindowIcon;
        }
        globalWindowIcon = QApplication::windowIcon();
        if (!globalWindowIcon.isNull()) {
            return globalWindowIcon;
        }
#ifdef _WIN32
        // Use system default application icon which doesn't need margin
        this->m_horizontalLayout->takeAt(
            this->m_horizontalLayout->indexOf(this->m_leftMargin));
        this->m_leftMargin->setParent(nullptr);
        HICON winIcon = ::LoadIconW(nullptr, IDI_APPLICATION);
        globalWindowIcon.addPixmap(
            QtWinBackports::qt_pixmapFromWinHICON(winIcon));
#else
#if !defined(__APPLE__)
        if (globalWindowIcon.isNull()) {
            globalWindowIcon = QIcon::fromTheme("application-x-executable");
        }
#endif
#endif
        return globalWindowIcon;
    }();
    this->m_buttonCaptionIcon->setIcon(icon);
    this->m_horizontalLayout->addWidget(this->m_buttonCaptionIcon);

    auto *mainWindow = qobject_cast<QMainWindow *>(this->window());
    if (mainWindow != nullptr) {
        this->m_menuBar = mainWindow->menuBar();
        this->m_horizontalLayout->addWidget(this->m_menuBar);
        this->m_menuBar->setFixedHeight(30);
    }

    auto *emptySpace = new QWidget(this);
    emptySpace->setAttribute(Qt::WA_TransparentForMouseEvents);
    this->m_horizontalLayout->addWidget(emptySpace, 1);

    this->m_buttonModeWelcome = new TitleBarButton(TitleBarButton::Tool, this);
    this->m_buttonModeWelcome->setObjectName("ButtonModeWelcome");
    this->m_buttonModeWelcome->setMinimumSize(QSize(30, 30));
    this->m_buttonModeWelcome->setMaximumSize(QSize(30, 30));
    this->m_buttonModeWelcome->setText("W");
    this->m_horizontalLayout->addWidget(this->m_buttonModeWelcome);

    this->m_buttonModeEdit = new TitleBarButton(TitleBarButton::Tool, this);
    this->m_buttonModeEdit->setObjectName("ButtonModeEdit");
    this->m_buttonModeEdit->setMinimumSize(QSize(30, 30));
    this->m_buttonModeEdit->setMaximumSize(QSize(30, 30));
    this->m_buttonModeEdit->setText("E");
    this->m_horizontalLayout->addWidget(this->m_buttonModeEdit);

    this->m_buttonModeDesign = new TitleBarButton(TitleBarButton::Tool, this);
    this->m_buttonModeDesign->setObjectName("ButtonModeDesign");
    this->m_buttonModeDesign->setMinimumSize(QSize(30, 30));
    this->m_buttonModeDesign->setMaximumSize(QSize(30, 30));
    this->m_buttonModeDesign->setText("D");
    this->m_buttonModeDesign->setEnabled(false);
    this->m_horizontalLayout->addWidget(this->m_buttonModeDesign);

    this->m_buttonModeDebug = new TitleBarButton(TitleBarButton::Tool, this);
    this->m_buttonModeDebug->setObjectName("ButtonModeDebug");
    this->m_buttonModeDebug->setMinimumSize(QSize(30, 30));
    this->m_buttonModeDebug->setMaximumSize(QSize(30, 30));
    this->m_buttonModeDebug->setText("D");
    this->m_horizontalLayout->addWidget(this->m_buttonModeDebug);

    this->m_buttonModeProjects =
        new TitleBarButton(TitleBarButton::Tool, this);
    this->m_buttonModeProjects->setObjectName("ButtonModeProjects");
    this->m_buttonModeProjects->setMinimumSize(QSize(30, 30));
    this->m_buttonModeProjects->setMaximumSize(QSize(30, 30));
    this->m_buttonModeProjects->setText("P");
    this->m_horizontalLayout->addWidget(this->m_buttonModeProjects);

    this->m_buttonModeHelp = new TitleBarButton(TitleBarButton::Tool, this);
    this->m_buttonModeHelp->setObjectName("ButtonModeHelp");
    this->m_buttonModeHelp->setMinimumSize(QSize(30, 30));
    this->m_buttonModeHelp->setMaximumSize(QSize(30, 30));
    this->m_buttonModeHelp->setText("H");
    this->m_horizontalLayout->addWidget(this->m_buttonModeHelp);

    QObject::connect(Core::ModeManager::instance(),
                     &Core::ModeManager::currentModeChanged,
                     this,
                     [this](Core::Id mode, Core::Id) {
                         this->resetModeButtonStates();
                         if (mode == Core::Constants::MODE_WELCOME) {
                             this->m_buttonModeWelcome->setKeepDown(true);
                         } else if (mode == Core::Constants::MODE_EDIT) {
                             this->m_buttonModeEdit->setKeepDown(true);
                         } else if (mode == Core::Constants::MODE_DESIGN) {
                             this->m_buttonModeDesign->setKeepDown(true);
                         } else if (mode == Debugger::Constants::MODE_DEBUG) {
                             this->m_buttonModeDebug->setKeepDown(true);
                         } else if (mode ==
                                    ProjectExplorer::Constants::MODE_SESSION) {
                             this->m_buttonModeProjects->setKeepDown(true);
                         } else if (mode == Help::Constants::ID_MODE_HELP) {
                             this->m_buttonModeHelp->setKeepDown(true);
                         }
                     });

    QObject::connect(ProjectExplorer::SessionManager::instance(),
                     &ProjectExplorer::SessionManager::projectAdded,
                     this,
                     [this](ProjectExplorer::Project *) {
                         this->m_buttonModeProjects->setEnabled(true);
                     });

    QObject::connect(
        ProjectExplorer::SessionManager::instance(),
        &ProjectExplorer::SessionManager::projectRemoved,
        this,
        [this](ProjectExplorer::Project *) {
            this->m_buttonModeProjects->setEnabled(
                ProjectExplorer::SessionManager::instance()->hasProjects());
        });

    QTimer::singleShot(0, this, [this]() {
        QObject::connect(Core::DesignMode::instance(),
                         &Core::IMode::enabledStateChanged,
                         this,
                         [this](bool enabled) {
                             this->m_buttonModeDesign->setEnabled(enabled);
                         });
    });

    this->m_buttonMinimize =
        new TitleBarButton(TitleBarButton::Minimize, this);
    this->m_buttonMinimize->setObjectName("ButtonMinimize");
    this->m_buttonMinimize->setMinimumSize(QSize(46, 30));
    this->m_buttonMinimize->setMaximumSize(QSize(46, 30));
    this->m_buttonMinimize->setFocusPolicy(Qt::NoFocus);
    this->m_buttonMinimize->setIconSize(QSize(10, 10));
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
    this->m_buttonMaximizeRestore->setIconSize(QSize(10, 10));
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
    this->m_buttonClose->setIconSize(QSize(10, 10));
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

TitleBar::~TitleBar() {
    auto *mainWindow = qobject_cast<QMainWindow *>(this->window());
    if (mainWindow != nullptr) {
        mainWindow->setMenuBar(this->m_menuBar);
    }
    this->m_menuBar = nullptr;
}

#if !defined(_WIN32) && !defined(__APPLE__)
void TitleBar::mousePressEvent(QMouseEvent *event) {
    if (!QX11Info::isPlatformX11() || event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    QWidget *tlw = titleBarTopLevelWidget(this);

    if (tlw->isWindow() && tlw->windowHandle() &&
        !(tlw->windowFlags() & Qt::X11BypassWindowManagerHint) &&
        !tlw->testAttribute(Qt::WA_DontShowOnScreen) &&
        !tlw->hasHeightForWidth()) {
        QPlatformWindow *platformWindow = tlw->windowHandle()->handle();
        const QPoint globalPos = QHighDpi::toNativePixels(
            platformWindow->mapToGlobal(this->mapTo(tlw, event->pos())),
            platformWindow->screen());

        const xcb_atom_t moveResizeAtom = []() -> xcb_atom_t {
            xcb_intern_atom_cookie_t cookie = xcb_intern_atom(
                QX11Info::connection(),
                false,
                static_cast<std::uint16_t>(std::strlen(_NET_WM_MOVERESIZE)),
                _NET_WM_MOVERESIZE);
            xcb_intern_atom_reply_t *reply =
                xcb_intern_atom_reply(QX11Info::connection(), cookie, nullptr);
            const xcb_atom_t moveResizeAtomCopy = reply->atom;
            free(reply);
            return moveResizeAtomCopy;
        }();

        xcb_client_message_event_t xev;
        xev.response_type = XCB_CLIENT_MESSAGE;
        xev.type = moveResizeAtom;
        xev.sequence = 0;
        xev.window = static_cast<xcb_window_t>(platformWindow->winId());
        xev.format = 32;
        xev.data.data32[0] = static_cast<std::uint32_t>(globalPos.x());
        xev.data.data32[1] = static_cast<std::uint32_t>(globalPos.y());
        xev.data.data32[2] = 8; // move
        xev.data.data32[3] = XCB_BUTTON_INDEX_1;
        xev.data.data32[4] = 0;

        const xcb_window_t rootWindow = [platformWindow]() -> xcb_window_t {
            xcb_query_tree_cookie_t queryTreeCookie = xcb_query_tree(
                QX11Info::connection(),
                static_cast<xcb_window_t>(platformWindow->winId()));
            xcb_query_tree_reply_t *queryTreeReply = xcb_query_tree_reply(
                QX11Info::connection(), queryTreeCookie, nullptr);
            xcb_window_t rootWindowCopy = queryTreeReply->root;
            free(queryTreeReply);
            return rootWindowCopy;
        }();

        std::uint32_t eventFlags = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                                   XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;

        xcb_ungrab_pointer(QX11Info::connection(), XCB_CURRENT_TIME);
        xcb_send_event(QX11Info::connection(),
                       false,
                       rootWindow,
                       eventFlags,
                       reinterpret_cast<const char *>(&xev));
    }
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

        this->m_buttonMinimize->setIcon(
            QIcon(":/resources/chrome-minimize-dark.svg"));
        if (this->m_maximized) {
            this->m_buttonMaximizeRestore->setIcon(
                QIcon(":/resources/chrome-restore-dark.svg"));
        } else {
            this->m_buttonMaximizeRestore->setIcon(
                QIcon(":/resources/chrome-maximize-dark.svg"));
        }
        this->m_buttonClose->setIcon(
            QIcon(":/resources/chrome-close-dark.svg"));
    } else {
        auto palette = this->palette();
        palette.setColor(QPalette::Background, QColor(33, 37, 43));
        this->setPalette(palette);

        this->m_buttonMinimize->setIcon(
            QIcon(":/resources/chrome-minimize-dark-disabled.svg"));
        if (this->m_maximized) {
            this->m_buttonMaximizeRestore->setIcon(
                QIcon(":/resources/chrome-restore-dark-disabled.svg"));
        } else {
            this->m_buttonMaximizeRestore->setIcon(
                QIcon(":/resources/chrome-maximize-dark-disabled.svg"));
        }
        this->m_buttonClose->setIcon(
            QIcon(":/resources/chrome-close-dark-disabled.svg"));
    }
}

bool TitleBar::isMaximized() const {
    return this->m_maximized;
}

void TitleBar::setMaximized(bool maximized) {
    this->m_maximized = maximized;
    if (this->m_active) {
        this->m_buttonMinimize->setIcon(
            QIcon(":/resources/chrome-minimize-dark.svg"));
        if (this->m_maximized) {
            this->m_buttonMaximizeRestore->setIcon(
                QIcon(":/resources/chrome-restore-dark.svg"));
        } else {
            this->m_buttonMaximizeRestore->setIcon(
                QIcon(":/resources/chrome-maximize-dark.svg"));
        }
        this->m_buttonClose->setIcon(
            QIcon(":/resources/chrome-close-dark.svg"));
    } else {
        this->m_buttonMinimize->setIcon(
            QIcon(":/resources/chrome-minimize-dark-disabled.svg"));
        if (this->m_maximized) {
            this->m_buttonMaximizeRestore->setIcon(
                QIcon(":/resources/chrome-restore-dark-disabled.svg"));
        } else {
            this->m_buttonMaximizeRestore->setIcon(
                QIcon(":/resources/chrome-maximize-dark-disabled.svg"));
        }
        this->m_buttonClose->setIcon(
            QIcon(":/resources/chrome-close-dark-disabled.svg"));
    }
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

QColor TitleBar::hoverColor() const {
    return this->m_hoverColor;
}

void TitleBar::setHoverColor(QColor hoverColor) {
    this->m_hoverColor = std::move(hoverColor);
    this->m_buttonMinimize->setHoverColor(this->m_hoverColor);
    this->m_buttonMaximizeRestore->setHoverColor(this->m_hoverColor);
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

void TitleBar::resetModeButtonStates() {
    this->m_buttonModeWelcome->setKeepDown(false);
    this->m_buttonModeEdit->setKeepDown(false);
    this->m_buttonModeDesign->setKeepDown(false);
    this->m_buttonModeDebug->setKeepDown(false);
    this->m_buttonModeProjects->setKeepDown(false);
    this->m_buttonModeHelp->setKeepDown(false);
}

} // namespace CSD
