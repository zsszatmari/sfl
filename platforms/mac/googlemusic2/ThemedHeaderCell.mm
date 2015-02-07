//
//  ThemedHeaderCell.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/4/13.
//
//

#import "ThemedHeaderCell.h"
#import "CocoaThemeManager.h"
#include "Gradient.h"
#include "Color.h"
#include "Painter.h"
#include "Writer.h"
#include "IApp.h"
#include "ThemeManager.h"
#include "ITheme.h"
#import "AppDelegate.h"

@interface ThemedHeaderCell () {
}

@end

@implementation ThemedHeaderCell

using namespace Gear;

- (void)drawSortIndicatorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView ascending:(BOOL)ascending priority:(NSInteger)priority
{
    // nada
}


static const CGFloat kIndent = 3;


- (void)drawWithFrame:(NSRect)cellFrame inView:(NSTableHeaderView *)controlView
{
    // this would ruin translucency on yosemite:
    //[super drawWithFrame:cellFrame inView:controlView];
 
    NSTableView *tableView = [controlView tableView];
    NSArray *sortDescriptors = [tableView sortDescriptors];
    BOOL sortIndicatorNeeded = NO;
    BOOL sortIndicatorAscending = NO;
    if ([sortDescriptors count] > 0) {
        NSSortDescriptor *descriptor = [sortDescriptors objectAtIndex:0];
        if ([[descriptor key] isEqualToString:self.identifier]) {
            sortIndicatorNeeded = YES;
            sortIndicatorAscending = [descriptor ascending];
        }
    }
    
    NSGradient *gradient = [[CocoaThemeManager sharedManager] songsHeaderGradient];
    NSColor *colorTop;
    NSColor *colorBottom;
    if (gradient != nil) {
        [gradient drawInRect:cellFrame angle:90];
    
        colorTop = [NSColor colorWithDeviceWhite:131.0f/255.0f alpha:1.0f];
        colorBottom = [NSColor colorWithDeviceWhite:140.0f/255.0f alpha:1.0f];
        NSGradient *separator = [[NSGradient alloc] initWithStartingColor:colorBottom endingColor:colorTop];
        
        [separator drawInRect:NSMakeRect(cellFrame.origin.x+cellFrame.size.width-1, 4, 1, cellFrame.size.height-1) angle:270];
        
    } else {
        colorTop = [NSColor colorWithDeviceWhite:223.0f/255.0f alpha:1.0f];
        colorBottom = [NSColor colorWithDeviceWhite:223.0f/255.0f alpha:1.0f];
        
        if (yosemiteOrBetter()) {
            // translucency
        } else {
            // we have to 'extend' the normal gradient because it's too narrow
            using namespace Gui;
            
            //237,255
            Color from(237.0f/255.0f,237.0f/255.0f,237.0f/255.0f);
            Color to(1,1,1);
            Gradient gradient(from, to);
            
            Painter::paint(gradient, cellFrame);
        
            colorTop = [NSColor whiteColor];
            colorBottom = [NSColor colorWithDeviceWhite:203.0f/255.0f alpha:1.0f];
        }
        
        NSGradient *separator = [[NSGradient alloc] initWithStartingColor:colorBottom endingColor:colorTop];
        
        [separator drawInRect:NSMakeRect(cellFrame.origin.x+cellFrame.size.width-1, 5, 1, cellFrame.size.height-8) angle:270];
    }
    
    [colorBottom setFill];
    NSRectFill(CGRectMake(cellFrame.origin.x, cellFrame.origin.y + cellFrame.size.height -1, cellFrame.size.width, 1));
    
    
    NSAttributedString *attributedString = [self attributedStringValue];
    if ([attributedString length] == 0) {
        // avoid exception in next line
        return;
    }
    NSMutableDictionary *attrs = [[attributedString
                                   attributesAtIndex:0 effectiveRange:NULL] mutableCopy];
    //[attrs setObject:aParagraphStyle forKey:NSParagraphStyleAttributeName];
    [attrs setObject:[[CocoaThemeManager sharedManager] songsHeaderTextColor] forKey:NSForegroundColorAttributeName];
    auto themeManager = IApp::instance()->themeManager();
    auto current = themeManager->current();
    NSFont *font;
    if (sortIndicatorNeeded) {
        font = [NSFont fontWithName:@"HelveticaNeue-Bold" size:11.0f];
    } else {
        font = [NSFont fontWithName:@"HelveticaNeue-Medium" size:11.0f];
    }
    [attrs setObject:font forKey:NSFontAttributeName];
    
    cellFrame.origin.y += 3 + 2 + 1;
    cellFrame.origin.x += (kIndent+2);
    if ([[self stringValue] isEqualToString:@"#"]) {
        cellFrame.origin.x -= 1;
    }
    
    cellFrame.size.width -= (kIndent+2);
    [[self stringValue] drawInRect:cellFrame withAttributes:attrs];
    
    if (sortIndicatorNeeded) {
        cellFrame.origin.x -= 3;
        cellFrame.origin.y -= 3;
        [super drawSortIndicatorWithFrame:cellFrame inView:controlView ascending:sortIndicatorAscending priority:0];
    }
}

@end
