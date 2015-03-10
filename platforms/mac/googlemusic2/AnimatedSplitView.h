//
//  AnimtedSplitView.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 10/15/12.
//
//

#import <Cocoa/Cocoa.h>

@interface AnimatedSplitView : NSView {
    float targetPosition;
}

- (void)setPositionOfDivider:(CGFloat)position animated:(BOOL)animated;
- (CGFloat)positionOfDivider;

@end
