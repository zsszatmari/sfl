//
//  ThemeCocoa.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 4/30/13.
//
//

#import "AppDelegate.h"
#import "ThemeCocoa.h"

@implementation ThemeCocoa

- (NSColor *)textColor
{
    return [NSColor blackColor];
}

- (NSGradient *)playbackIconGradient;
{
    return [[NSGradient alloc] initWithColors:@[[NSColor blackColor],[NSColor blackColor]]];
}

- (NSString *)themePrefix
{
    return nil;
}

- (NSGradient *)ovalButtonGradient:(BOOL)pushed
{
    return nil;
    //return [[NSGradient alloc] initWithStartingColor:[NSColor colorWithDeviceWhite:167/255.0f alpha:1.0f] endingColor:[NSColor colorWithDeviceWhite:252/255.0f alpha:1.0f]];
}

- (NSGradient *)rectButtonGradient:(BOOL)pushed
{
    if (pushed) {
        return [[NSGradient alloc] initWithStartingColor:[NSColor colorWithDeviceWhite:186.0f/255.0f alpha:1.0f] endingColor:[NSColor colorWithDeviceWhite:76.0f/255.0f alpha:1.0f]];
    } else {
        return [[NSGradient alloc] initWithStartingColor:[NSColor colorWithDeviceWhite:52.0f/255.0f alpha:1.0f] endingColor:[NSColor colorWithDeviceWhite:76.0f/255.0f alpha:1.0f]];
    }
}


- (NSColor *)playlistsRowColor
{
    return nil;
}

- (NSColor *)playlistsRowColorTop
{
    return nil;
}

- (NSColor *)playlistsRowColorBottom
{
    return nil;
}

- (NSColor *)playlistsBackgroundColor
{
    if (yosemiteOrBetter()) {
        if (vibrancyEnabled()) {
            return [NSColor clearColor];
        } else {
            return [NSColor colorWithDeviceWhite:240.0f/255.0f alpha:1.0f];
        }
    }
    return [NSColor colorWithDeviceRed:225.0f/255.0f green:229.0f/255.0f blue:235.0f/255.0f alpha:1.0f];
}

- (NSColor *)playlistsRowSelectionBorder
{
    return [NSColor colorWithDeviceRed:92.0f/255.0f green:114.0f/225.0f blue:196.0f/255.0f alpha:1.0f];
}

- (NSGradient *)playlistsRowSelectedGradient
{
    /*return [[NSGradient alloc] initWithColorsAndLocations:
            [NSColor colorWithDeviceRed:71.0/255.0f green:96.0f/255.0f blue:186.0f/255.0f alpha:1.0f], 0.0f,
            [NSColor colorWithDeviceRed:83.0/255.0f green:121.0f/255.0f blue:198.0f/255.0f alpha:1.0f], 0.6f,
            [NSColor colorWithDeviceRed:71.0/255.0f green:96.0f/255.0f blue:185.0f/255.0f alpha:1.0f], 1.0f,
            nil];*/
    return [[NSGradient alloc] initWithColorsAndLocations:
                         [NSColor colorWithDeviceRed:38/255.0f green:130.0f/255.0f blue:213.0f/255.0f alpha:1.0f], 0.0f,
                         [NSColor colorWithDeviceRed:78.0/255.0f green:162.0f/255.0f blue:226.0f/255.0f alpha:1.0f], 1.0f,
                         nil];
}

- (NSGradient *)playlistCategoryGradient
{
    return nil;
}

- (NSColor *)playlistsTextColor
{
    return [NSColor colorWithDeviceWhite:66.0f/255.0f alpha:1.0f];
}

- (NSColor *)playlistsTextSelectedColor
{
    return [NSColor colorWithDeviceWhite:28.0f/255.0f alpha:1.0f];
}

- (NSFont *)playlistsTextFont
{
    return [NSFont systemFontOfSize:13.0f];
}

- (NSColor *)playlistsCategoryColor
{
    return [NSColor colorWithDeviceRed:128.0f/255.0f green:128.0f/255.0f blue:128.0f/255.0f alpha:1.0f];
}

- (NSArray *)songsAlternatingColors
{
    return @[[NSColor colorWithDeviceRed:243.0f/255.0f green:246.0f/255.0f blue:250.0f/255.0f alpha:1.0f],
             [NSColor colorWithDeviceWhite:255.0f/255.0f alpha:1.0f]];
}

- (NSColor *)songsTextColor
{
    return [NSColor colorWithDeviceWhite:0.0f alpha:1.0f];
}

- (NSColor *)songsTextSelectedColor
{
    return [NSColor whiteColor];
}

