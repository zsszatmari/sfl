//
//  GearSlider.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 8/17/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface GearSlider : UISlider

@property(nonatomic,assign) NSTimeInterval disableTime;

- (void)applyTheme;
 
@end
