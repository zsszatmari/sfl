//
//  PlaylistProtocol.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 9/19/12.
//
//

#import <Foundation/Foundation.h>
#include "SongPredicate.h"

@protocol PlaylistProtocol <NSObject>

- (Gear::SongPredicate)playlistPredicate;
- (NSString *)name;

@end
