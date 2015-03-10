//
//  HttpDownloader.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/15/13.
//
//

#ifndef __G_Ear_Player__HttpDownloader__
#define __G_Ear_Player__HttpDownloader__

#include "stdplus.h"
#include "IDownloader.h"

namespace Gear
{
    class HttpDownloader
    {
    public:
        static shared_ptr<IDownloader> create(const string &url);
        static shared_ptr<IDownloader> create(const string &url, const map<string,string> &extraHeaders, const string &method = "GET", const string &body = "");
        static string curlDump(const string &url, const map<string,string> &extraHeaders, const string &method = "GET", const string &body = "");
        static bool isOnline();
        static std::string upgradeToHttps(const std::string &original);
        
    private:
        HttpDownloader();   // delete
        ~HttpDownloader();  // delete
    };
}

#endif /* defined(__G_Ear_Player__HttpDownloader__) */
