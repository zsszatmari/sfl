//
//  ButtonCellDrawOneWhenMixed.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 8/31/12.
//
//

#import "ButtonCellDrawOneWhenMixed.h"
#import "CocoaThemeManager.h"

@implementation ButtonCellDrawOneWhenMixed

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    [super drawInteriorWithFrame:cellFrame inView:controlView];
    
    if ([self intValue] == NSMixedState) {
        NSColor *color = [[CocoaThemeManager sharedManager] buttonContentColorPressed];
        [@"1" drawAtPoint:CGPointMake(CGRectGetMaxX(cellFrame)-7, CGRectGetMidY(cellFrame)-3) withAttributes:@{NSFontAttributeName: [NSFont systemFontOfSize:8], NSForegroundColorAttributeName : color}];
    }
}
    
@end
