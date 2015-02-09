//
//  ScriptSession.m
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 02/04/14.
//
//

#include <algorithm>
#import "ScriptSession.h"
#include "SongEntry.h"
#include "PlaylistSession.h"
#include "StringUtility.h"
#include "IPlaylist.h"

using namespace Gear;

@implementation ScriptSession {
    shared_ptr<PlaylistSession> _session;
}

- (id)initWithSession:(const shared_ptr<PlaylistSession> &)session
{
    self = [super init];
    if (self) {
        _session = session;
    }
    return self;
}

- (SongEntry)searchFor:(NSString *)aTitle artist:(NSString *)aArtist album:(NSString *)aAlbum
{
    if (!_session) {
        return SongEntry();
    }
    string t;
    string title = Base::toLower(convertString(aTitle));
    string album = Base::toLower(convertString(aAlbum));
    string artist = Base::toLower(convertString(aArtist));
    string searchString = title + " " + artist;

    std::vector<SongEntry> results = _session->searchSync(searchString, t);
    std::vector<SongEntry> filteredResults;
    std::remove_copy_if(results.begin(), results.end(), back_inserter(filteredResults), [&](const SongEntry &entry){
        bool match = Base::toLower(entry.song()->title()) == title && Base::toLower(entry.song()->artist()) == artist;
        return !match;
    });
    
    if (!album.empty()) {
        std::vector<SongEntry> albumFiltered;
        std::remove_copy_if(filteredResults.begin(), filteredResults.end(), back_inserter(albumFiltered), [&](const SongEntry &entry){
            return Base::toLower(entry.song()->album()) != album;
        });
        
        if (!albumFiltered.empty()) {
            return albumFiltered.at(0);
        }
    }
    if (!filteredResults.empty()) {
        return filteredResults.at(0);
    }
    if (!results.empty()) {
        return results.at(0);
    }
    
    return SongEntry();
}

- (shared_ptr<Gear::IPlaylist>)playlistNamed:(NSString *)aName
{
    auto name = convertString(aName);
    auto playlists = _session->playlists();
    for (auto &playlist : playlists) {
        if (playlist->name() == name) {
            return playlist;
        }
    }
    
    shared_ptr<IPlaylist> ret;
    bool gotResult = false;
    std::condition_variable cond;
    std::mutex m;
    _session->createUserPlaylist(name, [&](const shared_ptr<IPlaylist> &playlist){
        std::lock_guard<std::mutex> l(m);
        ret = playlist;
        gotResult = true;
        cond.notify_all();
    });
    
    std::unique_lock<std::mutex> l(m);
    while (!gotResult) {
        cond.wait(l);
    }
    
    return ret;
}


@end

