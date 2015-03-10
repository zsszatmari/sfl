//
//  RoundedButtonCell.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 3/10/13.
//
//

#import <Cocoa/Cocoa.h>

@interface RoundedButtonCell : NSButtonCell

@property(nonatomic, assign) int xOffset;
@property(nonatomic, strong) NSImage *bezelImage;
@property(nonatomic, strong) NSImage *bezelAlternateImage;

@end
