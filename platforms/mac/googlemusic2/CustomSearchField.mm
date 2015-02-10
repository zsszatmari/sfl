//
//  CustomSearchField.m
//  Gear for Google Play
//
//  Created by Zsolt Szatmari on 10/11/14.
//
//

#import "CustomSearchField.h"
#import "CustomSearchFieldCell.h"
#import "CocoaThemeManager.h"

@implementation CustomSearchField

+ (Class)cellClass
{
    return [CustomSearchFieldCell class];
}

@end

@implementation CustomSearchFieldEditor

- (void)drawRect:(NSRect)dirtyRect
{
    CGFloat radius = self.bounds.size.height / 2;
    NSGradient *searchBackground = [[CocoaThemeManager sharedManager] searchBackground];

    NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:self.bounds xRadius:radius yRadius:radius];
    [searchBackground drawInBezierPath:path angle:90.0f];
    [super drawRect:dirtyRect];
}

@end