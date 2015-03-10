#import "FKP4.h"
#import <objc/runtime.h>


@interface FKP4 (private)
@property (strong) id _1;
@property (strong) id _2;
@property (strong) id _3;
@property (strong) id _4;
@end

@implementation FKP4

- (id)initWith_1:(id)new_1 _2:(id)new_2 _3:(id)new_3 _4:(id)new_4 {
    if (self = [super init]) {
        __1 = new_1;
        __2 = new_2;
        __3 = new_3;
        __4 = new_4;
    }
    return self;
}

+ (id)_1:(id)new_1 _2:(id)new_2 _3:(id)new_3 _4:(id)new_4 {
  return [[self alloc] initWith_1:(id)new_1 _2:(id)new_2 _3:(id)new_3 _4:(id)new_4];
}

- (NSString *)description {
	return [NSString stringWithFormat:@"<%s: _1=%@ _2=%@ _3=%@ _4=%@>", class_getName([self class]), __1, __2, __3, __4];
}

#pragma mark NSObject methods.
- (BOOL)isEqual:(id)object {
    if (object == nil || ![[object class] isEqual:[self class]]) {
        return NO;
    } else {
        FKP4 *other = (FKP4 *)object;
		return [__1 isEqual:other._1] && [__2 isEqual:other._2] && [__3 isEqual:other._3] && [__4 isEqual:other._4];
    }
}

- (NSUInteger)hash {
    return [__1 hash] + [__2 hash] + [__3 hash] + [__4 hash];
}
@end
