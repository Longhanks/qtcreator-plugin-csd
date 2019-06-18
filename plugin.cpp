#include "plugin.h"

#include "csdtitlebar.h"
#include "csdtitlebarbutton.h"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/icore.h>
#include <projectexplorer/projectexplorerconstants.h>

#include <QApplication>
#include <QBoxLayout>
#include <QMenuBar>
#include <QTimer>

namespace CSD::Internal {

CSDPlugin::CSDPlugin() {
#ifdef _WIN32
    QTimer::singleShot(0, this, [this]() {
        auto *mainWindow = Core::ICore::mainWindow();
        this->m_filter = new Win32ClientSideDecorationFilter(this);
        QCoreApplication::instance()->installNativeEventFilter(this->m_filter);
        this->m_filter->makeWidgetCSD(
            mainWindow, []() -> bool { return false; }, []() {});
    });
#elseif defined(__APPLE__)
#else
    this->m_filter = new LinuxClientSideDecorationFilter(this);
#endif
}

bool CSDPlugin::initialize([[maybe_unused]] const QStringList &arguments,
                           [[maybe_unused]] QString *errorString) {
    QMainWindow *mainWindow = Core::ICore::mainWindow();
    mainWindow->menuBar()->hide();
    this->m_titleBar = new TitleBar(mainWindow);

    QWidget *menuBarContainer = new QWidget(this->m_titleBar);
    QHBoxLayout *menuBarContainerLayout = new QHBoxLayout(menuBarContainer);
    menuBarContainerLayout->setSpacing(0);
    menuBarContainerLayout->setContentsMargins(0, 0, 0, 0);

    this->m_titleBar->setActiveColor(QColor(40, 44, 52));
    auto *titleBarLayout =
        static_cast<QHBoxLayout *>(this->m_titleBar->layout());
    titleBarLayout->insertWidget(2, menuBarContainer);

    // Menu: File
    this->m_buttonMenuFile = new CSD::TitleBarButton(
        "File", CSD::TitleBarButton::MenuBarItem, this->m_titleBar);
    this->m_buttonMenuFile->setMinimumSize(46, 30);
    this->m_buttonMenuFile->setMaximumSize(46, 30);
    this->m_buttonMenuFile->setFocusPolicy(Qt::NoFocus);
    auto *filemenu =
        Core::ActionManager::createMenu(Core::Constants::M_FILE)->menu();
    this->m_buttonMenuFile->setMenu(filemenu);
    menuBarContainerLayout->addWidget(this->m_buttonMenuFile);

    // Menu: Edit
    this->m_buttonMenuEdit = new CSD::TitleBarButton(
        "Edit", CSD::TitleBarButton::MenuBarItem, this->m_titleBar);
    this->m_buttonMenuEdit->setMinimumSize(46, 30);
    this->m_buttonMenuEdit->setMaximumSize(46, 30);
    this->m_buttonMenuEdit->setFocusPolicy(Qt::NoFocus);
    auto *editmenu =
        Core::ActionManager::createMenu(Core::Constants::M_EDIT)->menu();
    this->m_buttonMenuEdit->setMenu(editmenu);
    menuBarContainerLayout->addWidget(this->m_buttonMenuEdit);

    // Menu: Build
    this->m_buttonMenuBuild = new CSD::TitleBarButton(
        "Build", CSD::TitleBarButton::MenuBarItem, this->m_titleBar);
    this->m_buttonMenuBuild->setMinimumSize(46, 30);
    this->m_buttonMenuBuild->setMaximumSize(46, 30);
    this->m_buttonMenuBuild->setFocusPolicy(Qt::NoFocus);
    auto *buildmenu = Core::ActionManager::createMenu(
                          ProjectExplorer::Constants::M_BUILDPROJECT)
                          ->menu();
    this->m_buttonMenuBuild->setMenu(buildmenu);
    menuBarContainerLayout->addWidget(this->m_buttonMenuBuild);

    // Menu: Debug
    this->m_buttonMenuDebug = new CSD::TitleBarButton(
        "Debug", CSD::TitleBarButton::MenuBarItem, this->m_titleBar);
    this->m_buttonMenuDebug->setMinimumSize(46, 30);
    this->m_buttonMenuDebug->setMaximumSize(46, 30);
    this->m_buttonMenuDebug->setFocusPolicy(Qt::NoFocus);
    auto *debugmenu =
        Core::ActionManager::createMenu(ProjectExplorer::Constants::M_DEBUG)
            ->menu();
    this->m_buttonMenuDebug->setMenu(debugmenu);
    menuBarContainerLayout->addWidget(this->m_buttonMenuDebug);

    // Menu: Analyze
    this->m_buttonMenuAnalyze = new CSD::TitleBarButton(
        "Analyze", CSD::TitleBarButton::MenuBarItem, this->m_titleBar);
    this->m_buttonMenuAnalyze->setMinimumSize(46, 30);
    this->m_buttonMenuAnalyze->setMaximumSize(46, 30);
    this->m_buttonMenuAnalyze->setFocusPolicy(Qt::NoFocus);
    //    auto *filemenu =
    //        Core::ActionManager::createMenu(Core::Constants::M_FILE)->menu();
    //    this->m_buttonMenuAnalyze->setMenu(filemenu);
    menuBarContainerLayout->addWidget(this->m_buttonMenuAnalyze);

    // Menu: Tools
    this->m_buttonMenuTools = new CSD::TitleBarButton(
        "Tools", CSD::TitleBarButton::MenuBarItem, this->m_titleBar);
    this->m_buttonMenuTools->setMinimumSize(46, 30);
    this->m_buttonMenuTools->setMaximumSize(46, 30);
    this->m_buttonMenuTools->setFocusPolicy(Qt::NoFocus);
    auto *toolsmenu =
        Core::ActionManager::createMenu(Core::Constants::M_TOOLS)->menu();
    this->m_buttonMenuTools->setMenu(toolsmenu);
    menuBarContainerLayout->addWidget(this->m_buttonMenuTools);

    // Menu: Window
    this->m_buttonMenuWindow = new CSD::TitleBarButton(
        "Window", CSD::TitleBarButton::MenuBarItem, this->m_titleBar);
    this->m_buttonMenuWindow->setMinimumSize(46, 30);
    this->m_buttonMenuWindow->setMaximumSize(46, 30);
    this->m_buttonMenuWindow->setFocusPolicy(Qt::NoFocus);
    auto *windowmenu =
        Core::ActionManager::createMenu(Core::Constants::M_WINDOW)->menu();
    this->m_buttonMenuWindow->setMenu(windowmenu);
    menuBarContainerLayout->addWidget(this->m_buttonMenuWindow);

    // Menu: Help
    this->m_buttonMenuHelp = new CSD::TitleBarButton(
        "Help", CSD::TitleBarButton::MenuBarItem, this->m_titleBar);
    this->m_buttonMenuHelp->setMinimumSize(46, 30);
    this->m_buttonMenuHelp->setMaximumSize(46, 30);
    this->m_buttonMenuHelp->setFocusPolicy(Qt::NoFocus);
    auto *helpmenu =
        Core::ActionManager::createMenu(Core::Constants::M_HELP)->menu();
    this->m_buttonMenuHelp->setMenu(helpmenu);
    menuBarContainerLayout->addWidget(this->m_buttonMenuHelp);

    auto *emptySpace = new QWidget(mainWindow);
    titleBarLayout->insertWidget(3, emptySpace, 1);

    connect(
        this->m_titleBar, &TitleBar::minimizeClicked, this, [mainWindow]() {
            mainWindow->setWindowState(mainWindow->windowState() |
                                       Qt::WindowMinimized);
        });
    connect(this->m_titleBar,
            &TitleBar::maximizeRestoreClicked,
            this,
            [mainWindow]() {
                mainWindow->setWindowState(mainWindow->windowState() ^
                                           Qt::WindowMaximized);
            });
    connect(this->m_titleBar,
            &TitleBar::closeClicked,
            QCoreApplication::instance(),
            &QCoreApplication::quit);

    this->m_filter->apply(
        mainWindow,
        [this]() {
            const bool on = this->m_titleBar->window()->isActiveWindow();
            this->m_titleBar->setActive(on);
            this->m_buttonMenuFile->setActive(on);
            this->m_buttonMenuEdit->setActive(on);
            this->m_buttonMenuBuild->setActive(on);
            this->m_buttonMenuDebug->setActive(on);
            this->m_buttonMenuAnalyze->setActive(on);
            this->m_buttonMenuTools->setActive(on);
            this->m_buttonMenuWindow->setActive(on);
            this->m_buttonMenuHelp->setActive(on);
        },
        [this]() {
            this->m_titleBar->onWindowStateChange(
                this->m_titleBar->window()->windowState());
        });

    if (mainWindow->centralWidget()->objectName() == "CSDWrapper") {
        auto wrapperLayout =
            static_cast<QVBoxLayout *>(mainWindow->centralWidget()->layout());
        wrapperLayout->insertWidget(0, this->m_titleBar);
        mainWindow->show();
        return true;
    }

    mainWindow->layout()->setSpacing(0);

    auto *wrapper = new QWidget(mainWindow);
    wrapper->setObjectName("CSDWrapper");
    wrapper->setMinimumHeight(0);

    auto *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(this->m_titleBar);
    layout->addWidget(mainWindow->centralWidget());

    wrapper->setLayout(layout);

    mainWindow->setCentralWidget(wrapper);

    return true;
}

void CSDPlugin::extensionsInitialized() {}

} // namespace CSD::Internal
