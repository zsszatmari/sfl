//
//  SongListTextFieldCell.m
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 09/06/14.
//
//

#import "SongListTextFieldCell.h"
#import "CocoaThemeManager.h"
#import "HighlightingTextField.h"

@implementation SongListTextFieldCell

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{    
    BOOL highlighted = [[self.tableView selectedRowIndexes] containsIndex:self.row];
     NSColor *color;
    if (highlighted) {
        color = [[CocoaThemeManager sharedManager] songsTextSelectedColor];
    } else {
        color = [[CocoaThemeManager sharedManager] songsTextColor];
    }


    NSColor *shouldHighlightWithColor = nil;
    
    if (self.mayHighlightOnHover && self.row == self.tableView.hoverRow && self.column == self.tableView.hoverColumn) {
        
        color = [HighlightingTextField highlightColorFrom:color];
    }
    
    [self setTextColor:color];
    
    //cellFrame.origin.y += 1;
    //cellFrame.size.height -= 2;
    
    // if we override drawWithFrame, it causes glitches
    //NSLog(@"textf: %@ %@", NSStringFromRect(cellFrame), NSStringFromRect([self.controlView.superview convertRect:self.controlView.frame toView:nil]));
    
    [super drawInteriorWithFrame:cellFrame inView:controlView];
}


@end
