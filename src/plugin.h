#pragma once

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

class CSDPlugin : public ExtensionSystem::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "csd.json")

public:
    CSDPlugin();
    ~CSDPlugin() override;
    bool initialize(const QStringList &arguments,
                    QString *errorString) override;
    void extensionsInitialized() override;
    ShutdownFlag aboutToShutdown() override;

private:
#ifdef _WIN32
    Win32ClientSideDecorationFilter *m_filter;
#elif defined(__APPLE__)
#else
    LinuxClientSideDecorationFilter *m_filter;
#endif
#ifndef __APPLE__
    TitleBar *m_titleBar;
#endif
};

} // namespace Internal
} // namespace CSD
