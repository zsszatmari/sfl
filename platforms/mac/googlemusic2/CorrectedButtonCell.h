//
//  CorrectedButtonCell.h
//  G-Ear
//
//  Created by Zsolt Szatmari on 6/6/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "RectButtonCell.h"

@interface CorrectedButtonCell : RectButtonCell

+ (BOOL)isRetina:(NSView *)view;

@end
