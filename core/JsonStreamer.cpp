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
    
    static const bool debugWriteFile = false;

    static std::string debugWriteFileName()
    {
        return std::string(getenv("HOME")) + "/Desktop/googlemusiclibrarydata.dat";
    }


    method JsonStreamer(const shared_ptr<IDownloader> &downloader) :
        _downloader(downloader),
        _position(0)
    {
        if (debugWriteFile) {
            std::ofstream s(debugWriteFileName().c_str(), std::ios_base::trunc);
        }
    }
    
    bool method nextChunk(Json::Value &chunk)
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
        
        static const int kChunkSize = 4096;
        const string kScriptBegin("<script");
        const string kScriptEnd("</script>");
        
        chunk.clear();
        size_t checked = 0; // this is relative to _bufferHtml's begin
        size_t startPosition = _position;
        
        string currentHtml;
        for (;;) {
            size_t endpos = _bufferHtml.find(kScriptEnd, checked);
            if (endpos != string::npos) {
                size_t beginpos = _bufferHtml.find(kScriptBegin);
                if (beginpos == string::npos) {
                    beginpos = 0;
                }
                assert(beginpos < endpos);
                if (beginpos > endpos) {
                    beginpos = endpos;
                }
                endpos += kScriptEnd.length();
                currentHtml = _bufferHtml.substr(beginpos, endpos-beginpos);
                _bufferHtml = _bufferHtml.substr(endpos);
                break;
            } else {
                long c = _position - startPosition - kScriptEnd.length();
                if (c < 0) {
                    c = 0;
                }
                checked = c;
            }
            
            
            bool end = false;
            _downloader->waitSync(_position + kChunkSize);
            _downloader->accessChunk(_position, [&](const char *ptr, int available){
                
                if (available == 0) {
                    end = true;
                }
                _bufferHtml.append(ptr, available);
                _position += available;

                if (debugWriteFile) {
                    std::ofstream s(debugWriteFileName().c_str(), std::ios_base::app);
                    s << std::string(ptr, available);
                }
            });
            if (end) {
                return false;
            }
        }
        
        //std::cout << currentHtml << std::endl;
        
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
        
        size_t beginpos = currentHtml.find(kBegin);
        if (beginpos == string::npos) {
            return true;
        }
        beginpos += kBegin.length();
        // this does not seem to work
        //size_t endpos = currentHtml.rfind(kEnd, beginpos);
        size_t endpos = currentHtml.rfind(kEnd);
        //size_t endpos = string::npos;
        
        if (endpos == string::npos || beginpos > endpos) {
            return true;
        }
        
        endpos += 1;
        
        string content = currentHtml.substr(beginpos, endpos-beginpos);
        
        //std::cout << "fetched " << content.size() << " bytes for " << duration_cast<milliseconds>(steady_clock::now() - now).count() << " millisecs" << std::endl;

        //now = steady_clock::now();
        GoogleMusicConnection::fixJson(content);
        //std::cout << "fixed " << content.size() << " bytes for " << duration_cast<milliseconds>(steady_clock::now() - now).count() << " millisecs" << std::endl;
        Json::Reader reader;
        //std::cout << "start parsing " << content.size() << " bytes" << std::endl;
        //now = steady_clock::now();
        reader.parse(content, chunk);
        
        //std::cout << "parsed " << content.size() << " bytes for " << duration_cast<milliseconds>(steady_clock::now() - now).count() << " millisecs" << std::endl;
        
        return true;
    }
}


