//
//  SongsTableView.m
//  G-Ear
//
//  Created by Zsolt Szatmari on 4/30/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "SongsTableView.h"
#import "MainWindowController.h"
#import "Debug.h"
#import "CocoaThemeManager.h"
#include "SortDescriptorConverter.h"
#include "SongSortOrder.h"
#include "StringUtility.h"
#include "Painter.h"
#import "ColoredTextFieldCell.h"

@interface SongsTableView ()

@property(nonatomic, assign) int hoverRow;
@property(nonatomic, assign) int hoverColumn;

@end

@implementation SongsTableView {
    NSTrackingArea *tracker;
    CGPoint tooltipPoint;
    __weak NSTimer *tooltipTimer;
    NSTextField *tooltip;
    
    id observerBounds;
    id observerResize;
}

using namespace Base;
using namespace Gear;

- (void)awakeFromNib
{
    self.hoverRow = -1;
    
    id clipView = [[self enclosingScrollView] contentView];
    observerBounds = [[NSNotificationCenter defaultCenter] addObserverForName:NSViewBoundsDidChangeNotification object:clipView queue:nil usingBlock:^(NSNotification *note) {
        // the same as mouse movement from this perspective...
        [self mouseMoved:nil];
    }];
    
    NSView *enclosingScrollView = self.superview.superview;
    observerResize = [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowDidResizeNotification object:[enclosingScrollView window] queue:nil usingBlock:^(NSNotification *note) {
        // the same as mouse movement from this perspective...
        [self refreshTracker];
    }];
    [self refreshTracker];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:observerBounds];
    [[NSNotificationCenter defaultCenter] removeObserver:observerResize];
}

- (void)refreshTracker
{
    NSView *enclosingScrollView = self.superview.superview;
    
    if (tracker) {
        [enclosingScrollView removeTrackingArea:tracker];
    }
    //NSLog(@"tracking: %@", NSStringFromRect(enclosingScrollView.bounds));
    tracker = [[NSTrackingArea alloc] initWithRect:enclosingScrollView.bounds options:NSTrackingAssumeInside | NSTrackingMouseEnteredAndExited|NSTrackingMouseMoved|NSTrackingActiveAlways owner:self userInfo:nil];
    [enclosingScrollView addTrackingArea:tracker];
}

- (void)reloadRow:(NSUInteger)row column:(NSUInteger)column
{
    if (row == -1) {
        return;
    }
    [self reloadDataForRowIndexes:[NSIndexSet indexSetWithIndex:row] columnIndexes:[NSIndexSet indexSetWithIndex:column]];
}

- (void)editColumn:(NSInteger)columnIndex row:(NSInteger)rowIndex withEvent:(NSEvent *)theEvent select:(BOOL)flag
{
    [super editColumn:columnIndex row:rowIndex withEvent:theEvent select:flag];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    NSPoint p = theEvent.locationInWindow;
    NSPoint tablePoint = [self convertPoint:p fromView:nil];
    NSInteger newRowNum = [self rowAtPoint:tablePoint];
    NSInteger newColNum = [self columnAtPoint:tablePoint];
    
    if (newColNum != self.hoverColumn || newRowNum != self.hoverRow) {
        
        int oldRow = self.hoverRow;
        int oldColumn = self.hoverColumn;
        self.hoverRow = newRowNum;
        self.hoverColumn = newColNum;
        
        [self reloadRow:oldRow column:oldColumn];
        [self reloadRow:self.hoverRow column:self.hoverColumn];
        //[self reloadVisible];
    }
    
    [self hideTooltip];
    tooltipPoint = tablePoint;
    [tooltipTimer invalidate];
    tooltipTimer = [NSTimer scheduledTimerWithTimeInterval:1.0f target:self selector:@selector(showTooltip:) userInfo:nil repeats:NO];
}

- (void)hideTooltip
{
    tooltip.hidden = YES;
}

