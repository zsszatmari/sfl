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
    void buildConnect(bool isToConnect);

protected:
    void qmlWindowReady();
};

#endif // SETTINGPANELCONTROLLER_H
