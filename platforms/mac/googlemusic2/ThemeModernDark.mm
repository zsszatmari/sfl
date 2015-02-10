//
//  ThemeModernDark.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 4/30/13.
//
//

#import "ThemeModernDark.h"
#import "NSImage+Tinting.h"

@implementation ThemeModernDark

- (NSColor *)textColor
{
    return [NSColor whiteColor];
}

- (NSGradient *)playbackIconGradient;
{
    NSGradient *gradient = [[NSGradient alloc] initWithStartingColor:[NSColor colorWithDeviceWhite:67.0f/255.0f alpha:1.0f] endingColor:[NSColor colorWithDeviceWhite:27.0f/255.0f alpha:1.0f]];
    return gradient;
}

- (NSString *)themePrefix
{
    return @"darkmodern";
}

- (NSGradient *)ovalButtonGradient:(BOOL)pushed
{
    if (pushed) {
        return [[NSGradient alloc] initWithStartingColor:[NSColor colorWithDeviceWhite:186.0f/255.0f alpha:1.0f] endingColor:[NSColor colorWithDeviceWhite:76.0f/255.0f alpha:1.0f]];
    } else {
        return [[NSGradient alloc] initWithStartingColor:[NSColor colorWithDeviceWhite:94.0f/255.0f alpha:1.0f] endingColor:[NSColor colorWithDeviceWhite:170.0f/255.0f alpha:1.0f]];
    }
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
    NSColor *color = [NSColor colorWithDeviceWhite:29.0f/255.0f alpha:1.0f];
    return color;
}

- (NSColor *)playlistsRowColorTop
{
    NSColor *color = [NSColor colorWithDeviceWhite:37.0f/255.0f alpha:1.0f];
    return color;
}

- (NSColor *)playlistsRowColorBottom
{
    NSColor *color = [NSColor colorWithDeviceWhite:7.0f/255.0f alpha:1.0f];
    return color;
}

- (NSColor *)playlistsBackgroundColor
{
    return [NSColor colorWithDeviceWhite:19.0f/255.0f alpha:1.0f];
}

- (NSColor *)playlistsRowSelectionBorder
{
    return [NSColor colorWithDeviceRed:92.0f/255.0f green:114.0f/225.0f blue:196.0f/255.0f alpha:1.0f];
}

- (NSGradient *)playlistsRowSelectedGradient
{
    return [[NSGradient alloc] initWithColorsAndLocations:
            [NSColor colorWithDeviceRed:71.0/255.0f green:96.0f/255.0f blue:186.0f/255.0f alpha:1.0f], 0.0f,
            [NSColor colorWithDeviceRed:83.0/255.0f green:121.0f/255.0f blue:198.0f/255.0f alpha:1.0f], 0.6f,
            [NSColor colorWithDeviceRed:71.0/255.0f green:96.0f/255.0f blue:185.0f/255.0f alpha:1.0f], 1.0f,
            nil];
}

- (NSGradient *)playlistCategoryGradient
{
    return [[NSGradient alloc] initWithColors:@[[NSColor colorWithDeviceWhite:45.0f/255.0f alpha:1.0f],[NSColor colorWithDeviceWhite:59.0f/255.0f alpha:1.0f]]];
}

- (NSColor *)playlistsTextColor
{
    // originally: blackColor
    return [NSColor colorWithDeviceWhite:207.0f/255.0f alpha:1.0f];
}

- (NSFont *)playlistsTextFont
{
    return [NSFont fontWithName:@"HelveticaNeue-Bold" size:10.77f * 1.1f];
}

- (NSColor *)playlistsCategoryColor
{
    // originally: NSColor colorWithCalibratedRed:112.0f/255.0f green:126.0f/255.0f blue:140.0f/255.0f alpha:1.0f]];
    return [NSColor whiteColor];
}

- (NSArray *)songsAlternatingColors
{
    return @[[NSColor colorWithDeviceWhite:46.0f/255.0f alpha:1.0f],
             [NSColor colorWithDeviceWhite:40.0f/255.0f alpha:1.0f]];
}

- (NSColor *)songsTextColor
{
    return [self playlistsTextColor];
}

- (NSColor *)songsTextSelectedColor
{
    return [NSColor whiteColor];
}

- (Gui::Color)songsSelectedColor
{
    return Gui::Color(83.0/255.0f, 121.0f/255.0f, 198.0f/255.0f, 1.0f);
}

- (Gui::Color)songsPlayIconColor
{
    return Gui::Color(63.0/255.0f, 101.0f/255.0f, 178.0f/255.0f, 1.0f);
}

- (Gui::Color)songsPlayIconHighlightedColor
{
    return Gui::Color(180.0/255.0f, 216.0f/255.0f, 255.0f/255.0f, 1.0f);
}

