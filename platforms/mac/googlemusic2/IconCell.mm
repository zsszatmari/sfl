//
//  IconCell.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 9/23/12.
//
//

#include <map>
#import "IconCell.h"

@interface IconCell () {
    CGFloat fullWidth;
    
    int number;
    shared_ptr<Gear::ISong> _song;
}
@end

@implementation IconCell

- (void)setSong:(std::shared_ptr<Gear::ISong>)song
{
    _song = song;
}

- (std::shared_ptr<Gear::ISong>)song
{
    return _song;
}

- (BOOL)isHighlighted
{
    return [[self.tableView selectedRowIndexes] containsIndex:self.row];
}

// warning: it is assumed that all icons are of the same size
- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    //[[NSColor redColor] setFill]; NSRectFill(cellFrame);
    
    NSGraphicsContext *ctx = [NSGraphicsContext currentContext];
    [ctx saveGraphicsState];
    NSRectClip(cellFrame);
    
    CGFloat xPos = 4;
    for (int i = 0 ; i < [self numberOfIcons] ; i++) {
        CGFloat yOffset;
        BOOL respectFlipped;
        float alpha;
        NSImage *image = [self imageAtIndex:i respectFlipped:&respectFlipped yOffset:&yOffset alpha:&alpha];
        
        CGRect fromRect = CGRectMake(0, 0, image.size.width, image.size.height);
        
        //CGFloat w = 14;//17.0f // * 0.8f;
        //CGFloat h = 15;//19.0f // * 0.8f;
        
        CGFloat w = image.size.width;
        CGFloat h = image.size.height;
        
        yOffset += 1;
        
        CGRect inRect = CGRectMake(cellFrame.origin.x + xPos /*+(cellFrame.size.width - w)/2*/, cellFrame.origin.y + yOffset + (int)((cellFrame.size.height -h)/2), w, h);// image.size.width, image.size.height);
        [image drawInRect:inRect fromRect:fromRect operation:NSCompositeSourceOver fraction:alpha respectFlipped:respectFlipped hints:nil];
        
        xPos += (int)(image.size.width);
    }
    fullWidth = xPos;
    [ctx restoreGraphicsState];
}

- (NSUInteger)hitTestForEvent:(NSEvent *)event inRect:(NSRect)cellFrame ofView:(NSView *)controlView
{
    BOOL ignore = NO;
    
    static NSEvent *lastEvent;
    if (lastEvent == event) {
        ignore = YES;
    }
    lastEvent = event;
    
    CGPoint pointInWindow = [event locationInWindow];
    CGPoint pointInView = [controlView convertPoint:pointInWindow fromView:nil];
    CGPoint pointInCell = CGPointMake(pointInView.x - cellFrame.origin.x, pointInView.y - cellFrame.origin.y);
    
    if (pointInCell.x >= 0 && pointInCell.x <= fullWidth) {
        
        if (event.type == NSLeftMouseDown && !ignore) {
            [self selectIndex:pointInCell.x / ((fullWidth / [self numberOfIcons]))];
        }
        return NSCellHitContentArea;
    }
    
    return NSCellHitNone;
}

- (int)numberOfIcons
{
    NSAssert(NO,@"abstract method");
    return 0;
}

- (NSImage *)imageAtIndex:(int)index respectFlipped:(BOOL *)respectFlipped yOffset:(CGFloat *)yOffset
{
    NSAssert(NO,@"abstract method");
    return nil;
}

- (NSImage *)imageAtIndex:(int)index respectFlipped:(BOOL *)respectFlipped yOffset:(CGFloat *)yOffset alpha:(float *)alpha
{
    *alpha = 1.0f;
    return [self imageAtIndex:index respectFlipped:respectFlipped yOffset:yOffset];
}

- (void)selectIndex:(int)index
{
    NSAssert(NO,@"abstract method");
}

@end
