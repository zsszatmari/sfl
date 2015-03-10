//
//  DataTransformer.cpp
//  Cast
//
//  Created by Zsolt Szatmari on 02/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "DataTransformer.h"

namespace Cast
{
#define method DataTransformer::
    
    method DataTransformer() :
        _error(0)
    {
    }
    
    method ~DataTransformer()
    {
    }
    
    void method setError(int error)
    {
        _error = error;
    }
    
    int method error() const
    {
        return _error;
    }
    
    bool method pending()
    {
        return false;
    }
}