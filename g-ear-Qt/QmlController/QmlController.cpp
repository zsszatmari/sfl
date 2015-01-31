#include "QmlController.h"

QmlController::QmlController(QQmlEngine *engine,
                             QObject *parent) :
    QObject(parent)
    , _qmlEngine(engine)
{
    Q_ASSERT(_qmlEngine);
}
