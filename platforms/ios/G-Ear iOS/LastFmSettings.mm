//
//  LastFmSettings.m
//  G-Ear iOS
//
//  Created by Zsolt Szatmari on 17/07/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#import "LastFmSettings.h"
#include "SignalConnection.h"
#include "IApp.h"
#include "LastFmController.h"
#include "StringUtility.h"
#import "AppDelegate.h"

using namespace Gear;
using namespace Base;

@implementation LastFmSettings {
    UILabel *text;
    UISwitch *onOff;
    
    SignalConnection messageConnection;
    SignalConnection checkboxConnection;
}

- (instancetype)init
{
    self = [super initWithNibName:nil bundle:nil];
    if (self) {
    }
    return self;
}

- (void)loadView
{
    self.view = [[UIView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    [self.view setBackgroundColor:[UIColor colorWithRed:239.0f/255.0f green:238.0f/255.0f blue:244.0f/255.0f alpha:1.0f]];
    
    UIView *topBack = [[UIView alloc] initWithFrame:CGRectMake(0,0,self.view.bounds.size.width,56)];
    topBack.layer.borderWidth = 1;
    topBack.layer.borderColor = [UIColor colorWithRed:221.0f/255.0f green:220.0f/255.0f blue:223.0f/255.0f alpha:1.0f].CGColor;
    [topBack setBackgroundColor:[UIColor whiteColor]];
    [self.view addSubview:topBack];
    
    UIView *onOffBack = [[UIView alloc] initWithFrame:CGRectMake(0, 103-17, self.view.bounds.size.width, 20+2*17)];
    onOffBack.layer.borderWidth = 1;
    onOffBack.layer.borderColor = [UIColor colorWithRed:221.0f/255.0f green:220.0f/255.0f blue:223.0f/255.0f alpha:1.0f].CGColor;
    [onOffBack setBackgroundColor:[UIColor whiteColor]];
    [self.view addSubview:onOffBack];
    
    UIButton *cancel_butt = [UIButton buttonWithType: UIButtonTypeCustom];
    [cancel_butt setTitleColor: [UIColor colorWithRed: 66. / 255. green: 127. / 255. blue: 237. / 255. alpha: 1.] forState: UIControlStateNormal];
    cancel_butt.frame = CGRectMake(0, 17, 80, 22);
    [cancel_butt setTitle: @"Back" forState: UIControlStateNormal];
    cancel_butt.titleLabel.font = [UIFont systemFontOfSize:15.0f];
    [cancel_butt addTarget:self action: @selector(cancel_action:) forControlEvents: UIControlEventTouchUpInside];
    [self.view addSubview: cancel_butt];

    UILabel *title = [[UILabel alloc] initWithFrame:CGRectMake(80,17,self.view.bounds.size.width - 2*80, 22)];
    title.text = @"Last.fm";
    title.textAlignment = NSTextAlignmentCenter;
    title.font = [UIFont boldSystemFontOfSize:17.0f];

    [self.view addSubview:title];
    
    
    UILabel *onOffText = [[UILabel alloc] initWithFrame:CGRectMake(20, 103, 200, 20)];
    [onOffText setText:@"Enable Last.fm scrobbling"];
    [self.view addSubview:onOffText];
    onOff = [[UISwitch alloc] initWithFrame:CGRectMake(self.view.bounds.size.width - 70, 97, 20, 20)];
    if ([AppDelegate runningPreIOS7]) {
        onOff = [[UISwitch alloc] initWithFrame:CGRectMake(self.view.bounds.size.width - 90, 97, 20, 20)];
    }
    [self.view addSubview:onOff];
    
    text = [[UILabel alloc] initWithFrame:CGRectMake(20, 162, self.view.bounds.size.width - 40, 200)];
    text.lineBreakMode = NSLineBreakByWordWrapping;
    text.numberOfLines = 0; // unlimited
    text.textColor = [UIColor colorWithWhite:103.0f/255.0f alpha:1.0f];
    text.font = [UIFont systemFontOfSize:13.0f];
    text.backgroundColor = [UIColor clearColor];
    [self.view addSubview:text];
    
    [onOff addTarget:self action:@selector(onOffPushed:) forControlEvents:UIControlEventValueChanged];
    auto controller = IApp::instance()->lastFmController();
    messageConnection = controller->messageConnector().connect([=](const std::string &value){
        
        NSString *str = convertString(value);
        [text setText:str];
        CGSize size = [str sizeWithFont:text.font constrainedToSize:CGSizeMake(self.view.bounds.size.width-40, 300) lineBreakMode:text.lineBreakMode];
        [text setFrame:CGRectMake(text.frame.origin.x,text.frame.origin.y,ceil(size.width),ceil(size.height))];
    });
    checkboxConnection = controller->enabledConnector().connect([=](bool value){
        
        [onOff setOn:value animated:YES];
    });
}

- (void) cancel_action:(id)sender
{
    [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)onOffPushed:(UISwitch *)sender
{
    if (sender.on) {
        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"Last.fm" message:@"Please enter your credentials" delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"Log In", nil];
        [alertView setAlertViewStyle:UIAlertViewStyleLoginAndPasswordInput];
        [alertView show];
    } else {
        auto controller = IApp::instance()->lastFmController();
        controller->setEnabled(false);
    }
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (buttonIndex == 1) {
        auto controller = IApp::instance()->lastFmController();
        controller->loginMobile(convertString([[alertView textFieldAtIndex:0] text]),convertString([[alertView textFieldAtIndex:1] text]));

    } else {
        [onOff setOn:NO animated:NO];
    }
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
