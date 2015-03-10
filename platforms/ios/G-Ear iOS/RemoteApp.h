//
//  RemoteApp.h
//  G-Ear iOS
//
//  Created by Szabo Attila on 10/20/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>
@protocol remote_control_delegate <NSObject>
- (void) remote_responder: (UIEvent *) event;
@end

@interface RemoteApp : UIApplication

+ (void) set_remote: (id) r;

@end
