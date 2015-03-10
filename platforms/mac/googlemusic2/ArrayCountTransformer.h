//
//  ArrayCountTransformer.h
//  googlemusic2
//
//  Created by Zsolt Szatmári on 3/11/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ArrayCountTransformer : NSValueTransformer

- (NSString *)transformedInt:(NSUInteger)count;

@end
