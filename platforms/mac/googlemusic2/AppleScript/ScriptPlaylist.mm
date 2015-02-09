//
//  ScriptPlaylist.m
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 30/03/14.
//
//

#import "ScriptPlaylist.h"
#import "Bridge.h"
#import "ScriptSong.h"

using namespace Gear;

@implementation ScriptPlaylist {
    shared_ptr<IPlaylist> _playlist;
}

- (id)initWithPlaylist:(shared_ptr<IPlaylist>)playlist
{
    self = [super init];
    if (self) {
        _playlist = playlist;
    }
    return self;
}

- (void)addSong:(ScriptSong *)aSong;
{
    SongEntry entry = aSong.entry;
    if (entry.song()) {
        vector<SongEntry> songs;
        songs.push_back(entry);
        _playlist->addSongEntries(songs);
    }
}

@end
