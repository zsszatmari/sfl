#ifndef _SYSTEMTRAYICON_H
#define _SYSTEMTRAYICON_H

#include <QSystemTrayIcon>
#include <memory>

class QAction;

class SystemTrayIcon : public QObject
{
    Q_OBJECT
public:
    explicit SystemTrayIcon(QObject *parent = 0);
    ~SystemTrayIcon();

public:
    inline const std::shared_ptr<QSystemTrayIcon> &getIcon() const
    {
        return _systemTrayIcon;
    }

private:
    std::shared_ptr<QSystemTrayIcon> _systemTrayIcon;
};

#endif
