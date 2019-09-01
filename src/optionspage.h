#pragma once

#include "settings.h"

#include <coreplugin/dialogs/ioptionspage.h>

#include <QPointer>

namespace CSD::Internal {

class OptionsDialog;

class OptionsPage : public Core::IOptionsPage {
    Q_OBJECT

public:
    OptionsPage(const Settings &settings, QObject *parent);

    void setSettings(const Settings &settings);

    QWidget *widget() override;
    void apply() override;
    void finish() override;

signals:
    void settingsChanged(const Settings &settings);

private:
    QPointer<OptionsDialog> m_widget;
    Settings m_settings;
};

} // namespace CSD::Internal
