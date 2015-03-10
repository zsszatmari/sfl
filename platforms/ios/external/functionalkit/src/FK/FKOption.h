#import <Foundation/Foundation.h>
#import "FK/FKEither.h"
#import "FK/FKMacros.h"

// An optional value that may be none (no value) or some (a value). This type is a replacement for the use of nil to denote non-existence.
@interface FKOption : NSObject

@property (readonly) id some;

@property (readonly) BOOL isNone;
@property (readonly) BOOL isSome;

+ (FKOption *)fromNil:(id)maybeNil;

+ (FKOption *)fromNil:(id)maybeNil ofType:(Class)cls;

+ (FKOption *)none;

+ (FKOption *)some:(id)someObject;

// Takes an array of options, and returns an array of all the Some values
+ (NSArray *)somes:(NSArray *)options;

// TODO Add identity function, then do concat.
// Concatenates an option containing an option into an option. Will fail if any item is not an FKOption.
// concat :: FKOption[FKOption[a]] -> FKOption[a]
//+ (FKOption *)concat:(FKOption *)nested;

// Returns this optional value if there is one, otherwise, returns the argument optional value.
- (FKOption *)orElse:(FKOption *)other;

// Returns the value in the some of this option or if none, the given argument.
- (id)orSome:(id)some;

// Returns the value in the some of this option or if none, nil
- (id)orNil;

// Maps the given function across the option
- (FKOption *)map:(id (^)(id))f;

// Binds the given function across the projection.
// f should be a fucntion with the following type: a -> FKOption[b].
- (FKOption *)bind:(FKOption *(^)(id))f;

- (FKOption *)filter:(BOOL (^)(id))f;

// Returns an either projection of this optional value; |left| in a Left if this optional holds no value, or this optional's value in Right.
- (FKEither *)toEither:(id)left;

// Returns an either projection of this optional value; an NSError in the Left if this optional holds no value, or this optional's value in Right.
- (FKEither *)toEitherWithError:(NSString *)reason;

// Side-effects on some if some;
- (void)foreach:(void (^)(id))effect;

@end
