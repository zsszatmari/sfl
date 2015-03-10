//
//  YouTubeSessionBase.cpp
//  G-Ear Player
//
//  Created by Ági Asztalos on 10/8/13.
//
//

#include <iostream>
#include <sstream>
#include "YouTubeSessionBase.h"
#include "PlaybackData.h"
#include "ISong.h"
#include "YouTubeProtocol.h"
#include "HttpDownloader.h"
#include "json.h"
#include "SongEntry.h"
#include "YouTubeSession.h"
#include "NamedImage.h"

namespace Gear
{
#define method YouTubeSessionBase::
    
    void method playbackData(const string &uniqueId, const function<void(const shared_ptr<IPlaybackData> &)> &callback)
    {
        callback(playbackDataSync(uniqueId));
    }

    shared_ptr<IPlaybackData> method playbackDataSync(const string &videoId)
    {
#ifdef DEBUG
#define LOG_YOUTUBE
#endif
        string videoUrl;
        {
            // https is not really needed here per se, but this way force apple's http implementation on ourselves, because there is some strangeness going on otherwise...
	        string url = "https://www.youtube.com/watch?v=" + videoId;
			auto downloader = HttpDownloader::create(url);
			downloader->waitUntilFinished();
			auto fail = downloader->failed();
            
#ifdef LOG_YOUTUBE
            std::cout << "download1 " << url << " " << fail << std::endl;
#endif
			if (fail) {
				return shared_ptr<PlaybackData>();
            }

            string videoInfo = *downloader;
	        videoUrl = YouTubeProtocol::videoUrlWeb(videoInfo);
        
#ifdef LOG_YOUTUBE
            std::cout << "download1 decoded " << url << " " <<videoInfo.length() << " " << videoUrl << std::endl;
#endif
        }
        if (videoUrl.empty()) {
        	// fallback on safer method (in case html changes)
        	// eurl is needed for some restricted videos
			string url = "https://www.youtube.com/get_video_info?video_id=" + videoId + "&eurl=http%3A%2F%2Fyoutube%2Ecom";
			auto downloader = HttpDownloader::create(url);
			downloader->waitUntilFinished();
			auto fail = downloader->failed();
#ifdef LOG_YOUTUBE
            std::cout << "download2 " << url << " " << fail << std::endl;
#endif
			if (fail) {
				return shared_ptr<PlaybackData>();
			}

            string videoInfo = *downloader;
			videoUrl = YouTubeProtocol::videoUrl(videoInfo);
#ifdef LOG_YOUTUBE
            std::cout << "download2 decoded " << url << " " <<videoInfo.length() << " " << videoUrl << std::endl;
#endif
        }
        
        if (videoUrl.empty()) {
            return shared_ptr<PlaybackData>();
        }

        videoUrl = HttpDownloader::upgradeToHttps(videoUrl);

        
        vector<string> urls;
        urls.push_back(videoUrl);
        
        shared_ptr<PlaybackData> ret(new PlaybackData(urls, PlaybackData::Format::Mp4));
        return ret;
    }
    
    static vector<string> artistSeparators()
    {
        vector<string> ret;
        ret.push_back(" - ");
        ret.push_back(" : ");
        ret.push_back(": ");
        ret.push_back("-");
        return ret;
    }
    
    void method callForSongs(const CallAsync &callAsync, const string &str, const function<void(const vector<Json::Value> &)> &result)
    {
        return callForSongs(callAsync, str, "", [=](const vector<Json::Value> &entries, const string &token){
            result(entries);
        });
    }
    
