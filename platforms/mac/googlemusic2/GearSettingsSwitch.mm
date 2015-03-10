//
//  GearSettingsSwitch.mm
//  Gear Player
//
//  Created by Attila Szabo on 29/07/14.
//
//

#import "GearSettingsSwitch.h"

using namespace Gear;

@interface GearSettingsSwitch () {
	Preference _preference;
	NSProgressIndicator *_wheel;
}

@end

@implementation GearSettingsSwitch

- (void) setPreference: (Gear::Preference &) preference {
	_preference = preference;
	[self setTarget: self onAction: @selector(on) offAction: @selector(off)];
}

- (void) on {
	if (_preference.valueOnOff() != Preference::OnOffState::On) {
		_preference.setValue(static_cast<int64_t> (Preference::OnOffState::On));
	}
}

- (void) off {
	if (_preference.valueOnOff() != Preference::OnOffState::Off) {
		_preference.setValue(static_cast<int64_t> (Preference::OnOffState::Off));
	}
}

- (void) checkSpinning {
	if (_preference.valueOnOff() == Preference::OnOffState::OnSpinning) {
		if (_wheel == nil) {
			_wheel = [[NSProgressIndicator alloc] initWithFrame: NSMakeRect(self.frame.origin.x - self.frame.size.height - 10, self.frame.origin.y, self.frame.size.height, self.frame.size.height)];
			[_wheel setStyle: NSProgressIndicatorSpinningStyle];
			[self.superview addSubview: _wheel];
		}
		[_wheel setHidden: NO];
		[_wheel startAnimation: self];
		[self sliderToOnWoStatus];
	} else {
		[_wheel stopAnimation: self];
		[_wheel setHidden: YES];
	}
}


- (void) checkPrefStatus {
	[self checkSpinning];
	if (self.clicked) {
		return;
	}
	switch (_preference.valueOnOff()) {
		case Preference::OnOffState::Disabled:
			self.disabled = YES;
			self.inactive = NO;
			break;
		case Preference::OnOffState::Inactive:
			self.disabled = NO;
			self.inactive = YES;
			break;
		case Preference::OnOffState::On:
			self.disabled = NO;
			self.inactive = NO;
			self.status = GearSwitchStatusOn;
			break;
		case Preference::OnOffState::Off:
			self.disabled = NO;
			self.inactive = NO;
			self.status = GearSwitchStatusOff;
			break;
		default:
			break;
	}

}

@end
