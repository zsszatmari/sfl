//
//  SettingsControllerViewController.h
//  G-Ear iOS
//
//  Created by Gál László on 7/30/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>
#include <vector>

namespace Gear {
    class PreferenceGroup;
};

@interface SettingsViewController : UIViewController<UIActionSheetDelegate>
@property (weak, nonatomic) IBOutlet UITableView *tableView;
@property (strong, nonatomic) IBOutlet UISwipeGestureRecognizer *swipeGesture;
@property (nonatomic, assign) std::vector<Gear::PreferenceGroup> preferenceGroup;

- (IBAction)swipe:(id)sender;
@end
