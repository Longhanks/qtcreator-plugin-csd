#include "linuxcsd.h"

#include <QEvent>
#include <QWidget>

namespace CSD::Internal {

LinuxClientSideDecorationFilter::WidgetCallbacks::WidgetCallbacks(
    Callback onActivationChanged, Callback onWindowStateChanged)
    : onActivationChanged(std::move(onActivationChanged)),
      onWindowStateChanged(std::move(onWindowStateChanged)) {}

LinuxClientSideDecorationFilter::LinuxClientSideDecorationFilter(
    QObject *parent)
    : QObject(parent) {}

LinuxClientSideDecorationFilter::~LinuxClientSideDecorationFilter() {
    for (const auto &pair : this->m_callbacks) {
        pair.first->removeEventFilter(this);
    }
}

bool LinuxClientSideDecorationFilter::eventFilter(QObject *watched,
                                                  QEvent *event) {
    QWidget *widget = static_cast<QWidget *>(watched);
    auto resultIterator = this->m_callbacks.find(widget);

    if (event->type() == QEvent::ActivationChange) {
        resultIterator->second.onActivationChanged();
    } else if (event->type() == QEvent::WindowStateChange) {
        resultIterator->second.onWindowStateChanged();
    }

    return false;
}

void LinuxClientSideDecorationFilter::apply(QWidget *widget,
                                            Callback onActivationChanged,
                                            Callback onWindowStateChanged) {
    this->m_callbacks.emplace(
        widget,
        WidgetCallbacks(std::move(onActivationChanged),
                        std::move(onWindowStateChanged)));
    widget->installEventFilter(this);
    widget->setWindowFlag(Qt::FramelessWindowHint);
}

} // namespace CSD::Internal
