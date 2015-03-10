//
//  PlaylistWrapper.m
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/5/13.
//
//

#include "IPlaylist.h"
#import "PlaylistWrapper.h"
#include "StringUtility.h"

@interface PlaylistWrapper () {
    std::shared_ptr<Gear::IPlaylist> _playlist;
    std::string _name;
}
@end

@implementation PlaylistWrapper

- (id)initWithPlaylist:(std::shared_ptr<Gear::IPlaylist>)playlist
{
    self = [super init];
    if (self) {
        _playlist = playlist;
        _name = _playlist->name();
    }
    return self;
}

- (std::shared_ptr<Gear::IPlaylist>)playlist
{
    return _playlist;
}

- (Gear::SongPredicate)playlistPredicate
{
    return Gear::SongPredicate();
}

- (Class)playlistFilterClass
{
    return nil;
}

- (NSString *)name
{
    return Gear::convertString(_name);
}

- (std::string)cppName
{
    return _name;
}

- (NSString *)description
{
    return [self name];
}

- (BOOL)isEqual:(id)anObject
{
    if ([anObject isKindOfClass:[PlaylistWrapper class]]) {
        PlaylistWrapper *rhs = (PlaylistWrapper *)anObject;
        
        // if the name changed meanwhile, we drop this wrapper object
        // '&& _name == [rhs cppName]' is very wrong: it makes playlist selection non-working after a name change
        return equals(_playlist,[rhs playlist]) ;
    }
    return NO;
}

- (NSUInteger)hash
{
    return 31337;
}

@end
