//
//  RemoteApp.m
//  G-Ear iOS
//
//  Created by Szabo Attila on 10/20/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "RemoteApp.h"
#include "IApp.h"
#include "IPlayer.h"

using namespace Gear;

@implementation RemoteApp

static id <remote_control_delegate> remote_delegate;
+ (void) set_remote: (id) r {
	remote_delegate = r;
}

- (void) sendEvent: (UIEvent *) event {
	if (event.type == UIEventTypeRemoteControl)
       		switch (event.subtype) {
			case UIEventSubtypeRemoteControlPause:
			case UIEventSubtypeRemoteControlPlay:
			case UIEventSubtypeRemoteControlTogglePlayPause:
				IApp::instance()->player()->play();
				break;
			case UIEventSubtypeRemoteControlPreviousTrack:
				IApp::instance()->player()->prev();
				break;
			case UIEventSubtypeRemoteControlNextTrack:
				IApp::instance()->player()->next();
				break;
			default:
				NSLog(@"remote: %d", event.subtype);
		}
	else
		[super sendEvent: event];
}

@end