- (void)showTooltip:(id)sender
{
    if (!tooltip) {
        tooltip = [[NSTextField alloc] init];
        tooltip.backgroundColor = [NSColor colorWithDeviceRed:240.0f/255.0f green:239.0f/255.0f blue:240.0f/255.0f alpha:1.0f];
    }
    
    [self hideTooltip];
    [tooltip removeFromSuperview];
    [self addSubview:tooltip];
    
    NSUInteger row = [self rowAtPoint:tooltipPoint];
    NSUInteger column = [self columnAtPoint:tooltipPoint];
    if (row == NSNotFound || column == NSNotFound || row >= [self.dataSource numberOfRowsInTableView:self]) {
        return;
    }
    
    CGRect rectRow = [self rectOfRow:row];
    CGRect rectColumn = [self rectOfColumn:column];
    
    CGRect rect = CGRectMake(rectColumn.origin.x, rectRow.origin.y, rectColumn.size.width, rectRow.size.height);
    NSArray *columns = [self tableColumns];
    if (column >= [columns count]) {
        return;
    }
    
    NSString *value = [self.dataSource tableView:self objectValueForTableColumn:[columns objectAtIndex:column] row:row];
    if (![value isKindOfClass:[NSString class]]) {
        return;
    }
    NSTextField *field = [self viewAtColumn:column row:row makeIfNecessary:YES];
    if (![field isKindOfClass:[NSTextField class]]) {
        return;
    }
    NSAttributedString *string = [field.cell attributedStringValue];
    CGFloat textWidth = [string size].width;
    if (textWidth < field.frame.size.width) {
        return;
    }
    rect.size.width = textWidth + 8;
    
    tooltip.stringValue = value;
    tooltip.frame = rect;
    tooltip.hidden = NO;
}

-(void)mouseEntered:(NSEvent *)theEvent {
    //[self reloadVisible];
}

-(void)mouseExited:(NSEvent *)theEvent {
    int oldRow = self.hoverRow;
    self.hoverRow = -1;
    [self reloadRow:oldRow column:self.hoverColumn];
    //[self reloadVisible];
}

+ (NSArray *)sortDescriptorsForKey:(NSString *)protoKey ascending:(BOOL)ascending
{
    using namespace Gear;
    
    return arrayFromSortDescriptor(SongSortOrder::sortDescriptor(convertString(protoKey), ascending));
}

+ (NSArray *)defaultSortOrder
{
    using namespace Gear;
    
    NSMutableArray *ret = [NSMutableArray array];
    auto order = SongSortOrder::defaultSortOrder();
    for (auto &s : order) {
        [ret addObject:convertString(s)];
    }
    return ret;
}

+ (NSString *)defaultKey
{
    return [SongsTableView defaultSortOrder][0];
}

- (void)keyDown:(NSEvent *)event
{
    
    unsigned short keyCode = [event keyCode];
    unsigned long flags = [event modifierFlags];
    
    
    const int kVK_Return = 0x24;
    const int kVK_ANSI_KeypadEnter = 0x4C;
    
    if (keyCode == kVK_Return || keyCode == kVK_ANSI_KeypadEnter) {
        NSInteger selectedRow = [self selectedRow];
        
        if (selectedRow < 0) {
            return;
        }
        
        if ((flags & NSFunctionKeyMask) == NSFunctionKeyMask) {
            [self editColumn:1 row:selectedRow withEvent:event select:YES];
            return;
        } else {
            MainWindowController *target = [self target];
            [target userInitiatedPlaySong:[target objectAtIndex:selectedRow]];
            return;
        }
    }
    
    [super keyDown:event];
}

- (void)setSortDescriptors:(NSArray *)originalDescriptors
{
    BOOL ascending = YES;
    NSString *key = @"artist";
    if ([originalDescriptors count] > 0) {
        NSSortDescriptor *intent = [originalDescriptors objectAtIndex:0];
        ascending = [intent ascending];
        key = [intent key];
    }
    if ([key isEqualToString:@"currentInstant"]) {
        return;
    }
    self.sortKey = key;
    
    NSArray *sortDescriptors = [SongsTableView sortDescriptorsForKey:key ascending:ascending];
    
    [[NSUserDefaults standardUserDefaults] setValue:[[NSValueTransformer valueTransformerForName:NSKeyedUnarchiveFromDataTransformerName] reverseTransformedValue:sortDescriptors] forKey:@"sortDescriptors"];
    
    [super setSortDescriptors:sortDescriptors];
}

- (void)resetSortDescriptors
{
    if (self.sortKey != nil) {
        [self setSortDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:self.sortKey ascending:YES]]];
    }
}

@end


@interface NSColor (CustomColors)
@end

@implementation NSColor (CustomColors)

using namespace Gui;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-protocol-method-implementation"
+ (NSColor *)alternateSelectedControlColor
{
    return Painter::convertColor([[CocoaThemeManager sharedManager] songsSelectedColor]);
}

+ (NSColor *)secondarySelectedControlColor
{
    return Painter::convertColor([[CocoaThemeManager sharedManager] songsSelectedColor]);
}

+ (NSColor *)alternateSelectedControlTextColor
{
    return [NSColor blackColor];
}

+ (NSArray*)controlAlternatingRowBackgroundColors
{
    return [[CocoaThemeManager sharedManager] songsAlternatingColors];
}

+ (NSColor *)selectedTextBackgroundColor
{
    return [[CocoaThemeManager sharedManager] selectedTextBackgroundColor];
}


#pragma clang diagnostic pop

@end


