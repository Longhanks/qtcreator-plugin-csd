#pragma once

#include <Windows.h>
#include <dwmapi.h>
#include <windowsx.h>

#include <QAbstractNativeEventFilter>
#include <QMargins>
#include <QMetaType>
#include <QObject>

#include <functional>
#include <unordered_map>

Q_DECLARE_METATYPE(QMargins)

class QWidget;

namespace CSD::Internal {

class Win32ClientSideDecorationFilter : public QObject,
                                        public QAbstractNativeEventFilter {
private:
    struct HWNDData {
        QWidget *m_ptr;
        std::function<bool()> m_isCaptionCallback;
        std::function<void()> m_activationOrWindowStateChange;
        HWNDData(QWidget *ptr,
                 std::function<bool()> isCaptionCallback,
                 std::function<void()> activationOrWindowStateChange);
    };

    std::unordered_map<HWND, HWNDData> csdHWNDs;

public:
    explicit Win32ClientSideDecorationFilter(QObject *parent = nullptr);
    ~Win32ClientSideDecorationFilter() override;

    void makeWidgetCSD(QWidget *w,
                       std::function<bool()> isCaptionCallback,
                       std::function<void()> activationOrWindowStateChange);

    bool eventFilter(QObject *watched, QEvent *event) override;

    bool nativeEventFilter(const QByteArray &eventType,
                           void *message,
                           long *result) override;
};
} // namespace CSD::Internal
