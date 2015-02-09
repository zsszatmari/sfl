//
//  EQController.m
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/24/13.
//
//

#import "EQController.h"
#import "AppDelegate.h"
#import "MainWindowController.h"

@interface EQController () {
    BOOL windowVisible;
}
@end

@implementation EQController

using namespace Gear;

static const NSTimeInterval kShowHideDuration = 0.25f;

- (void)showWindow
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        Class vibrantClass = NSClassFromString(@"NSVisualEffectView");
        if (vibrantClass)
        {
            //self.window.titleVisibility = NSWindowTitleHidden;
            self.window.titlebarAppearsTransparent = YES;
            self.window.styleMask |= NSFullSizeContentViewWindowMask;

            NSView *contentView = self.window.contentView;
            NSVisualEffectView *vibrant = [[vibrantClass alloc] initWithFrame:contentView.frame];
            [vibrant setAutoresizingMask:contentView.autoresizingMask];
            [vibrant setBlendingMode:NSVisualEffectBlendingModeBehindWindow];
            [vibrant setMaterial:NSVisualEffectMaterialTitlebar];
            
            [self.window setContentView:vibrant];
            [vibrant addSubview:contentView];
            
            contentView.translatesAutoresizingMaskIntoConstraints = NO;
            NSDictionary *views = NSDictionaryOfVariableBindings(contentView);
            [vibrant addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[contentView]|" options:0 metrics:nil views:views]];
            [vibrant addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|-22-[contentView]|" options:0 metrics:nil views:views]];
        }
    });
    NSPanel *window = self.window;
    if (!self.windowVisible || [window alphaValue] < 1.0f) {
        [window setAlphaValue:0.0f];
        [NSAnimationContext beginGrouping];
        [[NSAnimationContext currentContext] setDuration:kShowHideDuration];
        [[window animator] setAlphaValue:1.0f];
        [NSAnimationContext endGrouping];
    }
    
    windowVisible = YES;
    
    [window makeKeyAndOrderFront:nil];
    [[AppDelegate sharedDelegate].window addChildWindow:self.window ordered:NSWindowAbove];
}

- (void)hideWindow
{
    windowVisible = NO;
    
    NSWindow *window = self.window;

    [NSAnimationContext beginGrouping];
    [[NSAnimationContext currentContext] setDuration:kShowHideDuration];
    [[window animator] setAlphaValue:0.0f];
    [NSAnimationContext endGrouping];
    
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, kShowHideDuration * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        if (!self.windowVisible) {
            [window orderOut:nil];
        }
    });
}

- (BOOL)windowShouldClose:(id)sender
{
    self.windowVisible = NO;
    return NO;
}


- (BOOL)windowVisible
{
    return windowVisible;
}

- (void)setWindowVisible:(BOOL)value
{
    if (value) {
        [self showWindow];
    } else {
        [self hideWindow];
    }
}

@end
