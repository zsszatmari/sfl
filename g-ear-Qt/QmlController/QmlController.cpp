#include "QmlController.h"

QmlController::QmlController(QQmlEngine *engine) :
    QObject(engine)
    , _qmlEngine(engine)
{
    Q_ASSERT(_qmlEngine);
}
