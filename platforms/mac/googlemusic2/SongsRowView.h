//
//  SongsRowView.h
//  Gear for Google Play
//
//  Created by Zsolt Szatmari on 21/11/14.
//
//

#import <Cocoa/Cocoa.h>

namespace Gear
{
    class SongEntry;
}

@interface SongsRowView : NSTableRowView

@property(nonatomic,strong) NSTableView *tableView;
@property(nonatomic,assign) NSUInteger row;
@property(nonatomic,assign) Gear::SongEntry entry;

@end
