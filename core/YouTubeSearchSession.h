//
//  YouTubeSearchSession.h
//  G-Ear Player
//
//  Created by Ági Asztalos on 10/8/13.
//
//

#ifndef __G_Ear_Player__YouTubeSearchSession__
#define __G_Ear_Player__YouTubeSearchSession__

#include "YouTubeSessionBase.h"
#include "stdplus.h"

namespace Gear
{
    class YouTubeSearchSession final : public YouTubeSessionBase
    {
    public:
        static shared_ptr<YouTubeSearchSession> create(const shared_ptr<IApp> &app);
        
        virtual const vector<SongEntry> searchSync(const string &filter, string &token);
        
        virtual bool manipulationEnabled() const;
        virtual vector<shared_ptr<IPlaylist>> playlists();
        virtual void refresh();
        virtual void addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs);
        virtual ValidPtr<const vector<PlaylistCategory>> categories() const;
        virtual shared_ptr<IPlaylist> libraryPlaylist();
        
        virtual void addSongsToPlaylistAsync(const string &playlistId, const vector<shared_ptr<ISong>> &songs, const function<void(const vector<SongEntry> &)> &result);
        virtual void removePlaylist(const shared_ptr<IPlaylist> &playlist);
        virtual void deletePlaylistEntriesSync(const vector<string> &entryIds, const vector<string> &songIds, const string &playlistId);
        virtual void changePlaylistOrderSync(const string &playlistId, const vector<SongEntry> &songs, const SongEntry &afterId, const SongEntry &before);
        virtual void changePlaylistNameSync(const string &newName, const string &playlistId);
        virtual vector<int> possibleRatings() const;
        
    private:
        virtual void createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const string &name, const function<void(const shared_ptr<IPlaylist> &)> &result);
        virtual void modifySongs(const vector<string> &ids, const std::map<string, string> &changedStrings, const std::map<string, uint64_t> &changedInts);
        
        YouTubeSearchSession(const shared_ptr<IApp> &app);
        string callSync(const string &str);
        string lastSearchFilter;
        int lastSearchRepeat;
        int searchRelevance;
        
        AtomicPtr<vector<PlaylistCategory>> _categories;
    };
}

#endif /* defined(__G_Ear_Player__YouTubeSearchSession__) */
