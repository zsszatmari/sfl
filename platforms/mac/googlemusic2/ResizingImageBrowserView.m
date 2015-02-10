//
//  ResizingImageBrowserView.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 10/21/12.
//
//

#import "Debug.h"
#import "ResizingImageBrowserView.h"

@interface GearImageBrowserCell : IKImageBrowserCell
@end

@implementation GearImageBrowserCell

- (NSRect)subtitleFrame
{
    NSRect frame = [super subtitleFrame];
    frame.origin.y += 5;
    return frame;
}

- (NSRect)titleFrame
{
    NSRect frame = [super titleFrame];
    frame.origin.y += 2;
    return frame;
}

- (NSRect)imageContainerFrame
{
    // everything slightly up, otherwise bottom content is sliced...
    NSRect frame = [super imageContainerFrame];
    frame.origin.y += 10;
    return frame;
}

@end

@implementation ResizingImageBrowserView

#ifndef DISABLE_IMAGEKIT

- (void)setFrame:(CGRect)frame
{
    NSUInteger index = NSNotFound;
    NSIndexSet *selected = [self selectionIndexes];
    if ([selected count] > 0) {
        index = [selected firstIndex];
    } else {
        NSIndexSet *visible = [self visibleItemIndexes];
        if ([visible count] > 0 && [visible firstIndex] > 0) {
            index = [visible firstIndex];
        }
    }
    
    [super setFrame:frame];
    NSRect bounds = [self bounds];
    bounds.origin.y = 25;
    [self setBounds:bounds];
    
    if (index != NSNotFound) {
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [NSAnimationContext beginGrouping];
            [[NSAnimationContext currentContext] setDuration:0];
            [self scrollIndexToVisible:index];
            [NSAnimationContext endGrouping];
        });
    }
}

#endif


- (IKImageBrowserCell *)newCellForRepresentedItem:(id)anItem
{
    return [[GearImageBrowserCell alloc] init];
}

@end

