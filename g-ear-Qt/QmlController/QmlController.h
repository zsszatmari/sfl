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

    inline void setWindow(QQuickWindow *window)
    {
        _qmlWindow = window;
    }

protected:
    inline QQmlEngine *qmlEngine()
    {
        return _qmlEngine;
    }

    inline QQuickWindow *qmlWindow()
    {
        return _qmlWindow;
    }

private:
    QQmlEngine *_qmlEngine = nullptr;
    QQuickWindow *_qmlWindow = nullptr;
};

#endif // QMLCONTROLLER_H
