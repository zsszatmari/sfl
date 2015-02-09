//
//  ShareManager.h
//  G-Ear
//
//  Created by Zsolt Szatmari on 7/15/12.
//
//

#import <Foundation/Foundation.h>
#include "ISong.h"

@class SongAbstract;

@interface ShareManager : NSObject<NSSharingServicePickerDelegate>;

@property(assign, nonatomic) std::shared_ptr<Gear::ISong> song;

- (void)shareFromView:(NSView *)view;

@end
