#pragma once

#include <QPushButton>

namespace CSD {

class TitleBar;

class TitleBarButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(double fader READ fader WRITE setFader)
    Q_PROPERTY(bool keepDown READ keepDown WRITE setKeepDown)

public:
    enum Role { CaptionIcon, Minimize, MaximizeRestore, Close, Tool };
    Q_ENUM(Role)

    explicit TitleBarButton(Role role, TitleBar *parent = nullptr);
    explicit TitleBarButton(const QString &text,
                            Role role,
                            TitleBar *parent = nullptr);
    explicit TitleBarButton(const QIcon &icon,
                            const QString &text,
                            Role role,
                            TitleBar *parent = nullptr);

    double fader() const;
    void setFader(double value);
    QColor hoverColor() const;
    void setHoverColor(QColor hoverColor);
    bool keepDown() const;
    void setKeepDown(bool keepDown);

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    Role m_role;
    double m_fader = 0.0;
    QColor m_hoverColor = QColor(62, 68, 81);
    bool m_keepDown = false;
};

} // namespace CSD
