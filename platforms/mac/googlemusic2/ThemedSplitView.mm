//
//  ThemedSplitView.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 4/30/13.
//
//

#import "ThemedSplitView.h"
#import "CocoaThemeManager.h"
#import "AppDelegate.h"

@implementation ThemedSplitView {
    NSView *leftView;
}

- (NSColor *)dividerColor
{
    return [[CocoaThemeManager sharedManager] playlistsBackgroundColor];
}

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    leftView = [[self subviews] objectAtIndex:0];
    CGRect frame = leftView.frame;
    NSView *holder = [[NSView alloc] initWithFrame:frame];
    
    [self replaceSubview:leftView with:holder];
    [holder addSubview:leftView];
    holder.autoresizingMask = leftView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    
    [self adjustSubviews];
}

- (void)applyTheme:(CGFloat)translucencyOffset
{
    CGFloat margin = ([[CocoaThemeManager sharedManager] themePrefix] == nil && !yosemiteOrBetter()) ? 1 : 0;
    CGFloat bottomMargin = 0;
    CGFloat topMargin = 0;
    if ([[CocoaThemeManager sharedManager] themePrefix] == nil && yosemiteOrBetter()) {
        bottomMargin = 1;
    }
    if ([[CocoaThemeManager sharedManager] themePrefix] != nil) {
        topMargin = 1;
    }
    
    
    CGRect frame = leftView.frame;
    
    frame.origin.y = margin + bottomMargin;
    frame.size.height = self.frame.size.height - 2*margin -bottomMargin - topMargin -translucencyOffset;
    leftView.frame = frame;
    //leftView.hidden = YES;
}

- (NSView *)leftView
{
    return leftView;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    //[[NSColor redColor] setFill];
    //NSRectFill(dirtyRect);
    
    if ([[CocoaThemeManager sharedManager] themePrefix] == nil) {
        [[NSColor colorWithDeviceWhite:184.0f/255.0f alpha:1.0f] setFill];
        NSRectFill(dirtyRect);
    } else {
        [[NSColor colorWithDeviceWhite:8.0f/255.0f alpha:1.0f] setFill];
        NSRectFill(dirtyRect);
    }
    
    
    [[[CocoaThemeManager sharedManager] frameColor] set];
    
    
    CGRect upperRect = CGRectMake(dirtyRect.origin.x, 0, dirtyRect.size.width, 1);
    CGRect lowerRect = CGRectMake(dirtyRect.origin.x, self.frame.size.height-1, dirtyRect.size.width, 1);
    NSRectFill(upperRect);
    NSRectFill(lowerRect);
}


@end
