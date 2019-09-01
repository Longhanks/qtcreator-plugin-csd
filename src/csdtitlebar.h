#pragma once

#include "captionbuttonstyle.h"

#include <QColor>
#include <QIcon>
#include <QStringView>
#include <QWidget>

#include <array>
#include <optional>

class QHBoxLayout;
class QLayout;
class QLabel;
class QMenuBar;

namespace CSD {

class TitleBarButton;

class TitleBar : public QWidget {
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive WRITE setActive)
    Q_PROPERTY(bool maximized READ isMaximized WRITE setMaximized)

private:
#ifdef _WIN32
    std::optional<QColor> readDWMColorizationColor();
#endif
    bool m_active = false;
    bool m_maximized = false;
    QColor m_activeColor;
    QColor m_hoverColor = QColor(62, 68, 81);
    QHBoxLayout *m_horizontalLayout;
    QMenuBar *m_menuBar;
    QWidget *m_leftMargin;
    CaptionButtonStyle m_captionButtonStyle;
    TitleBarButton *m_buttonCaptionIcon;
    TitleBarButton *m_buttonRun;
    TitleBarButton *m_buttonDebug;
    TitleBarButton *m_buttonBuild;
    TitleBarButton *m_buttonModeWelcome;
    TitleBarButton *m_buttonModeEdit;
    TitleBarButton *m_buttonModeDesign;
    TitleBarButton *m_buttonModeDebug;
    TitleBarButton *m_buttonModeProjects;
    TitleBarButton *m_buttonModeHelp;
    TitleBarButton *m_buttonMinimize;
    TitleBarButton *m_buttonMaximizeRestore;
    TitleBarButton *m_buttonClose;

protected:
#if !defined(_WIN32) && !defined(__APPLE__)
    void mousePressEvent(QMouseEvent *event) override;
#endif
    void paintEvent(QPaintEvent *event) override;

public:
    explicit TitleBar(CaptionButtonStyle captionButtonStyle,
                      const QIcon &captionIcon = QIcon(),
                      QWidget *parent = nullptr);
    ~TitleBar() override;

    bool isActive() const;
    void setActive(bool active);
    bool isMaximized() const;
    void setMaximized(bool maximized);
    void setMinimizable(bool on);
    void setMaximizable(bool on);
    QColor activeColor();
    void setActiveColor(const QColor &activeColor);
    QColor hoverColor() const;
    void setHoverColor(QColor hoverColor);
    CaptionButtonStyle captionButtonStyle() const;
    void setCaptionButtonStyle(CaptionButtonStyle captionButtonStyle);
    void onWindowStateChange(Qt::WindowStates state);
    bool hovered() const;

    bool isCaptionButtonHovered() const;
    void triggerCaptionRepaint();

signals:
    void minimizeClicked();
    void maximizeRestoreClicked();
    void closeClicked();

private:
    void resetModeButtonStates();
};

namespace Internal {

std::array<QStringView, 3> captionIconPathsForState(bool active,
                                                    bool maximized,
                                                    bool hovered,
                                                    bool pressed,
                                                    CaptionButtonStyle style);

}

} // namespace CSD
