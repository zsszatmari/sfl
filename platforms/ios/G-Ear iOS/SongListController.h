//
//  SongListController.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/11/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>
#include "SongEntry.h"
#import "GearImageView.h"

namespace Gui
{
    class IPaintable;
}
namespace Gear
{
    class IPlaylist;
}

@class FastScrollIndex;

@interface SongListController : UIViewController<UISearchBarDelegate, UIGestureRecognizerDelegate>

@property (strong, nonatomic) IBOutlet UITapGestureRecognizer *tapGesture;
@property (strong, nonatomic) IBOutlet UISwipeGestureRecognizer *swipeGesture;
@property (strong, nonatomic) IBOutlet GearImageView *backgroundView;
@property (weak, nonatomic) IBOutlet UITableView *tableView;
//@property(nonatomic, assign) shared_ptr<Gui::IPaintable> background;

@property (nonatomic) BOOL preventUpdates;

@property (nonatomic, strong) id animator;
@property (nonatomic, assign) shared_ptr<Gear::IPlaylist> playlist;

- (FastScrollIndex *)fastScrollIndex;
- (IBAction)tap:(id)sender;
- (void) handleLongPress: (UILongPressGestureRecognizer *) gestureRecognizer;
- (void)setParentView:(UIView *)parent;

@end
