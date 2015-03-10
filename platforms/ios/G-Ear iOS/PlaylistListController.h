//
//  playlistListController.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/12/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "GearImageView.h"
#import "SongsViewController.h"

@class FastScrollIndex;

@interface PlaylistListController : UIViewController<UIActionSheetDelegate>

@property (strong, nonatomic) IBOutlet GearImageView *backgroundView;

@property (weak, nonatomic) IBOutlet UITableView *tableView;

//@property(nonatomic, assign) shared_ptr<Gui::IPaintable> background;

@property (nonatomic, strong) id animator;
@property (nonatomic, strong) UIView *snapshot_view;
//@property (nonatomic) BOOL tab_forward;
@property (nonatomic) TransitionDirection transition_dir;
@property (nonatomic) BOOL preventUpdates;

- (FastScrollIndex *)fastScrollIndex;
- (void)setParentView:(UIView *)parent;

@end

