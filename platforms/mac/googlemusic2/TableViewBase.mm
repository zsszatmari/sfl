//
//  TableViewBase.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 1/21/13.
//
//

#import "TableViewBase.h"
#import "Debug.h"
#import "PlaylistManager.h"
#import "MainWindowController.h"

@interface  TableViewBase () {
	NSTrackingRectTag trackingTag;
	NSTrackingArea *trackingArea;
	BOOL isMouseOver;
}

@end

@implementation TableViewBase

- (void) awakeFromNib {
//	[[self window] setAcceptsMouseMovedEvents: YES];
	_prevRow = _currRow = -1;
	[self updateTrackingArea];
}

- (void) updateTrackingTag {
	[self removeTrackingRect: trackingTag];
	trackingTag = [self addTrackingRect: self.frame owner: self userData: nil assumeInside: NO];
}

- (void) updateTrackingArea {
//	NSLog(@"update");
	[self removeTrackingArea: trackingArea];
	trackingArea = [[NSTrackingArea alloc] initWithRect: self.frame
		options: NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveAlways | NSTrackingInVisibleRect
		owner: self userInfo: nil];
        [self addTrackingArea: trackingArea];
}

- (void) updateTrackingAreas {
	[self updateTrackingArea];
	[super updateTrackingAreas];
}

#ifndef DISABLE_CONTEXT_MENU
- (void)keyDown:(NSEvent *)theEvent
{
    const unsigned short kBackspace = 51;
    
    if ([theEvent keyCode] == kBackspace) {
        id delegate = [self delegate];
        if ([delegate respondsToSelector:@selector(deleteRow:)]) {
            [(id)delegate performSelector:@selector(deleteRow:) withObject:nil];
        }
    } else {
        [super keyDown:theEvent];
    }
}
#endif


-(NSMenu*)menuForEvent:(NSEvent*)event
{
#ifdef DISABLE_CONTEXT_MENU
    return nil;
#endif
 
    if (![self.delegate respondsToSelector:@selector(tableView:menuForRows:column:)]) {
        return nil;
    }
    
    NSIndexSet *selected = [self selectedRowIndexes];
    NSPoint menuPoint = [self convertPoint:[event locationInWindow] fromView:nil];
    if ([selected count] < 2) {
        

        NSUInteger row = [self rowAtPoint:menuPoint];
        if (row == NSNotFound) {
            if ([selected count] == 0) {
                return nil;
            } else {
                // keep what we have
            }
        } else {
            selected = [NSIndexSet indexSetWithIndex:row];
        }
    }
    NSUInteger column = [self columnAtPoint:menuPoint];
    
    return [(id)self.delegate tableView:self menuForRows:selected column:column];
}

- (void) mouseEntered: (NSEvent *) event {
//	NSLog(@"MENTER");
	isMouseOver = YES;
}

- (void) mouseExited: (NSEvent *) event {
//	NSLog(@"MEXIT");
	NSInteger row = _currRow;
	isMouseOver = NO;
	_currRow = _prevRow = -1;
	[self setNeedsDisplayInRect: [self rectOfRow: row]];
	[self displayIfNeeded];
}

- (void) mouseMoved: (NSEvent *) event {
	if (! isMouseOver) {
		return;
	}
	if ((_currRow = [self rowAtPoint: [self convertPoint: [event locationInWindow] fromView: nil]]) == _prevRow) {
		return;
	}
//	} else {
//		prevRow = _currRow;
//	}
//	NSLog(@"moved %ld", _currRow);
	[self setNeedsDisplayInRect: [self rectOfRow: _currRow]];
	[self setNeedsDisplayInRect: [self rectOfRow: _prevRow]];
	[self displayIfNeeded];
	_prevRow = _currRow;
}

- (void) updateRow {

}

- (void) resetCursorRects {
	[self updateTrackingArea];
//	NSLog(@"reset");
}

@end
