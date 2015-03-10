//
//  ScriptSession.h
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 02/04/14.
//
//

#import <Foundation/Foundation.h>
#include "stdplus.h"
#include "SongEntry.h"

namespace Gear
{
    class PlaylistSession;
    class IPlaylist;
}
    
@interface ScriptSession : NSObject

- (id)initWithSession:(const shared_ptr<Gear::PlaylistSession> &)session;
- (Gear::SongEntry)searchFor:(NSString *)title artist:(NSString *)artist album:(NSString *)album;
- (shared_ptr<Gear::IPlaylist>)playlistNamed:(NSString *)aName;

@end
