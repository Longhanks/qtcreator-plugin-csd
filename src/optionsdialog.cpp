#include "optionsdialog.h"

#include "captionbuttonstyle.h"
#include "settings.h"

#include <QBoxLayout>
#include <QGroupBox>
#include <QRadioButton>

namespace CSD::Internal {

OptionsDialog::OptionsDialog(QWidget *parent) : QWidget(parent) {
    auto layout = new QVBoxLayout(this);

    this->groupBoxCaptionButtonStyle =
        new QGroupBox(tr("Caption button style"), this);
    auto groupBoxLayout = new QVBoxLayout(this->groupBoxCaptionButtonStyle);

    auto radioButtonCaptionButtonStyleCustomText = tr("Custom");
    this->radioButtonCaptionButtonStyleCustom =
        new QRadioButton(radioButtonCaptionButtonStyleCustomText,
                         this->groupBoxCaptionButtonStyle);
    groupBoxLayout->addWidget(this->radioButtonCaptionButtonStyleCustom);

    auto radioButtonCaptionButtonStyleWinText = tr("Windows");
    this->radioButtonCaptionButtonStyleWin =
        new QRadioButton(radioButtonCaptionButtonStyleWinText,
                         this->groupBoxCaptionButtonStyle);
    groupBoxLayout->addWidget(this->radioButtonCaptionButtonStyleWin);

    auto radioButtonCaptionButtonStyleMacText = tr("Mac");
    this->radioButtonCaptionButtonStyleMac =
        new QRadioButton(radioButtonCaptionButtonStyleMacText,
                         this->groupBoxCaptionButtonStyle);
    groupBoxLayout->addWidget(this->radioButtonCaptionButtonStyleMac);

    this->groupBoxCaptionButtonStyle->setLayout(groupBoxLayout);

    layout->addWidget(this->groupBoxCaptionButtonStyle);
    layout->addStretch();
    this->setLayout(layout);
}

void OptionsDialog::setSettings(const Settings &settings) {
    switch (settings.captionButtonStyle) {
    case CaptionButtonStyle::custom: {
        this->radioButtonCaptionButtonStyleCustom->setChecked(true);
        break;
    }
    case CaptionButtonStyle::win: {
        this->radioButtonCaptionButtonStyleWin->setChecked(true);
        break;
    }
    case CaptionButtonStyle::mac: {
        this->radioButtonCaptionButtonStyleMac->setChecked(true);
        break;
    }
    }
}

Settings OptionsDialog::settings() {
    Settings settings;
    settings.captionButtonStyle = [this] {
        if (this->radioButtonCaptionButtonStyleCustom->isChecked()) {
            return CaptionButtonStyle::custom;
        }
        if (this->radioButtonCaptionButtonStyleWin->isChecked()) {
            return CaptionButtonStyle::win;
        }
        if (this->radioButtonCaptionButtonStyleMac->isChecked()) {
            return CaptionButtonStyle::mac;
        }
        return CaptionButtonStyle::custom;
    }();
    return settings;
}

} // namespace CSD::Internal
