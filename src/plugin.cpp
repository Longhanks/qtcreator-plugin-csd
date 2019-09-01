#include "plugin.h"

#include "csdtitlebar.h"
#include "csdtitlebarbutton.h"
#include "optionspage.h"

#include <coreplugin/coreicons.h>
#include <coreplugin/icore.h>

#include <utils/theme/theme.h>

#include <QApplication>
#include <QBoxLayout>
#include <QMenuBar>

inline void init_resource() {
    Q_INIT_RESOURCE(csd);
}

namespace CSD::Internal {

CSDPlugin::CSDPlugin() {
    init_resource();
#ifdef _WIN32
    this->m_filter = new Win32ClientSideDecorationFilter(this);
    QCoreApplication::instance()->installNativeEventFilter(this->m_filter);
#elif defined(__APPLE__)
#else
    this->m_filter = new LinuxClientSideDecorationFilter(this);
#endif
}

CSDPlugin::~CSDPlugin() {
    delete this->m_filter;
    this->m_filter = nullptr;
    this->m_titleBar = nullptr;
}

bool CSDPlugin::initialize([[maybe_unused]] const QStringList &arguments,
                           [[maybe_unused]] QString *errorString) {
    this->m_settings.load(Core::ICore::settings());

    QMainWindow *mainWindow = Core::ICore::mainWindow();
    auto wrapperLayout =
        static_cast<QVBoxLayout *>(mainWindow->centralWidget()->layout());

    this->m_titleBar = new TitleBar(this->m_settings.captionButtonStyle,
                                    Core::Icons::QTCREATORLOGO_BIG.icon(),
                                    mainWindow->centralWidget());
    this->m_titleBar->setHoverColor(
        Utils::creatorTheme()->color(Utils::Theme::FancyToolButtonHoverColor));
    this->m_titleBar->setActiveColor(QColor(40, 44, 52));
    wrapperLayout->insertWidget(0, this->m_titleBar);

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
            mainWindow,
            &QWidget::close);

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

    this->m_optionsPage = new OptionsPage(this->m_settings, this);

    connect(m_optionsPage,
            &OptionsPage::settingsChanged,
            this,
            &CSDPlugin::settingsChanged);

    connect(Core::ICore::instance(),
            &Core::ICore::saveSettingsRequested,
            this,
            [this] { this->m_settings.save(Core::ICore::settings()); });

    return true;
}

CSDPlugin::ShutdownFlag CSDPlugin::aboutToShutdown() {
#ifdef _WIN32
    QCoreApplication::instance()->removeNativeEventFilter(this->m_filter);
#endif
    return SynchronousShutdown;
}

void CSDPlugin::extensionsInitialized() {}

void CSDPlugin::settingsChanged(const Settings &settings) {
    settings.save(Core::ICore::settings());
    this->m_settings = settings;
    this->m_optionsPage->setSettings(this->m_settings);
    this->m_titleBar->setCaptionButtonStyle(
        this->m_settings.captionButtonStyle);
    for (TitleBarButton *btn :
         this->m_titleBar->findChildren<TitleBarButton *>()) {
        btn->update();
    }
}

} // namespace CSD::Internal
