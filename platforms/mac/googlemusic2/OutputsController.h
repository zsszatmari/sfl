//
//  OutputsController.h
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 28/01/14.
//
//

#import <Foundation/Foundation.h>

@interface OutputsController : NSViewController<NSPopoverDelegate,NSWindowDelegate>

+ (OutputsController *)sharedController;
- (void)show:(NSButton *)sender;

@end
