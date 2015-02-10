//
//  AnimtedSplitView.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 10/15/12.
//
//

#import "AnimatedSplitView.h"
#import "AppDelegate.h"
#import "MainWindowController.h"
#import "CocoaThemeManager.h"

@interface AnimatedSplitView () {
    NSLayoutConstraint *positionConstraint;
    NSLayoutConstraint *fullWidthAlbumViewConstraint;
    NSLayoutConstraint *leftEdgeConstraint;
}
@end

@implementation AnimatedSplitView


static const int kDividerWidth = 1.0f;

static NSComparisonResult sortSubviews(NSView *lhs, NSView *rhs, void *context)
{
    NSUInteger lhsIndex = [[[lhs superview] subviews] indexOfObject:lhs];
    if (lhsIndex == 0) {
        return NSOrderedAscending;
    } else {
        return NSOrderedDescending;
    }
}

- (void)awakeFromNib
{
    targetPosition = -1;
    
    // z order couldn't be set in interface builder...
 
    NSView *leftView = [[self subviews] objectAtIndex:0];
    NSView *rightView = [[self subviews] objectAtIndex:1];
    
    [leftView removeFromSuperview];
    
    NSView *leftContainer = [[NSView alloc] initWithFrame:leftView.frame];
    leftContainer.autoresizingMask = NSViewHeightSizable | NSViewMaxYMargin;
    leftView.autoresizingMask = NSViewHeightSizable | NSViewWidthSizable;
    [leftContainer addSubview:leftView];
    [self addSubview:leftContainer];
    
    NSView *rightContainer = [[NSView alloc] initWithFrame:CGRectMake(0,0,self.frame.size.width,self.frame.size.height)];
    rightContainer.autoresizingMask = NSViewHeightSizable | NSViewWidthSizable;
    rightView.autoresizingMask = NSViewHeightSizable | NSViewWidthSizable;
    rightView.frame = rightContainer.frame;
    [rightContainer addSubview:rightView];
    [self addSubview:rightContainer];
}

- (void)drawRect:(NSRect)dirtyRect
{
    
    NSColor *dividerColor;
    [[[CocoaThemeManager sharedManager] albumViewBackground] getColor:&dividerColor location:NULL atIndex:0];
    
    [dividerColor setFill];
    NSRectFill(dirtyRect);
}

- (CGFloat)positionOfDivider
{
    return targetPosition;
}

- (void)logTree:(NSView *)view prefix:(NSString *)prefix
{
    for (NSView *sub in [view subviews]) {
        NSLog(@"%@%@ %@", prefix, sub,NSStringFromRect(sub.frame));
        [self logTree:sub prefix:[NSString stringWithFormat:@"%@  ",prefix]];
    }
}

- (void)logTree
{
    [self logTree:self prefix:@""];
}

- (void)setPositionOfDivider:(CGFloat)position animated:(BOOL)animated
{
    if (targetPosition == position) {
        return;
    }
    targetPosition = position;

    
    //[[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"NSConstraintBasedLayoutVisualizeMutuallyExclusiveConstraints"];
    
    if (!positionConstraint) {
        
        NSView *leftView = [[self subviews] objectAtIndex:0];
        NSView *leftRealView = [[leftView subviews] objectAtIndex:0];  // AlbumViewBackground
        NSView *rightView = [[self subviews] objectAtIndex:1];
        //NSView *rightRealView = [[rightView subviews] objectAtIndex:0];
        
        leftView.translatesAutoresizingMaskIntoConstraints = NO;
        rightView.translatesAutoresizingMaskIntoConstraints = NO;
        NSDictionary *views = NSDictionaryOfVariableBindings(leftView,rightView,leftRealView);
        [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"[leftView]-(==1)-[rightView]|" options:0 metrics:@{@"dividerWidth": @(kDividerWidth)} views:views]];
        [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[leftView]|" options:0 metrics:nil views:views]];
        [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[rightView]|" options:0 metrics:nil views:views]];
        
        // without this album view background tends to be out of sync
        leftRealView.translatesAutoresizingMaskIntoConstraints = NO;
        [leftView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[leftRealView]|" options:0 metrics:nil views:views]];
        [leftView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[leftRealView]|" options:0 metrics:nil views:views]];
        
        //[self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[leftView]-(<=1)-[rightView]|" options:0 metrics:nil views:NSDictionaryOfVariableBindings(leftView,rightView)]];
        positionConstraint = [NSLayoutConstraint constraintWithItem:rightView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeLeft multiplier:1 constant:position];
        
        //positionConstraint = [NSLayoutConstraint constraintWithItem:leftView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1 constant:position + 10];
        
        [self addConstraint:positionConstraint];
        
        fullWidthAlbumViewConstraint = [NSLayoutConstraint constraintWithItem:leftView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeWidth multiplier:1.0f constant:0];
        [self addConstraint:fullWidthAlbumViewConstraint];
        
        leftEdgeConstraint = [NSLayoutConstraint constraintWithItem:leftView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeLeft multiplier:1 constant:-1];
    }
    
    if (position <= 0.1f) {
        
        [self removeConstraint:leftEdgeConstraint];
        [self addConstraint:fullWidthAlbumViewConstraint];
        positionConstraint.constant = position;
        [self addConstraint:positionConstraint];
        
    } else if (position <= (self.frame.size.width - 0.1)) {
        
        [self addConstraint:leftEdgeConstraint];
        [self removeConstraint:fullWidthAlbumViewConstraint];
        positionConstraint.constant = position;
        [self addConstraint:positionConstraint];
        
    } else {
        
        [self removeConstraint:leftEdgeConstraint];
        [self removeConstraint:positionConstraint];
        [self addConstraint:fullWidthAlbumViewConstraint];
    }
    
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext* context) {
        context.duration = 0.4;
        // no animation on lion, but yes animation on mountain lion
        if ([context respondsToSelector:@selector(setAllowsImplicitAnimation:)]) {
            context.allowsImplicitAnimation = YES;
        }
        [self layoutSubtreeIfNeeded];
    } completionHandler:nil];
    
    // setNeedsLayout would cause problems here
    //[self setNeedsLayout:YES];
}

@end
