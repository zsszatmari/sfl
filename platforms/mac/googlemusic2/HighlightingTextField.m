//
//  HighlightingTextFiel.m
//  googlemusic2
//
//  Created by Zsolt Szatmári on 3/18/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "HighlightingTextField.h"

@interface HighlightingTextField () {
    NSTrackingArea *trackingArea;
    NSAttributedString *originalString;
}
@end

@implementation HighlightingTextField

static NSMutableArray *highlighted;
static NSString * const kHighlight = @"TopTextHighlightedNotification";

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if (self) {
        
        [self setup];
    }
    return self;
}

- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self) {
        [self setup];
    }
    return self;;
}

- (void)setup
{
    if (!highlighted) {
        highlighted = [NSMutableArray array];
    }
    
    trackingArea = [[NSTrackingArea alloc] initWithRect:CGRectMake(0, 0, 0, 0) options:NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveAlways | NSTrackingInVisibleRect owner:self userInfo:nil];
    [self addTrackingArea:trackingArea];
    
    [self performSelector:@selector(mouseExited:) withObject:nil afterDelay:0.0f];
    
    [[NSNotificationCenter defaultCenter] addObserverForName:kHighlight object:nil queue:nil usingBlock:^(NSNotification *note) {
        
        if ([highlighted count] > 0) {
            [self highlight];
        } else {
            [self unhighlight];
        }
    }];
}

+ (NSColor *)highlightColorFrom:(NSColor *)color
{
    NSColor *value = [color colorUsingColorSpaceName:NSDeviceRGBColorSpace];
    return [NSColor colorWithDeviceRed:value.redComponent green:value.greenComponent blue:value.blueComponent alpha:[value alphaComponent] * 0.75f];
}

- (void)highlight
{
    if (originalString) {
        return;
    }
    
    originalString = [self attributedStringValue];
    NSMutableAttributedString *shadowString = [[NSMutableAttributedString alloc] initWithAttributedString:originalString];
    
    [originalString enumerateAttributesInRange:NSMakeRange(0, shadowString.length) options:0 usingBlock:^(NSDictionary *attrs, NSRange range, BOOL *stop) {
        
        NSMutableDictionary *newAttrs = [attrs mutableCopy];
        NSColor *val = [newAttrs objectForKey:NSForegroundColorAttributeName];
        if (val) {
            NSColor *newColor = [[self class] highlightColorFrom:val];
            [newAttrs setObject:newColor forKey:NSForegroundColorAttributeName];
            [shadowString setAttributes:newAttrs range:range];
        }
    }];
    [self setAttributedStringValue:shadowString];
}

- (void)unhighlight
{
    if (originalString) {
        [self setAttributedStringValue:originalString];
        originalString = nil;
    }
}

- (void)mouseEntered:(NSEvent *)theEvent
{    
    CGPoint point = [self convertPoint:[[self window] mouseLocationOutsideOfEventStream] fromView:nil];
    CGSize size = [self.cell attributedStringValue].size;
    BOOL inside = CGRectContainsPoint(CGRectMake(0, 0, size.width, size.height), point);
    
    if (inside) {
        [highlighted addObject:self];
    } else {
        [highlighted removeObject:self];
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:kHighlight object:nil];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    [self mouseEntered:theEvent];
}

- (void)mouseExited:(NSEvent *)theEvent
{
    [highlighted removeObject:self];
    [[NSNotificationCenter defaultCenter] postNotificationName:kHighlight object:nil];
}

-(void)mouseDown:(NSEvent *)event
{
    if ([[self stringValue] length] > 0) {
        [self sendAction:[self action] to:[self target]];
    }
}


@end
