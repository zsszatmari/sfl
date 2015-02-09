//
//  InstantCell.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 9/23/12.
//
//

#import "InstantCell.h"

@implementation InstantCell

- (int)numberOfIcons
{
    if (self.song->uIntForKey("notInLibrary")) {
        return 0;
    }
    return 1;
}

- (NSImage *)imageAtIndex:(int)index respectFlipped:(BOOL *)respectFlipped yOffset:(CGFloat *)yOffset alpha:(float *)alpha
{
    static dispatch_once_t onceToken;
    static NSImage *instantImage;
    static NSImage *instantImageSelected;
    dispatch_once(&onceToken, ^{
        instantImage = [NSImage imageNamed:@"brain"];
        instantImageSelected = [NSImage imageNamed:@"brain"];
    });
    
    //BOOL instant = (self.song == [InstantManager sharedManager].currentInstant);
    BOOL instant = NO;
    
    *yOffset = 0;
    *alpha = instant ? 1.0f : 0.3f;
    *respectFlipped = YES;
    return instant ? instantImageSelected : instantImage;
}


- (void)selectIndex:(int)index
{
    //[InstantManager sharedManager].currentInstant = self.song;
}


@end
