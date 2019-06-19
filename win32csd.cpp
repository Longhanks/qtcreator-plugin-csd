#include "win32csd.h"

#include <QEvent>
#include <QGuiApplication>
#include <QWidget>
#include <QWindow>

#include <qpa/qplatformnativeinterface.h>

namespace CSD::Internal {

Win32ClientSideDecorationFilter::HWNDData::HWNDData(
    QWidget *widget,
    std::function<bool()> isCaptionHovered,
    std::function<void()> onActivationChanged,
    std::function<void()> onWindowStateChanged)
    : widget(widget), isCaptionHovered(std::move(isCaptionHovered)),
      onActivationChanged(std::move(onActivationChanged)),
      onWindowStateChanged(std::move(onWindowStateChanged)) {}

Win32ClientSideDecorationFilter::Win32ClientSideDecorationFilter(
    QObject *parent)
    : QObject(parent) {}

Win32ClientSideDecorationFilter::~Win32ClientSideDecorationFilter() = default;

bool Win32ClientSideDecorationFilter::eventFilter(QObject *watched,
                                                  QEvent *event) {
    QWidget *widget = static_cast<QWidget *>(watched);
    auto resultIterator =
        std::find_if(std::begin(this->appliedHWNDs),
                     std::end(this->appliedHWNDs),
                     [watched](const auto &hwndDataPair) {
                         return hwndDataPair.second.widget == watched;
                     });

    if (event->type() == QEvent::ActivationChange) {
        resultIterator->second.onActivationChanged();
        return false;
    } else if (event->type() == QEvent::WindowStateChange) {
        resultIterator->second.onWindowStateChanged();
        return false;
    }

    if (event->type() != QEvent::Show) {
        return false;
    }

    QWindow *window = widget->windowHandle();
    if (window == nullptr) {
        return false;
    }

    auto rect = ::RECT{0, 0, 0, 0};
    DWORD style = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
                  WS_THICKFRAME | WS_DLGFRAME;
    ::AdjustWindowRectEx(&rect, style, FALSE, 0);

    const auto systemCaptionMargin = ::GetSystemMetrics(SM_CYCAPTION);
    const auto marginBottom = std::abs(rect.bottom) + systemCaptionMargin;
    const auto margins = QMargins(-8, -marginBottom, -8, -8);
    const auto variantMargins = qVariantFromValue(margins);
    window->setProperty("_q_windowsCustomMargins", variantMargins);

    QPlatformWindow *platformWindow = window->handle();
    if (platformWindow == nullptr) {
        return false;
    }

    QGuiApplication::platformNativeInterface()->setWindowProperty(
        platformWindow,
        QStringLiteral("WindowsCustomMargins"),
        variantMargins);

    return false;
}

bool Win32ClientSideDecorationFilter::nativeEventFilter(
    [[maybe_unused]] const QByteArray &eventType,
    void *message,
    long *result) {
    auto msg = static_cast<MSG *>(message);
    if (msg->hwnd == nullptr) {
        return false;
    }
    auto resultIterator = this->appliedHWNDs.find(msg->hwnd);
    if (resultIterator == std::end(this->appliedHWNDs)) {
        return false;
    }
    if (msg->message == WM_CREATE) {
        auto clientRect = ::RECT();
        ::GetWindowRect(msg->hwnd, &clientRect);
        ::SetWindowPos(msg->hwnd,
                       nullptr,
                       clientRect.left,
                       clientRect.top,
                       clientRect.right - clientRect.left,
                       clientRect.bottom - clientRect.top,
                       SWP_FRAMECHANGED);
    }

    if (msg->message == WM_ACTIVATE) {
        auto margins = ::MARGINS();
        margins.cxLeftWidth = 1;
        margins.cxRightWidth = 1;
        margins.cyBottomHeight = 1;
        margins.cyTopHeight = 1;
        ::DwmExtendFrameIntoClientArea(msg->hwnd, &margins);
    }

    if (msg->message == WM_NCCALCSIZE && msg->wParam == TRUE) {
        auto calcSizeParams =
            reinterpret_cast<NCCALCSIZE_PARAMS *>(msg->lParam);

        calcSizeParams->rgrc[0].left = calcSizeParams->rgrc[0].left + 0;
        calcSizeParams->rgrc[0].top = calcSizeParams->rgrc[0].top + 0;
        calcSizeParams->rgrc[0].right = calcSizeParams->rgrc[0].right - 0;
        calcSizeParams->rgrc[0].bottom = calcSizeParams->rgrc[0].bottom - 0;

        auto windowPlacement = ::WINDOWPLACEMENT();
        if (::GetWindowPlacement(msg->hwnd, &windowPlacement)) {
            if (windowPlacement.showCmd == SW_MAXIMIZE) {
                auto monitor =
                    ::MonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONULL);
                if (monitor) {
                    auto monitorInfo = ::MONITORINFO();
                    monitorInfo.cbSize = sizeof(monitorInfo);
                    if (::GetMonitorInfoW(monitor, &monitorInfo)) {
                        calcSizeParams->rgrc[0] = monitorInfo.rcWork;
                    }
                }
            }
        }

        *result = 0;
        return true;
    }

