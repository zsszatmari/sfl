//
//  GearVolumeSliderCell.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/2/13.
//
//

#import "CocoaThemeManager.h"
#import "GearVolumeSliderCell.h"
#include "IApp.h"
#include "ThemeManager.h"
#include "ITheme.h"
#include "Painter.h"

@implementation GearVolumeSliderCell

/*
- (NSImage *)knobImage
{
    NSGradient *gradient = [[CocoaThemeManager sharedManager] ovalButtonGradient:NO];
    if (gradient == nil) {
        
        auto knob = Gear::IApp::instance()->themeManager()->current()->volumeSlider().thumb;
        return Gui::Painter::convertImage(*knob);
    } else {
        return [NSImage imageNamed:@"knob-big"];
    }
}*/

- (CGFloat)barWidth
{
    return 10;
}

/*
- (void)drawKnob:(NSRect)knobRect
{
    NSGradient *gradient = [[CocoaThemeManager sharedManager] ovalButtonGradient:NO];
    if (gradient == nil) {
        knobRect.origin.y -= 1;
    }
    [super drawKnob:knobRect];
}

- (CGFloat)knobSize 
{
    NSGradient *gradient = [[CocoaThemeManager sharedManager] ovalButtonGradient:NO];
    if (gradient == nil) {
        return 0;
    } else {
        return 22;
    }
}
*/

@end