    void method callForSongs(const CallAsync &callAsync, const string &aStr, const string &oldToken, const function<void(const vector<Json::Value> &, const string &token)> &result)
    {
        string str;
        if (!oldToken.empty()) {
            std::stringstream ss;
            ss << aStr << "&pageToken=" << oldToken;
            str = ss.str();
        } else {
            str = aStr;
        }
        
        callAsync(str, [=](const string &got){

            Json::Reader reader;
            Json::Value json;
            bool success = reader.parse(got, json);
            if (!success) {
                vector<Json::Value> ret;
                result(ret, oldToken);
                return;
            }
            
            string token;
            vector<string> ids;
            {
                token = json.get("nextPageToken", "").asString();
                auto items = json.get("items", Json::arrayValue);
                ids.reserve(items.size());
                transform(items.begin(), items.end(), back_inserter(ids), [](const Json::Value &jsonItem){
                    
                    auto idJson = jsonItem.get("id", Json::objectValue);
                    if (idJson.isObject()) {
                        return idJson.get("videoId", "").asString();
                    } else {
                        return idJson.asString();
                    }
                });
            }
            {
                std::stringstream ss;
                ss << "https://www.googleapis.com/youtube/v3/videos?part=contentDetails,snippet&id=";
                bool first = true;
                for (auto it = ids.begin() ; it != ids.end() ; ++it) {
                    if (first) {
                        first = false;
                    } else {
                        ss << ",";
                    }
                    auto &currentId = *it;
                    ss << currentId;
                }
                callForSongsCommon(callAsync, "all", ss.str(), [=](const vector<Json::Value> &entries){
                    result(entries, token);
                }, 1);
            }
        });
    }
    
    static long long isoDurationToMillisec(const string &str)
    {
    	const char *s = str.c_str();
        long ret = 0;
        long current = 0;
        while (*s != '\0') {
            char c = *s;
            if (c >= '0' && c <= '9') {
                int digit = c - '0';
                current = (current * 10) + digit;
            } else {
                switch(c) {
                    case 'H':
                        ret += (current * 60 * 60);
                        break;
                    case 'M':
                        ret += (current * 60);
                        break;
                    case 'S':
                        ret += current;
                        break;
                }
                current = 0;
            }

            ++s;
        }
        return ret * 1000;
    }
    
    shared_ptr<YouTubeSession> method loggedInSession()
    {
        return YouTubeSession::activeSession();
    }
    
    shared_ptr<const YouTubeSession> method loggedInSession() const
    {
    	return (const_cast<YouTubeSessionBase *>(this))->loggedInSession();
    }

