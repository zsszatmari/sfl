#import <SenTestingKit/SenTestingKit.h>
#import "FK/FKOption.h"
#import "FK/FKMacros.h"

@interface FKOptionUnitTest : SenTestCase {
    NSObject *object;
}
@end

@implementation FKOptionUnitTest

- (void)setUp {
    object = [[NSObject alloc] init];
}

- (void)testANoneIsNone {
    STAssertTrue([[FKOption none] isNone], nil);
    STAssertFalse([[FKOption none] isSome], nil);
}

- (void)testASomeIsSome {
    STAssertTrue([[FKOption some:object] isSome], nil);
    STAssertFalse([[FKOption some:object] isNone], nil);
}

- (void)testCanPullTheSomeValueOutOfASome {
    STAssertEqualObjects(object, [[FKOption some:object] some], nil);
}

- (void)testTransformsNilsIntoNones {
    STAssertTrue([[FKOption fromNil:nil] isNone], nil);
    STAssertTrue([[FKOption fromNil:object] isSome], nil);
}

- (void)testMaps {
	STAssertTrue([[[FKOption none] map:^(id v) { return [v description]; }] isNone], nil);
	NSString *description = [object description];
	FKOption *r = [[FKOption some:object] map:^(id v) { return [v description]; }];
	STAssertTrue([r isSome], nil);	
	STAssertEqualObjects([r some], description, nil);
}

- (void)testTypes {
	STAssertTrue([[FKOption fromNil:@"54" ofType:[NSString class]] isSome], nil);
	STAssertTrue([[FKOption fromNil:nil ofType:[NSString class]] isNone], nil);
	STAssertTrue([[FKOption fromNil:@"54" ofType:[NSArray class]] isNone], nil);
}

- (void)testBindingAcrossANoneGivesANone {
    id result = [[FKOption none] bind:^(id v) { return [FKOption none]; }];
    STAssertTrue([result isKindOfClass:[FKOption class]], nil);
    STAssertTrue([result isNone], nil);
}

- (void)testBindingAcrossASomeWithANoneGivesANone {
    id result = [[FKOption some:@"foo"] bind:^(id v) { return [FKOption none]; }];
    STAssertTrue([result isKindOfClass:[FKOption class]], nil);
    STAssertTrue([result isNone], nil);
}

- (void)testBindingAcrossASomeWithASomeGivesANone {
    id result = [[FKOption some:@"foo"] bind:^(id v) { return [FKOption some:v]; }];
    STAssertTrue([result isKindOfClass:[FKOption class]], nil);
    STAssertTrue([result isSome], nil);
    STAssertEqualObjects(@"foo", [result some], nil);
}

- (void)testSomes {
	NSArray *options = @[[FKOption some:@"54"], [FKOption none]];
	NSArray *somes = [FKOption somes:options];
	STAssertEqualObjects(@[@"54" ], somes, nil);
}

- (BOOL)isString:(id)arg {
    return [arg isKindOfClass:[NSString class]];
}

- (void)testFilter {
    FKOption *o1 = [FKOption some:[NSNumber numberWithInt:5]];
    FKOption *o2 = [FKOption some:@"Okay"];
    STAssertTrue([[[FKOption none] filter:^(id v){return [self isString:v];}] isNone], nil);
    STAssertTrue([[o1 filter:^(id v){return [self isString:v];}] isNone], nil);
    STAssertTrue([[o2 filter:^(id v){return [self isString:v];}] isSome], nil);
}
@end
