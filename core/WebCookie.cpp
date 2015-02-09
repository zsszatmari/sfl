//
//  WebCookie.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/4/13.
//
//

#include <sstream>
#include "WebCookie.h"
//#include "Logger.h"

namespace Gear
{    
#define method WebCookie::

	using std::map;
    
    method WebCookie()
    {
    }
    
    method WebCookie(const map<string, string> &properties) :
        _properties(properties)
    {
    }
    
    vector<WebCookie> method cookies(const string &str, const string &defaultDomain, const string &separator)
	{
		//Base::Logger::stream() << "interpreting cookies: " << str;
        
		// example: SID=DQAAANIAAAChueQU-uUTcj6r-DCXawEJdTr7JzfTH4tJsSxMOuo27CE1Rel1KJvEhka7OPJQkfjL4PizWqmrmSgn-apIo6YXLUQrrABIw_LFSAXdLP6m94PP4wJD-RrZNxsIp8FVOq2SEoGazloN3ikGjKncb7pgPW48nfh3yzVf2IMQN1mnmGC0veO6JZxTXMjdHpIhmFDeOgDFwPaGj8Sdbz5W-ZM_U8pU5z8brSO0esSsSLY8tXCKvNsW_GaL-b6c5Qsa5tyL_MDJ7n417lCcUMeJByW0F84V_aSLqtwQQV_UzKlbcg;Domain=.google.com;Path=/;Expires=Sun, 06-Sep-2015 10:20:42 GMT, xt=CjUKATASMEFNLVdiWGhwRmFYOThYRFpLTTFpY0I1N2lPc1llR0xvUlE6MTM3ODQ2Mjg0MjI0NA==; Secure
        
		vector<WebCookie> ret;
		map<string, string> current;
		size_t pos = 0;
		while(true) {
			if (pos >= str.length()) {
				break;
			}
            
			string key;
			string value;
            
			size_t nextPosEquals = str.find("=", pos);
			size_t nextPosSemi;
			size_t nextPosComma = str.find(separator, pos);
			if (separator == ";") {
				nextPosSemi = string::npos;
			} else {
				nextPosSemi = str.find(";", pos);
			}
			
			size_t nextPosGmt = str.find("GMT,", pos);
			if (nextPosGmt != string::npos) {
				nextPosGmt += 3;
			}
			bool startNew = false;
            
			auto nextPosMin = nextPosEquals < nextPosSemi ? nextPosEquals : nextPosSemi;
			nextPosMin = nextPosMin < nextPosComma ? nextPosMin : nextPosComma;
			if (nextPosMin == string::npos) {
				key = str.substr(pos);
				pos = str.length();
			} else {
				key = str.substr(pos, nextPosMin - pos);
				pos = nextPosMin + 1;
			}
			if (nextPosMin == nextPosSemi || nextPosMin == nextPosComma) {
				value = "True";
			}
			if (nextPosMin == nextPosComma) {
				startNew = true;
			}
            
			if (value.empty()) {
				if (key == "Expires") {
					nextPosComma = nextPosGmt;
				}
                
				auto nextPosMin = nextPosSemi < nextPosComma ? nextPosSemi : nextPosComma;
				if (nextPosMin == string::npos) {
					value = str.substr(pos);
					pos = str.length();
				} else {
					value = str.substr(pos, nextPosMin - pos);
					pos = nextPosMin + 1;
				}
                
				// this is also true on end!
				if (nextPosMin == nextPosComma) {
					startNew = true;
				}
			}
            
			if (current.empty()) {
				std::stringstream trimmer;
				trimmer << key;
				trimmer >> key;
				current.insert(make_pair("Name", key));
				current.insert(make_pair("Value", value));
			} else {
				current.insert(make_pair(key, value));
			}
            
			if (startNew) {
				if (!defaultDomain.empty()) {
					auto it = current.find("Domain");
					if (it == current.end()) {
						current.insert(make_pair("Domain", defaultDomain));
					}
				}
				ret.push_back(WebCookie(current));
				current.clear();
			}
		}
		return ret;
	}
    
    const map<string, string> & method properties() const
    {
        return _properties;
    }
    
    const string method value() const
    {
        return stringForKey("Value");
    }
    
    const string method name() const
    {
        return stringForKey("Name");
    }
    
    const string method domain() const
    {
        return stringForKey("Domain");
    }
    
    const string method stringForKey(const string &key) const
    {
        auto found = _properties.find(key);
        if (found == _properties.end()) {
            return string();
        } else {
            return found->second;
        }
    }
    
    method operator bool() const
    {
        return !name().empty();
    }
    
    static std::vector<WebCookie> filterCookiesByDomain(const std::vector<WebCookie> &cookies, const string &aDomain)
    {
#ifdef TIZEN
    	return cookies;
#endif
        std::vector<WebCookie> filtered;
        for (auto it = cookies.begin() ; it != cookies.end() ; ++it) {
        	auto &cookie = *it;
        	auto cDomain = cookie.domain();
            if (cDomain.length() >= aDomain.length() && cDomain.compare(cDomain.length() - aDomain.length(), aDomain.length(), aDomain) == 0) {
                
                filtered.push_back(cookie);
            }
        }
        
        return filtered;
    }
    
    string method cookieHeaderValue(const std::vector<WebCookie> &cookies, const string &domain)
    {
        std::stringstream value;
        auto filtered = filterCookiesByDomain(cookies, domain);
        for (auto it = filtered.begin() ; it != filtered.end() ; ++it) {
            value << it->name() << "=" << it->value() << "; ";
        }
        return value.str();
    }
}
