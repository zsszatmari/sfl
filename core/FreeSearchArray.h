//
//  FreeSearchArray.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/4/13.
//
//

#ifndef __G_Ear_Player__FreeSearchArray__
#define __G_Ear_Player__FreeSearchArray__
    
#include "stdplus.h"
#include ATOMIC_H
#include MEMORY_H
#include "NonLocalArray.h"
#include "ISession.h"
#include "SerialExecutor.h"
    
namespace Gear
{
    using namespace Base;
    
    class FreeSearchArray : public NonLocalArray, public MEMORY_NS::enable_shared_from_this<FreeSearchArray>
    {
    public:
        static shared_ptr<FreeSearchArray> create(const shared_ptr<ISession> &session);
        
        virtual bool operator==(const ISongArray &rhs);
        virtual const string emptyText() const;
        
        virtual void setFilterPredicate(const SongPredicate &f);
        virtual bool keepSongOrder() const;

    private:
        FreeSearchArray(const shared_ptr<ISession> &session);
        FreeSearchArray(const FreeSearchArray &rhs); // delete
        FreeSearchArray &operator=(const FreeSearchArray &rhs); // delete
        
        //virtual bool filterNeeded() const;
        virtual bool needsPredicate() const;
        
        class MutableState final
        {
        public:
            MutableState();
            
            ATOMIC_NS::atomic<bool> runningSearch;
        };
        
        mutable MutableState m;
        
        SerialExecutor executor;
    };
};


#endif /* defined(__G_Ear_Player__FreeSearchArray__) */
