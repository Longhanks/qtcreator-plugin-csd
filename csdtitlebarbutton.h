#pragma once

#include <QPushButton>

namespace CSD {

class TitleBar;

class TitleBarButton : public QPushButton {
    Q_OBJECT

public:
    enum Role { CaptionIcon, Minimize, MaximizeRestore, Close };
    Q_ENUM(Role)

    explicit TitleBarButton(Role role, TitleBar *parent = nullptr);
    explicit TitleBarButton(const QString &text,
                            Role role,
                            TitleBar *parent = nullptr);
    explicit TitleBarButton(const QIcon &icon,
                            const QString &text,
                            Role role,
                            TitleBar *parent = nullptr);

    [[nodiscard]] bool isActive() const;
    void setActive(bool active);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Role m_role;
    bool m_active = false;
};

} // namespace CSD
