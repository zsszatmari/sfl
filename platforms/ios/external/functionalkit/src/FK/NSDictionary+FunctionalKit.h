#import <Foundation/Foundation.h>
#import "FK/FKOption.h"

@interface NSDictionary (FunctionalKitExtensions)

// Projects this dictionary as an array of key/value pairs: NSArray[FKP2[id,id]].
- (NSArray *)toArray;

// Returns an optional value for the given |key|. None if the object for this key is nil, Some with the value if it is non-nil.
// maybeObjectForKey :: id -> FKOption[id]
- (FKOption *)maybeObjectForKey:(id)key;

@end
