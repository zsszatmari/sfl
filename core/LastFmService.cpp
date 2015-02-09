//
//  LastFmService.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/21/13.
//
//

#include "json.h"
#include "LastFmService.h"
#include "LastFmProtocol.h"
#include "IExecutor.h"
#include "Environment.h"
#include "LastFmController.h"
#include "IApp.h"

namespace Gear
{
#define method LastFmService::
    
    using std::function;
    
    static std::pair<string,ImageToDeliver::Rating> doGetAlbumArtUrlForArtist(const string &artist, const string &album, const string &title, int preferredSize, const function<bool()> &stillNeeded)
    {
        string ret;
        ImageToDeliver::Rating rating = ImageToDeliver::Rating::Nothing;
        LastFmProtocol::executor()->addTaskAndWait([&]{
            if (!stillNeeded()) {
                return;
            }
            
            Json::Value images = "";
            
            if (artist.empty()) {
                return;
            }
            if (!album.empty()) {
                map<string, string> params;
                params.insert(std::make_pair("artist", artist));
                params.insert(std::make_pair("album", album));
                params.insert(std::make_pair("autocorrect", "1"));
                Json::Value dictionary = LastFmProtocol::getAlbumInfo(params);
                if (!dictionary.empty()) {
                    images = dictionary.get("album", Json::objectValue).get("image","");
                    rating = ImageToDeliver::Rating::ProperLastFm;
                }
            } else {
                if (!title.empty()) {
                    map<string, string> params;
                    params.insert(std::make_pair("artist", artist));
                    params.insert(std::make_pair("track", title));
                    params.insert(std::make_pair("autocorrect", "1"));
                    Json::Value dictionary = LastFmProtocol::getTrackInfo(params);
                    if (!dictionary.empty()) {
                        images = dictionary.get("track", Json::objectValue).get("album", Json::objectValue).get("image","");
                        rating = ImageToDeliver::Rating::ProperLastFm;
                    }
                }
                
                if (!images.isArray()) {
                    map<string, string> params;
                    params.insert(std::make_pair("artist", artist));
                    params.insert(std::make_pair("autocorrect", "1"));
                    Json::Value dictionary = LastFmProtocol::getArtistInfo(params);
                    if (!dictionary.empty()) {
                        
                        //Json::FastWriter writer;
                        //std::cout << "got lastfm album data: " << writer.write(dictionary) << std::endl;
                        
                        images = dictionary.get("artist", Json::objectValue).get("image","");

                        // this causes more trouble than good on youtube, see maroon 5 postmodern jukebox maps
                        // so it's better to only use this when there is nothing else
                        rating = ImageToDeliver::Rating::BasedOnArtistOnly;
                    }
                }
            }
            
            if (!images.isArray()) {
                return;
            }
            
            map<int,string> sizesToUrls;
            for (auto it = images.begin() ; it != images.end() ; ++it) {
                auto &image = *it;
                
                auto value = image.get("#text", "").asString();
                auto sizeTag = image.get("size", "").asString();
                if (value.empty() || sizeTag.empty()) {
                    continue;
                }
                int size = 0;
                if (sizeTag == "small") {
                    size = 34;
                } else if (sizeTag == "medium") {
                    size = 64;
                } else if (sizeTag == "large") {
                    size = 174;
                } else if (sizeTag == "xlarge" || sizeTag == "extralarge") {
                    size = 300;
                } else if (sizeTag == "mega") {
                    size = 1500;
                }
                if (size > 0) {
                    sizesToUrls.insert(make_pair(size, value));
                }
            }
            
            // sorted by key, cool
            for (auto it = sizesToUrls.rbegin() ; it != sizesToUrls.rend() ; ++it) {
                if (!ret.empty() && it->first < preferredSize) {
                    break;
                }
                ret = it->second;
            }
        });
        return std::make_pair(ret,rating);
    }

    
    std::pair<string,ImageToDeliver::Rating> method getAlbumArtUrlForArtist(const string &artist, const string &album, const string &title, int dimension, weak_ptr<void> stillNeeded)
    {
        auto lastFmUrl = doGetAlbumArtUrlForArtist(artist, album, title, dimension, [stillNeeded]{
                               
               auto image = stillNeeded.lock();
               if (image) {
                   return true;
               } else {
                   return false;
               }
        });
        return lastFmUrl;
    }
    
    void method playTimePassed(float duration)
    {
        IApp::instance()->lastFmController()->playTimePassed(duration);
    }

    void method songBegan(const shared_ptr<ISong> &song)
    {
        IApp::instance()->lastFmController()->songBegan(song);
    }
}
