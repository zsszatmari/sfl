//
//  CorrectedButtonCell.m
//  G-Ear
//
//  Created by Zsolt Szatmari on 6/6/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "CorrectedButtonCell.h"

@implementation CorrectedButtonCell

+ (BOOL)isRetina:(NSView *)controlView
{
    CGSize offset = NSMakeSize(0, 1.0f);
    if ([controlView respondsToSelector:@selector(convertSizeFromBacking:)]) {
        offset = [controlView convertSizeFromBacking:offset];
    }
    offset.height = floor(offset.height) - 1.0f;
    
    // offset: -1 for retina
    
    return offset.height < -0.1f;
}

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{    
    CGSize offset = NSMakeSize(0, 0.0f);
    
    cellFrame.origin.y = cellFrame.origin.y + offset.height;
    cellFrame.size.height = cellFrame.size.height + 2.0f;
    
    static NSInteger customOffset = [[NSUserDefaults standardUserDefaults] integerForKey:@"ButtonOffset"];
    if ((controlView.autoresizingMask & NSViewMinXMargin) != 0) {
        cellFrame.origin.y += customOffset;
    }
    
    [super drawInteriorWithFrame:cellFrame inView:controlView];
}

@end
