#include "csdtitlebar.h"

#include "csdtitlebarbutton.h"

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/designmode.h>
#include <coreplugin/modemanager.h>
#include <debugger/debuggerconstants.h>
#include <help/helpconstants.h>
#include <projectexplorer/buildmanager.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectexplorericons.h>
#include <projectexplorer/session.h>
#include <utils/icon.h>
#include <utils/stylehelper.h>

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
#include <qpa/qplatformscreen.h>
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

TitleBar::TitleBar(CaptionButtonStyle captionButtonStyle,
                   const QIcon &captionIcon,
                   QWidget *parent)
    : QWidget(parent), m_captionButtonStyle(captionButtonStyle) {
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

    Core::Command *commandRun =
        Core::ActionManager::command("ProjectExplorer.Run");

    auto onModeBarRunActionChanged = [commandRun, this] {
        this->m_buttonRun->setEnabled(commandRun->action()->isEnabled());
        this->m_buttonRun->setIcon(commandRun->action()->icon());
        this->m_buttonRun->disconnect(SIGNAL(clicked()));
        QObject::connect(this->m_buttonRun,
                         &QPushButton::clicked,
                         commandRun->action(),
                         &QAction::trigger);
    };

    this->m_buttonRun = new TitleBarButton(TitleBarButton::Tool, this);
    this->m_buttonRun->setMinimumSize(QSize(30, 30));
    this->m_buttonRun->setMaximumSize(QSize(30, 30));
    QObject::connect(commandRun->action(),
                     &QAction::changed,
                     this->m_buttonRun,
                     onModeBarRunActionChanged);
    this->m_horizontalLayout->addWidget(m_buttonRun);

    Core::Command *commandDebug =
        Core::ActionManager::command("Debugger.Debug");

    auto onModeBarDebugActionChanged = [commandDebug, this] {
        this->m_buttonDebug->setEnabled(commandDebug->action()->isEnabled());
        this->m_buttonDebug->setIcon(commandDebug->action()->icon());
        this->m_buttonDebug->disconnect(SIGNAL(clicked()));
        QObject::connect(this->m_buttonDebug,
                         &QPushButton::clicked,
                         commandDebug->action(),
                         &QAction::trigger);
    };

    this->m_buttonDebug = new TitleBarButton(TitleBarButton::Tool, this);
    this->m_buttonDebug->setMinimumSize(QSize(30, 30));
    this->m_buttonDebug->setMaximumSize(QSize(30, 30));
    QObject::connect(commandDebug->action(),
                     &QAction::changed,
                     this->m_buttonDebug,
                     onModeBarDebugActionChanged);
    this->m_horizontalLayout->addWidget(m_buttonDebug);

    Core::Command *commandBuild =
        Core::ActionManager::command(ProjectExplorer::Constants::BUILD);
    Core::Command *commandCancelBuild =
        Core::ActionManager::command("ProjectExplorer.CancelBuild");

    auto onModeBarBuildActionChanged =
        [commandBuild, commandCancelBuild, this] {
            if (ProjectExplorer::BuildManager::isBuilding(
                    ProjectExplorer::SessionManager::startupProject())) {
                this->m_buttonBuild->setEnabled(
                    commandCancelBuild->action()->isEnabled());
                this->m_buttonBuild->setIcon(
                    ProjectExplorer::Icons::CANCELBUILD_FLAT.icon());
                this->m_buttonBuild->disconnect(SIGNAL(clicked()));
                QObject::connect(this->m_buttonBuild,
                                 &QPushButton::clicked,
                                 commandCancelBuild->action(),
                                 &QAction::trigger);
            } else {
                this->m_buttonBuild->setEnabled(
                    commandBuild->action()->isEnabled());
                this->m_buttonBuild->setIcon(commandBuild->action()->icon());
                this->m_buttonBuild->disconnect(SIGNAL(clicked()));
                QObject::connect(this->m_buttonBuild,
                                 &QPushButton::clicked,
                                 commandBuild->action(),
                                 &QAction::trigger);
            }
        };

    this->m_buttonBuild = new TitleBarButton(TitleBarButton::Tool, this);
    this->m_buttonBuild->setMinimumSize(QSize(30, 30));
    this->m_buttonBuild->setMaximumSize(QSize(30, 30));
    this->m_buttonBuild->setIcon(commandBuild->action()->icon());
    QObject::connect(commandBuild->action(),
                     &QAction::changed,
                     this->m_buttonBuild,
                     onModeBarBuildActionChanged);
    QObject::connect(commandCancelBuild->action(),
                     &QAction::changed,
                     this->m_buttonBuild,
                     onModeBarBuildActionChanged);
    this->m_horizontalLayout->addWidget(m_buttonBuild);

    this->m_buttonModeWelcome = new TitleBarButton(TitleBarButton::Tool, this);
    this->m_buttonModeWelcome->setObjectName("ButtonModeWelcome");
    this->m_buttonModeWelcome->setMinimumSize(QSize(30, 30));
    this->m_buttonModeWelcome->setMaximumSize(QSize(30, 30));
    this->m_buttonModeWelcome->setIcon(Utils::Icon::modeIcon(
        {":/resources/mode/mode-welcome.svg"},
        {{":/resources/mode/mode-welcome.svg", Utils::Theme::IconsBaseColor}},
        {{":/resources/mode/mode-welcome.svg",
          Utils::Theme::IconsModeWelcomeActiveColor}}));
    QObject::connect(this->m_buttonModeWelcome,
                     &QPushButton::clicked,
                     this->m_buttonModeWelcome,
                     [] {
                         Core::ModeManager::instance()->activateMode(
                             Core::Constants::MODE_WELCOME);
                     });
    this->m_horizontalLayout->addWidget(this->m_buttonModeWelcome);

    this->m_buttonModeEdit = new TitleBarButton(TitleBarButton::Tool, this);
    this->m_buttonModeEdit->setObjectName("ButtonModeEdit");
    this->m_buttonModeEdit->setMinimumSize(QSize(30, 30));
    this->m_buttonModeEdit->setMaximumSize(QSize(30, 30));
    this->m_buttonModeEdit->setIcon(Utils::Icon::modeIcon(
        {":/resources/mode/mode-edit.svg"},
        {{":/resources/mode/mode-edit.svg", Utils::Theme::IconsBaseColor}},
        {{":/resources/mode/mode-edit.svg",
          Utils::Theme::IconsModeEditActiveColor}}));
    QObject::connect(this->m_buttonModeEdit,
                     &QPushButton::clicked,
                     this->m_buttonModeEdit,
                     [] {
                         Core::ModeManager::instance()->activateMode(
                             Core::Constants::MODE_EDIT);
                     });
    this->m_horizontalLayout->addWidget(this->m_buttonModeEdit);

    this->m_buttonModeDesign = new TitleBarButton(TitleBarButton::Tool, this);
    this->m_buttonModeDesign->setObjectName("ButtonModeDesign");
    this->m_buttonModeDesign->setMinimumSize(QSize(30, 30));
    this->m_buttonModeDesign->setMaximumSize(QSize(30, 30));
    this->m_buttonModeDesign->setEnabled(false);
    this->m_buttonModeDesign->setIcon(Utils::Icon::modeIcon(
        {":/resources/mode/mode-design.svg"},
        {{":/resources/mode/mode-design.svg", Utils::Theme::IconsBaseColor}},
        {{":/resources/mode/mode-design.svg",
          Utils::Theme::IconsModeDesignActiveColor}}));
    QObject::connect(this->m_buttonModeDesign,
                     &QPushButton::clicked,
                     this->m_buttonModeDesign,
                     [] {
                         Core::ModeManager::instance()->activateMode(
                             Core::Constants::MODE_DESIGN);
                     });
    this->m_horizontalLayout->addWidget(this->m_buttonModeDesign);

    this->m_buttonModeDebug = new TitleBarButton(TitleBarButton::Tool, this);
    this->m_buttonModeDebug->setObjectName("ButtonModeDebug");
    this->m_buttonModeDebug->setMinimumSize(QSize(30, 30));
    this->m_buttonModeDebug->setMaximumSize(QSize(30, 30));
    this->m_buttonModeDebug->setIcon(Utils::Icon::modeIcon(
        {":/resources/mode/mode-debug.svg"},
        {{":/resources/mode/mode-debug.svg", Utils::Theme::IconsBaseColor}},
        {{":/resources/mode/mode-debug.svg",
          Utils::Theme::IconsModeDebugActiveColor}}));
    QObject::connect(this->m_buttonModeDebug,
                     &QPushButton::clicked,
                     this->m_buttonModeDebug,
                     [] {
                         Core::ModeManager::instance()->activateMode(
                             Debugger::Constants::MODE_DEBUG);
                     });
    this->m_horizontalLayout->addWidget(this->m_buttonModeDebug);

    this->m_buttonModeProjects =
        new TitleBarButton(TitleBarButton::Tool, this);
    this->m_buttonModeProjects->setObjectName("ButtonModeProjects");
    this->m_buttonModeProjects->setMinimumSize(QSize(30, 30));
    this->m_buttonModeProjects->setMaximumSize(QSize(30, 30));
    this->m_buttonModeProjects->setEnabled(false);
    this->m_buttonModeProjects->setIcon(Utils::Icon::modeIcon(
        {":/resources/mode/mode-project.svg"},
        {{":/resources/mode/mode-project.svg", Utils::Theme::IconsBaseColor}},
        {{":/resources/mode/mode-project.svg",
          Utils::Theme::IconsModeProjectActiveColor}}));
    QObject::connect(this->m_buttonModeProjects,
                     &QPushButton::clicked,
                     this->m_buttonModeProjects,
                     [] {
                         Core::ModeManager::instance()->activateMode(
                             ProjectExplorer::Constants::MODE_SESSION);
                     });
    this->m_horizontalLayout->addWidget(this->m_buttonModeProjects);

    this->m_buttonModeHelp = new TitleBarButton(TitleBarButton::Tool, this);
    this->m_buttonModeHelp->setObjectName("ButtonModeHelp");
    this->m_buttonModeHelp->setMinimumSize(QSize(30, 30));
    this->m_buttonModeHelp->setMaximumSize(QSize(30, 30));
    this->m_buttonModeHelp->setIcon(Utils::Icon::modeIcon(
        {":/resources/mode/mode-help.svg"},
        {{QLatin1String(":/resources/mode/mode-help.svg"),
          Utils::Theme::IconsBaseColor}},
        {{QLatin1String(":/resources/mode/mode-help.svg"),
          Utils::Theme::IconsModeHelpActiveColor}}));
    QObject::connect(this->m_buttonModeHelp,
                     &QPushButton::clicked,
                     this->m_buttonModeHelp,
                     [] {
                         Core::ModeManager::instance()->activateMode(
                             Help::Constants::ID_MODE_HELP);
                     });
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

    int captionButtonsWidth = 0;
    switch (this->m_captionButtonStyle) {
    case CaptionButtonStyle::custom: {
        captionButtonsWidth = 30;
        break;
    }
    case CaptionButtonStyle::win: {
        captionButtonsWidth = 46;
        break;
    }
    case CaptionButtonStyle::mac: {
        captionButtonsWidth = 26;
        break;
    }
    }

    this->m_buttonMinimize =
        new TitleBarButton(TitleBarButton::Minimize, this);
    this->m_buttonMinimize->setObjectName("ButtonMinimize");
    this->m_buttonMinimize->setMinimumSize(QSize(captionButtonsWidth, 30));
    this->m_buttonMinimize->setMaximumSize(QSize(captionButtonsWidth, 30));
    this->m_buttonMinimize->setFocusPolicy(Qt::NoFocus);
    this->m_buttonMinimize->setIconSize(
        this->m_captionButtonStyle == CaptionButtonStyle::mac ? QSize(16, 16)
                                                              : QSize(12, 12));
    this->m_horizontalLayout->addWidget(this->m_buttonMinimize);
    connect(this->m_buttonMinimize, &QPushButton::clicked, this, [this]() {
        emit this->minimizeClicked();
    });

    this->m_buttonMaximizeRestore =
        new TitleBarButton(TitleBarButton::MaximizeRestore, this);
    this->m_buttonMaximizeRestore->setObjectName("ButtonMaximizeRestore");
    this->m_buttonMaximizeRestore->setMinimumSize(
        QSize(captionButtonsWidth, 30));
    this->m_buttonMaximizeRestore->setMaximumSize(
        QSize(captionButtonsWidth, 30));
    this->m_buttonMaximizeRestore->setFocusPolicy(Qt::NoFocus);
    this->m_buttonMaximizeRestore->setIconSize(
        this->m_captionButtonStyle == CaptionButtonStyle::mac ? QSize(16, 16)
                                                              : QSize(12, 12));
    this->m_horizontalLayout->addWidget(this->m_buttonMaximizeRestore);
    connect(this->m_buttonMaximizeRestore,
            &QPushButton::clicked,
            this,
            [this]() { emit this->maximizeRestoreClicked(); });

    this->m_buttonClose = new TitleBarButton(TitleBarButton::Close, this);
    this->m_buttonClose->setObjectName("ButtonClose");
    this->m_buttonClose->setMinimumSize(QSize(captionButtonsWidth, 30));
    this->m_buttonClose->setMaximumSize(QSize(captionButtonsWidth, 30));
    this->m_buttonClose->setFocusPolicy(Qt::NoFocus);
    this->m_buttonClose->setIconSize(
        this->m_captionButtonStyle == CaptionButtonStyle::mac ? QSize(16, 16)
                                                              : QSize(12, 12));
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
            platformWindow->screen()->screen());

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
        palette.setColor(QPalette::Window, this->m_activeColor);
        this->setPalette(palette);
    } else {
        auto palette = this->palette();
        palette.setColor(QPalette::Window, QColor(33, 37, 43));
        this->setPalette(palette);
    }

    auto iconsPaths =
        Internal::captionIconPathsForState(this->m_active,
                                           this->m_maximized,
                                           false,
                                           false,
                                           this->m_captionButtonStyle);

    this->m_buttonMinimize->setIcon(QIcon(iconsPaths[0].toString()));
    this->m_buttonMaximizeRestore->setIcon(QIcon(iconsPaths[1].toString()));
    this->m_buttonClose->setIcon(QIcon(iconsPaths[2].toString()));
}

