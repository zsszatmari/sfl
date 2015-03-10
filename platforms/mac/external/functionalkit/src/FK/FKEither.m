#import "FK/FKEither.h"
#import "FK/FKOption.h"

NSString *FKFunctionalKitErrorDomain = @"FunctionalKit";

@interface FKEither ()
@property (strong) id value;
@property (assign) BOOL isRight;
@end

@implementation FKEither

//@synthesize value = _value, isRight = _isRight;

- (id)valueOrFailWithMessage:(NSString *)errorMessage {
    if (_isRight) {
        return _value;
    } else {
        @throw [NSException exceptionWithName:NSInvalidArgumentException reason:errorMessage userInfo:@{}];
    }
}

- (FKEither *)map:(id (^)(id))f {
	return _isRight ? [FKEither rightWithValue:f(_value)] : self;
}

- (FKEither *)bind:(FKEither *(^)(id))f {
	return _isRight ? f(_value) : self;
}

- (void)foreach:(void (^)(id))effect {
    if (_isRight) {
        effect(_value);
    }
}

- (FKOption *)toOption {
	return _isRight? [FKOption some:_value] : [FKOption none];
}

- (id)valueOr:(id)value {
	return _isRight ? _value : value;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"<FKEither %@ value: %@>", _isRight ? @"right" : @"left", _value];
}

+ (FKEither *)leftWithValue:(id)value {
    return [[FKEither alloc] initWithValue:value isRight:NO];
}

+ (FKEither *)rightWithValue:(id)value {
    return [[FKEither alloc] initWithValue:value isRight:YES];
}

+ (FKEither *)errorWithReason:(NSString *)reason {
    NSDictionary *userInfo = [NSDictionary dictionaryWithObject:reason forKey:NSLocalizedFailureReasonErrorKey];
    return [FKEither leftWithValue:[NSError errorWithDomain:FKFunctionalKitErrorDomain code:0 userInfo:userInfo]];
}

+ (FKEither *)errorWithReason:(NSString *)reason description:(NSString *)description {
    NSDictionary *userInfo = [NSDictionary dictionaryWithObjectsAndKeys:reason, NSLocalizedFailureReasonErrorKey, description, NSLocalizedDescriptionKey, nil];
    return [FKEither leftWithValue:[NSError errorWithDomain:FKFunctionalKitErrorDomain code:0 userInfo:userInfo]];
}

+ (FKEither *)errorWithReason:(NSString *)reason underlyingError:(NSError *)error {
    NSDictionary *userInfo = [NSDictionary dictionaryWithObjectsAndKeys:reason, NSLocalizedFailureReasonErrorKey, error, NSUnderlyingErrorKey, nil];
    return [FKEither leftWithValue:[NSError errorWithDomain:FKFunctionalKitErrorDomain code:0 userInfo:userInfo]];
}

- (BOOL)isLeft {
    return !_isRight;
}

- (FKEither *)swap {
    return [[FKEither alloc] initWithValue:_value isRight:!_isRight];
}

#pragma mark NSObject methods.
- (BOOL)isEqual:(id)object {
    if (object == nil || ![[object class] isEqual:[self class]]) {
        return NO;
    } else {
        FKEither *other = (FKEither *) object;
        return (self.isRight == other.isRight && [_value isEqual:other->_value]);
    }
}

- (NSUInteger)hash {
    return [_value hash];
}

#pragma mark Private methods.
- (FKEither *)initWithValue:(id)newValue isRight:(BOOL)newIsRight {
    if (self = [super init]) {
        _value = newValue;
        _isRight = newIsRight;
    }
    return self;
}

@end
