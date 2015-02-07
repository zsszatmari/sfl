//
//  CategoryCell.m
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 29/03/14.
//
//

#import "CategoryField.h"
#import "CocoaThemeManager.h"
#import "AppDelegate.h"
#import "PlaylistManager.h"

@interface SingleClickButtonCell : NSButtonCell
@end

@implementation SingleClickButtonCell

@end

@interface CategoryField () {
	NSRect buttonFrame;
	NSTrackingArea *trackingArea;
	NSTrackingRectTag trackingTag;
	NSString *showOrHide;
    NSString *showTitle;
}
@end

@implementation CategoryField

- (id)init
{
    self = [super init];
    if (self) {
        trackingArea = [[NSTrackingArea alloc] initWithRect:CGRectMake(0, 0, 0, 0) options:NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveAlways | NSTrackingInVisibleRect owner:self userInfo:nil];
        [self addTrackingArea:trackingArea];
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
    //CGRect cellFrame = self.bounds;
    
    NSDictionary *cellAttribs = [[CocoaThemeManager sharedManager] categoryAttributes];
    //CGRect cellFrame = [self titleRect:cellAttribs];
    CGRect cellFrame = self.bounds;

//	NSLog(@"CELLDRAW %@, row: %ld, deleg row: %@ %@", controlView, self.row, [self stringValue], [[_actionDelegate cellCurrRow] uppercaseString]);
	//NSRect origFrame = cellFrame;
    cellFrame.origin.x += kCategoryPositionLeft -1;
    cellFrame.size.width -= 46;

    cellFrame.origin.y -= 1;
    
    cellFrame.origin.y += self.topSpacing;
    cellFrame.size.height -= self.topSpacing;
    
    CGContextRef ctx = (CGContextRef)[NSGraphicsContext currentContext].graphicsPort;
    CGContextSetShouldSmoothFonts(ctx, false);
	[[self stringValue] drawInRect: cellFrame withAttributes: cellAttribs];
    
    if (showTitle == nil) {
        showOrHide = [_actionDelegate showHideLabel:[self stringValue]];
        [self mouseExited:nil];
    }
	buttonFrame = (NSRect) {{self.bounds.size.width - 33, cellFrame.origin.y}, {44, cellFrame.size.height}};
    [showTitle drawInRect:buttonFrame withAttributes:cellAttribs];
}
/*
- (NSRect)titleRect:(NSDictionary *)cellAttribs
{
    NSRect titleFrame = self.bounds;
    NSSize titleSize = [[self stringValue] sizeWithAttributes:cellAttribs];
    titleFrame.origin.y = self.bounds.origin.y + (self.bounds.size.height - titleSize.height) / 2.0;
    return titleFrame;
}*/

- (void)mouseEntered: (NSEvent *) event
{
    showTitle = showOrHide;
    [self setNeedsDisplay:YES];
}

- (void) mouseExited: (NSEvent *) event
{
    showTitle = @"";
    [self setNeedsDisplay:YES];
}

- (void)mouseUp:(NSEvent *)theEvent
{
    showOrHide = [_actionDelegate cellAction: [self stringValue]];
    showTitle = showOrHide;
}

@end
