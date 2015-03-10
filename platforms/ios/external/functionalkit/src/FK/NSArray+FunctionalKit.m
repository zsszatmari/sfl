#import "NSArray+FunctionalKit.h"
#import "FKMacros.h"

@implementation NSArray (FunctionalKitExtensions)

+ (NSArray *(^)(NSArray *))liftFunction:(id (^)(id))f {
    return ^(NSArray *v) {
        return [v map:f];
    };
}

+ (NSArray *)concat:(NSArray *)nested {
    NSMutableArray *concatenated = [NSMutableArray array];
    for (id item in nested) {
        if ([item isKindOfClass:[NSArray class]]) {
            [concatenated addObjectsFromArray:item];
        } else {
            NSString *message = @"Cannot concat a non-array value";
            @throw [NSException exceptionWithName:NSInvalidArgumentException reason:message userInfo:@{}];
        }
    }
    return [NSArray arrayWithArray:concatenated];
}

- (id)head {
    if ([self count] == 0) {
        @throw [NSException exceptionWithName:NSInvalidArgumentException reason:@"Cannot get the head of an empty array" userInfo:@{}];
    } else {
        return [self objectAtIndex:0];
    }
}

- (NSArray *)tail {
    if ([self count] == 0) {
        @throw [NSException exceptionWithName:NSInvalidArgumentException reason:@"Cannot get the tail of an empty array" userInfo:@{}];
    } else {
        return [self subarrayWithRange:NSMakeRange(1, [self count] - 1)];
    }
}

- (NSArray *)take:(NSUInteger)firstItems {
    if ([self count] > firstItems) {
        return [self subarrayWithRange:NSMakeRange(0, firstItems)];
    }
    return [NSArray arrayWithArray:self];
}

- (FKP2 *)span:(BOOL (^)(id))f {
    NSMutableArray *matching = [NSMutableArray array];
    NSMutableArray *rest = [NSMutableArray array];
	for (id item in self) {
		if (f(item)) {
            [matching addObject:item];
		} else {
            [rest addObject:item];
        }
	}
    return [[FKP2 alloc] initWith_1:[NSArray arrayWithArray:matching] _2:[NSArray arrayWithArray:rest]];
}

- (BOOL)all:(BOOL (^)(id))f {
	for (id item in self) {
		if (!f(item)) {
			return NO;
		}
	}
	return YES;
}

- (BOOL)any:(BOOL (^)(id))f {
	for (id item in self) {
		if (f(item)) {
			return YES;
		}
	}
	return NO;
}

- (NSArray *)filter:(BOOL (^)(id))f {
    NSMutableArray *filtered = [NSMutableArray arrayWithCapacity:[self count]];
	for (id item in self) {
		if (f(item)) {
            [filtered addObject:item];
		}
	}
    return [NSArray arrayWithArray:filtered];
}

- (NSArray *)drop:(BOOL (^)(id))f {
    NSMutableArray *filtered = [NSMutableArray arrayWithCapacity:[self count]];
	for (id item in self) {
		if (!f(item)) {
            [filtered addObject:item];
		}
	}
    return [NSArray arrayWithArray:filtered];
}

- (NSDictionary *)groupByKey:(id (^)(id))f {
    NSMutableDictionary *grouped = [NSMutableDictionary dictionary];
	for (id item in self) {
        id key = f(item);
        id nilsafeKey = key == nil ? [NSNull null] : key;
        NSMutableArray *values = [grouped objectForKey:nilsafeKey];
        if (values == nil) {
            values = [NSMutableArray array];
            [grouped setObject:values forKey:nilsafeKey];
        }
        [values addObject:item];
	}   
    return [NSDictionary dictionaryWithDictionary:grouped];
}

- (id)foldLeft:(id)acc f:(id (^)(id, id))f {
    __strong id accC = acc;
    for (id item in self) {
        accC = f(accC, item);
    }
    return accC;
}

- (NSArray *)map:(id (^)(id))f {
	NSMutableArray *r = [NSMutableArray arrayWithCapacity:[self count]];
	for (id item in self) {
		[r addObject:f(item)];
	}
	return [NSArray arrayWithArray:r];
}

- (NSArray *)mapOption:(FKOption *(^)(id))f {
	NSMutableArray *r = [NSMutableArray array];
	for (id item in self) {
        FKOption *o = f(item);
        if (o.isSome) {
            [r addObject:o.some];
        }
	}
	return [NSArray arrayWithArray:r];
}

- (void)foreach:(void (^)(id))f {
	for (id item in self) {
		f(item);
	}
}

- (NSArray *)intersperse:(id)object {
    NSMutableArray *interspersed = [NSMutableArray array];
    for (NSUInteger i = 0; i < [self count]; ++i) {
        [interspersed addObject:[self objectAtIndex:i]];
        if (i != [self count] - 1) [interspersed addObject:object];
    }    
    return [NSArray arrayWithArray:interspersed];
}

- (NSArray *)reverse {
	return [[self reverseObjectEnumerator] allObjects];
}

- (NSArray *)unique {
	return [[NSSet setWithArray:self] allObjects];
}

- (FKOption *)toOption {
    return  [self count] == 0 ? [FKOption none] : [FKOption some:[self objectAtIndex:0]];
}

- (NSArray *)zip:(NSArray *)rhs {
    NSMutableArray *zipped = [NSMutableArray array];
    NSEnumerator *enumL = [self objectEnumerator];
    NSEnumerator *enumR = [rhs objectEnumerator];
    
    while (true) {
        id l = [enumL nextObject];
        id r = [enumR nextObject];
        if (l == nil || r == nil) {
            break;
        }
        [zipped addObject:[FKP2 _1:l _2:r]];
    }
    return zipped;
}

+ (NSArray *)replicate:(NSUInteger)n x:(id)x
{
    NSMutableArray *array = [NSMutableArray array];
    for (int i = 0 ; i < n ; ++i) {
        [array addObject:x];
    }
    return array;
}

- (NSArray *)bind:(NSArray *(^)(id))f
{
    return [NSArray concat:[self map:f]];
}

@end
