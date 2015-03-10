//
//  HighlightingTextFiel.h
//  googlemusic2
//
//  Created by Zsolt Szatmári on 3/18/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "VerticallyCenteredTextField.h"

@interface HighlightingTextField : VerticallyCenteredTextField

+ (NSColor *)highlightColorFrom:(NSColor *)color;

@end
