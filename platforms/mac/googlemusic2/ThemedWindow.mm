//
//  ThemedWindow.m
//  test
//
//  Created by Zsolt Szatmári on 3/9/13.
//  Copyright (c) 2013 Zsolt Szatmári. All rights reserved.
//

#import "ThemedWindow.h"
#import "AppDelegate.h"
#import "CustomSearchField.h"
#import "CocoaThemeManager.h"

@interface GradientView : NSView

@end

@interface ThemedWindow () {
    NSArray *gradients;
    NSTimeInterval disabledEditingUntil;
}
@end

@implementation ThemedWindow

- (void)awakeFromNib
{
    if (yosemiteOrBetter() && vibrancyEnabled()) {
        //self.backgroundColor = [NSColor colorWithCalibratedRed: .5 green: .6 blue: .7 alpha: .9];
        //self.opaque = NO;
       
        
        self.titleVisibility = NSWindowTitleHidden;
        self.titlebarAppearsTransparent = YES;
        self.styleMask |= NSFullSizeContentViewWindowMask;
    }

    
//    NSView *view = [[self contentView] superview];
    NSView *view = [self contentView];
 	NSMutableArray *arr = [view.superview.subviews mutableCopy];
	NSUInteger i = [arr indexOfObject: view];
	if (i > 0 && i != NSNotFound) {
		arr[i] = arr[0];
		arr[0] = view;
	}
	view.superview.subviews = [arr copy];

	view.frame = (NSRect) {{0, 0}, self.frame.size};
    //CGRect windowFrame = CGRectMake(0, 0, self.frame.size.width, self.frame.size.height);
    
	
    [self setTitle:@""];
    // because there is no title it won't be added to the window menu automatically
    [[NSApplication sharedApplication] addWindowsItem:self title:@"Gear Player" filename:NO];
    
    //[self setTitle:@"G-Ear"];
}

- (void)temporaryDisableEditing
{
    disabledEditingUntil = [NSDate timeIntervalSinceReferenceDate] + 2.0f;
}

- (NSText *)fieldEditor:(BOOL)createWhenNeeded
              forObject:(id)anObject
{
    if ([NSDate timeIntervalSinceReferenceDate] < disabledEditingUntil) {
        return nil;
    }
    
    NSGradient *searchBackground = [[CocoaThemeManager sharedManager] searchBackground];
    if ([anObject isKindOfClass:[CustomSearchField class]] && searchBackground != nil) {
        // hack is needed because drawn color affects text rendering because of some mystic reason...
        // we can't subclass NSTextView because there is some hidden undecypherable magic going on in the original fieldEditor subclass (and even encoding/decoding does not help)
        NSText *text = [super fieldEditor:createWhenNeeded forObject:anObject];
        // bad results on mavericks, and also, only needed on yosemite after all
        //if ([[NSImage class] respondsToSelector:@selector(imageWithSize:flipped:drawingHandler:)]) {
        if (yosemiteOrBetter()) {
            CGSize backgroundSize = [anObject bounds].size;
            backgroundSize.height = 16;
            [text setBackgroundColor:[NSColor colorWithPatternImage:[NSImage imageWithSize:backgroundSize flipped:YES drawingHandler:^BOOL(NSRect dstRect) {
                
                CGFloat radius = 0;
                NSGradient *gradient = [[NSGradient alloc] initWithColors:@[[NSColor colorWithDeviceWhite:110.0f/255.0f alpha:1.0f], [NSColor colorWithDeviceWhite:84.0f/255.0f alpha:1.0f]]];
                NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:dstRect xRadius:radius yRadius:radius];
                [gradient drawInBezierPath:path angle:90.0f];
                return YES;
            }]]];
            [text setDrawsBackground:YES];
        } else {
            // leave it as it is
        }
        return text;
    } else {
        NSText *text = [super fieldEditor:createWhenNeeded forObject:anObject];
        return text;
    }
}

@end
