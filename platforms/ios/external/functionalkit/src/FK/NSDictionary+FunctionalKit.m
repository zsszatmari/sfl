#import "NSDictionary+FunctionalKit.h"
#import "FKP2.h"

@implementation NSDictionary (FunctionalKitExtensions)

- (NSArray *)toArray {
    NSMutableArray *pairs = [NSMutableArray arrayWithCapacity:[self count]];
    for (id key in self) {
        [pairs addObject:[[FKP2 alloc] initWith_1:key _2:[self objectForKey:key]]];
    }    
    return [NSArray arrayWithArray:pairs];
}

- (FKOption *)maybeObjectForKey:(id)key {
    return [FKOption fromNil:[self objectForKey:key]];
}

@end
