#ifndef SETTINGPANELCONTROLLER_H
#define SETTINGPANELCONTROLLER_H

#include <QObject>
#include "QmlController.h"

class SettingPanelController : public QmlController
{
    Q_OBJECT
public:
    SettingPanelController(QQmlEngine *engine);

    void addSettingsTabs(QQuickWindow *window);

signals:

public slots:
    void connect(bool isToConnect);

};

#endif // SETTINGPANELCONTROLLER_H
