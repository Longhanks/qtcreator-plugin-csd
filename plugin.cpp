#include "plugin.h"

#include "csdtitlebar.h"
#include "csdtitlebarbutton.h"

#include <coreplugin/icore.h>

#include <QApplication>
#include <QBoxLayout>
#include <QMenuBar>
#include <QTimer>

namespace CSD::Internal {

CSDPlugin::CSDPlugin() {
#ifdef _WIN32
    this->m_filter = new Win32ClientSideDecorationFilter(this);
    QCoreApplication::instance()->installNativeEventFilter(this->m_filter);
#elif defined(__APPLE__)
#else
    this->m_filter = new LinuxClientSideDecorationFilter(this);
#endif
}

bool CSDPlugin::initialize([[maybe_unused]] const QStringList &arguments,
                           [[maybe_unused]] QString *errorString) {
    QMainWindow *mainWindow = Core::ICore::mainWindow();

    auto wrapperLayout =
        static_cast<QVBoxLayout *>(mainWindow->centralWidget()->layout());
    this->m_titleBar = new TitleBar(mainWindow->centralWidget());
    wrapperLayout->insertWidget(0, this->m_titleBar);

    this->m_titleBar->setActiveColor(QColor(40, 44, 52));

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
#ifdef _WIN32
        [this]() { return this->m_titleBar->hovered(); },
#endif
        [this]() {
            const bool on = this->m_titleBar->window()->isActiveWindow();
            this->m_titleBar->setActive(on);
        },
        [this]() {
            this->m_titleBar->onWindowStateChange(
                this->m_titleBar->window()->windowState());
        });

    return true;
}

void CSDPlugin::extensionsInitialized() {}

} // namespace CSD::Internal
