#include "plugin.h"

#include "csdtitlebar.h"

#include <coreplugin/icore.h>

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

    QWidget *menuBarMarginContainer = new QWidget(mainWindow);
    QVBoxLayout *menuBarMarginContainerLayout =
        new QVBoxLayout(menuBarMarginContainer);
    menuBarMarginContainerLayout->setSpacing(0);
    menuBarMarginContainerLayout->setContentsMargins(0, 0, 0, 0);
    menuBarMarginContainerLayout->addStretch();
    menuBarMarginContainerLayout->addWidget(mainWindow->menuBar());
    menuBarMarginContainerLayout->addStretch();

    this->m_titleBar = new TitleBar(mainWindow);
    this->m_titleBar->setActiveColor(QColor(40, 44, 52));
    auto *titleBarLayout =
        static_cast<QHBoxLayout *>(this->m_titleBar->layout());
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
    titleBarLayout->insertWidget(2, menuBarMarginContainer);

    this->m_filter->apply(
        mainWindow,
        [this]() {
            const bool on = this->m_titleBar->window()->isActiveWindow();
            this->m_titleBar->setActive(on);
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