- (NSGradient *)songsHeaderGradient
{
    /*return [[NSGradient alloc] initWithColorsAndLocations:
            [NSColor colorWithDeviceWhite:0.0f/255.0f alpha:0.5f],0.0f,
            [NSColor colorWithDeviceWhite:6.0f/255.0f alpha:0.5f],0.3f,
            [NSColor colorWithDeviceWhite:5.0f/255.0f alpha:0.5f],1.0f,
            nil];*/

    return [[NSGradient alloc] initWithColorsAndLocations:
            [NSColor colorWithDeviceWhite:8.0f/255.0f alpha:1.0f],0.0f,
            [NSColor colorWithDeviceWhite:8.0f/255.0f alpha:1.0f],0.1f,
            [NSColor colorWithDeviceWhite:131.0f/255.0f alpha:1.0f],0.1f,
            [NSColor colorWithDeviceWhite:125.0f/255.0f alpha:1.0f],0.5f,
            [NSColor colorWithDeviceWhite:120.0f/255.0f alpha:1.0f],0.5f,
            [NSColor colorWithDeviceWhite:120.0f/255.0f alpha:1.0f],1.0f,
            nil];
}

- (NSColor *)songsHeaderTextColor
{
    return [NSColor whiteColor];
}

- (NSColor *)songsHeaderSeparatorColor
{
    return [NSColor colorWithDeviceWhite:0.0f alpha:0.9f];
}

- (NSColor *)buttonContentColor;
{
    return [self playlistsTextColor];
}

- (NSScrollerKnobStyle)knobStyle
{
    return NSScrollerKnobStyleLight;
}

- (NSColor *)frameColor
{
    // was: [NSColor colorWithDeviceWhite:153.0f/255.0f alpha:1.0f]
    
    //return [NSColor colorWithDeviceWhite:153.0f/255.0f alpha:1.0f];
    return [NSColor colorWithDeviceWhite:8.0f/255.0f alpha:1.0f];
}

- (NSGradient *)albumViewBackground
{
    return [[NSGradient alloc] initWithColors:@[[NSColor colorWithDeviceWhite:39.0f/255.0f alpha:1.0f], [NSColor colorWithDeviceWhite:70.0f/255.0f alpha:1.0f]]];
}

- (NSDictionary *)albumViewTitleAttributes
{
    return @{NSForegroundColorAttributeName: [NSColor colorWithDeviceWhite:235.0f/255.0f alpha:1.0f], NSFontAttributeName: [NSFont fontWithName:@"HelveticaNeue-Medium" size:11.0f]};
}

- (NSDictionary *)albumViewSubtitleAttributes
{
    return @{NSForegroundColorAttributeName: [NSColor colorWithDeviceWhite:225.0f/255.0f alpha:1.0f], NSFontAttributeName: [NSFont fontWithName:@"HelveticaNeue-Medium" size:10.0f]};
}

- (NSGradient *)searchBackground;
{
    return [[NSGradient alloc] initWithColors:@[[NSColor colorWithDeviceWhite:116.0f/255.0f alpha:1.0f], [NSColor colorWithDeviceWhite:78.0f/255.0f alpha:1.0f]]];
}

- (NSColor *)sliderTop
{
    return [NSColor colorWithDeviceRed:39.0f/255.0f green:39.0f/255.0f blue:39.0f/255.0f alpha:1.0f];
}

- (NSColor *)sliderBottom
{
    return [NSColor colorWithDeviceWhite:36.0f/255.0f alpha:1.0f];
}

- (NSColor *)sliderLeft
{
    return [NSColor colorWithDeviceWhite:70.0f/255.0f alpha:1.0f];
}

- (NSColor *)sliderRight
{
    return [NSColor colorWithDeviceWhite:105.0f/255.0f alpha:1.0f];
}

- (NSGradient *)albumArtBorderGradient
{
    return [[NSGradient alloc] initWithColors:@[[NSColor colorWithDeviceWhite:63.0f/255.0f alpha:1.0f], [NSColor colorWithDeviceWhite:110.5/255.0f alpha:1.0f], [NSColor colorWithDeviceWhite:59.0f/255.0f alpha:1.0f]]];
}

- (NSColor *)fullScreenButtonColor
{
    return [NSColor colorWithDeviceWhite:170.0f/255.0f alpha:255.0f];
}

- (NSColor *)selectedTextBackgroundColor
{
    return [NSColor colorWithDeviceRed:80.0f/255.0f green:102.0f/255.0f blue:195.0f/255.0f alpha:1.0f];
}

- (NSColor *)bigTextColor
{
    return [NSColor colorWithDeviceWhite:163.0f/255.0f alpha:1.0f];
}

- (NSImage *)repeatIcon:(BOOL)pressed
{
    return [NSImage imageNamed:@"repeatsmall-white"];
}

- (NSImage *)shuffleIcon:(BOOL)pressed
{
    return [NSImage imageNamed:@"shufflesmall-white"];
}

@end
