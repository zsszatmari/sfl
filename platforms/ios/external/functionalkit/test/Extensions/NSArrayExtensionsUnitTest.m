#import <SenTestingKit/SenTestingKit.h>
#import "NSArray+FunctionalKit.h"
#import "FKP2.h"

@interface NSArrayExtensionsUnitTest : SenTestCase
@end

@implementation NSArrayExtensionsUnitTest

- (void)testCanGetTheHeadOfAnArray {
    NSArray *source = @[@"1", @"2", @"3", @"4"];
    STAssertEqualObjects(@"1", source.head, nil);
}

- (void)testCanGetTheTailOfAnArray {
    NSArray *source = @[@"1", @"2", @"3", @"4"];
    NSArray *expected = @[@"2", @"3", @"4"];
    STAssertEqualObjects(expected, source.tail, nil);
}

- (void)testCanGetASpanMatchingAPredicate {
    NSArray *source = @[@"1", @"1", @"2", @"4"];
    FKP2 *span = [source span:^(id v) { return [self isStringContainingOne:v]; }];
    FKP2 *expected = [[FKP2 alloc] initWith_1:@[@"1", @"1"] _2:@[@"2", @"4"]];
    STAssertEqualObjects(expected, span, nil);
}

- (void)testCanTestAPredicateAgainstAllItems {
    NSArray *source = @[@"1", @"1"];
    BOOL allOnes = [source all:^(id v) { return [self isStringContainingOne:v]; }];
    STAssertTrue(allOnes, nil);
}

- (void)testCanFilterUsingAPredicate {
    NSArray *source = @[@"1", @"1", @"2", @"1"];
    NSArray *onlyOnes = [source filter:^(id v) { return [self isStringContainingOne:v]; }];
    NSArray *expected = @[@"1", @"1", @"1"];
    STAssertEqualObjects(expected, onlyOnes, nil);
}

- (void)testCanGroupItemsUsingAKeyFunctionIntoADictionary {
    NSArray *source = @[@"1", @"1", @"2", @"1", @"3", @"3", @"4"];
    NSDictionary *grouped = [source groupByKey:^(id v) { return [v description]; }];
    NSDictionary *expected = @{@"1": @[@"1", @"1", @"1"],@"2":@[@"2"], @"3": @[@"3", @"3"], @"4": @[@"4"]};
    STAssertEqualObjects(expected, grouped, nil);
}

- (void)testCanMapAFunctionAcrossAnArray {
    id (^foo)(id) = ^(id v) { return [v uppercaseString]; };
	STAssertEqualObjects([@[@"test"] map:foo], @[@"TEST"], nil);
}

- (void)testCanCreateANewArrayByConcatenatingAnotherOne {
    NSArray *source = @[@[@"1", @"2"], @[@"3", @"4"]];
    NSArray *expected = @[@"1", @"2", @"3", @"4"];
    STAssertEqualObjects(expected, [NSArray concat:source], nil);
}

- (void)testConcatFailsOnNonArray {
    NSArray *source = @[@[@"1", @"2"], @"3"];
    @try {
        [NSArray concat:source];
        STFail(@"Expected concat to fail with no-array argument", nil);
    }
    @catch (NSException * e) {
        // expected
    }
}

- (void)testCanLiftAFunctionIntoAnArray {
    NSArray *array = @[@"a", @"b", @"c"];
    id (^liftedF)(id) = [NSArray liftFunction:^(id v) { return [v uppercaseString]; }];
    NSArray *expected = @[@"A", @"B", @"C"];
    STAssertEqualObjects(expected, liftedF(array), nil);
}

- (void)testCanIntersperseAnObjectWithinAnArray {
    NSArray *array = @[@"A", @"B", @"C"];
    NSArray *expected = @[@"A", @",", @"B", @",", @"C"];
    STAssertEqualObjects(expected, [array intersperse:@","], nil);
}

