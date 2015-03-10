#import "FK/FKOption.h"
#import <objc/runtime.h>

@interface FKNone : FKOption
@end

@interface FKSome : FKOption

@property (strong) id someObject;
@end

@implementation FKNone

- (BOOL)isSome {
    return false;
}

#pragma mark NSObject methods.
- (BOOL)isEqual:(id)object {
    return object == nil || ![[object class] isEqual:[self class]] ? NO : YES;
}

- (NSUInteger)hash {
    return 42;
}

- (NSString *)description {
    return @"<FKNone>";
}

- (id)some {
    NSString *message = @"Attempt to access some but this is None";
    @throw [NSException exceptionWithName:NSInvalidArgumentException reason:message userInfo:@{}];
}

@end

@interface FKSome (FKSomePrivate)
- (FKSome *)initWithSome:(id)someObject;
@end

@implementation FKSome

- (id)some {
    return _someObject;
}

- (BOOL)isSome {
    return true;
}

#pragma mark NSObject methods.
- (BOOL)isEqual:(id)object {
    return object == nil || ![[object class] isEqual:[self class]] ? NO : [self.some isEqual:((FKSome *) object).some];
}

- (NSUInteger)hash {
    return [self.some hash];
}

- (NSString *)description {
    return [NSString stringWithFormat:@"<%s some: %@>", class_getName([self class]), self.some];
}

#pragma mark Private methods.
- (FKSome *)initWithSome:(id)newSomeObject {
    if ((self = [super init])) {
        _someObject = newSomeObject;
    }
    return self;
}

@end

@implementation FKOption

+ (FKOption *)fromNil:(id)maybeNil {
    return maybeNil == nil ? [FKOption none] : [FKOption some:maybeNil];
}

+ (FKOption *)fromNil:(id)maybeNil ofType:(Class)cls {
	//TODO add bind and re-use fromNil
	return (maybeNil != nil && [maybeNil isKindOfClass:cls]) ? [FKOption some:maybeNil] : [FKOption none];
}

+ (FKOption *)none {
    return [[FKNone alloc] init];
}

+ (FKOption *)some:(id)someObject {
    return [[FKSome alloc] initWithSome:someObject];
}

+ (NSArray *)somes:(NSArray *)options {
	NSMutableArray *result = [NSMutableArray array];
	for (FKOption *o in options) {
        [o foreach: ^(id v) {
            [result addObject:v];
        }];
	}
	return [NSArray arrayWithArray:result];
}

- (BOOL)isNone {
    return ![self isSome];
}

- (FKOption *)orElse:(FKOption *)other {
    return self.isSome ? self : other;
}

- (id)orSome:(id)some {
    return self.isSome ? [self some] : some;
}

- (FKOption *)map:(id (^)(id))f {
	return self.isSome ? [FKOption some:f(self.some)] : self;
}

- (FKOption *)bind:(FKOption *(^)(id))f {
	return self.isSome ? f(self.some) : self;
}

- (FKOption *)filter:(BOOL (^)(id))f {
    return (self.isSome && f(self.some)) ? self : [FKOption none];
}

- (FKEither *)toEither:(id)left {
	return self.isSome ? [FKEither rightWithValue:self.some] : [FKEither leftWithValue:left];
}

- (FKEither *)toEitherWithError:(NSString *)reason {
	return self.isSome ? [FKEither rightWithValue:self.some] : [FKEither errorWithReason:reason];
}

- (void)foreach:(void (^)(id))effect {
    if (self.isSome) {
        effect(self.some);
    }
}

@end
