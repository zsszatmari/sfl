//
//  Bridge.m
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 01/04/14.
//
//

#import "Bridge.h"

@implementation Bridge {
    NSMutableDictionary *objects;
    NSDate *lastSet;
}

// TODO: forget after timeout (1min)

- (id)init
{
    self = [super init];
    if (self) {
        objects = [NSMutableDictionary dictionary];
        [NSTimer scheduledTimerWithTimeInterval:10*60 target:self selector:@selector(purge:) userInfo:nil repeats:YES];
    }
    return self;
}

- (void)purge:(id)timer
{
    @synchronized(objects) {
        if ([lastSet timeIntervalSinceNow] < - 10*60) {
            [objects removeAllObjects];
        }
    }
}

+ (Bridge *)sharedBridge
{
    static Bridge *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[Bridge alloc] init];
    });
    return instance;
}

- (NSString *)registerObject:(id)object
{
    if (!object) {
        return nil;
    }
    CFUUIDRef uuid = CFUUIDCreate(NULL);
    CFStringRef str = CFUUIDCreateString(NULL, uuid);
    CFRelease(uuid);
    NSString *identifier = CFBridgingRelease(str);
    @synchronized(objects) {
        [objects setObject:object forKey:identifier];
        lastSet = [NSDate date];
    }
    return identifier;
}

- (id)objectFor:(NSString *)identifier
{
    if (!identifier) {
        return nil;
    }
    @synchronized(objects) {
        return [objects objectForKey:identifier];
    }
}

@end
