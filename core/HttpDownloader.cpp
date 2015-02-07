//
//  HttpDownloader.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/15/13.
//
//

#include <sstream>
#include "HttpDownloader.h"
#include "Environment.h"
#include "CoreDebug.h"

#ifdef TARGET_OS_IPHONE
#include "AppleHttpDownloader.h"
#endif
#include "GenericHttpDownloader.h"


namespace Gear
{
#define method HttpDownloader::
    
#ifdef TARGET_OS_IPHONE
    static bool chooseNative(const string &url)
    {
#if TARGET_OS_IPHONE
        // GenericHttpDownloader does not verify ssl certs. also, it might have problems fetching great amount of metadata (but pauses to not eat all memory)
        string prefix("https");
        return url.compare(0, prefix.length(), prefix) == 0;
#else
        // GenericHttpDownloader does not know proxies

        return true;
#endif
    }
#endif
    
    shared_ptr<IDownloader> method create(const string &url)
    {
        map<string,string> noHeaders;
#ifdef TARGET_OS_IPHONE
        return chooseNative(url) ? shared_ptr<IDownloader>(AppleHttpDownloader::create(url, noHeaders)) : GenericHttpDownloader::create(url, noHeaders);
#else
		return GenericHttpDownloader::create(url, noHeaders);
#endif
	}
    
    shared_ptr<IDownloader> method create(const string &url, const map<string,string> &extraHeaders, const string &m, const string &body)
    {
#ifdef TARGET_OS_IPHONE
        return chooseNative(url) ? shared_ptr<IDownloader>(AppleHttpDownloader::create(url, extraHeaders, m, body)) : GenericHttpDownloader::create(url, extraHeaders, m, body);
#else
		return GenericHttpDownloader::create(url, extraHeaders, m, body);
#endif
    }
    
    string method curlDump(const string &url, const map<string,string> &extraHeaders, const string &m, const string &body)
    {
        std::ostringstream ss;
        ss << "curl -i ";
        for (auto it=extraHeaders.begin(); it != extraHeaders.end() ; ++it) {
            ss << "-H \"" << it->first << ": " << it->second << "\" ";
        }
        ss << "-X " << m << " ";
        ss << "-d \"" << body << "\" ";
        ss << "\"" << url << "\" ";
        return ss.str();
    }
    
    bool method isOnline()
    {
#ifdef TARGET_OS_IPHONE
        return AppleHttpDownloader::isOnline();
#else
		return true;
#endif
    }

    std::string method upgradeToHttps(const std::string &original)
    {
        const std::string kLastFm = "last.fm";
        if (original.find(kLastFm) != std::string::npos) {
            // last.fm 's image servers don't have a cert
            return original;
        }

        const std::string kHttp = "http://";
        const std::string kHttps = "https://";

        if (original.compare(0,kHttp.length(),kHttp) == 0) {
            std::string ret = original;
            ret.replace(0, kHttp.length(), kHttps);
            return ret;
        }
        return original;
    }
}
