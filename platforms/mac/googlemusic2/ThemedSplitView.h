//
//  ThemedSplitView.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 4/30/13.
//
//

#import <Cocoa/Cocoa.h>

@interface ThemedSplitView : NSSplitView

- (void)applyTheme:(CGFloat)translucencyOffset;
- (NSView *)leftView;

@end
