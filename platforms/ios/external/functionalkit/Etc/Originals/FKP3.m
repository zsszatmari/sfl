#import "FKP3.h"
#import <objc/runtime.h>


@interface FKP3 (private)
@property (strong) id _1;
@property (strong) id _2;
@property (strong) id _3;
@end

@implementation FKP3

- (id)initWith_1:(id)new_1 _2:(id)new_2 _3:(id)new_3 {
    if (self = [super init]) {
        __1 = new_1;
        __2 = new_2;
        __3 = new_3;
    }
    return self;
}

- (NSString *)description {
	return [NSString stringWithFormat:@"<%s: _1=%@ _2=%@ _3=%@>", class_getName([self class]), __1, __2, __3];
}

#pragma mark NSObject methods.
- (BOOL)isEqual:(id)object {
    if (object == nil || ![[object class] isEqual:[self class]]) {
        return NO;
    } else {
        FKP3 *other = (FKP3 *)object;
		return [__1 isEqual:other._1] && [__2 isEqual:other._2] && [__3 isEqual:other._3];
    }
}

- (NSUInteger)hash {
    return [__1 hash] + [__2 hash] + [__3 hash];
}
@end
