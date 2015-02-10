//
//  GearSettingsSwitch.h
//  Gear Player
//
//  Created by Attila Szabo on 29/07/14.
//
//

#import "GearSwitch.h"
#include "Preference.h"

@interface GearSettingsSwitch : GearSwitch

- (void) setPreference: (Gear::Preference &) preference;
- (void) checkPrefStatus;
@end
