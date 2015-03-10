# FunctionalKit: It's Functional for Objective-C.

FunctionalKit is an attempt to use functional paradigms in Objective-C. It is a set of low level
functional types & APIs. It contains types such as either, option, etc. that allow your to write
correct, clean, tight, succinct and (where possible) typesafe code. It also provides more advanced
concepts such as lifting functions into monads.

FunctionalKit is loosely modelled on Functional Java.


## Setup/Installation

1. Bring this project's structure into your project's structure, a git submodule is a good start. We place it into <code>Source/External/functionalkit</code>.
1. Add <code>functionalkit.xcodeproj</code> to your project.
1. For each of your targets, add FunctionalKit's <code>functionalkit</code> target as a direct dependency of the target (Target -> Command-I -> General -> Direct Dependencies).
1. For each of your targets, ensure <code>libfunctionalkit.a</code> is included (linked against).
1. In your project, add FunctionalKit's main directory to your Header Search Paths (<code>HEADER_SEARCH_PATHS</code>), e.g. <code>$(SRCROOT)/Source/External/functionalkit/src</code>.
1. Import <code>FK/FKPrelude.h</code> where you want to use FunctionalKit, your prefix header is a good spot.

## Usage

* [Simplifying JSON Parsing Using FunctionalKit](http://adams.id.au/blog/2009/04/simplifying-json-parsing-using-functionalkit/)
* [Taming Errors with FunctionalKit](http://www.slideshare.net/nkpart/taming-errors-with-functionalkit)

## Examples

### Function creation

Create a function from a selector.

```objc
id <FKFunction> doSomethingFunction = [FKFunction functionFromSelector:@selector(doSomething:) target:self];
```

Use a nested function (requires -fnested-functions):

```objc
// With -fnested-functions enabled.
- (void)testDummy {
  NSString *f(NSString *a) {
      return [a substringToIndex:1];
  }
  NSDictionary *d = [NSARRAY(@"one", @"two", @"three") groupByKey:functionP(f)];
}
```

### Mapping

Map across the elements of an array of names and turn them into people.

```objc
NSArray *names = [NSArray arrayWithObjects:@"Fred", @"Mary", @"Pete", nil];
NSArray *people = [names map:[FKFunction functionFromSelector:@selector(makePersonFromName:) target:self]];

// Given each chapter has an array of pages
NSArray *chapters = ...;
int pageCount = [[NSArray concat:[chapters map:functionS(pages)]] count];
```

### Nil values

Handle a possibly nil value safely.

```objc
NSDictionary *dict = ...;
FKOption *couldBeNil = [FKOption fromNil:[dict objectForKey:@"SomeKey"]];
```

### Handling failures

Construct an either representing failure.

```objc
FKEither *failed = [FKEither errorWithReason:@"Credentials have not been saved"];
```

Perform an operation that may fail, apply a selector to the result if successful, otherwise on failure propagate the error.

```objc
MVEither *maybeResponse = [HttpApi makeRequestToUrl:@"http://www.foo.com/json-api"];
MVEither *maybeParsedJson = [maybeResponse.right mapWithSelector:@selector(JSONValue)];
```

Perform an operation that returns nil & an error on failue. Return the error on the left on failure or the returned object on the right.

```objc
NSError *error;
SBJSON *parser = [[SBJSON new] autorelease];
id parsedObject = [parser objectWithString:self error:&error];
FKEither maybeParsedObject = [[FKOption fromNil:parsedObject] toEither:error];
```

### Validate parsed values, turn them into a domain model class on success

Note. This is a bit messy, could be cleaner.

```objc
FKOption *maybeTitle = [FKOption fromNil:[dictionary objectForKey:@"title"] ofType:[NSString class]];
FKOption *maybeOwnerName = [FKOption fromNil:[dictionary objectForKey:@"owner_name"] ofType:[NSString class]];
FKOption *maybeHeadlineImgId = [FKOption fromNil:[dictionary objectForKey:@"headline_img_id"] ofType:[NSString class]];
if ([NSARRAY(maybeTitle, maybeOwnerName, maybeHeadlineImgId) all:@selector(isSome)]) {
  return [FKOption some:[FlickrGallery galleryWithTitle:[maybeTitle some] ownerName:[maybeOwnerName some] sampleImgId:[maybeHeadlineImgId some]]];
} else {
  return [FKOption none];
}
```

### Side effects

Comap a function with an effect, to have the function execute then perform a side effect using the function's result.

```objc
id <FKFunction> getPhotosF = [FKFunction functionFromSelector:@selector(photos)];
id <FKEffect> galleriesOp = [FKEffect comap:[self effectThatDoesSomethingWithPhotos] :getPhotosF];
```

### Fold and intersperse

```objc
// Declare some distinct search terms.
NSArray *searchTerms = NSARRAY(@"foo", @"bar", @"baz");

// Intersperse "OR" between each term, and flatten into a single string.
NSString *term = [[searchTerms intersperse:@" OR "] foldLeft:@"" f:[NSString concatF]];
NSLog(term);  // @"foo OR bar OR baz"
```

### Lifting

The following example lifts a function into the array monad, applying the function to each element of the array.

```objc
// Parse a photo's details out of a dictionary, return Some(FlickrPhoto) on success or None on failure.
- (FKOption *)parsePhotoForDictionary:(NSDictionary *)dictionary {
  FKOption *maybeTitle = [FKOption fromNil:[dictionary objectForKey:@"title"] ofType:[NSString class]];
  FKOption *maybeId = [FKOption fromNil:[dictionary objectForKey:@"photo_id"] ofType:[NSString class]];
  if ([NSARRAY(maybeTitle, maybeId) all:@selector(isSome)]) {
    return [FKOption some:[FlickrPhoto photoWithId:[maybeId some] title:[maybeTitle some]]];
  } else {
    return [FKOption none];
  }
}

// Retrieve the array of photos.
FKOption *maybePhotos = [FKOption fromNil:[dictionary objectForKey:@"photos"] ofType:[NSArray class]];

// Still within the option monad, lift the parse function (above) into the array monad and map across the option.
id <FKFunction> parsePhotoF = [FKFunction functionFromSelector:@selector(parsePhotoForDictionary:) target:self];
FKOption *maybeParsedPhotos = [maybePhotos map:[NSArray liftFunction:parsePhotoF]];
```

## Other Work

Here's some other work we know of that does similar things to FunctionalKit.

* [More Functional Reusability in C/C++/Objective-c with Curried Functions](http://asg.unige.ch/site/papers/Dami91a.pdf)
* [Map, filter and reduce in Objective-C/Cocoa](https://github.com/nst/nsarray-functional)
* [Simple HOM](http://www.metaobject.com/blog/2009/01/simple-hom.html)
* [Higher Order Messaging](http://cocoadev.com/index.pl?HigherOrderMessaging)
