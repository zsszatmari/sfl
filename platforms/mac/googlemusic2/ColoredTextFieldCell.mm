//
//  ColoredTextFieldCell.m
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/23/13.
//
//

#import "ColoredTextFieldCell.h"
#import "CocoaThemeManager.h"
#include "Writer.h"
#include "IApp.h"
#include "ThemeManager.h"
#include "ITheme.h"
#import "AppDelegate.h"

@implementation ColoredTextFieldCell

using namespace Gear;

static NSText *lastEditor;

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if (self) {
    }
    return self;
}

- (NSText *)setUpFieldEditorAttributes:(NSText *)textObj
{
    NSText *text = [super setUpFieldEditorAttributes:textObj];
    [ColoredTextFieldCell setupColorForEditing:text];
    dispatch_async(dispatch_get_main_queue(), ^{
        [ColoredTextFieldCell setupColorForEditing:text];
    });
    lastEditor = text;
    
    return text;
}

+ (void)setupColorForEditing:(NSText *)text
{
    NSColor *color = [[CocoaThemeManager sharedManager] textColor];
    [text setBackgroundColor:[[[CocoaThemeManager sharedManager] songsAlternatingColors] objectAtIndex:0]];
    [text setTextColor:color];
}

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    cellFrame.origin.x += 2;
    cellFrame.size.width -= 2;
    NSFont *font = [[NSFontManager sharedFontManager] convertFont:[self font] toNotHaveTrait:NSBoldFontMask];
    int textHeight = [font boundingRectForFont].size.height -0;
    //int textHeight = [[self font] ascender] - [[self font] descender];
    //int textHeight = [[self font] pointSize];
    
    //NSLog(@"textheight: %d cell height:%f", textHeight, cellFrame.size.height);
    
    CGFloat yPos = -_verticalOffset;
    // snow leopard protection...
    if (textHeight == 20) {
        yPos = (/*cellFrame.size.height*/ 17.0f - textHeight)*(-1) +0.0f;
    }
    cellFrame.origin.y += yPos;
    cellFrame.size.height -= yPos;

    CGContextRef ctx = (CGContextRef)[NSGraphicsContext currentContext].graphicsPort;
    
    if (self.superBackgroundColor && mountainLionOrBetter()) {
        
        CGRect backgroundFrame = cellFrame;
        backgroundFrame.size.height -= 1;
        CGContextSetShouldSmoothFonts(ctx, true);
        CGContextSetFillColorWithColor(ctx, self.superBackgroundColor.CGColor);
        CGContextFillRect(ctx, backgroundFrame);
    }
    
    [super drawInteriorWithFrame:cellFrame inView:controlView];
}

- (NSRect)expansionFrameWithFrame:(NSRect)cellFrame inView:(NSView *)view
{
    NSRect rect = [super expansionFrameWithFrame:cellFrame inView:view];
    rect.size.height = cellFrame.size.height;
    return rect;
}

- (void)drawWithExpansionFrame:(NSRect)cellFrame inView:(NSView *)view
{
    NSColor *pushedColor = [self textColor];
    [self setTextColor:[NSColor blackColor]];
    
    [super drawWithExpansionFrame:cellFrame inView:view];
    
    [self setTextColor:pushedColor];
}

@end
