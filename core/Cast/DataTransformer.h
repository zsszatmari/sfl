//
//  DataTransformer.h
//  Cast
//
//  Created by Zsolt Szatmari on 02/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __Cast__DataTransformer__
#define __Cast__DataTransformer__

namespace Cast
{
    class DataTransformer
    {
    public:
        DataTransformer();
        virtual ~DataTransformer();
        
        void setError(int error);
        int error() const;
        virtual bool pending();
        
    private:
        int _error;
    };
}

#endif /* defined(__Cast__DataTransformer__) */