- (NSColor *)selectedTextBackgroundColor
{
    return [NSColor colorWithDeviceRed:177.0f/255.0f green:211.0f/255.0f blue:255.0f/255.0f alpha:1.0f];
}

- (NSGradient *)songsHeaderGradient
{
    return nil;
}

- (NSColor *)songsHeaderTextColor
{
    return [NSColor colorWithDeviceRed:116.0f/255.0f green:130.0f/255.0f blue:146.0f/255.0f alpha:1.0f];
    //return [NSColor blackColor];
}

- (NSColor *)songsHeaderSeparatorColor
{
    return [NSColor colorWithDeviceWhite:0.5f alpha:0.5f];
}
    
- (NSColor *)buttonContentColor;
{
    //return [self playlistsTextColor];
    return [NSColor colorWithDeviceWhite:48.0f/255.0f alpha:1.0f];
}

- (NSColor *)buttonContentColorPressed
{
    return [NSColor colorWithDeviceRed:0.0f/255.0f green:133.0f/255.0f blue:239.0f/255.0f alpha:1.0f];
}

- (NSScrollerKnobStyle)knobStyle
{
    return NSScrollerKnobStyleDefault;
}  

- (NSColor *)frameColor
{
    return [NSColor colorWithDeviceWhite:162.0f/255.0f alpha:1.0f];
}

- (NSGradient *)albumViewBackground
{
    return [[NSGradient alloc] initWithColors:@[[NSColor colorWithDeviceWhite:255.0f/255.0f alpha:1.0f], [NSColor colorWithDeviceWhite:255.0f/255.0f alpha:1.0f]]];
}

- (NSDictionary *)albumViewTitleAttributes
{
    return @{NSForegroundColorAttributeName: [NSColor colorWithDeviceWhite:0.0f/255.0f alpha:1.0f], NSFontAttributeName: [NSFont fontWithName:@"HelveticaNeue-Medium" size:11.0f]};
}

- (NSDictionary *)albumViewSubtitleAttributes
{
    return @{NSForegroundColorAttributeName: [NSColor colorWithDeviceWhite:0.0f/255.0f alpha:1.0f], NSFontAttributeName: [NSFont fontWithName:@"HelveticaNeue" size:10.0f]};
}

- (NSGradient *)searchBackground;
{
    return nil;
}

- (Gui::Color)songsSelectedColor
{
    return Gui::Color(56/255.0f, 108.0f/255.0f, 218.0f/255.0f, 1.0f);
}

- (NSColor *)sliderTop
{
    return [NSColor colorWithDeviceRed:152.0f/255.0f green:152.0f/255.0f blue:152.0f/255.0f alpha:1.0f];
}

- (NSColor *)sliderBottom
{
    return [NSColor colorWithDeviceWhite:172.0f/255.0f alpha:1.0f];
}

- (NSColor *)sliderLeft
{
    return [NSColor colorWithDeviceWhite:190.0f/255.0f alpha:1.0f];
}

- (NSColor *)sliderRight
{
    return [NSColor colorWithDeviceWhite:205.0f/255.0f alpha:1.0f];
}

- (NSGradient *)albumArtBorderGradient
{
    //return [[NSGradient alloc] initWithColors:@[[NSColor colorWithDeviceWhite:63.0f/255.0f alpha:1.0f], [NSColor colorWithDeviceWhite:110.5/255.0f alpha:1.0f], [NSColor colorWithDeviceWhite:59.0f/255.0f alpha:1.0f]]];
    return [[NSGradient alloc] initWithStartingColor:[self frameColor] endingColor:[self frameColor]];
}

- (NSColor *)fullScreenButtonColor
{
    return nil;
}

- (Gui::Color)songsPlayIconColor
{
    return Gui::Color(0.0f,0.0f,0.0f,1.0f);
}

- (Gui::Color)songsPlayIconHighlightedColor
{
    return Gui::Color(1.0f,1.0f,1.0f,1.0f);
}

- (NSShadow *)categoryShadow
{
    // no more shadows since yosemite
    return nil;
    
    /*static NSShadow *shadow = nil;
    if (!shadow) {
        shadow = [[NSShadow alloc] init];
        //[shadow setShadowColor: [NSColor colorWithCalibratedRed: .94 green: .94 blue: .94 alpha: 1.]];
        [shadow setShadowColor: [NSColor colorWithCalibratedRed: 1 green: 0 blue: 0 alpha: 1.]];
        [shadow setShadowOffset: CGSizeMake(.0, -1.)];
    }
    return shadow;*/
}

@end
