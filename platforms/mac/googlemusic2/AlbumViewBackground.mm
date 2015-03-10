//
//  AlbumViewBackground.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 4/30/13.
//
//

#import "AlbumViewBackground.h"
#import "CocoaThemeManager.h"
#import "AppDelegate.h"
#import "MainWindowController.h"

@implementation AlbumViewBackground {
    NSVisualEffectView *vibrant;
}

- (void)awakeFromNib
{
    // translucencyOffset might be 0
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self setupWithTranslucency];
    });
}

- (void)setupWithTranslucency
{
    if (yosemiteOrBetter() && vibrancyEnabled()) {
        CGRect frame = self.bounds;
        CGFloat translucencyOffset = [AppDelegate sharedDelegate].mainWindowController.translucencyOffset;
        frame.size.height -= translucencyOffset;
        vibrant = [[NSVisualEffectView alloc] initWithFrame:frame];
        vibrant.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        [self addSubview:vibrant positioned:NSWindowBelow relativeTo:nil];
    }
}

- (void)drawRect:(NSRect)dirtyRect
{
    [vibrant setHidden:[[CocoaThemeManager sharedManager] themePrefix] != nil];
    NSRect frame = [self bounds];
    // full vertical size, but only the needed horizontal size
    frame.origin.x = dirtyRect.origin.x;
    frame.size.width = dirtyRect.size.width;
    [[[CocoaThemeManager sharedManager] albumViewBackground] drawInRect:frame angle:90.0f];
}

@end