    if (msg->message == WM_NCHITTEST) {
        *result = 0;
        const LONG borderWidth = 8;
        auto clientRect = ::RECT();
        ::GetWindowRect(msg->hwnd, &clientRect);

        auto x = GET_X_LPARAM(msg->lParam);
        auto y = GET_Y_LPARAM(msg->lParam);

        auto resizeWidth = resultIterator->second.widget->minimumWidth() !=
                           resultIterator->second.widget->maximumWidth();
        auto resizeHeight = resultIterator->second.widget->minimumHeight() !=
                            resultIterator->second.widget->maximumHeight();

        if (resizeWidth) {
            if (x >= clientRect.left && x < clientRect.left + borderWidth) {
                *result = HTLEFT;
            }
            if (x < clientRect.right && x >= clientRect.right - borderWidth) {
                *result = HTRIGHT;
            }
        }
        if (resizeHeight) {
            if (y < clientRect.bottom &&
                y >= clientRect.bottom - borderWidth) {
                *result = HTBOTTOM;
            }
            if (y >= clientRect.top && y < clientRect.top + borderWidth) {
                *result = HTTOP;
            }
        }
        if (resizeWidth && resizeHeight) {
            if (x >= clientRect.left && x < clientRect.left + borderWidth &&
                y < clientRect.bottom &&
                y >= clientRect.bottom - borderWidth) {
                *result = HTBOTTOMLEFT;
            }
            if (x < clientRect.right && x >= clientRect.right - borderWidth &&
                y < clientRect.bottom &&
                y >= clientRect.bottom - borderWidth) {
                *result = HTBOTTOMRIGHT;
            }
            if (x >= clientRect.left && x < clientRect.left + borderWidth &&
                y >= clientRect.top && y < clientRect.top + borderWidth) {
                *result = HTTOPLEFT;
            }
            if (x < clientRect.right && x >= clientRect.right - borderWidth &&
                y >= clientRect.top && y < clientRect.top + borderWidth) {
                *result = HTTOPRIGHT;
            }
        }

        if (*result != 0) {
            return true;
        }

        if (resultIterator->second.isCaptionHovered()) {
            *result = HTCAPTION;
            return true;
        }
    }

    if (msg->message == WM_NCACTIVATE) {
        auto enabled = FALSE;
        auto success = ::DwmIsCompositionEnabled(&enabled) == S_OK;
        if (!(enabled && success)) {
            *result = 1;
            return true;
        }
    }

    if (msg->message == WM_GETMINMAXINFO) {
        auto minMaxInfo = reinterpret_cast<MINMAXINFO *>(msg->lParam);
        auto windowPlacement = ::WINDOWPLACEMENT();
        if (::GetWindowPlacement(msg->hwnd, &windowPlacement)) {
            if (windowPlacement.showCmd == SW_MAXIMIZE) {
                auto clientRect = ::RECT();

                if (!::GetWindowRect(msg->hwnd, &clientRect)) {
                    return false;
                }

                auto monitor =
                    ::MonitorFromRect(&clientRect, MONITOR_DEFAULTTONULL);
                if (!monitor) {
                    return false;
                }

                auto monitorInfo = ::MONITORINFO();
                monitorInfo.cbSize = sizeof(monitorInfo);
                ::GetMonitorInfo(monitor, &monitorInfo);

                auto workingArea = monitorInfo.rcWork;
                auto monitorArea = monitorInfo.rcMonitor;

                minMaxInfo->ptMaxPosition.x =
                    std::abs(workingArea.left - monitorArea.left);
                minMaxInfo->ptMaxPosition.y =
                    std::abs(workingArea.top - monitorArea.top);

                minMaxInfo->ptMaxSize.x =
                    std::abs(workingArea.right - workingArea.left);
                minMaxInfo->ptMaxSize.y =
                    std::abs(workingArea.bottom - workingArea.top);
                minMaxInfo->ptMaxTrackSize.x = minMaxInfo->ptMaxSize.x;
                minMaxInfo->ptMaxTrackSize.y = minMaxInfo->ptMaxSize.y;

                *result = 1;
                return true;
            }
        }
    }

    if (msg->message == WM_SIZE) {
        auto windowPlacement = ::WINDOWPLACEMENT();
        windowPlacement.length = sizeof(WINDOWPLACEMENT);
        ::GetWindowPlacement(msg->hwnd, &windowPlacement);
        if (windowPlacement.showCmd == SW_MAXIMIZE) {
            ::SetWindowPos(msg->hwnd,
                           nullptr,
                           0,
                           0,
                           0,
                           0,
                           SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
        }
    }
    return false;
}

void Win32ClientSideDecorationFilter::apply(
    QWidget *widget,
    std::function<bool()> isCaptionHovered,
    std::function<void()> onActivationChanged,
    std::function<void()> onWindowStateChanged) {
    this->appliedHWNDs.emplace(reinterpret_cast<HWND>(widget->winId()),
                               HWNDData(widget,
                                        std::move(isCaptionHovered),
                                        std::move(onActivationChanged),
                                        std::move(onWindowStateChanged)));
    widget->installEventFilter(this);
}

} // namespace CSD::Internal
