//
//  Grouping.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 10/15/12.
//
//

#import <Foundation/Foundation.h>
#include "SongPredicate.h"

namespace Gear
{
    class SongGrouping;
}

@class AlbumViewController;

// IKImageBrowserItem informal protocol
@interface Grouping : NSObject

@property(nonatomic, readonly) NSArray *imageArray;
@property(nonatomic, strong) NSString *title;
@property(nonatomic, strong) NSString *subtitle;
@property(nonatomic, assign) Gear::SongPredicate predicate;
@property(nonatomic, assign) BOOL multiImage;
@property(nonatomic, unsafe_unretained) AlbumViewController *controller;

- (NSImage *)imageRepresentationWithRefresh:(dispatch_block_t)aRefreshHandler;
+ (Grouping *)groupingWith:(Gear::SongGrouping &)song;
- (void)refresh;
//- (void)awakeFromVisible;

@end
