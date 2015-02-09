//
//  TextImageFieldCell.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 2/6/13.
//
//

#import "TextImageFieldCell.h"
#import "NSImage+Tinting.h"
#import "CocoaThemeManager.h"
#include "Painter.h"

// for some strange reason using an NSImageCell would change fonts in all other cells
// also sizing is not good enough

@interface TextImageFieldCell () {
}
@end

@implementation TextImageFieldCell

using namespace Gui;

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
}

@end
