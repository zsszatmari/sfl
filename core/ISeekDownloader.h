//
//  ISeekDownloader.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 09/12/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__ISeekDownloader__
#define __G_Ear_core__ISeekDownloader__

#include "IDownloader.h"

namespace Gear
{
    class ISeekDownloader : public IDownloader
    {
    public:
        virtual void seek(int offset) = 0;
    };
}

#endif /* defined(__G_Ear_core__ISeekDownloader__) */
