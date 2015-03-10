//
//  GearApplication.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 4/7/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <IOKit/hidsystem/ev_keymap.h>

#import "SPMediaKeyTap.h"
#import "GearApplication.h"
#import "AppDelegate.h"
#import "MainWindowController.h"
#include "App.h"

@implementation GearApplication

- (void)removeWindowsItem:(NSWindow *)aWindow
{
    if (aWindow == [AppDelegate sharedDelegate].window) {
        // do nothing
    } else {
        [super removeWindowsItem:aWindow];
    }
}


- (void)sendEvent:(NSEvent *)theEvent
{
    // lion behaviour
    BOOL shouldHandleMediaKeyEventLocally = ![SPMediaKeyTap usesGlobalMediaKeyTap];

    if(shouldHandleMediaKeyEventLocally && [theEvent type] == NSSystemDefined && [theEvent subtype] == SPSystemDefinedEventMediaKeys) {
        
		[[AppDelegate sharedDelegate] mediaKeyTap:nil receivedMediaKeyEvent:theEvent];
	}
	[super sendEvent:theEvent];
}

- (id)valueInBridgedObjectsWithUniqueID:(id)unique
{
    return nil;
}


+ (id)valueInBridgedObjectsWithUniqueID:(id)unique
{
    return nil;
}

@end
