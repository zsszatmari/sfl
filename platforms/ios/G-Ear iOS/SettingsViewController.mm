//
//  SettingsControllerViewController.m
//  G-Ear iOS
//
//  Created by Gál László on 7/30/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "SettingsViewController.h"
#import "SettingsTextCell.h"
#import "SettingsSwitchCell.h"
#include "IApp.h"
#include "IPreferencesPanel.h"
#include "PreferenceGroup.h"
#include "Preference.h"
#include "StringUtility.h"
#import "AppDelegate.h"
#include "CocoaTouchWebWindow.h"
#import "SongsViewController.h"
#import "GearTopBar.h"
#import "LastFmSettings.h"


#define PRE_IOS7_FONT [UIFont boldSystemFontOfSize: 17.]

using namespace Gear;

@implementation SettingsViewController {
    std::vector<Gear::PreferenceGroup> _preferenceGroup;
    std::vector<std::function<void()>> actionSheetSelect;
}

@synthesize tableView;

- (void)viewWillAppear:(BOOL)animated
{
    // if the stuck viewcontrollers problem manifests itself next time, checking this would be a good idea:
    // (lldb) po [[SongsViewController topNavigationController] viewControllers]
    // but appears normal
    
    [super viewWillAppear:animated];
    [[(SongsViewController *) self.navigationController.parentViewController topBar] settingsShown];
	
    // hacky but otherwise can't scroll down. will have to find a clearer solution
    self.tableView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    
    //self.tableView.autoresizingMask = 0;
    //self.tableView.frame = CGRectMake(0, 0, self.view.frame.size.width, self.navigationController.view.frame.size.height - 36);
    
    
    if ([AppDelegate runningPreIOS7]) {
		self.tableView.backgroundColor = [UIColor whiteColor];
		self.tableView.backgroundView = nil;
	}
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
    [super willRotateToInterfaceOrientation:toInterfaceOrientation duration:duration];
    
    self.tableView.frame = self.tableView.superview.bounds;
    self.tableView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
}

- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
    
    SongsViewController *controller = [SongsViewController sharedController];
    GearTopBar *topBar = [controller topBar];
    [topBar settingsHidden];
}

- (void)setPreferenceGroup:(std::vector<Gear::PreferenceGroup>)preferenceGroup
{
    _preferenceGroup = preferenceGroup;
}

