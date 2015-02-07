//
//  PlainCastMessage.h
//  Cast
//
//  Created by Zsolt Szatmari on 02/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __Cast__PlainCastMessage__
#define __Cast__PlainCastMessage__

#include <vector>
#include "cast_channel.pb.h"
#include "DataTransformer.h"

namespace Cast
{
    using std::vector;
    using namespace extensions::api::cast_channel;
    
    class PlainCastMessage : public DataTransformer
    {
    public:
        vector<char> encode(const CastMessage &source);
        CastMessage decode(const vector<char> &source);
        
    private:
        vector<char> _gathered;
    };
}

#endif /* defined(__Cast__PlainCastMessage__) */
