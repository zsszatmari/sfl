//
//  ThemeHelper.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/5/13.
//
//

#import <Foundation/Foundation.h>

extern NSString * const kThemeModernDark
;
extern NSString * const kThemeCocoa;
extern NSString * const kUserDefaultsKeyTheme;

@interface ThemeHelper : NSObject

@property (nonatomic, readonly) NSArray *availableThemes;
@property (nonatomic, readonly) NSArray *themeNames;

@end
