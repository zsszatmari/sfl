//
//  PlaylistCell.h
//  G-Ear iOS
//
//  Created by Gál László on 7/25/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "GearImageView.h"
#include "IPlaylist.h"

namespace Gui
{
    class TextAttributes;
    class IPaintable;
}

@interface PlaylistCell : UITableViewCell

@property (weak, nonatomic) IBOutlet UILabel *nameLabel;
@property (weak, nonatomic) IBOutlet GearImageView *separator;
@property (nonatomic, assign) Gui::TextAttributes nameAttributes;
@property (nonatomic, assign) int separatorThickness;
@property (nonatomic, strong) GearImageView *sourceImage;

- (void)setPlaylist:(shared_ptr<Gear::IPlaylist>)playlist;
- (void)setSeparatorThickness:(int)separatorThickness;
@end
