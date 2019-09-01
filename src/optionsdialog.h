#pragma once

#include <QWidget>

class QGroupBox;
class QRadioButton;

namespace CSD::Internal {

struct Settings;

class OptionsDialog : public QWidget {
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = nullptr);
    ~OptionsDialog() override = default;

    void setSettings(const Settings &settings);
    Settings settings();

private:
    QGroupBox *groupBoxCaptionButtonStyle = nullptr;
    QRadioButton *radioButtonCaptionButtonStyleCustom = nullptr;
    QRadioButton *radioButtonCaptionButtonStyleWin = nullptr;
    QRadioButton *radioButtonCaptionButtonStyleMac = nullptr;
};

} // namespace CSD::Internal