- (std::vector<Gear::PreferenceGroup>)preferenceGroup
{
    return _preferenceGroup;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return _preferenceGroup.size() + 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    if (section == _preferenceGroup.size()) {
        return 2;
    }
    PreferenceGroup &group = _preferenceGroup.at(section);
    NSUInteger numOfRows = group.subgroups().size() + group.preferences().size();
#ifdef DEBUG
    NSLog(@"rows in section: %lu", (unsigned long)numOfRows);
#endif
    return numOfRows;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    if ([indexPath section] == _preferenceGroup.size()) {
        
        UITableViewCell *cell = [[UITableViewCell alloc] initWithFrame:CGRectMake(0, 0, self.view.frame.size.width, self.tableView.rowHeight)];
        UIButton *button = [[UIButton alloc] initWithFrame:cell.bounds];
        button.autoresizingMask = UIViewAutoresizingFlexibleWidth;
        button.titleLabel.font = [UIFont systemFontOfSize:15.0f];
        [button setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        if ([indexPath row] == 0) {
            [button setTitle:@"Feedback" forState:UIControlStateNormal];
            [button addTarget:self action:@selector(feedbackTapped:) forControlEvents:UIControlEventTouchUpInside];
        } else if ([indexPath row] == 1) {
            [button setTitle:@"Credits" forState:UIControlStateNormal];
            [button addTarget:self action:@selector(creditsTapped:) forControlEvents:UIControlEventTouchUpInside];
        }
        [cell addSubview:button];
        return cell;
    }
    
    Preference::Type prefType;
    SettingsCell *cell;
    
    auto prefGroup = _preferenceGroup.at(indexPath.section);
    if (indexPath.row < prefGroup.preferences().size())
    {
        auto pref = prefGroup.preferences().at(indexPath.row);
        prefType = pref.type();
        switch (prefType) {
            case Preference::Type::Text:
                cell = [self.tableView dequeueReusableCellWithIdentifier:@"settingsTextCell"];
                break;
                
            case Preference::Type::OnOff:
                cell = [self.tableView
                        dequeueReusableCellWithIdentifier:@"settingsSwitchCell"];
                break;
                
            case Preference::Type::Button:
                cell = [self.tableView
                        dequeueReusableCellWithIdentifier:@"settingsButtonCell"];
                break;
            case Preference::Type::Special:
            case Preference::Type::Options:
                cell = [self.tableView
                        dequeueReusableCellWithIdentifier:@"settingsTextCell"];
                break;
            default:
                break;
        }
        cell.selectionStyle = UITableViewCellSelectionStyleNone;
        if ([AppDelegate runningPreIOS7]) {
            cell.textLabel.font = PRE_IOS7_FONT;
        }
        
        [cell setPreference:pref];
        
    }
    else
    {
        auto sub = [self subgroupAtIndexPath:indexPath];
        cell = [self.tableView
                dequeueReusableCellWithIdentifier:@"settingsSubgroupCell"];
//	if (is_pre_ios7())
	if ([AppDelegate runningPreIOS7])
		cell.textLabel.font = PRE_IOS7_FONT;

        cell.textLabel.text = convertString(sub.title());
    }
    return cell;
    
}

- (void)feedbackTapped:(id)sender
{
    [[AppDelegate sharedDelegate] sendFeedback];
}

- (void)creditsTapped:(id)sender
{
    static shared_ptr<CocoaTouchWebWindow> webWindow;
    webWindow = shared_ptr<CocoaTouchWebWindow>(new CocoaTouchWebWindow(nullptr));
    
    NSBundle *bundle = [NSBundle mainBundle];
    NSURL *url = [bundle URLForResource:@"about" withExtension:@"html"];
    NSString *string = [NSString stringWithContentsOfURL:url encoding:NSUTF8StringEncoding error:nil];
    string = [string stringByReplacingOccurrencesOfString:@"==VERSION==" withString:[NSString stringWithFormat:@"v%@ (%@)", [bundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"], [bundle objectForInfoDictionaryKey:@"CFBundleVersion"]]];
    NSError *err;
    NSURL *processedUrl = [[[NSFileManager defaultManager] URLForDirectory:NSCachesDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil] URLByAppendingPathComponent:@"about.html"];
    BOOL success = [string writeToURL:processedUrl atomically:YES encoding:NSUTF8StringEncoding error:&err];
    if (success) {
        webWindow->loadUrl(convertString([processedUrl absoluteString]));
        webWindow->show("");
    }
}

- (PreferenceGroup) subgroupAtIndexPath: (NSIndexPath*)indexPath
{
    auto prefGroup = _preferenceGroup.at(indexPath.section);
    return prefGroup.subgroups().at(indexPath.row - prefGroup.preferences().size());
}


- (NSString *) tableView: (UITableView *) tableView titleForHeaderInSection: (NSInteger) section
{
    if (section == _preferenceGroup.size()) {
        return @"About";
    }
        
    return convertString(_preferenceGroup.at(section).title());
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    if ([tableView cellForRowAtIndexPath: indexPath].selectionStyle == UITableViewCellSelectionStyleNone) {
		return;
    }
    auto prefGroup = _preferenceGroup.at(indexPath.section);

    if (indexPath.row < prefGroup.preferences().size())
    {
        Preference pref = prefGroup.preferences().at(indexPath.row);
        if (pref.type() == Preference::Type::Special && pref.key() == "lastfm") {
            [self.navigationController presentViewController:[[LastFmSettings alloc] init] animated:YES completion:nil];
        } else if (pref.type() == Preference::Type::Options) {
            UIActionSheet *options = [[UIActionSheet alloc]
                                      initWithTitle:convertString(pref.title())
                                      delegate:self cancelButtonTitle:nil
                                      destructiveButtonTitle:nil
                                      otherButtonTitles: nil];
            options.actionSheetStyle = UIActionSheetStyleBlackOpaque;
            
            int i = 0;
            actionSheetSelect.clear();
            for (auto &option : pref.options()) {
                
                NSString *title = convertString(option);
                /*while ([title length] < 15) {
                    title = [title stringByAppendingString:@" "];
                }*/
                if (i == pref.value()) {
                    title = [NSString stringWithFormat:@"   %@ ✓",title];
                }
                [options addButtonWithTitle:title];
                actionSheetSelect.push_back([i,pref]() mutable {
                    pref.setValue(i);
                });
                i++;
            }
            //[self addIntents:playlistIntents to:options];
            [options showInView: [UIApplication.sharedApplication.keyWindow.subviews lastObject]];
        }
    } else {
        SettingsViewController *settingsController = [[self storyboard] instantiateViewControllerWithIdentifier:@"settings"];
        settingsController.preferenceGroup = prefGroup.subgroups();
        
        [[SongsViewController sharedController] performTransition:^{
            
            [self.navigationController pushViewController:settingsController animated:YES];
        }];
    }
    
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (buttonIndex < actionSheetSelect.size()) {
        actionSheetSelect.at(buttonIndex)();
    }
}

/*
- (void)viewDidLoad
{
	self.navigationController.navigationBarHidden = NO;
	[self.navigationItem setHidesBackButton: YES];
    [super viewDidLoad];
    
}


- (void) viewWillDisappear: (BOOL) animated {
	self.navigationController.navigationBarHidden = YES;
	[super viewWillDisappear: animated];
}
*/
- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)viewDidUnload
{
    [super viewDidUnload];
}

- (IBAction)swipe:(id)sender
{
    NSUInteger n = self.navigationController.viewControllers.count;
    if (n >= 2 && ![self.navigationController.viewControllers[n - 2]
         isKindOfClass:[SettingsViewController class]]) {
        return;
    }
    
    [[SongsViewController sharedController] performTransition:^{
        
        [self.navigationController popViewControllerAnimated:YES];
    }];
}

@end
