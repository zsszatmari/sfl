#ifndef SHAREDMEMORYSTATUSTHREAD_HPP
#define SHAREDMEMORYSTATUSTHREAD_HPP

#include <QApplication>
#include <QThread>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <cstring>

#include <QDebug>

class SharedMemoryStatusThread : public QThread
{
    Q_OBJECT

public:
    SharedMemoryStatusThread(QObject *parent = nullptr) : QThread(parent)
    {

    }

protected:
    virtual void run() Q_DECL_OVERRIDE
    {
        while (1)
        {
            QSystemSemaphore sema("GEAR_MUSIC_PLAYER_KEY", 1, QSystemSemaphore::Open);
            sema.acquire();

            QSharedMemory mem("GEAR_MUSIC_PLAYER_KEY_WIN");

            mem.attach();
            mem.lock();
            const void *dataArea = mem.data();
            bool quitApp;
            std::memcpy(&quitApp, dataArea, 1);
            mem.unlock();
            mem.detach();

            sema.release();

//            qDebug() << "quitApp: " << quitApp;

//            QThread::sleep(3);

            if (quitApp)
            {
                break;
            }
        }
    }
};

#endif // SHAREDMEMORYSTATUSTHREAD_HPP
