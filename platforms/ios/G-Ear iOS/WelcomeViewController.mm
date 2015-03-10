//
//  WelcomeViewController.m
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/5/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "WelcomeViewController.h"
#import "AppDelegate.h"
#include "App.h"
#include "ISession.h"
#include "ServiceManager.h"
#include "IService.h"
#include "SessionManager.h"

using namespace Gear;

@implementation WelcomeViewController {
    SignalConnection connectedConnection;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    connectedConnection = App::instance()->sessionManager()->connectedEvent().connect([=] {
        
        [self performSegueWithIdentifier:@"showSongs" sender:nil];
    });
    [AppDelegate sharedDelegate].topViewController = self.navigationController;
}

- (IBAction)connectGoogleTapped:(id)sender
{
    IApp::instance()->serviceManager()->googlePlayService()->connect();
}

- (void)viewDidUnload
{
    [self setConnectButton:nil];
    [super viewDidUnload];
}

@end
