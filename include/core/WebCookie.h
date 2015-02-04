//
//  WebCookie.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/4/13.
//
//

#ifndef __G_Ear_Player__WebCookie__
#define __G_Ear_Player__WebCookie__

#include <map>
#include <vector>
#include <string>
#include "Environment.h"

namespace Gear
{
    using std::string;
    using std::vector;
    
    class core_export WebCookie final
    {
    public:
        WebCookie();
        WebCookie(const std::map<string, string> &properties);
		static vector<WebCookie> cookies(const string &str, const string &domain, const string &separator = ",");
        const std::map<string, string> &properties() const;
        const string name() const;
        const string value() const;
        const string domain() const;
        explicit operator bool() const;
		static string cookieHeaderValue(const std::vector<WebCookie> &cookies, const string &domain);
    private:
        const string stringForKey(const string &key) const;
        
        std::map<string, string> _properties;
    };
}

#endif /* defined(__G_Ear_Player__WebCookie__) */
