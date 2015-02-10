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
    
    method YouTubeSessionBase(const shared_ptr<IApp> &app) :
        SongManipulationSession(app)
    {
    }
    
    shared_ptr<IPlaybackData> method playbackDataSync(const ISong &song) const
    {
        auto videoId = song.uniqueId();
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
    
    const vector<SongEntry> method callForSongs(const string &str)
    {
        string token;
        return callForSongs(str, token);
    }
    
    const vector<SongEntry> method callForSongs(const string &aStr, string &token)
    {
        string str;
        if (!token.empty()) {
            std::stringstream ss;
            ss << aStr << "&pageToken=" << token;
            str = ss.str();
        } else {
            str = aStr;
        }
        
        string result;
        {
            result = callSync(str);
        }
        Json::Reader reader;
        Json::Value json;
        bool success = reader.parse(result, json);
        if (!success) {
            vector<SongEntry> ret;
            return ret;
        }
        
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
            return callForSongsCommon("all", ss.str(), 1);
        }
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

    const vector<SongEntry> method callForSongsCommon(const string &playlistId, const string &str, int maximumCalls)
    {
        vector<SongEntry> ret;
        
        int callNumber = 0;
        string nextPageToken;

        do {
            ++callNumber;
            std::stringstream ss;
            ss << str;
            if (!nextPageToken.empty()) {
                ss << "&pageToken=" << nextPageToken;
            }
            string result = callSync(ss.str());
            Json::Reader reader;
            Json::Value json;

            bool success = reader.parse(result, json);
            nextPageToken = "";
            
            if (success) {
                nextPageToken = json.get("nextPageToken", "").asString();
                
                auto items = json.get("items", Json::arrayValue);
                //std::cout << "count of items " << items.size() << std::endl;
                
                map<string,shared_ptr<ISong>> durationsToFetch;
                vector<SongEntry> resultPart;
                
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
                    
                    SongEntry songEntry(sessionIdentifier(), playlistId, entryId, uniqueId);
                    auto song = songEntry.song();
                    
                    static auto separators = artistSeparators();
                    for (auto itSeparator = separators.begin() ; itSeparator != separators.end() ; ++itSeparator) {
                        auto &sep = *itSeparator;
                        auto found = title.find(sep);
                        if (found == string::npos) {
                            continue;
                        }
                        string artist = title.substr(0, found);
                        song->setStringForKey("artist", artist);
                        title = title.substr(found + sep.length());
                        break;
                    }
                    song->setStringForKey("title", title);
                    auto thumbnails = snippetpart.get("thumbnails", Json::objectValue);
                    song->setStringForKey("albumArtUrl", thumbnails.get("default",Json::objectValue).get("url","").asString());
                    song->setStringForKey("albumArtUrlHigh", thumbnails.get("high",Json::objectValue).get("url","").asString());
                    
                    
                    auto contentDetailsPart = item.get("contentDetails", Json::objectValue);
                    auto durationString = contentDetailsPart.get("duration","").asString();

                    if (!durationString.empty()) {
                        
                        song->setUintForKey("durationMillis", isoDurationToMillisec(durationString));
                    } else {
                        durationsToFetch.insert(std::make_pair(uniqueId, song));
                    }
                    resultPart.push_back(songEntry);
                }
                
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
                    auto result = callSync(ss.str());
                    json.clear();
                    
                    bool success = reader.parse(result, json);
                    if (success) {
                        auto items = json.get("items", Json::arrayValue);
                        for (auto it = items.begin() ; it != items.end() ; ++it) {
                            auto &item = *it;
                            auto videoId = item.get("id","").asString();
                            long long duration = isoDurationToMillisec(item.get("contentDetails",Json::objectValue).get("duration","").asString());
                            auto song = durationsToFetch.find(videoId);
                            if (song != durationsToFetch.end()) {
                                song->second->setUintForKey("durationMillis", duration);
                            }
                        }
                    }
                }
                
                auto activeSession = loggedInSession();
                if (activeSession) {
                    activeSession->fetchRatings(resultPart);
                }
                
                result.reserve(result.size() + resultPart.size());
                
                // the buffering problem is solved with GenericHttpDownloader
                /*std::remove_copy_if(resultPart.begin(), resultPart.end(), back_inserter(ret), [](const SongEntry &entry){
                    auto duration = entry.song()->uIntForKey("durationMillis");
                    // skip too long songs because of a buffering problem
                	return !(duration > 0) || !(duration < 20 * 60 * 1000);
                });*/
                copy(resultPart.begin(), resultPart.end(), back_inserter(ret));
            }

        } while (!nextPageToken.empty() && (maximumCalls == 0 || callNumber < maximumCalls));
        
        
        return ret;
    }

    shared_ptr<Gui::IPaintable> method sessionIcon(const shared_ptr<ISong> &song) const
	{
    	return shared_ptr<Gui::IPaintable>(new Gui::NamedImage("favicon-youtube"));
	}

    shared_ptr<Gui::IPaintable> method sessionIcon(const shared_ptr<IPlaylist> &song) const
	{
    	return shared_ptr<Gui::IPaintable>(new Gui::NamedImage("favicon-youtube"));
	}
    
    std::string method sessionIdentifier() const
    {
        return "youtube";
    }
}
