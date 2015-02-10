//
//  ThemeHelper.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/5/13.
//
//

#import "ThemeHelper.h"

NSString * const kThemeModernDark = @"ThemeModernDark";
NSString * const kThemeCocoa = @"ThemeCocoa";
NSString * const kUserDefaultsKeyTheme = @"VisualTheme";

@implementation ThemeHelper

+ (NSDictionary *)themesToNames
{
    return @{kThemeModernDark : @"Modern Dark",
             kThemeCocoa      : @"Cocoa"};
}

- (NSDictionary *)themesToNames
{
    return [ThemeHelper themesToNames];
}

- (NSArray *)availableThemes
{
    return [[self themesToNames] allKeys];
}

- (NSArray *)themeNames
{
    return [[self themesToNames] allValues];
}

@end

@interface NSString (ThemeNames)
@end

@implementation NSString (ThemeNames)

- (NSString *)themeName
{
    return [[ThemeHelper themesToNames] objectForKey:self];
}

@end