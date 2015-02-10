//
//  AppleKeychain.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/25/13.
//
//

#ifndef __G_Ear_Player__AppleKeychain__
#define __G_Ear_Player__AppleKeychain__

#include "IKeychain.h"

namespace Gear
{
    class AppleKeychain final : public IKeychain
    {
    public:
        virtual string username(const string &identifier) const;
        virtual string pass(const string &identifier) const;
        virtual void save(const string &identifier, const string &username, const string &data);
        virtual void forget(const string &identifier);
    };
}

#endif /* defined(__G_Ear_Player__AppleKeychain__) */
