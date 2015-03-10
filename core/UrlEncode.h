//
//  UrlEncode.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/5/13.
//
//

#ifndef __G_Ear_Player__UrlEncode__
#define __G_Ear_Player__UrlEncode__

#include <string>

namespace Gear
{
    using std::string;
    
    class UrlEncode
    {
    public:
        static string encode(const string &str);
        static string decode(const string &str);
    };
}

#endif /* defined(__G_Ear_Player__UrlEncode__) */
