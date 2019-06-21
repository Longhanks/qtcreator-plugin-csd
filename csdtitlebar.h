#pragma once

#include <QColor>
#include <QWidget>

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
    QHBoxLayout *m_horizontalLayout;
    QMenuBar *m_menuBar;
    QWidget *m_leftMargin;
    TitleBarButton *m_buttonCaptionIcon;
    TitleBarButton *m_buttonMinimize;
    TitleBarButton *m_buttonMaximizeRestore;
    TitleBarButton *m_buttonClose;

protected:
    void paintEvent(QPaintEvent *event) override;

public:
    explicit TitleBar(QWidget *parent = nullptr);

    bool isActive() const;
    void setActive(bool active);
    bool isMaximized() const;
    void setMaximized(bool maximized);
    void setMinimizable(bool on);
    void setMaximizable(bool on);
    QColor activeColor();
    void setActiveColor(const QColor &activeColor);
    void onWindowStateChange(Qt::WindowStates state);
    bool hovered() const;

signals:
    void minimizeClicked();
    void maximizeRestoreClicked();
    void closeClicked();
};

} // namespace CSD
