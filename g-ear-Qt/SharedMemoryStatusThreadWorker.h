#ifndef SHAREDMEMORYSTATUSTHREADWORKER_HPP
#define SHAREDMEMORYSTATUSTHREADWORKER_HPP

#include <QApplication>
#include <QObject>
#include <QThread>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <cstring>

#include <QDebug>

class SharedMemoryStatusThreadWorker : public QObject
{
    Q_OBJECT

public:
    SharedMemoryStatusThreadWorker(QObject *parent = nullptr) : QObject(parent)
    {

    }

public slots:
    void doWork()
    {
        QSystemSemaphore sema("GEAR_MUSIC_PLAYER_KEY", 1, QSystemSemaphore::Open);
        sema.acquire();

        QSharedMemory mem("GEAR_MUSIC_PLAYER_KEY_WIN");

        bool quitApp;
        bool bringToFront;

        mem.attach();
        mem.lock();
        const void *dataArea = mem.data();
        std::memcpy(&quitApp, dataArea, 1);
        std::memcpy(&bringToFront, dataArea + 1, 1);
        mem.unlock();
        mem.detach();

//        qDebug() << "quitApp: " << quitApp;
//        qDebug() << "bringToFront: " << bringToFront;

        if (quitApp)
        {
            qDebug() << "About to quit application";
            qDebug() << "Job done";
            emit jobDone();
            sema.release();
            return;
        }
        else if (bringToFront)
        {
            emit bringToFrontSignal();

            mem.attach();
            mem.lock();
            void *dataArea = mem.data();
            bringToFront = false;
            std::memcpy(dataArea + 1, &bringToFront, 1);
            mem.unlock();
            mem.detach();
        }

        sema.release();
    }

signals:
    void bringToFrontSignal();
    void jobDone();
};

#endif // SHAREDMEMORYSTATUSTHREADWORKER_HPP
