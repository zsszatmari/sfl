//
//  LightScrollView.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 2/4/13.
//
//

#import "LightScrollView.h"
#import "CocoaThemeManager.h"

@implementation LightScrollView

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    //[self lockFocus];
    [[[CocoaThemeManager sharedManager] playlistsBackgroundColor] set];
    NSFrameRect(self.bounds);
    //[self unlockFocus];
}

@end
