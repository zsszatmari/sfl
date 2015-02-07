#include <QApplication>
#include <QAction>
#include <QMenu>
#include "SystemTrayIcon.h"

SystemTrayIcon::SystemTrayIcon(QObject *parent) : QObject(parent)
{
    _systemTrayIcon = std::make_shared<QSystemTrayIcon>(this);
    _systemTrayIcon->setIcon(QIcon(":/images/Image/Gear.ico"));
}

SystemTrayIcon::~SystemTrayIcon()
{

}