- (void)testCanFoldAcrossAnArray {
    NSArray *array = @[@"A", @"B", @"C"];
    STAssertEqualObjects(@"ABC", [array foldLeft:@"" f:^(NSString *a, NSString *b) { return [a stringByAppendingString:b]; }], nil);
}

- (void)testCanReverseAnArray {
    NSArray *array = @[@"A", @"B", @"C"];
    NSArray *expected = @[@"C", @"B", @"A"];
    STAssertEqualObjects(expected, [array reverse], nil);
}
//
//- (void)testCanUniquifyAnArray {
//    NSArray *array = @[@"A", @"B", @"C", @"C", @"A", @"A", @"B"];
//    STAssertEqualObjects(NSARRAY(@"A", @"B", @"C", [array unique], nil);
//}

- (void)testAnyReturnsTrue {
    BOOL (^pred)(id) = ^(id v) { return [v boolValue]; };
    NSArray *a = @[@1,@NO,@2];
    STAssertTrue([a any:pred], nil);
    a = @[@NO,@NO,@NO,@1];
    STAssertTrue([a any:pred], nil);
    a = @[@2,@1,@5,@1];
    STAssertTrue([a any:pred], nil);
}

- (void)testAnyReturnsFalse {
    BOOL (^pred)(id) = ^(id v) { return [v boolValue]; };
    NSArray *array = @[@NO, @NO, @NO];
    STAssertFalse([array any:pred], @"array %@ should return false");
}

- (void)testDropKeepsFalseEvaluations {
    NSArray *a = @[@1,@NO,@2];
    BOOL (^pred)(id) = ^(id v) { return [v boolValue]; };

    STAssertEqualObjects(@[@NO], [a drop:pred], nil);
}

- (void)testDropReturnsEmpty {
    NSArray *a = @[@1,@2,@3];
    BOOL (^pred)(id) = ^(id v) { return [v boolValue]; };

    STAssertEqualObjects(@[], [a drop:pred], nil);
}

- (void)testTakeTooMany {
    NSArray *a = @[@1,@2,@3];
    STAssertEquals(3U, [[a take:4] count], nil);
}

- (void)testTakeReturnsCorrectSizedArray {
    NSArray *a = @[@1,@2,@3];
    STAssertEquals(0U, [[a take:0] count], nil);
    STAssertEquals(1U, [[a take:1] count], nil);
    STAssertEquals(2U, [[a take:2] count], nil);
    STAssertEquals(3U, [[a take:3] count], nil);
}

- (void)testTakeFromEmptyArray {
    NSArray *a = @[];
    STAssertEquals(0U, [[a take:42] count], nil);
}

- (BOOL)isStringContainingOne:(id)string {
    return [string isEqual:@"1"];
}

- (void)testZip
{
    NSArray *left = @[@"a",@"b",@"c"];
    NSArray *right = @[@1,@2];
    NSArray *zipped = [left zip:right];
    STAssertEquals((NSInteger)[zipped count], (NSInteger)2, nil);
    STAssertEqualObjects([zipped[0] _1], @"a", nil);
    STAssertEqualObjects([zipped[0] _2], @1, nil);
    STAssertEqualObjects([zipped[1] _1], @"b", nil);
    STAssertEqualObjects([zipped[1] _2], @2, nil);
}

- (void)testReplicate
{
    NSArray *replicated = [NSArray replicate:2 x:@"x"];
    STAssertEquals((NSInteger)[replicated count], (NSInteger)2, nil);
    STAssertEqualObjects(replicated[0], @"x", nil);
    STAssertEqualObjects(replicated[1], @"x", nil);
}

- (void)testBind
{
    NSArray *bound = [@[@0,@2,@1] bind:^(NSNumber *n){return [NSArray replicate:[n intValue] x:n];}];
    STAssertEquals((NSInteger)[bound count], (NSInteger)3, nil);
    STAssertEqualObjects(bound[0], @2, nil);
    STAssertEqualObjects(bound[1], @2, nil);
    STAssertEqualObjects(bound[2], @1, nil);
}

@end
