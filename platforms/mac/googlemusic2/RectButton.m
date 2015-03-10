//
//  RectButton.m
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 18/12/13.
//
//

#import "RectButton.h"

@implementation RectButton

- (void)awakeFromNib
{
    [super awakeFromNib];
    CGRect rect = self.frame;
    rect.size.height = 19;
    self.frame = rect;
}

@end
