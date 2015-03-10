#import <SenTestingKit/SenTestingKit.h>
#import "NSDictionary+FunctionalKit.h"
#import "FKP2.h"

@interface NSDictionaryExtensions : SenTestCase
@end

@implementation NSDictionaryExtensions

- (void)testDictionaryToArray {
    NSDictionary *dict = @{@"k1": @"v1", @"k2": @"v2"};
    NSArray *expected = @[[[FKP2 alloc] initWith_1:@"k2" _2:@"v2"], [[FKP2 alloc] initWith_1:@"k1" _2:@"v1"]];
    STAssertEqualObjects(expected, [dict toArray], nil);
}

- (void)testAskingForANonExistentValueReturnsANone {
    NSDictionary *dict = @{@"key" : @"value"};
    STAssertTrue([[dict maybeObjectForKey:@"not_there"] isNone] , nil);
}

- (void)testAskingForANonExistentValueReturnsASomeWithTheValue {
    NSDictionary *dict = @{@"key" : @"value"};
    FKOption *maybeValue = [dict maybeObjectForKey:@"key"];
    STAssertTrue([maybeValue isSome] , nil);
    STAssertEqualObjects(@"value", maybeValue.some, nil);
}

@end