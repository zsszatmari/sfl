//
//  PrefAccounts.mm
//  Gear Player
//
//  Created by Attila Szabo on 23/07/14.
//
//

#import "PrefAccounts.h"
#import "GearSettingsSwitch.h"
#include "IApp.h"
#include "IPreferencesPanel.h"
#include "PreferenceGroup.h"
#include "Preference.h"
#include "StringUtility.h"

@interface FlippedView : NSView
@end

@implementation PrefAccounts

using namespace Gear;

- (void) loadView {
	self.view = [[FlippedView alloc] init];
}

- (void) viewDidLoad
{
    auto preferencesPanel = IApp::instance()->preferencesPanel();
	std::vector<Gear::PreferenceGroup> preferenceGroup = preferencesPanel->preferenceGroups();
	auto prefGroup = preferenceGroup.at(0);
	auto preferences = prefGroup.preferences();
	int i;
	CGFloat x = (400. - 220.) / 2.;
	//NSLog(@"rect: %@", NSStringFromRect(self.view.frame));
//	NSMutableArray *switches = [[NSMutableArray alloc] initWithCapacity: 4];
	for (i = 0; i < preferences.size(); i++) {
		auto pref = preferences.at(i);
		auto prefType = pref.type();
		if (prefType == Preference::Type::OnOff) {
			NSTextField *text = [[NSTextField alloc] initWithFrame: NSMakeRect(x, 10 + i * 30, 110, 20)];
			[text setDrawsBackground: NO];
//			text.backgroundColor = [NSColor redColor];
			[text setEditable: NO];
			[text setSelectable: NO];
			[text setBezeled: NO];
			[text setStringValue: convertString(pref.title())];
			[self.view addSubview: text];

			GearSettingsSwitch *s = [[GearSettingsSwitch alloc] initWithFrame: NSMakeRect(x + 160, 10 + i * 30, 60, 20)];
			[self.view addSubview: s];
			[s setPreference: pref];
//			[switches addObject: s];

			dispatch_source_t _timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
			dispatch_source_set_timer(_timer, dispatch_time(DISPATCH_TIME_NOW, 0), 500 * USEC_PER_SEC, 100 * USEC_PER_SEC);
			dispatch_source_set_event_handler(_timer, ^{
				[s checkPrefStatus];
			});
			dispatch_resume(_timer);
		}
	}
/*
	dispatch_source_t _timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0));
	dispatch_source_set_timer(_timer, dispatch_time(DISPATCH_TIME_NOW, 0), 500 * USEC_PER_SEC, 100 * USEC_PER_SEC);
	dispatch_source_set_event_handler(_timer, ^{
		[switches makeObjectsPerformSelector: @selector(checkPrefStatus)];
	});
	dispatch_resume(_timer);
*/
}

- (void) startPolling {
	
}

@end
