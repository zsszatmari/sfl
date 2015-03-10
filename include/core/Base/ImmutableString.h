//
//  ImmutableString.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 08/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__ImmutableString__
#define __G_Ear_core__ImmutableString__

#include "stdplus.h"

namespace Base
{
    class core_export ImmutableString final
    {
    public:
        ImmutableString();
        ~ImmutableString();
        static std::string normalizeS(const std::string &str);
        static ImmutableString normalize(const std::string &str);
        ImmutableString(const char *);
        ImmutableString(const std::string &);
        ImmutableString(const ImmutableString &);
        ImmutableString(ImmutableString &&);
        ImmutableString &operator=(const ImmutableString &);
        
        bool operator<(const ImmutableString &rhs) const;
        bool operator>(const ImmutableString &rhs) const;
        bool operator==(const std::string &rhs) const;
        bool operator==(const ImmutableString &rhs) const;
        bool operator!=(const ImmutableString &rhs) const;
        operator std::string() const;
        
        bool contains(const char *token) const;
        
        bool empty() const;
        char at(int i) const;
        // not a null terminated string!
        const char *data() const;
        int compare (size_t pos, size_t len, const std::string& str) const;
        size_t length() const;
        
    private:
        
        struct ControlBlock
        {
            atomic<uint16_t> useCount;
            uint16_t len;
        };
    
        char *_storage;

        mutable int8_t _compareOffset;
    
        ControlBlock &block();
        const ControlBlock &block() const;
        char *str();
        const char *str() const;
        int32_t compare(const ImmutableString &rhs) const;

        friend void swap(ImmutableString &a, ImmutableString &b);
        friend class ImmutableStringHelper;
    };
}

#endif /* defined(__G_Ear_core__ImmutableString__) */
