//
//  EqSlider.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmari on 14/02/15.
//  Copyright (c) 2015 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>
#include <string>

@interface EqSlider : UIView

@property (nonatomic,strong) dispatch_block_t onUpdate;
@property (nonatomic,assign) std::string labelText;
@property (nonatomic,assign) std::string key;

- (void)update:(BOOL)animated;
- (void)userInput:(CGPoint)point;

@end