bool TitleBar::isMaximized() const {
    return this->m_maximized;
}

void TitleBar::setMaximized(bool maximized) {
    this->m_maximized = maximized;
    auto iconsPaths =
        Internal::captionIconPathsForState(this->m_active,
                                           this->m_maximized,
                                           false,
                                           false,
                                           this->m_captionButtonStyle);

    this->m_buttonMinimize->setIcon(QIcon(iconsPaths[0].toString()));
    this->m_buttonMaximizeRestore->setIcon(QIcon(iconsPaths[1].toString()));
    this->m_buttonClose->setIcon(QIcon(iconsPaths[2].toString()));
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

CaptionButtonStyle TitleBar::captionButtonStyle() const {
    return this->m_captionButtonStyle;
}

void TitleBar::setCaptionButtonStyle(CaptionButtonStyle captionButtonStyle) {
    this->m_captionButtonStyle = captionButtonStyle;

    auto iconSize = this->m_captionButtonStyle == CaptionButtonStyle::mac
                        ? QSize(16, 16)
                        : QSize(12, 12);
    int requiredWidth = 0;
    switch (this->m_captionButtonStyle) {
    case CaptionButtonStyle::custom: {
        requiredWidth = 30;
        break;
    }
    case CaptionButtonStyle::win: {
        requiredWidth = 46;
        break;
    }
    case CaptionButtonStyle::mac: {
        requiredWidth = 26;
        break;
    }
    }
    this->m_buttonMinimize->setIconSize(iconSize);
    this->m_buttonMinimize->setMinimumWidth(requiredWidth);
    this->m_buttonMinimize->setMaximumWidth(requiredWidth);
    this->m_buttonMaximizeRestore->setIconSize(iconSize);
    this->m_buttonMaximizeRestore->setMinimumWidth(requiredWidth);
    this->m_buttonMaximizeRestore->setMaximumWidth(requiredWidth);
    this->m_buttonClose->setIconSize(iconSize);
    this->m_buttonClose->setMinimumWidth(requiredWidth);
    this->m_buttonClose->setMaximumWidth(requiredWidth);

    auto iconsPaths =
        Internal::captionIconPathsForState(this->m_active,
                                           this->m_maximized,
                                           false,
                                           false,
                                           this->m_captionButtonStyle);

    this->m_buttonMinimize->setIcon(QIcon(iconsPaths[0].toString()));
    this->m_buttonMaximizeRestore->setIcon(QIcon(iconsPaths[1].toString()));
    this->m_buttonClose->setIcon(QIcon(iconsPaths[2].toString()));
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

bool TitleBar::isCaptionButtonHovered() const {
    return this->m_buttonMinimize->underMouse() ||
           this->m_buttonMaximizeRestore->underMouse() ||
           this->m_buttonClose->underMouse();
}

void TitleBar::triggerCaptionRepaint() {
    this->m_buttonMinimize->update();
    this->m_buttonMaximizeRestore->update();
    this->m_buttonClose->update();
}

void TitleBar::resetModeButtonStates() {
    this->m_buttonModeWelcome->setKeepDown(false);
    this->m_buttonModeEdit->setKeepDown(false);
    this->m_buttonModeDesign->setKeepDown(false);
    this->m_buttonModeDebug->setKeepDown(false);
    this->m_buttonModeProjects->setKeepDown(false);
    this->m_buttonModeHelp->setKeepDown(false);
}

namespace Internal {

std::array<QStringView, 3> captionIconPathsForState(bool active,
                                                    bool maximized,
                                                    bool hovered,
                                                    bool pressed,
                                                    CaptionButtonStyle style) {
    std::array<QStringView, 3> buf;

    switch (style) {
    case CaptionButtonStyle::custom: {
        if (active || hovered) {
            buf[0] = u":/resources/titlebar/custom/chrome-minimize-dark.svg";
            if (maximized) {
                buf[1] =
                    u":/resources/titlebar/custom/chrome-restore-dark.svg";
            } else {
                buf[1] =
                    u":/resources/titlebar/custom/chrome-maximize-dark.svg";
            }
            if (hovered) {
                buf[2] = u":/resources/titlebar/custom/chrome-close-light.svg";
            } else {
                buf[2] = u":/resources/titlebar/custom/chrome-close-dark.svg";
            }
        } else {
            buf[0] = u":/resources/titlebar/custom/"
                     u"chrome-minimize-dark-disabled.svg";
            if (maximized) {
                buf[1] = u":/resources/titlebar/custom/"
                         u"chrome-restore-dark-disabled.svg";
            } else {
                buf[1] = u":/resources/titlebar/custom/"
                         u"chrome-maximize-dark-disabled.svg";
            }
            buf[2] =
                u":/resources/titlebar/custom/chrome-close-dark-disabled.svg";
        }
        break;
    }
    case CaptionButtonStyle::win: {
        if (active || hovered) {
            buf[0] = u":/resources/titlebar/win/chrome-minimize-dark.svg";
            if (maximized) {
                buf[1] = u":/resources/titlebar/win/chrome-restore-dark.svg";
            } else {
                buf[1] = u":/resources/titlebar/win/chrome-maximize-dark.svg";
            }
            if (hovered) {
                buf[2] = u":/resources/titlebar/win/chrome-close-light.svg";
            } else {
                buf[2] = u":/resources/titlebar/win/chrome-close-dark.svg";
            }
        } else {
            buf[0] =
                u":/resources/titlebar/win/chrome-minimize-dark-disabled.svg";
            if (maximized) {
                buf[1] = u":/resources/titlebar/win/"
                         u"chrome-restore-dark-disabled.svg";
            } else {
                buf[1] = u":/resources/titlebar/win/"
                         u"chrome-maximize-dark-disabled.svg";
            }
            buf[2] =
                u":/resources/titlebar/win/chrome-close-dark-disabled.svg";
        }
        break;
    }
    case CaptionButtonStyle::mac: {
        if (pressed) {
            buf[0] = u":/resources/titlebar/mac/minimize-pressed.png";
            if (maximized) {
                buf[1] = u":/resources/titlebar/mac/"
                         "maximize-restore-maximized-pressed.png";
            } else {
                buf[1] = u":/resources/titlebar/mac/"
                         u"maximize-restore-normal-pressed.png";
            }
            buf[2] = u":/resources/titlebar/mac/close-pressed.png";
        } else {
            if (hovered) {
                buf[0] = u":/resources/titlebar/mac/minimize-hovered.png";
                if (maximized) {
                    buf[1] = u":/resources/titlebar/mac/"
                             u"maximize-restore-maximized-hovered.png";
                } else {
                    buf[1] = u":/resources/titlebar/mac/"
                             u"maximize-restore-normal-hovered.png";
                }
                buf[2] = u":/resources/titlebar/mac/close-hovered.png";
            } else {
                if (active) {
                    buf[0] = u":/resources/titlebar/mac/minimize.png";
                    buf[1] = u":/resources/titlebar/mac/maximize-restore.png";
                    buf[2] = u":/resources/titlebar/mac/close.png";
                } else {
                    buf[0] = u":/resources/titlebar/mac/inactive.png";
                    buf[1] = u":/resources/titlebar/mac/inactive.png";
                    buf[2] = u":/resources/titlebar/mac/inactive.png";
                }
            }
        }
        break;
    }
    }

    return buf;
}

} // namespace Internal

} // namespace CSD
