//
//  CircularIndicator.m
//  G-Ear iOS
//
//  Created by Zsolt Szatmari on 03/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#import "CircularIndicator.h"

@implementation CircularIndicator {
    float ratio;
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self setBackgroundColor:[UIColor clearColor]];
    }
    return self;
}

- (void)setRatio:(float)aRatio
{
    ratio = aRatio;
    [self setNeedsDisplay];
}

- (void)drawRect:(CGRect)rect
{
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGRect bounds = [self bounds];
    bounds.origin.x += 1;
    bounds.origin.y += 1;
    bounds.size.width -= 2;
    bounds.size.height -= 2;
    CGContextAddEllipseInRect(ctx, bounds);
    CGColorRef color = [[UIColor colorWithRed:5.0f/255.0f green:55.0f/255.0f blue:94.0f/255.0f alpha:1.0f] CGColor];
    CGContextSetStrokeColorWithColor(ctx, color);
    CGContextSetFillColorWithColor(ctx, color);
    
    CGContextStrokePath(ctx);
    
    CGContextBeginPath(ctx);
    CGContextMoveToPoint(ctx, CGRectGetMidX(bounds), CGRectGetMidY(bounds));
    CGContextAddArc(ctx, CGRectGetMidX(bounds), CGRectGetMidY(bounds), CGRectGetWidth(bounds)/2, - M_PI/2, -M_PI/2 + 2* M_PI * ratio, false);
    CGContextClosePath(ctx);
    
    CGContextFillPath(ctx);
}

@end
