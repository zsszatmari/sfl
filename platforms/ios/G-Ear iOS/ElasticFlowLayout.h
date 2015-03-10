//
//  ElasticFlowLayout.h
//  G-Ear iOS
//
//  Created by Szabo Attila on 12/22/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ElasticFlowLayout : UICollectionViewFlowLayout

@property (nonatomic, strong) UIDynamicAnimator *animator;
@property (nonatomic, strong) UIGravityBehavior *gravity;
@property (nonatomic, strong) UICollisionBehavior *collision;

@end
