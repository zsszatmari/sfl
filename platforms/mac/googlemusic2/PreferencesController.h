//
//  PreferencesController.h
//  G-Ear
//
//  Created by Zsolt Szatmari on 6/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
@class PrefAccounts;
@interface PreferencesController : NSObject<NSToolbarDelegate, NSWindowDelegate> {
	PrefAccounts *accounts;
}

@property (nonatomic, assign) BOOL windowVisible;
@property (unsafe_unretained) IBOutlet NSToolbar *preferencesToolbar;
@property (unsafe_unretained) IBOutlet NSTabView *tabView;
@property (unsafe_unretained) IBOutlet NSTextView *keyUtilText;
@property (unsafe_unretained) IBOutlet NSButton *notificationCenterCheckbox;
@property (unsafe_unretained) IBOutlet NSButton *notificationCenterCheckboxOnlyLast;
@property (unsafe_unretained) IBOutlet NSButton *showShareCheckbox;

- (void)viewDidLoad;
- (IBAction)selectAccounts:(id)sender;
- (IBAction)selectGeneral:(id)sender;
- (IBAction)selectLastfm:(id)sender;
- (IBAction)selectShortcuts:(id)sender;
- (IBAction) selectTest: (id) sender;

- (IBAction)showWindow:(id)sender;
- (void)showWindow;
- (void)hideWindow;

@end
