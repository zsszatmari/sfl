//
//  YouTubeSessionBase.h
//  G-Ear Player
//
//  Created by Ági Asztalos on 10/8/13.
//
//

#ifndef __G_Ear_Player__YouTubeSessionBase__
#define __G_Ear_Player__YouTubeSessionBase__

#include "SongManipulationSession.h"
#include SHAREDFROMTHIS_H

namespace Gear
{
    class YouTubeSession;
    
    class YouTubeSessionBase final
    {
    public:
        typedef function<void(const string &request, const function<void(const string &)> &response)> CallAsync;
        
        static shared_ptr<IPlaybackData> playbackDataSync(const string &song);
        static void playbackData(const string &uniqueId, const function<void(const shared_ptr<IPlaybackData> &)> &callback);
        std::string sessionIdentifier() const;
        
        shared_ptr<YouTubeSession> loggedInSession();
        shared_ptr<const YouTubeSession> loggedInSession() const;
        static void callForSongs(const CallAsync &callAsync, const string &str, const function<void(const vector<Json::Value> &)> &result);
        static void callForSongs(const CallAsync &callAsync, const string &str, const string &oldToken, const function<void(const vector<Json::Value> &, const string &token)> &result);
        static void callForSongsCommon(const CallAsync &callAsync, const string &playlistId, const string &str, const function<void(const vector<Json::Value> &)> &result, int maximumCalls = 10);
        
        vector<SongEntry> songsFromReply(const string &str);
    };
}

#endif /* defined(__G_Ear_Player__YouTubeSessionBase__) */
