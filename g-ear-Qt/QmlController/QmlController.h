#ifndef QMLCONTROLLER_H
#define QMLCONTROLLER_H

#include <QObject>
#include <QQmlEngine>
#include <QQuickWindow>

class QmlController : public QObject
{
    Q_OBJECT
public:
    explicit QmlController(QQmlEngine *engine,
                           QObject *parent = 0);

protected:
    inline QQmlEngine *qmlEngine()
    {
        return _qmlEngine;
    }

private:
    QQmlEngine *_qmlEngine = nullptr;
};

#endif // QMLCONTROLLER_H
