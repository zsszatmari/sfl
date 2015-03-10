
/*
 A bit more sane way to define lambdas:
 
 [@[@"bob", @"fred", @"wilma"] map:lambda(s, [s uppercaseString])]; //outputs @[@"BOB", @"FRED", @"WILMA"]

 A bit mental (but a bit like scala):
 
 [@[@"bob", @"fred", @"wilma"] map:_([_ uppercaseString])]; //outputs @[(@"BOB", @"FRED", @"WILMA"]

 */

#define lambda(s, statement) ^(id s){return statement;}
#define _(statement) ^(id _){return statement;}




// (borrowed from OCTotallyLazy)