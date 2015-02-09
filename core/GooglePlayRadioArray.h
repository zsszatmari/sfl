//
//  GooglePlayRadioArray.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 6/4/13.
//
//

#ifndef __G_Ear__GooglePlayRadioArray__
#define __G_Ear__GooglePlayRadioArray__

#include "GooglePlayNonLocalArray.h"
#include "GooglePlaySession.h"
#include "SerialExecutor.h"
#include ATOMIC_H

namespace Gear
{
    using namespace Base;
    
    class GooglePlayRadioArray final : public GooglePlayNonLocalArray, public MEMORY_NS::enable_shared_from_this<GooglePlayRadioArray>
    {
    public:
        static shared_ptr<GooglePlayRadioArray> create(const string &radioId,const shared_ptr<GooglePlaySession> &session);
        
        virtual void songsLeft(unsigned long count);
    
    private:
        GooglePlayRadioArray(const string &radioId, const shared_ptr<GooglePlaySession> &session);
        
        class MutableRadio
        {
        public:
            MutableRadio();
            
            bool fetchedFirst;
            ATOMIC_NS::atomic<bool> fetching;
            SerialExecutor executor;
        };
        mutable MutableRadio mr;
        
        const string _radioId;
        
        virtual const bool orderedArray() const;
        void fetchMore() const;
        virtual void songsAccessed() const override;
    };
}

#endif /* defined(__G_Ear__GooglePlayRadioArray__) */
