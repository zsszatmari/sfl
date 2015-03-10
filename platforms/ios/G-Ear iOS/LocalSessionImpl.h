//
//  LocalSessionImpl.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmari on 08/11/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_iOS__LocalSessionImpl__
#define __G_Ear_iOS__LocalSessionImpl__

#include "LocalSession.h"

namespace Gear
{
    class LocalSessionImpl : public LocalSession
    {
    public:
        virtual void refresh();
    };
}

#endif /* defined(__G_Ear_iOS__LocalSessionImpl__) */
