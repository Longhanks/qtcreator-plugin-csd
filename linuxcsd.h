#pragma once

#include <QObject>

#include <functional>
#include <unordered_map>

namespace CSD::Internal {

class LinuxClientSideDecorationFilter : public QObject {

private:
    using Callback = std::function<void()>;
    struct WidgetCallbacks {
        Callback onActivationChanged;
        Callback onWindowStateChanged;
        WidgetCallbacks(Callback onActivationChanged,
                        Callback onWindowStateChanged);
    };
    std::unordered_map<QWidget *, WidgetCallbacks> m_callbacks;

public:
    explicit LinuxClientSideDecorationFilter(QObject *parent = nullptr);
    ~LinuxClientSideDecorationFilter() override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void apply(QWidget *widget,
               Callback onActivationChanged,
               Callback onWindowStateChanged);
};
} // namespace CSD::Internal