    static void callForSongsCommon(const YouTubeSessionBase::CallAsync &callAsync, const string &playlistId, const string &str, const function<void(const vector<Json::Value> &)> &result, int maximumCalls, int callNumber, const string &nextPageToken, vector<Json::Value> ret)
    {
        ++callNumber;
        std::stringstream ss;
        ss << str;
        if (!nextPageToken.empty()) {
            ss << "&pageToken=" << nextPageToken;
        }
        callAsync(ss.str(), [=](const string &callResult) mutable {
            Json::Reader reader;
            Json::Value json;

            bool success = reader.parse(callResult, json);
            string nextPageToken = "";

            auto next = [callAsync,playlistId,str,result,maximumCalls,callNumber](const string &nextPage, const vector<Json::Value> &resultPart, vector<Json::Value> ret){
                
                ret.reserve(ret.size() + resultPart.size());
                copy(resultPart.begin(), resultPart.end(), back_inserter(ret));

                if (!nextPage.empty() && (maximumCalls == 0 || callNumber < maximumCalls)) {
                    callForSongsCommon(callAsync, playlistId, str, result, maximumCalls, callNumber, nextPage, ret);
                } else {
                    result(ret);
                }
            };
            
            if (success) {
                nextPageToken = json.get("nextPageToken", "").asString();
                
                auto items = json.get("items", Json::arrayValue);
                //std::cout << "count of items " << items.size() << std::endl;
                
                // the offset int resultPart
                map<string,int> durationsToFetch;
                vector<Json::Value> resultPart;
                
                for (auto it = items.begin() ; it != items.end() ; ++it) {
                    
                    auto &item = *it;

                    auto snippetpart = item.get("snippet", Json::objectValue);
                    auto title = snippetpart.get("title","").asString();
                    if (title == "Deleted video") {
                        continue;
                    }
                    if (item.get("status",Json::objectValue).get("privacyStatus","").asString() == "private") {

                    	if (title == "Private video") {
                    		continue;
                    	} else {
//#ifdef _DEBUG
                    		// still no good because it's a blocked (forcefully private) video which we can't play
//                    		continue;
//#endif
                    	}
                    }
                    
                    string uniqueId(item.get("id","").asString());
                    string entryId = uniqueId;
                    auto resourceVideoId = snippetpart.get("resourceId",Json::objectValue).get("videoId","").asString();
                    if (!resourceVideoId.empty()) {
                        uniqueId = resourceVideoId;
                    }
                    if (entryId.empty()) {
                        entryId = uniqueId;
                    }
                    
                    Json::Value song;
                    song["playlist"] = playlistId;
                    song["id"] = uniqueId;
                    song["entryId"] = entryId;
                    
                    static auto separators = artistSeparators();
                    for (auto itSeparator = separators.begin() ; itSeparator != separators.end() ; ++itSeparator) {
                        auto &sep = *itSeparator;
                        auto found = title.find(sep);
                        if (found == string::npos) {
                            continue;
                        }
                        string artist = title.substr(0, found);
                        song["artist"] = artist;
                        title = title.substr(found + sep.length());
                        break;
                    }
                    song["title"] = title;
                    auto thumbnails = snippetpart.get("thumbnails", Json::objectValue);
                    song["albumArtUrl"] = thumbnails.get("default",Json::objectValue).get("url","").asString();
                    song["albumArtUrlHigh"] = thumbnails.get("high",Json::objectValue).get("url","").asString();
                    
                    
                    auto contentDetailsPart = item.get("contentDetails", Json::objectValue);
                    auto durationString = contentDetailsPart.get("duration","").asString();

                    if (!durationString.empty()) {
                        
                        song["durationMillis"] = isoDurationToMillisec(durationString);
                    } else {
                        durationsToFetch.insert(std::make_pair(uniqueId, resultPart.size()));
                    }
                    resultPart.push_back(song);
                }
                
                // get ratings for songs
#pragma message("TODO: get ratings for songs")
                /*auto activeSession = loggedInSession();
                if (activeSession) {
                    activeSession->fetchRatings(resultPart);
                }*/

#pragma message("TODO: get youtube song durations")
               
                if (!durationsToFetch.empty()) {
                    std::stringstream ss;
                    ss << "https://www.googleapis.com/youtube/v3/videos?part=contentDetails&id=";
                    bool first = true;
                    
                    for (auto it = durationsToFetch.begin(); it != durationsToFetch.end()  ; ++it) {
                        if (first) {
                            first = false;
                        } else {
                            ss << ",";
                        }
                        ss << it->first;
                    }
                    callAsync(ss.str(),[=](const string &result) mutable {
                        Json::Value json;
                        bool success = reader.parse(result, json);
                        if (success) {
                            auto items = json.get("items", Json::arrayValue);
                            for (auto it = items.begin() ; it != items.end() ; ++it) {
                                auto &item = *it;
                                auto videoId = item.get("id","").asString();
                                long long duration = isoDurationToMillisec(item.get("contentDetails",Json::objectValue).get("duration","").asString());
                                auto song = durationsToFetch.find(videoId);
                                if (song != durationsToFetch.end()) {
                                    Json::Value &s = resultPart[song->second];
                                    s["durationMillis"] = duration;
                                }
                            }
                        }
                        next(nextPageToken, resultPart, ret);
                    });
                    
                } else {
                    next(nextPageToken, resultPart, ret);
                }
                return;
            }

            next(nextPageToken, vector<Json::Value>(), ret);
        });
    }

    void method callForSongsCommon(const CallAsync &callAsync, const string &playlistId, const string &str, const function<void(const vector<Json::Value> &)> &result, int maximumCalls)
    {
        ::Gear::callForSongsCommon(callAsync, playlistId, str, result, maximumCalls, 0, "", vector<Json::Value>());
    }
    
    std::string method sessionIdentifier() const
    {
        return "youtube";
    }
}
