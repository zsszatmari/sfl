//
//  SongsRowView.m
//  Gear for Google Play
//
//  Created by Zsolt Szatmari on 21/11/14.
//
//

#import "SongsRowView.h"
#import "NSImage+Tinting.h"
#import "CocoaThemeManager.h"
#include "Painter.h"
#include "IApp.h"
#include "SongEntry.h"
#include "IPlayer.h"
#import "MainWindowController.h"
#import "NSView+Recursive.h"

@implementation SongsRowView


using namespace Gear;
using namespace Gui;

- (BOOL)isHighlighted
{
    return [[self.tableView selectedRowIndexes] containsIndex:self.row];
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];

    if (self.entry != IApp::instance()->player()->songEntryConnector().value()) {
        return;
    }
    
    NSString * const name = @"volume";
    
    NSImage *image;
        
    if ([self isHighlighted]) {
        image = [NSImage imageNamed:name tintWithColor:Painter::convertColor([[CocoaThemeManager sharedManager] songsPlayIconHighlightedColor])];
    } else {
        image = [NSImage imageNamed:name tintWithColor:Painter::convertColor([[CocoaThemeManager sharedManager] songsPlayIconColor])];
    }
    
    if (!image) {
        return;
    }
    
    CGFloat offsetX = [self.tableView rectOfColumn:[self.tableView columnWithIdentifier:@"position"]].origin.x;
    
    float ratio = image.size.height / image.size.width;
    
    //CGFloat width = 18.0f;
    CGFloat width = image.size.width;
    CGSize drawSize = CGSizeMake(width, width * ratio);
    CGRect drawRect = CGRectMake(0, 0, image.size.width, image.size.height);
    CGRect inRect = CGRectMake(offsetX + 4,
                               (int)(self.frame.size.height/2 - drawSize.height/2) +1,
                               drawSize.width,
                               drawSize.height);
    
    [image drawInRect:inRect fromRect:drawRect operation:NSCompositeSourceOver fraction:1.0f];
}

- (void)setSelected:(BOOL)value
{
    BOOL change = (value != self.selected);
    
    [super setSelected:value];
    if (change) {
        
        NSUInteger columnCount = self.numberOfColumns;
         for (int i = 0 ; i < columnCount ; ++i) {
            NSView *view = [self viewAtColumn:i];
            [MainWindowController refreshTableTextBackground:view row:self.row table:self.tableView];
            [view setNeedsDisplayRecursive];
        }
    }
}


@end
