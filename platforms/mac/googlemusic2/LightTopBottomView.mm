//
//  LightTopBottomScrollView.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 2/4/13.
//
//

#import "LightTopBottomView.h"
#import "CocoaThemeManager.h"


@interface LightView : NSView
@end

@implementation LightTopBottomView


- (void)awakeFromNib
{
    [self setupOverlay];
}

- (void)setupOverlay
{
    LightView *bottom = [[LightView alloc] initWithFrame:CGRectMake(0, 0, self.bounds.size.width, 1)];
    bottom.autoresizingMask = NSViewWidthSizable | NSViewMaxYMargin;
    [self addSubview:bottom];
    
    LightView *top = [[LightView alloc] initWithFrame:CGRectMake(0, self.bounds.size.height-1, self.bounds.size.width, 1)];
    top.autoresizingMask = NSViewWidthSizable | NSViewMinYMargin;
    [self addSubview:top];    
}


- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    if ([self isHiddenOrHasHiddenAncestor]) {
        return;
    }
    [self lockFocus];
    [[NSColor colorWithDeviceWhite:153.0f/255.0f alpha:1.0f] set];
    NSFrameRect(CGRectMake(0, 0, self.bounds.size.width, 5));
    //NSFrameRect(CGRectMake(0, self.bounds.size.height-1, self.bounds.size.width, 1));
    [self unlockFocus];
}

@end

@implementation LightView

- (void)drawRect:(NSRect)dirtyRect
{
    [[[CocoaThemeManager sharedManager] frameColor] set];
    NSRectFill(dirtyRect);
}

@end
