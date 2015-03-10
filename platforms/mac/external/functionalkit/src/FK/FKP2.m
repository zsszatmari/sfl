#import "FKP2.h"
#import <objc/runtime.h>


@interface FKP2 (private)
@property (strong) id _1;
@property (strong) id _2;
@end

@implementation FKP2

- (id)initWith_1:(id)new_1 _2:(id)new_2 {
    if (self = [super init]) {
        __1 = new_1;
        __2 = new_2;
    }
    return self;
}

+ (id)_1:(id)new_1 _2:(id)new_2 {
  return [[self alloc] initWith_1:(id)new_1 _2:(id)new_2];
}

- (NSString *)description {
	return [NSString stringWithFormat:@"<%s: _1=%@ _2=%@>", class_getName([self class]), __1, __2];
}

#pragma mark NSObject methods.
- (BOOL)isEqual:(id)object {
    if (object == nil || ![[object class] isEqual:[self class]]) {
        return NO;
    } else {
        FKP2 *other = (FKP2 *)object;
		return [__1 isEqual:other._1] && [__2 isEqual:other._2];
    }
}

- (NSUInteger)hash {
    return [__1 hash] + [__2 hash];
}
@end
