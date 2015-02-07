//
//  CorrectedNonSkinButtonCell.m
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 25/12/13.
//
//

#import "CorrectedNonSkinButtonCell.h"
#import "CorrectedButtonCell.h"

@implementation CorrectedNonSkinButtonCell

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    CGSize offset = NSMakeSize(0, 0.0f);
    
    cellFrame.origin.y = cellFrame.origin.y + offset.height;
    cellFrame.size.height = cellFrame.size.height + 2.0f;
    [super drawInteriorWithFrame:cellFrame inView:controlView];
}

@end
