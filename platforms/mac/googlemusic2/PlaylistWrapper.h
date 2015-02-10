//
//  PlaylistWrapper.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/5/13.
//
//

#import <Foundation/Foundation.h>
#import "PlaylistProtocol.h"

namespace Gear
{
    class IPlaylist;
}

@interface PlaylistWrapper : NSObject<PlaylistProtocol>

- (id)initWithPlaylist:(std::shared_ptr<Gear::IPlaylist>)playlist;
- (std::shared_ptr<Gear::IPlaylist>)playlist;

@end
