//
//  AlbumViewController.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 10/16/12.
//
//

#import <Foundation/Foundation.h>
#import "ResizingImageBrowserView.h"
#include "SongPredicate.h"

extern NSString *const kDefaultsKeyAlbumCellSize;

@class Grouping;
@class ImageBrowserView;
@class ReiszingImageBrowserView;

namespace Gear
{
    class IPlaylist;
}

@interface AlbumViewController : NSObject<NSTableViewDelegate,NSTableViewDataSource>



/*#ifndef DISABLE_IMAGEKIT
@property (unsafe_unretained) IBOutlet ResizingImageBrowserView *imageBrowserView;
#else
@property (unsafe_unretained) id imageBrowserView;
#endif
*/
@property (nonatomic,weak) NSTableView *imageBrowserView;

@property (nonatomic, assign) std::shared_ptr<Gear::IPlaylist> browserPlaylist;
@property (nonatomic, assign) Gear::SongPredicate filterPredicate;

- (CGSize)cellSize;
- (void)reload;
- (void)refresh;
- (void)applyTheme;
- (BOOL)isVisible:(Grouping *)grouping;

@end
