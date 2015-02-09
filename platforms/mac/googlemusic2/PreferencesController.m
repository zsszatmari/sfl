//
//  PreferencesController.m
//  G-Ear
//
//  Created by Zsolt Szatmari on 6/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "PreferencesController.h"
#import "AppDelegate.h"
#import "PrefAccounts.h"

@implementation PreferencesController

@synthesize preferencesToolbar;
@synthesize tabView;
@synthesize keyUtilText;
@synthesize notificationCenterCheckbox;
@synthesize notificationCenterCheckboxOnlyLast;
@synthesize windowVisible = _windowVisible;

- (void) awakeFromNib {
	NSLog(@"awake");
//	NSTabViewItem *item = [[NSTabViewItem alloc] initWithIdentifier: @"accounts_test"];
//	[item setLabel: @"Accounts (test)"];
	accounts = [[PrefAccounts alloc] init];
//	[item setView: accounts.view];
//	[tabView addTabViewItem: item];
	[[tabView tabViewItems][0] setView: accounts.view];
}

- (void)viewDidLoad
{
    // hidden functionality for the preview: last.fm
    //[preferencesToolbar removeItemAtIndex:2];
	[accounts viewDidLoad];
    [self selectAccounts:nil];
    [[tabView window] setLevel:NSFloatingWindowLevel];
    
    if (lionOrBetter()) {
        NSString *text;
        if (lionOrBetter()) {
            text = @"Please note that to make media keys on Mac keyboard and Apple Remote behave correctly, you need to install Magic Keys";
        } else {
            text = @"Please note that to be able to receive Apple Remote events, you need to install Magic Keys";
        }
        
        NSRange range = [text rangeOfString:@"Magic Keys"];
        NSMutableAttributedString *newText = [[NSMutableAttributedString alloc] initWithString:text];
        NSDictionary *attributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSURL URLWithString:@"http://www.gearmusicplayer.com/extras.html"], NSLinkAttributeName,
                                   nil];
        [newText setAttributes:attributes range:range];
        [[keyUtilText textStorage] setAttributedString:newText];
    } else {
        [keyUtilText setHidden:YES];
    }
    
    if (!mountainLionOrBetter()) {
        [notificationCenterCheckbox setEnabled:NO];
        [notificationCenterCheckboxOnlyLast setEnabled:NO];
        
        [self.showShareCheckbox setEnabled:NO];
    }
    
    [[self.tabView window] setDelegate:self];
}

- (IBAction)selectAccounts:(id)sender {
    [preferencesToolbar setSelectedItemIdentifier:@"accounts"];
    [self selectTabViewItemAtIndex:0];
}

- (IBAction)selectGeneral:(id)sender {
    [preferencesToolbar setSelectedItemIdentifier:@"general"];
    [self selectTabViewItemAtIndex:1];
}

- (IBAction)selectLastfm:(id)sender {
    [preferencesToolbar setSelectedItemIdentifier:@"lastfm"];
    [self selectTabViewItemAtIndex:2];
}

- (IBAction)selectShortcuts:(id)sender {
    [preferencesToolbar setSelectedItemIdentifier:@"shortcuts"];
    [self selectTabViewItemAtIndex:3];
}

- (IBAction)selectTest: (id) sender {
	[preferencesToolbar setSelectedItemIdentifier: @"test"];
	[self selectTabViewItemAtIndex: 4];
}

- (void)selectTabViewItemAtIndex:(int)index
{
    [tabView selectTabViewItemAtIndex:index];
    NSView *selectedView = [[tabView selectedTabViewItem] view];
    CGFloat height = 0;
	NSWindow *window = [tabView window];
	CGRect frame = window.frame;
	if (! selectedView.isFlipped) {
	    for (NSView *subview in [selectedView subviews]) {
        
	        if ([subview isHidden]) {
	            continue;
	        }
	        CGFloat viewBottom = selectedView.frame.size.height - subview.frame.origin.y;
	    
	        if (viewBottom > height) {
	            height = viewBottom;
	        }
	    }
	    height += 20.0f;
	} else {
		CGFloat low = .0;
		for (NSView *subview in [selectedView subviews]) {
			if ([subview isHidden]) {
				continue;
			}
			CGFloat maxy = NSMaxY(subview.frame);
			if (maxy > low) {
				low = maxy;
			}
		}
		height = low + 20.;
	}

    // add everything that is not the view itself
    height += frame.size.height - (tabView.frame.origin.y + tabView.frame.size.height);
    
    CGRect originalFrame = frame;
    CGFloat originalHeight = originalFrame.size.height;
    frame.size.height = height;
    frame.origin.y += originalHeight - height;

    static BOOL first = YES;
    [window setFrame:frame display:YES animate:!first];
    first = NO;
}

static const NSTimeInterval kShowHideDuration = 0.25f;

- (IBAction)showWindow:(id)sender
{
    [self showWindow];
}

- (void)showWindow
{
    NSWindow *window = [tabView window];
    if (!self.windowVisible || [window alphaValue] < 1.0f) {
        [window setAlphaValue:0.0f];
        [NSAnimationContext beginGrouping];
        [[NSAnimationContext currentContext] setDuration:kShowHideDuration];
        [[window animator] setAlphaValue:1.0f];
        [NSAnimationContext endGrouping];
    }
    
    self.windowVisible = YES;
    
    [window makeKeyAndOrderFront:nil];
    
}

- (void)hideWindow
{
    self.windowVisible = NO;
    
    NSWindow *window = [tabView window];
    
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
    [self hideWindow];
    return NO;
}

@end
