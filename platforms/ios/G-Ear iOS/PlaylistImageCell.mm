//
//  PlaylistImageCell.m
//  G-Ear iOS
//
//  Created by Gál László on 9/18/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "PlaylistImageCell.h"
#import "SongCell.h"

using namespace Gear;

@implementation PlaylistImageCell {
    BOOL constraintsDefined;
}

@synthesize albumArt;

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
    }
    return self;
}

- (void)defineConstraints
{
    self.nameLabel.translatesAutoresizingMaskIntoConstraints = NO;
    self.sourceImage = [[GearImageView alloc] initWithFrame:CGRectZero];
    self.sourceImage.translatesAutoresizingMaskIntoConstraints = NO;
    [self.contentView addSubview: self.sourceImage];
    self.albumArt.translatesAutoresizingMaskIntoConstraints = NO;
    
    CGFloat albumSize = self.frame.size.height - 8;
    NSDictionary *metrics = @{@"albumSize":@(albumSize),
                              @"sourceWidth":@(SourceWidth)};
    
    UIView *sourceImage = self.sourceImage;
    UIView *nameLabel = self.nameLabel;
    NSDictionary *views = NSDictionaryOfVariableBindings(sourceImage,nameLabel,albumArt);
    [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|-12-[albumArt(==albumSize)]-10-[nameLabel]-12-[sourceImage(==sourceWidth)]-23-|" options:0 metrics:metrics views:views]];
    
    [self addConstraint:[NSLayoutConstraint constraintWithItem:albumArt attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    [self addConstraint:[NSLayoutConstraint constraintWithItem:albumArt attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:0 constant:albumSize]];
    
    [self addConstraint:[NSLayoutConstraint constraintWithItem:sourceImage attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    [self addConstraint:[NSLayoutConstraint constraintWithItem:sourceImage attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:0 constant:SourceWidth]];
    
    [self addConstraint:[NSLayoutConstraint constraintWithItem:nameLabel attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
}

- (void)setPlaylist:(shared_ptr<Gear::IPlaylist>)playlist
{
    if (!constraintsDefined) {
        [self defineConstraints];
        constraintsDefined = YES;
    }
    
    [self.albumArt setPromise:playlist->image(self.albumArt.frame.size.width*2)];
    
    [super setPlaylist:playlist];
}

@end
