//
//  PlaybackWorker.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 11/13/12.
//
//

#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#include "stdplus.h"

namespace Gear
{
    class PlaybackController;
    class PlaybackWorker;
}
    
@interface PlaybackWorkerImpl : NSObject {
}

@property (nonatomic, assign) MEMORY_NS::shared_ptr<Gear::PlaybackController> controller;

- (id)initWithWorker:(const MEMORY_NS::shared_ptr<Gear::PlaybackWorker> &)worker;
- (void)play;
- (void)stop:(BOOL)release;
- (BOOL)playing;
+ (AudioStreamBasicDescription)usedPCMFormat;
#if !TARGET_OS_IPHONE
- (long long)defaultDeviceId;
- (BOOL)resetVolume;
#endif
- (void)setVolume:(float)volume;
#if !TARGET_OS_IPHONE
- (void)selectDeviceId:(unsigned long long)deviceId;
#endif

@end
