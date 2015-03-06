#ifndef STATUSBARCONTROLLER_H
#define STATUSBARCONTROLLER_H

#include "QmlController.h"

class StatusBarController : public QmlController
{
    Q_OBJECT
public:
    StatusBarController(QQmlEngine *engine);
    ~StatusBarController();

protected:
    void qmlWindowReady();
};

#endif // STATUSBARCONTROLLER_H
