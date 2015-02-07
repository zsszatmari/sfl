//
//  ThemeHandler.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 3/9/13.
//
//

#import "CocoaThemeManager.h"
#import "ThemeModernDark.h"
#import "ThemeCocoa.h"
#import "ThemeHelper.h"
#include "ModernDarkTheme.h"
#include "CocoaTheme.h"
#include "IApp.h"
#include "ThemeManager.h"

@implementation CocoaThemeManager {
    NSMutableDictionary *categoryAttributes;
    NSMutableDictionary *categoryHideButtonAttributes;
}

using namespace Gui;
using namespace Gear;

static CocoaThemeManager *manager = nil;

+ (CocoaThemeManager *)sharedManager
{
    if (manager == nil) {
        NSString *theme = [[NSUserDefaults standardUserDefaults] objectForKey:kUserDefaultsKeyTheme];
        if ([theme isEqualToString:kThemeModernDark]) {
            manager = [[ThemeModernDark alloc] init];
            IApp::instance()->themeManager()->setCurrent(shared_ptr<ITheme>(new ModernDarkTheme()));
        } else {
            manager = [[ThemeCocoa alloc] init];
            IApp::instance()->themeManager()->setCurrent(shared_ptr<ITheme>(new CocoaTheme()));
        }
    }
    
    return manager;
}

+ (void)refreshTheme
{
    manager = nil;
}

- (NSColor *)bigTextColor
{
    return [self buttonContentColorLight];
}

- (NSColor *)buttonContentColorLight
{
    return [[self buttonContentColor] colorWithAlphaComponent:0.8f];
}

- (NSImage *)repeatIcon:(BOOL)pressed
{
    if (pressed) {
        return [NSImage imageNamed:@"repeatsmall-pressed"];
    } else {
        return [NSImage imageNamed:@"repeatsmall"];
    }
}

- (NSImage *)shuffleIcon:(BOOL)pressed
{
    if (pressed) {
        return [NSImage imageNamed:@"shufflesmall-pressed"];
    } else {
        return [NSImage imageNamed:@"shufflesmall"];
    }
}

- (NSShadow *)categoryShadow
{
    return nil;
}

- (NSDictionary *)categoryAttributes
{
    if (!categoryAttributes) {
        NSShadow *shadow = [self categoryShadow];
        //CGFloat size = 12.0f;
        CGFloat size = 10.46f * 1.1f;
        size = 11.f;
        
        categoryAttributes = [@{
                        //NSFontAttributeName: [NSFont boldSystemFontOfSize:size],
                        NSFontAttributeName: [NSFont fontWithName: @"HelveticaNeue-Bold" size: size],
                        NSForegroundColorAttributeName: [[CocoaThemeManager sharedManager] playlistsCategoryColor]} mutableCopy];
        if (shadow) {
            [categoryAttributes setObject:shadow forKey:NSShadowAttributeName];
        }
    }
    
    return categoryAttributes;
}

- (NSDictionary *)categoryHideButtonAttributes
{
    if (!categoryHideButtonAttributes) {
        NSMutableParagraphStyle *paragraph = [[NSMutableParagraphStyle alloc] init];
        [paragraph setAlignment: NSRightTextAlignment];
        categoryHideButtonAttributes = [@{NSFontAttributeName: [NSFont fontWithName: @"HelveticaNeue-Bold" size: 10.46f * 1.1f],
                           NSForegroundColorAttributeName: [[CocoaThemeManager sharedManager] playlistsCategoryColor],
                           NSParagraphStyleAttributeName: paragraph} mutableCopy];
        NSShadow *shadow = [self categoryShadow];
        if (shadow) {
            [categoryHideButtonAttributes setObject:shadow forKey:NSShadowAttributeName];
        }
    }
    return categoryHideButtonAttributes;
}

- (Gui::Color)songsPlayIconHighlightedColor
{
    return [self songsPlayIconColor];
}

- (NSColor *)songsTextSelectedColor
{
    return [self songsTextColor];
}

- (NSColor *)buttonContentColorPressed
{
    return [self buttonContentColor];
}

- (Gui::Color)songsRatingIconHighlightedColor
{
    Gui::Color selectedColor = [self songsSelectedColor];
    const float ratio = 0.5f;
    return Gui::Color(selectedColor.red() * ratio, selectedColor.green() * ratio, selectedColor.blue() * ratio, selectedColor.alpha());
    //auto
}

- (NSColor *)playlistsTextSelectedColor
{
    return [self playlistsTextColor];
}

+ (BOOL) usingCocoaTheme {
	return [manager isKindOfClass: [ThemeCocoa class]];
}

@end
