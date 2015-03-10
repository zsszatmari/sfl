#import <SenTestingKit/SenTestingKit.h>
#import "FK/FKEither.h"
#import "FK/FKOption.h"

@interface FKEitherUnitTest : SenTestCase {
    NSObject *o1;
    NSObject *o2;
}
@end

@implementation FKEitherUnitTest

- (void)setUp {
    o1 = [[NSObject alloc] init];
    o2 = [[NSObject alloc] init];
}

- (void)testALeftIsLeft {
    FKEither *leftEither = [FKEither leftWithValue:o1];
    STAssertTrue(leftEither.isLeft, nil);
    STAssertFalse(leftEither.isRight, nil);
    STAssertEqualObjects(o1, [leftEither.swap valueOr:nil], nil);
}

- (void)testARightIsRight {
    FKEither *rightEither = [FKEither rightWithValue:o1];
    STAssertTrue(rightEither.isRight, nil);
    STAssertFalse(rightEither.isLeft, nil);
    STAssertEqualObjects(o1, [rightEither valueOr:nil], nil);
}

- (void)testTwoLeftsWithEqualValuesAreEqual {
    FKEither *l1 = [FKEither leftWithValue:o1];
    FKEither *l2 = [FKEither leftWithValue:o1];
    STAssertEqualObjects(l1, l2, nil);
}

- (void)testTwoRightsWithEqualValuesAreEqual {
    FKEither *r1 = [FKEither rightWithValue:o1];
    FKEither *r2 = [FKEither rightWithValue:o1];
    STAssertEqualObjects(r1, r2, nil);
}

- (void)testTwoLeftsWithUnEqualValuesAreNotEqual {
    FKEither *l1 = [FKEither leftWithValue:o1];
    FKEither *l2 = [FKEither leftWithValue:o2];
    STAssertTrue([l1 isEqual:l2] == NO, nil);
}

- (void)testTwoRightsWithUnEqualValuesAreNotEqual {
    FKEither *r1 = [FKEither rightWithValue:o1];
    FKEither *r2 = [FKEither rightWithValue:o2];
    STAssertTrue([r1 isEqual:r2] == NO, nil);
}

- (void)testALeftAndARightAreNotEqual {
    FKEither *l = [FKEither leftWithValue:o1];
    FKEither *r = [FKEither rightWithValue:o1];
    STAssertTrue([l isEqual:r] == NO, nil);
}

- (void)testAccessingTheRightValueInLeftThrowsAnError {
    FKEither *l = [FKEither leftWithValue:o1];
    @try {
        [l valueOrFailWithMessage:@""];
        STFail(@"Expected an exception to be thrown");
    }
    @catch (id exception) {
    }
}

- (void)testAccessingTheLeftValueInLeftThrowsAnError {
    FKEither *r = [FKEither rightWithValue:o1];
    @try {
        [r.swap valueOrFailWithMessage:@""];
        STFail(@"Expected an exception to be thrown");
    }
    @catch (id exception) {
    }
}

- (void)testAccessingTheLeftOrValue {
	FKEither *left = [FKEither leftWithValue:o1];
	FKEither *right = [FKEither rightWithValue:o1];
	STAssertEqualObjects([right.swap valueOr:@"54"], @"54", nil);
	STAssertEqualObjects([left valueOr:@"54"], @"54", nil);
}

- (void)testMappingAcrossTheLeft {
	FKEither *either = [FKEither leftWithValue:[NSNumber numberWithInt:54]];
	FKEither *mapped = [either.swap map:^(id v) { return [v description]; }].swap;
	STAssertTrue(mapped.isLeft,nil);
	STAssertEqualObjects([mapped.swap valueOr:nil], @"54",nil);
}

- (void)testMappingAcrossTheRightOfALeftIsIdentity {
	FKEither *either = [FKEither leftWithValue:[NSNumber numberWithInt:54]];
	FKEither *mapped = [either map:^(id v) { return [v description]; }];
	STAssertEqualObjects(either, mapped, nil);
}

- (void)testMappingAcrossTheRight {
	FKEither *either = [FKEither rightWithValue:[NSNumber numberWithInt:54]];
	FKEither *mapped = [either map:^(id v) { return [v description]; }];
	STAssertTrue(mapped.isRight,nil);
	STAssertEqualObjects([mapped valueOr:nil], @"54",nil);
}

- (void)testToOption {
	FKEither *either = [FKEither rightWithValue:@"v"];
	STAssertTrue([[either toOption] isSome], nil);
	STAssertTrue([[either.swap toOption] isNone], nil);
}

- (void)testBindRightConcatentatesToProduceASingleEither {
	FKEither *either = [FKEither rightWithValue:@"v"];
    FKEither *afterBind = [either bind:^(id v) { return [self toRight:v]; }];
    STAssertEqualObjects(either, afterBind, nil);
}

- (FKEither *)toLeft:(NSString *)string {
    return [FKEither leftWithValue:string];
}

- (FKEither *)toRight:(NSString *)string {
    return [FKEither rightWithValue:string];
}

@end
