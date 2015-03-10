//
//  JsonStreamer.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 26/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <cassert>
#include "JsonStreamer.h"
#include "IDownloader.h"
#include "json.h"
#include "GoogleMusicConnection.h"

namespace Gear
{
    using std::string;
    
#define method JsonStreamer::
    
#ifdef DEBUG
    static const bool debugWriteFile = false;
#else
    static const bool debugWriteFile = false;
#endif

    static std::string debugWriteFileName()
    {
        return std::string(getenv("HOME")) + "/Desktop/googlemusiclibrarydata.dat";
    }


    method JsonStreamer() 
        //: _scannedEnd(0)
    {
        if (debugWriteFile) {
            std::ofstream s(debugWriteFileName().c_str(), std::ios_base::trunc);
        }
    }
    
    const int JsonStreamer::ChunkSize = 4096;
    static const string kScriptEnd("</script>");

    static std::pair<std::string,std::string> extract(const string &data, int scannedEnd)
    {
        const string kScriptBegin("<script");

        auto beginpos = data.find(kScriptBegin);
        if (beginpos == std::string::npos) {
            return std::make_pair("",data);
        }

        auto endpos = data.find(kScriptEnd, std::max<int>(scannedEnd,beginpos));
        if (endpos == std::string::npos) {
            return std::make_pair("",data);
        }

        return std::make_pair(data.substr(beginpos,endpos-beginpos), data.substr(endpos));

        // returns a pair of (extracted, remaining)
    }

    static void processContent(const string &chunk, const function<void(const Json::Value &)> &gotChunk)
    {
        /*
         ends like this:
         ,["6d8445d2-eb5a-3642-9bdf-871ad9a7300b","Minus Celsius","//lh3.googleusercontent.com/n4Xu8XhakJkke_zAjA_-DZ6e5wA37IW-k_UeOwN-BbOkHJBrgeZFAHGe8y1F","Backyard Babies","Guitar Hero 3","Guitar Hero 3",,,,,"","Soundtrack",,216000,48,74,1,1,2003,0,,,0,0,1330447499543417,1393174804009259,,,,2,"",,,"Ak5fjc24maqsxby7udi6skmyhji",306,1330789477780000,"//lh4.googleusercontent.com/AURgqafEj_bfX_g1TrE3adaotqDficFodqAAWjNmyQAXEPFqGQzTpwSTTiezWM4E_WRZ8w6TGA",,,[]
         ]
         ]
         ,1393447523682000]
         );
         window.parent['slat_progress'](1.0);
         </script>
         */
        const string kBegin("window.parent['slat_process'](");
        const string kEnd("]\n);");
        
        size_t beginpos = chunk.find(kBegin);
        if (beginpos == string::npos) {
            return;
        }
        beginpos += kBegin.length();
        // this does not seem to work
        //size_t endpos = currentHtml.rfind(kEnd, beginpos);
        size_t endpos = chunk.rfind(kEnd);
        //size_t endpos = string::npos;
        
        if (endpos == string::npos || beginpos > endpos) {
            return;
        }
        
        endpos += 1;
        
        string content = chunk.substr(beginpos, endpos-beginpos);
        
        //std::cout << "fetched " << content.size() << " bytes for " << duration_cast<milliseconds>(steady_clock::now() - now).count() << " millisecs" << std::endl;

        //now = steady_clock::now();
        GoogleMusicConnection::fixJson(content);
        //std::cout << "fixed " << content.size() << " bytes for " << duration_cast<milliseconds>(steady_clock::now() - now).count() << " millisecs" << std::endl;
        Json::Reader reader;
        //std::cout << "start parsing " << content.size() << " bytes" << std::endl;
        //now = steady_clock::now();
        Json::Value j;
        reader.parse(content, j);
        gotChunk(j);
        
        //std::cout << "parsed " << content.size() << " bytes for " << duration_cast<milliseconds>(steady_clock::now() - now).count() << " millisecs" << std::endl;
    }

    void method gotData(const string &data, const function<void(const Json::Value &)> &gotChunk)
    {
        /*
         looks like this:
         
         <html><head></head><body>
         <script>window.parent['slat_progress'](0.02);</script>
         <script type='text/javascript'>
         window.parent['slat_process']([[["886fbd7f-e740-30b8-a594-ec9964fe1fa6","Soulhunter",,"yelworC","Blood In Face","",,,,,"","Electronic",,312000,2,0,0,0,1993,0,,,1,0,1330445063912506,1330445780811024,,,,2,"",,,"Autdebof5pvjunvhnxtvldfdefq",192,1330445780800000,"//lh6.googleusercontent.com/htYFM_ckLtngBR7neGBqx_Hq6F5NjSOtDKivGtZ5Z0G0iYFbdtI0H-kj08Ksue9Jq8BNhMMi",,,[]
         ]
         ,
         
         */
        //auto now = steady_clock::now();

        if (data.empty()) {
            gotChunk(Json::Value());
            return;
        }

        if (debugWriteFile) {
            //std::cout << "got library chunk " << data.size() << " bytes\n";
            std::ofstream s(debugWriteFileName().c_str(), std::ios_base::app);
            s << data;
        }
        _bufferHtml.append(data);
        while (true) {
            auto p = extract(_bufferHtml, 0/*_scannedEnd*/);
            auto &found = p.first;
            auto &remaining = p.second;

            if (found.empty()) {
                /*_scannedEnd = _bufferHtml.size() - kScriptEnd.size();
                if (_scannedEnd < 0) {
                    _scannedEnd = 0;
                }*/
                break;
            } else {
                processContent(found, gotChunk);
                _bufferHtml = remaining;
                //_scannedEnd = 0;
            }
        }
   }
}


