//
//  GuardedData.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/28/13.
//
//

#ifndef __G_Ear_Player__GuardedData__
#define __G_Ear_Player__GuardedData__

#include "stdplus.h"
#include "SerialExecutor.h"

namespace Base
{    
    template<typename T>
    class GuardedData final
    {
    public:
        GuardedData();
        GuardedData(const T &);
        void access(const std::function<int(T &)> &task);
        void accessAndWait(const std::function<int(T &)> &task);
        
    private:
        T _data;
        SerialExecutor _executor;
    };
    
    template<typename T>
    GuardedData<T>::GuardedData()
    {
    }
    
    template<typename T>
    GuardedData<T>::GuardedData(const T &data) :
        _data(data)
    {
    }
    
    template<typename T>
    void GuardedData<T>::access(const std::function<int(T &)> &task)
    {
        T &localData = _data;
        _executor.addTask([task,&localData]{
            task(localData);
        });
    }
    
    
    template<typename T>
    void GuardedData<T>::accessAndWait(const std::function<int(T &)> &task)
    {
        T &localData = _data;
        _executor.addTaskAndWait([task,&localData]{
            task(localData);
        });
    }
}

#endif /* defined(__G_Ear_Player__GuardedData__) */
