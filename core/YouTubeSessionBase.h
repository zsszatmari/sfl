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
    
    class YouTubeSessionBase : public SongManipulationSession, public MEMORY_NS::enable_shared_from_this<YouTubeSessionBase>
    {
    public:
        YouTubeSessionBase(const shared_ptr<IApp> &app);
        
        virtual shared_ptr<IPlaybackData> playbackDataSync(const ISong &song) const;
        virtual shared_ptr<Gui::IPaintable> sessionIcon(const shared_ptr<ISong> &song) const;
        virtual shared_ptr<Gui::IPaintable> sessionIcon(const shared_ptr<IPlaylist> &song) const;
        virtual std::string sessionIdentifier() const;
        
    protected:
        virtual string callSync(const string &str) = 0;
        virtual shared_ptr<YouTubeSession> loggedInSession();
        shared_ptr<const YouTubeSession> loggedInSession() const;
        const vector<SongEntry> callForSongs(const string &str);
        const vector<SongEntry> callForSongs(const string &str, string &token);
        const vector<SongEntry> callForSongsCommon(const string &playlistId, const string &str, int maximumCalls = 10);
        
        vector<SongEntry> songsFromReply(const string &str);
    };
}

#endif /* defined(__G_Ear_Player__YouTubeSessionBase__) */
