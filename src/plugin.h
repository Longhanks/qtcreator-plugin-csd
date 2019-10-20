#pragma once

#include "settings.h"

#include <extensionsystem/iplugin.h>

#include <QStringList>

#ifdef _WIN32
#include "win32csd.h"
#elif defined(__APPLE__)
#else
#include "linuxcsd.h"
#endif

namespace CSD {

class TitleBar;
class TitleBarButton;

namespace Internal {

class OptionsPage;

class CSDPlugin final : public ExtensionSystem::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "csd.json")

public:
    CSDPlugin() noexcept;
    ~CSDPlugin() noexcept override = default;
    bool initialize(const QStringList &arguments,
                    QString *errorString) override;
    void extensionsInitialized() override;
    ShutdownFlag aboutToShutdown() override;

private:
#ifdef _WIN32
    Win32ClientSideDecorationFilter *m_filter = nullptr;
#elif defined(__APPLE__)
#else
    LinuxClientSideDecorationFilter *m_filter = nullptr;
#endif
#ifndef __APPLE__
    TitleBar *m_titleBar = nullptr;
#endif

    OptionsPage *m_optionsPage = nullptr;
    Settings m_settings;

    void settingsChanged(const Settings &settings);
};

} // namespace Internal
} // namespace CSD
