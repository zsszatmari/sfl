//
//  GearVisualEffectView.m
//  Gear for Google Play
//
//  Created by Zsolt Szatmari on 19/11/14.
//
//

#import "GearVisualEffectView.h"

@implementation GearVisualEffectView {
    NSTrackingArea *trackingArea;
    CGPoint lastDragLocation;
    BOOL dragging;
}

- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self) {
        trackingArea = [[NSTrackingArea alloc] initWithRect:CGRectMake(0, 0, 0, 0) options:NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveAlways | NSTrackingInVisibleRect owner:self userInfo:nil];
        //[self addTrackingArea:trackingArea];
    }
    return self;
}

- (BOOL) acceptsFirstMouse:(NSEvent *)e
{
    return NO;
}

- (void)mouseDown:(NSEvent *) e
{
    if ([e clickCount] == 2) {
        [self.window miniaturize:nil];
        return;
    }
    CGFloat y = [e window].frame.size.height - [e locationInWindow].y;
    if (y > 22) {
        lastDragLocation = [NSEvent mouseLocation];
        dragging = YES;
    }
}

- (void)mouseUp:(NSEvent *) e
{
    dragging = NO;
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    if (!dragging) {
        return;
    }
    NSPoint newDragLocation = [NSEvent mouseLocation];
    CGRect frame = [self window].frame;
    NSPoint thisOrigin = frame.origin;
    CGFloat diffX = (-lastDragLocation.x + newDragLocation.x);
    CGFloat diffY = (-lastDragLocation.y + newDragLocation.y);
    // avoid sudden jumps...
    if (diffX < 200) {
        thisOrigin.x += (-lastDragLocation.x + newDragLocation.x);
    }
    if (diffY < 200) {
        thisOrigin.y += (-lastDragLocation.y + newDragLocation.y);
    }
    
    [[self window] setFrameOrigin:thisOrigin];
    lastDragLocation = newDragLocation;
}

@end
