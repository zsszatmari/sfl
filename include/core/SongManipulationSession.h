//
//  SongManipulationSession.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/20/13.
//
//

#ifndef __G_Ear_Player__SongManipulationSession__
#define __G_Ear_Player__SongManipulationSession__

#include "PlaylistSession.h"
#include "SerialExecutor.h"

namespace Gear
{
    using std::map;
    
    class SongManipulationSession : public PlaylistSession
    {
    public:
        SongManipulationSession(const shared_ptr<IApp> &app);
        
        virtual void modifySongs(const vector<string> &ids, const map<string, string> &changedStrings, const map<string, uint64_t> &changedInts) = 0;
        // TODO: this is not really async yet in GooglePlaySession's implementation!
        virtual void addSongsToPlaylistAsync(const string &playlistId, const vector<shared_ptr<ISong>> &songs, const function<void(const vector<SongEntry> &)> &result) = 0;
        virtual void removePlaylist(const shared_ptr<IPlaylist> &playlist) = 0;
        virtual void deletePlaylistEntriesSync(const vector<string> &entryIds, const vector<string> &songIds, const string &playlistId) = 0;
        virtual void changePlaylistOrderSync(const string &playlistId, const vector<SongEntry> &songs, const SongEntry &afterId, const SongEntry &before) = 0;
        virtual void changePlaylistNameSync(const string &newName, const string &playlistId) = 0;
        IExecutor &executor();
        
    protected:
        Base::SerialExecutor _executor;
    };
}

#endif /* defined(__G_Ear_Player__SongManipulationSession__) */
