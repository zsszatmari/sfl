#import "FKP1.h"
#import <objc/runtime.h>


@interface FKP1 (private)
@property (strong) id _1;
@end

@implementation FKP1

- (id)initWith_1:(id)new_1 {
    if (self = [super init]) {
        __1 = new_1;
    }
    return self;
}

- (NSString *)description {
	return [NSString stringWithFormat:@"<%s: _1=%@>", class_getName([self class]), __1];
}

#pragma mark NSObject methods.
- (BOOL)isEqual:(id)object {
    if (object == nil || ![[object class] isEqual:[self class]]) {
        return NO;
    } else {
        FKP1 *other = (FKP1 *)object;
		return [__1 isEqual:other._1];
    }
}

- (NSUInteger)hash {
    return [__1 hash];
}
@end
