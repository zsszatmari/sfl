//
//  ThemeHandler.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 3/9/13.
//
//

#import <Foundation/Foundation.h>
#include "Color.h"

@interface CocoaThemeManager : NSObject

+ (CocoaThemeManager *)sharedManager;
+ (void)refreshTheme;
+ (BOOL) usingCocoaTheme;
@end

@interface CocoaThemeManager (Abstract)

- (NSColor *)textColor;
- (NSString *)themePrefix;
- (NSGradient *)ovalButtonGradient:(BOOL)pushed;
- (NSGradient *)rectButtonGradient:(BOOL)pushed;
- (NSGradient *)playbackIconGradient;

- (NSColor *)playlistsBackgroundColor;
- (NSColor *)playlistsRowColor;
- (NSColor *)playlistsRowColorTop;
- (NSColor *)playlistsRowColorBottom;
- (NSGradient *)playlistsRowSelectedGradient;
- (NSColor *)playlistsRowSelectionBorder;
- (NSColor *)playlistsTextColor;
- (NSFont *)playlistsTextFont;
- (NSColor *)bigTextColor;
- (NSColor *)playlistsCategoryColor;
- (NSGradient *)playlistCategoryGradient;
- (NSArray *)songsAlternatingColors;
- (NSColor *)songsTextColor;
- (NSColor *)songsTextSelectedColor;
- (Gui::Color)songsSelectedColor;
- (NSGradient *)songsHeaderGradient;
- (NSColor *)songsHeaderTextColor;
- (NSColor *)songsHeaderSeparatorColor;
- (NSColor *)buttonContentColor;
- (NSColor *)buttonContentColorLight;
- (NSColor *)buttonContentColorPressed;
- (NSScrollerKnobStyle)knobStyle;
- (NSColor *)frameColor;
- (NSGradient *)albumViewBackground;
- (NSDictionary *)albumViewTitleAttributes;
- (NSDictionary *)albumViewSubtitleAttributes;
- (NSGradient *)searchBackground;
- (NSColor *)sliderTop;
- (NSColor *)sliderBottom;
- (NSColor *)sliderLeft;
- (NSColor *)sliderRight;
- (NSGradient *)albumArtBorderGradient;
- (NSColor *)fullScreenButtonColor;
- (Gui::Color)songsPlayIconColor;
- (Gui::Color)songsPlayIconHighlightedColor;
- (Gui::Color)songsRatingIconHighlightedColor;
- (NSColor *)selectedTextBackgroundColor;
- (NSImage *)repeatIcon:(BOOL)pressed;
- (NSImage *)shuffleIcon:(BOOL)pressed;
- (NSShadow *)categoryShadow;
- (NSDictionary *)categoryAttributes;
- (NSDictionary *)categoryHideButtonAttributes;
- (NSColor *)playlistsTextSelectedColor;

@end
