#import <Foundation/Foundation.h>
#import "FK/FKMacros.h"

// TODO Add foreach.

// TODO Move this into the prelude.
extern NSString *FKFunctionalKitErrorDomain;

@class FKEither;
@class FKOption;

// Right-biased either projection.
@protocol FKEitherProjection <NSObject>

// Returns the value of this projection or fails (throws an NSException) with the given error message.
- (id)valueOrFailWithMessage:(NSString *)errorMessage;

// Returns the value of this projection or, if the value is on the other side, |other|.
- (id)valueOr:(id)other;

// Maps the given function across the value of the projection.
// f should be a fucntion with the following type: a -> b.
- (FKEither *)map:(id (^)(id))f;

// Binds the given function across the projection.
// f :: a -> FKEither b.
- (FKEither *)bind:(FKEither *(^)(id))f;

// Side-effects on some if some;
- (void)foreach:(void (^)(id))effect;

// Returns Some value if either is of this projection, else returns None
- (FKOption *)toOption;
@end

// The Either type represents a value of one of two possible types (a disjoint union).
// The data constructors; Left and Right represent the two possible values. The Either type is often used as an alternative to Option where Left 
// represents failure (by convention) and Right is akin to Some.
@interface FKEither : NSObject <FKEitherProjection>

-(BOOL)isLeft;
-(BOOL)isRight;

// Construct a left value of either.
+ (FKEither *)leftWithValue:(id)value;

// Construct a right value of either.
+ (FKEither *)rightWithValue:(id)value;

// Construct an NSError on the left using |reason| as the NSLocalizedFailureReasonErrorKey of the error's userInfo dictionary.
// Example:
// FKEither *maybeFailed = [FKEither errorWithReason:@"FTL"];
// NSString *ftl = [maybeFailed.left.value localizedFailureReason];
+ (FKEither *)errorWithReason:(NSString *)reason;

// Construct an NSError on the left using |reason| as the NSLocalizedFailureReasonErrorKey of the error's userInfo dictionary.
// Example:
// FKEither *maybeFailed = [FKEither errorWithReason:@"FTL" description:@"It sucks."];
// NSString *ftl = [maybeFailed.left.value localizedFailureReason];
// NSString *itSucks = [maybeFailed.left.value localizedDescription];
+ (FKEither *)errorWithReason:(NSString *)reason description:(NSString *)description;

// Construct an NSError on the left using |reason| as the NSLocalizedFailureReasonErrorKey and |error| as the NSUnderlyingErrorKey of the error's 
// userInfo dictionary.
+ (FKEither *)errorWithReason:(NSString *)reason underlyingError:(NSError *)error;

// If this is a left, then return the left value in right, or vice versa.
- (FKEither *)swap;

@end
