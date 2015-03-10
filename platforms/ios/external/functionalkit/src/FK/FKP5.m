#import "FKP5.h"
#import <objc/runtime.h>


@interface FKP5 (private)
@property (strong) id _1;
@property (strong) id _2;
@property (strong) id _3;
@property (strong) id _4;
@property (strong) id _5;
@end

@implementation FKP5

- (id)initWith_1:(id)new_1 _2:(id)new_2 _3:(id)new_3 _4:(id)new_4 _5:(id)new_5 {
    if (self = [super init]) {
        __1 = new_1;
        __2 = new_2;
        __3 = new_3;
        __4 = new_4;
        __5 = new_5;
    }
    return self;
}

+ (id)_1:(id)new_1 _2:(id)new_2 _3:(id)new_3 _4:(id)new_4 _5:(id)new_5 {
  return [[self alloc] initWith_1:(id)new_1 _2:(id)new_2 _3:(id)new_3 _4:(id)new_4 _5:(id)new_5];
}

- (NSString *)description {
	return [NSString stringWithFormat:@"<%s: _1=%@ _2=%@ _3=%@ _4=%@ _5=%@>", class_getName([self class]), __1, __2, __3, __4, __5];
}

#pragma mark NSObject methods.
- (BOOL)isEqual:(id)object {
    if (object == nil || ![[object class] isEqual:[self class]]) {
        return NO;
    } else {
        FKP5 *other = (FKP5 *)object;
		return [__1 isEqual:other._1] && [__2 isEqual:other._2] && [__3 isEqual:other._3] && [__4 isEqual:other._4] && [__5 isEqual:other._5];
    }
}

- (NSUInteger)hash {
    return [__1 hash] + [__2 hash] + [__3 hash] + [__4 hash] + [__5 hash];
}
@end
