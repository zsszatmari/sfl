//
//  SignalConnection.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_iOS__SignalConnection__
#define __G_Ear_iOS__SignalConnection__

#include "IExecutor.h"
#include "stdplus.h"
#include MEMORY_H

namespace Base
{
    class core_export SignalConnectionTokenBase
    {
    protected:
        SignalConnectionTokenBase();
        virtual ~SignalConnectionTokenBase();
        
    private:
        SignalConnectionTokenBase(const SignalConnectionTokenBase &rhs); // delete
        SignalConnectionTokenBase &operator=(const SignalConnectionTokenBase &rhs); // delete
    };
    
    
    class SignalConnectionToken final : public SignalConnectionTokenBase
    {
    public:
        SignalConnectionToken(const std::function<void(void)> &f);
        void signal();
        
    private:
        std::function<void(void)> _f;
    };
    
    
    typedef MEMORY_NS::shared_ptr<SignalConnectionTokenBase> SignalConnection;
}

#endif /* defined(__G_Ear_iOS__SignalConnection__) */
