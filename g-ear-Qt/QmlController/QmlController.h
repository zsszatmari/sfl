#ifndef QMLCONTROLLER_H
#define QMLCONTROLLER_H

#include <QObject>
#include <QQmlEngine>
#include <QQuickWindow>

class QmlController : public QObject
{
    Q_OBJECT
public:
    explicit QmlController(QQmlEngine *engine);

    inline void setWindow(QQuickWindow *window)
    {
        _qmlWindow = window;
        if (_qmlWindow)
        {
            qmlWindowReady();
        }
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

    virtual void qmlWindowReady() = 0;

private:
    QQmlEngine *_qmlEngine = nullptr;
    QQuickWindow *_qmlWindow = nullptr;
};

#endif // QMLCONTROLLER_H
