#pragma once

#include <extensionsystem/iplugin.h>

#include <QStringList>

#ifdef _WIN32
#include "win32csd.h"
#elseif defined(__APPLE__)
#else
#include "linuxcsd.h"
#endif

namespace CSD {

class TitleBar;

namespace Internal {

class CSDPlugin : public ExtensionSystem::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "csd.json")

public:
    CSDPlugin();
    bool initialize(const QStringList &arguments,
                    QString *errorString) override;
    void extensionsInitialized() override;

private:
#ifdef _WIN32
    Win32ClientSideDecorationFilter *m_filter;
#elseif defined(__APPLE__)
#else
    LinuxClientSideDecorationFilter *m_filter;
    TitleBar *m_titleBar;
#endif
};

} // namespace Internal
} // namespace CSD
