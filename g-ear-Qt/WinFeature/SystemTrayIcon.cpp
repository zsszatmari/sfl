#include <QApplication>
#include <QAction>
#include <QMenu>
#include "SystemTrayIcon.h"

SystemTrayIcon::SystemTrayIcon(QObject *parent) : QObject(parent)
{
    _systemTrayIcon = std::make_shared<QSystemTrayIcon>(this);
    _systemTrayIcon->setIcon(QIcon(":/images/Image/Gear.ico"));

    _systemTrayIconContextMenu = std::make_shared<QMenu>(nullptr);

    // _systemTrayIconContextMenu would take the ownership of _exitAction
    // so we don't need to care about the memory thing here
    _exitAction = _systemTrayIconContextMenu->addAction(tr("Quit Gear"));

    connect(_exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    _systemTrayIcon->setContextMenu(_systemTrayIconContextMenu.get());
}

SystemTrayIcon::~SystemTrayIcon()
{

}


