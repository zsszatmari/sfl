//
//  SongCell.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "GearImageView.h"
#include "SongEntry.h"

namespace Gui
{
    class TextAttributes;
    class IPaintable;
}

extern const CGFloat SourceWidth;

@class CircularIndicator;

@interface SongCell : UITableViewCell

@property (strong, nonatomic) IBOutlet UILabel *artistLabel;
@property (strong, nonatomic) IBOutlet UILabel *albumLabel;
@property (strong, nonatomic) IBOutlet UILabel *songLabel;
@property (strong, nonatomic) CircularIndicator *offlineIndicator;
@property (strong, nonatomic) IBOutlet GearImageView *albumArt;
@property (strong, nonatomic) IBOutlet GearImageView *separator;
@property (nonatomic, assign) int separatorThickness;
@property (nonatomic, strong) GearImageView *sourceImage;

- (void)setSong:(Gear::SongEntry)songEntry;
- (void)setSeparatorThickness:(int)separatorThickness;
@end
