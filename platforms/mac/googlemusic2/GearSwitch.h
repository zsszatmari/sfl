//
//  GearSwitch.h
//  Gear Player
//
//  Created by Attila Szabo on 20/07/14.
//
//

#import <Cocoa/Cocoa.h>

typedef NS_ENUM(NSInteger, GearSwitchStatus) {
	GearSwitchStatusOff,
	GearSwitchStatusOn,
	GearSwitchStatusAnimatingToOff,
	GearSwitchStatusAnimatingToOn
};

@class GearSwitchSlider;
@interface GearSwitch : NSView   {
	GearSwitchSlider *slider;
	NSView *clipView;
}

@property (nonatomic) GearSwitchStatus status;
@property (nonatomic) BOOL disabled;
@property (nonatomic) BOOL inactive;
@property (nonatomic) BOOL clicked;
@property (nonatomic, unsafe_unretained) id onOffTarget;
@property (nonatomic, assign) SEL onAction;
@property (nonatomic, assign) SEL offAction;

- (void) setTarget: (id) target onAction: (SEL) onAction offAction: (SEL) offAction;
- (void) sliderToOnWoStatus;
- (void) sliderToOffWoStatus;

@end
