#ifndef SETTINGPANELCONTROLLER_H
#define SETTINGPANELCONTROLLER_H

#include <QObject>
#include "QmlController.h"

class SettingPanelController : public QmlController
{
    Q_OBJECT
public:
    SettingPanelController(QQmlEngine *engine);

public slots:
    void addSettingsTabs();
    void buildConnect(bool isToConnect);

};

#endif // SETTINGPANELCONTROLLER_H
