//
//  IKeychain.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/25/13.
//
//

#ifndef __G_Ear_Player__IKeychain__
#define __G_Ear_Player__IKeychain__

#include <vector>
#include <string>
#include "stdplus.h"

namespace Gear
{
    using std::string;
    using std::vector;
    
    class core_export IKeychain
    {
    public:
        virtual string username(const string &identifier) const = 0;
        virtual string pass(const string &identifier) const = 0;
        
        virtual void save(const string &identifier, const string &username, const string &data) = 0;
        virtual void forget(const string &identifier) = 0;
        virtual ~IKeychain();
    };
}

#endif /* defined(__G_Ear_Player__IKeychain__) */
