//
//  NowPlayingImageTitle.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/3/13.
//
//

#import "NowPlayingImageTitle.h"
#import "CocoaThemeManager.h"
#import "AppDelegate.h"
#import "MainWindowController.h"
#import "PlaylistManager.h"

@interface  NowPlayingImageTitle () {
	NSTrackingArea *trackingArea;
	BOOL isMouseOver;
	NSString *showOrHide;
}

@end

@implementation NowPlayingImageTitle

- (void) awakeFromNib {
	[self updateTrackingArea];
	showOrHide = @"Hide";
}

- (void)drawRect:(NSRect)dirtyRect
{
    //[[NSColor redColor] setFill]; NSRectFill(dirtyRect);
    
    NSInteger yOffset = self.yOffsetRelativeToCenter;
    
    yOffset -= 4;
    
    NSFont *font = [NSFont fontWithName:@"HelveticaNeue-Bold" size:10.46f];
    CGFloat fontHeight = 8;
    yOffset += (self.frame.size.height - fontHeight)/2;
    
    if (yosemiteOrBetter() && [[CocoaThemeManager sharedManager] themePrefix] == nil) {
        // draw separator line
        [[NSColor colorWithDeviceWhite:214.0f/255.0f alpha:1.0f] setFill];
        NSRectFill(CGRectMake(dirtyRect.origin.x, self.frame.size.height-1, dirtyRect.size.width, 1));
        yOffset += 1;
    }
    NSGradient *gradient = [[CocoaThemeManager sharedManager] playlistCategoryGradient];
    NSRect rect = [self bounds];

	//NSLog(@"RECT: %@ %@", NSStringFromRect(rect), NSStringFromRect(dirtyRect));
    rect.origin.x = dirtyRect.origin.x;
    rect.size.width = dirtyRect.size.width;
    [gradient drawInRect:rect angle:90.0f];

    NSDictionary *attributes = [[CocoaThemeManager sharedManager] categoryAttributes];
    if (attributes == nil) {
        return;
    }
    
    NSAttributedString *string = [[NSAttributedString alloc] initWithString:@"Now Playing" attributes:attributes];
    rect = [self bounds];
    
    const CGFloat kOffset = kCategoryPositionLeft;
    rect.origin.x += kOffset;
    rect.size.width -= kOffset;
    rect.origin.y += 2;
    
    rect.origin.y -= yOffset;
    [string drawInRect:rect];

	if (isMouseOver) {
		rect.origin.y += 2;
		rect.size.width -= 4;
		[[[NSAttributedString alloc] initWithString: showOrHide attributes: [[CocoaThemeManager sharedManager] categoryHideButtonAttributes]] drawInRect: rect];
	}
}

- (void) updateTrackingArea {
	//NSLog(@"update nowplaying");
	[self removeTrackingArea: trackingArea];
	trackingArea = [[NSTrackingArea alloc] initWithRect: self.frame
		options: NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways | NSTrackingInVisibleRect
		owner: self userInfo: nil];
        [self addTrackingArea: trackingArea];
	if (NSPointInRect([self convertPoint: [[self window] mouseLocationOutsideOfEventStream] fromView: nil], [self bounds])) {
		[self mouseEntered: nil];
	} else {
		[self mouseExited: nil];
	}
}

- (void) updateTrackingAreas {
	[self updateTrackingArea];
	[super updateTrackingAreas];
}

- (void) mouseEntered: (NSEvent *) event {
	//NSLog(@"menteredd");
	isMouseOver = YES;
	[self setNeedsDisplay: YES];
	[self displayIfNeeded];
}

- (void) mouseExited: (NSEvent *) event {
	isMouseOver = NO;
	[self setNeedsDisplay: YES];
	[self displayIfNeeded];
}

- (void) mouseMoved: (NSEvent *) event {
}

- (void) mouseDown: (NSEvent *) event {
	NSPoint location = [self convertPoint:[event locationInWindow] fromView:nil];
	//NSLog(@"mouse loc: %@", location);

	NSRect hitRect = (NSRect) {{self.frame.size.width - 50, 0}, {50, self.frame.size.height}};

	if (NSMouseInRect(location, hitRect, [self isFlipped])) {
		//NSLog(@"hide hit");
		NSString *newShowOrHide = [_delegate showHide: showOrHide];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.2f * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            // a little delay is necessary to avoid strangeness
            showOrHide = newShowOrHide;
            [self setNeedsDisplay: YES];
        });
		
//		NSAttributedString *title = [[NSAttributedString alloc] initWithString: showOrHide attributes:[[CocoaThemeManager sharedManager] categoryHideButtonAttributes]];
//		[showButton setAttributedTitle: title];
	} else {
        
        [[AppDelegate sharedDelegate].mainWindowController jumpToSong:nil];
    }
}


@end
