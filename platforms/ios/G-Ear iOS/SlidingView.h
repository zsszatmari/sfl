//
//  SlidingView.h
//  G-Ear iOS
//
//  Created by Gál László on 8/13/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface SlidingView : UIView
@property (weak, nonatomic) IBOutlet UIView *contentView;
- (UIImage*) renderToImage;
@end
