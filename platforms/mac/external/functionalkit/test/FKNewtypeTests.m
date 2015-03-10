#import <SenTestingKit/SenTestingKit.h>
#import "FKNewtype.h"


NEWTYPE(Age, NSString, age);
NEWTYPE(Name, NSString, name);
NEWTYPE2(Person, Age, age, Name, name);
NEWTYPE3(Position, Person, occupier, NSString, title, NSDate, started);

NEWTYPE2(Simple2, NSString, a, NSString, b); 
NEWTYPE3(Simple3, NSString, a, NSString, b, NSString, c); 

@interface FKNewtypeTests : SenTestCase
@end

@implementation FKNewtypeTests
- (void)testTypes {
	Age *age = [Age age:@"54"];
	Name *name = [Name name:@"Nick"];
	Person *nick = [Person age:age name:name];
	Position *p = [Position occupier:nick title:@"Dev" started:[NSDate date]];
	STAssertEqualObjects(age.age, @"54", nil);
	STAssertEqualObjects(nick.name.name, @"Nick", nil);
	STAssertEqualObjects(p.title, @"Dev", nil);
}

- (void)testValidArrayCreation {
	id fromValid = NSArrayToAge([NSArray arrayWithObject:@"54"]);
    
	STAssertTrue([fromValid isSome], nil);
	STAssertEqualObjects([fromValid some], [Age age:@"54"], nil);
	
	id fromValid2 = NSArrayToPerson([NSArray arrayWithObjects:[Age age:@"54"], [Name name:@"Nick"], nil]);
	STAssertTrue([fromValid2 isSome], nil);
	STAssertEqualObjects([fromValid2 some], [Person age:[Age age:@"54"] name:[Name name:@"Nick"]], nil);
	
	id fromValid3 = NSArrayToSimple3([NSArray arrayWithObjects:@"a", @"b", @"c",nil]);
	STAssertTrue([fromValid3 isSome], nil);
	STAssertEqualObjects([fromValid3 some], [Simple3 a:@"a" b:@"b" c:@"c"], nil);
}

- (void)testWrongSizeArrayCreation {

	id fromEmpty = NSArrayToAge(@[]);
	STAssertTrue([fromEmpty isKindOfClass:[FKOption class]],nil);
	STAssertTrue([fromEmpty isNone], nil);
	
	id fromTooBig = NSArrayToAge(@[@"54", @"55"]);
	STAssertTrue([fromTooBig isKindOfClass:[FKOption class]],nil);
	STAssertTrue([fromTooBig isNone], nil);
}

- (void)testWrongTypeArrayCreation {
	id fromWrongType = NSArrayToAge(@[[NSNumber numberWithInt:54]]);
	STAssertTrue([fromWrongType isKindOfClass:[FKOption class]],nil);
	STAssertTrue([fromWrongType isNone], nil);
}

- (void)testValidDictionaryCreation {
	FKOption *result = NSDictionaryToAge(@{@"age": @"54"});
	STAssertTrue(result.isSome, nil);
	STAssertEqualObjects(result.some,[Age age:@"54"], nil);
	
	result = NSDictionaryToSimple2(@{@"b": @"bval", @"a": @"aval"});
	STAssertTrue([result isSome], nil);
	STAssertEqualObjects([result some], [Simple2 a:@"aval" b:@"bval"], nil);
	
	result = NSDictionaryToSimple3(@{@"b": @"bval", @"a": @"aval", @"c": @"cval"});
	STAssertTrue([result isSome], nil);
	STAssertEqualObjects([result some], [Simple3 a:@"aval" b:@"bval" c:@"cval"], nil);
}

- (void)testInvalidDictionaryCreation {
	FKOption *result = NSDictionaryToAge(@{@"ages": @"54"});
	STAssertTrue(result.isNone, nil);
}
@end

