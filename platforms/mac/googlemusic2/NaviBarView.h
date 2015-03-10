//
//  NaviBarView.h
//  Gear for Google Play and YouTube
//
//  Created by Zsolt Szatmari on 20/02/15.
//
//

#import <Cocoa/Cocoa.h>
#include "NavibarInfo.h"

@interface NaviBarView : NSView

- (void)setInfo:(const Gear::NaviBarInfo &)info;

@end
