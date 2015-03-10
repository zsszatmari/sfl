//
//  PlayViewController.h
//  G-Ear iOS
//
//  Created by Gál László on 7/18/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//


#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIKit.h>
#import "GearImageView.h"
#import "GearSlider.h"

@interface PlayViewController : UIViewController<AVAudioSessionDelegate>

@property (strong, nonatomic) IBOutlet UISwipeGestureRecognizer *swipeGesture;

@property (weak, nonatomic) IBOutlet GearImageView *backgroundView;
@property (weak, nonatomic) IBOutlet GearImageView *bottomBar;
@property (weak, nonatomic) IBOutlet GearImageView *topBar;

@property (weak, nonatomic) IBOutlet GearImageView *albumArt;
@property (weak, nonatomic) IBOutlet UILabel *artistLabel;
@property (weak, nonatomic) IBOutlet UILabel *songLabel;
@property (weak, nonatomic) IBOutlet GearSlider *durationSlider;
@property (weak, nonatomic) IBOutlet UIButton *nextButton;
@property (weak, nonatomic) IBOutlet UIButton *shuffleButton;
@property (weak, nonatomic) IBOutlet UILabel *timeLeftLabel;
@property (weak, nonatomic) IBOutlet UILabel *currentTimeLabel;
@property (weak, nonatomic) IBOutlet UIButton *repeatButton;
@property (weak, nonatomic) IBOutlet UIButton *prevButton;
@property (weak, nonatomic) IBOutlet UIButton *playButton;
@property (weak, nonatomic) IBOutlet UIButton *shareButton;
@property (weak, nonatomic) IBOutlet UIButton *ratingButton;


- (IBAction)repeatTapped:(UIButton *)sender;
- (IBAction)shuffleTapped:(UIButton *)sender;
- (IBAction)shareTapped:(id)sender;
- (IBAction)ratingTapped:(id)sender;
- (void)applyTheme;

@end
