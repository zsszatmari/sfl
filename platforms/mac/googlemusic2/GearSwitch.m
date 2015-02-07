//
//  GearSwitch.m
//  Gear Player
//
//  Created by Attila Szabo on 20/07/14.
//
//

#import "GearSwitch.h"

static void *switch_context = &switch_context;

static float heightForString(NSString *string, NSFont *font, float width) {
	NSTextStorage *textStorage = [[NSTextStorage alloc] initWithString: string];
	NSTextContainer *textContainer = [[NSTextContainer alloc] initWithContainerSize: NSMakeSize(width, FLT_MAX)];
	NSLayoutManager *layoutManager = [[NSLayoutManager alloc] init];

	[layoutManager addTextContainer: textContainer];
	[textStorage addLayoutManager: layoutManager];
	[textStorage addAttribute: NSFontAttributeName value: font range: NSMakeRange(0, [textStorage length])];
	[textContainer setLineFragmentPadding: .0];
	[layoutManager glyphRangeForTextContainer: textContainer];

	return [layoutManager
        usedRectForTextContainer:textContainer].size.height;
}

@interface GearSwitchSlider : NSView

@property (nonatomic) BOOL disabled;
@property (nonatomic) BOOL inactive;
@end

@implementation GearSwitchSlider

- (id) initWithFrame: (NSRect) frame {
	if ((self = [super initWithFrame: frame])) {
	}
	return self;
}

- (BOOL) isFlipped {
	return YES;
}

- (void) drawRect: (NSRect) rect {
	rect = self.bounds;
	CGFloat parent_width = rect.size.width / 1.6;

	NSGraphicsContext *context = [NSGraphicsContext currentContext];

	[[NSColor colorWithCalibratedRed: .5 green: .6 blue: .7 alpha: 1.] setFill];

	NSRectFill(self.bounds);
	
	NSMutableParagraphStyle *center_align = [[NSMutableParagraphStyle alloc] init];
	[center_align setAlignment: NSCenterTextAlignment];
	[center_align setLineSpacing: .0];

	NSShadow *shadow = [[NSShadow alloc] init];
	shadow.shadowOffset = CGSizeMake(.0, 1.);
    shadow.shadowColor = [NSColor colorWithDeviceRed: .2 green: .4 blue: .5 alpha: 1.];

	NSDictionary *attributes_on = @{NSFontAttributeName: [NSFont fontWithName: @"HelveticaNeue-Bold" size: rect.size.height * .6],
		NSForegroundColorAttributeName: [NSColor whiteColor],
		//NSBackgroundColorAttributeName: [NSColor redColor],
		NSParagraphStyleAttributeName: center_align,
		NSShadowAttributeName: [shadow copy]};

	shadow.shadowColor = [NSColor colorWithDeviceWhite: .5 alpha: 1.];
	
	NSDictionary *attributes_off = @{NSFontAttributeName: [NSFont fontWithName: @"HelveticaNeue-Bold" size: rect.size.height * .6],
		NSForegroundColorAttributeName: [NSColor whiteColor],
		NSParagraphStyleAttributeName: center_align,
		NSShadowAttributeName: shadow};

	CGFloat slider_left, slider_right;
	slider_left = parent_width * .6;
	slider_right = parent_width;

	NSGradient *onGradient = [[NSGradient alloc] initWithStartingColor: [NSColor colorWithCalibratedRed: 37. / 255. green: 128. / 255. blue: 237. / 255. alpha: 1.]
		endingColor: [NSColor colorWithCalibratedRed: 79. / 255. green: 154. / 255. blue: 235. / 255. alpha: 1.]];
	[onGradient drawInRect: NSMakeRect(.0, .0, NSMidX(rect), rect.size.height) angle: 90.];

	NSGradient *offGradient = [[NSGradient alloc] initWithStartingColor: [NSColor colorWithCalibratedWhite: 147. / 255. alpha: 1.]
		endingColor: [NSColor colorWithCalibratedWhite: 182. / 255. alpha: 1.]];
	[offGradient drawInRect: NSMakeRect(NSMidX(rect), .0, NSMidX(rect), rect.size.height) angle: 90.];


	NSAttributedString *on = [[NSAttributedString alloc] initWithString: @"ON" attributes: attributes_on];

	//NSRect bounding = [on boundingRectWithSize: NSMakeSize(slider_left, self.frame.size.height) options: NSStringDrawingUsesFontLeading];

	float height = heightForString(@"ON", [NSFont fontWithName: @"HelveticaNeue-Bold" size: rect.size.height * .6], slider_left);

//	NSLog(@"RECT234: %@ %@ %@ %f", NSStringFromRect(rect), NSStringFromRect(NSMakeRect(.0, (self.frame.size.height - on.size.height) / 2., slider_left, on.size.height)), NSStringFromRect(bounding), height);
//	[on drawInRect: NSMakeRect(.0, (rect.size.height - height) / 2. - (rect.size.height > 15. ? 1 : 0), slider_left, on.size.height)];
//ok	[on drawInRect: NSMakeRect(.0, (rect.size.height - on.size.height) / 2. - (on.size.height - height) / 2., slider_left, on.size.height)];
	CGFloat yshift = (rect.size.height + height) / 2. - on.size.height;
//	NSLog(@"yshift: %f", yshift);
	[on drawInRect: NSMakeRect(.0, yshift, slider_left, on.size.height)];
	
	NSRect sliderRect = NSMakeRect(slider_left, .0, parent_width * .4, self.frame.size.height);
	
	[context saveGraphicsState];
	NSShadow *sliderShadow = [[NSShadow alloc] init];
	sliderShadow.shadowColor = [NSColor colorWithDeviceWhite: .1 alpha: .5];
	sliderShadow.shadowBlurRadius = 3.;
	[sliderShadow set];

	// slider background
	[[NSColor colorWithCalibratedWhite: 1. alpha: 1.] setFill];
	CGFloat radius = rect.size.height / 10. >= 3. ? rect.size.height / 10. : 3.;
	NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect: sliderRect xRadius: radius yRadius: radius];
	[path fill];

	NSGradient *sliderGradient = [[NSGradient alloc] initWithStartingColor: [NSColor colorWithCalibratedWhite: .95 alpha: 1.]
		endingColor:[NSColor colorWithCalibratedWhite: .8 alpha: 1.]];
	[path setClip];
	[sliderGradient drawInRect: NSOffsetRect(sliderRect, .0, 2.) angle: 90.];

	[context restoreGraphicsState];
	
	NSAttributedString *off = [[NSAttributedString alloc] initWithString: @"OFF" attributes: attributes_off];
	[off drawInRect: NSMakeRect(slider_right, yshift, slider_left, off.size.height)];


	if (_disabled || _inactive) {
		CGContextRef qContext = [context graphicsPort];
//		CGContextSetBlendMode(qContext, kCGBlendModeDarken);
        CGColorRef color = CGColorCreateGenericRGB(0.6f, 0.6f, 0.6f, 0.5f);
		CGContextSetFillColorWithColor(qContext, color);
        CGColorRelease(color);
		CGContextFillRect(qContext, NSRectToCGRect(self.bounds));
	}
}

/*
- (void) mouseDown: (NSEvent *) event {
}
*/

@end

@implementation GearSwitch

- (id) initWithFrame: (NSRect) frame {
	if ((self = [super initWithFrame: frame])) {
		_status = GearSwitchStatusOff;
		clipView = [[NSView alloc] initWithFrame: NSMakeRect(.0, .0, frame.size.width, frame.size.height - 1.)];
		slider = [[GearSwitchSlider alloc] initWithFrame: NSMakeRect(-frame.size.width * .6, .0, frame.size.width * 1.6, frame.size.height - 1)];
		[clipView addSubview: slider];
		clipView.wantsLayer = YES;
        clipView.layer = [CALayer layer];
		clipView.layer.cornerRadius = frame.size.height / 10. >= 3. ? frame.size.height / 10. : 3.;
		clipView.layer.borderWidth = 1.;
        CGColorRef color = CGColorCreateGenericRGB(0.46f, 0.46f, 0.46f, 1.0f);
        clipView.layer.borderColor = color;
        CGColorRelease(color);
        
		[self addSubview: clipView];

		[self addObserver: self forKeyPath: @"status" options: NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld context: switch_context];
		[self addObserver: self forKeyPath: @"disabled" options: NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld context: switch_context];
		[self addObserver: self forKeyPath: @"inactive" options: NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld context: switch_context];
	}
	return self;
}

- (BOOL) isFlipped {
	return YES;
}

- (void) observeValueForKeyPath: (NSString *) keyPath ofObject: (id) object change: (NSDictionary *) change context: (void *) context {
	if (context == switch_context) {
		if ([keyPath isEqualToString: @"status"]) {
			if ([change[NSKeyValueChangeNewKey] integerValue] != [change[NSKeyValueChangeOldKey] integerValue]) {
				dispatch_async(dispatch_get_main_queue(), ^{
					[self mouseDown: change[NSKeyValueChangeNewKey]];
				});
			}
		} else if ([keyPath isEqualToString: @"disabled"]) {
			if ([change[NSKeyValueChangeNewKey] boolValue] != [change[NSKeyValueChangeOldKey] boolValue]) {
				slider.disabled = [change[NSKeyValueChangeNewKey] boolValue];
				dispatch_async(dispatch_get_main_queue(), ^{
					[slider setNeedsDisplay: YES];
				});
			}
		} else if ([keyPath isEqualToString: @"inactive"]) {
			if ([change[NSKeyValueChangeNewKey] boolValue] != [change[NSKeyValueChangeOldKey] boolValue]) {
				slider.inactive = [change[NSKeyValueChangeNewKey] boolValue];
				dispatch_async(dispatch_get_main_queue(), ^{
					[slider setNeedsDisplay: YES];
				});
			}
		}
	}  else {
		[super observeValueForKeyPath: keyPath ofObject: object change: change context: context];
	}
}

- (void) setTarget: (id) target onAction: (SEL) onAction offAction: (SEL) offAction {
	self.onOffTarget = target;
	self.onAction = onAction;
	self.offAction = offAction;
}

- (void) drawRect: (NSRect) rect {
	rect = self.bounds;
	CGFloat shift = rect.size.height / 20.;
	shift = 1.;
	[[NSColor colorWithCalibratedWhite: .1 alpha: .7] setFill];
//	[[NSColor redColor] setFill];


	NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:
		NSMakeRect(shift, rect.size.height / 2., rect.size.width - (shift * 2), rect.size.height / 2.) xRadius: rect.size.height / 10. yRadius: rect.size.height / 10.];
//	[path setClip];

	NSGradient *shadowGradient = [[NSGradient alloc] initWithColorsAndLocations: [NSColor colorWithCalibratedWhite: 1. alpha: .3], .0,
		[NSColor colorWithCalibratedWhite: 1. alpha: 1.], .2,
		[NSColor colorWithCalibratedWhite: 1. alpha: 1.], .8,
		[NSColor colorWithCalibratedWhite: 1. alpha: .3], 1.,
		nil];
	[shadowGradient drawInBezierPath: path angle: .0];
}

- (void) sliderToOnWoStatus {
	[NSAnimationContext beginGrouping];
	[[NSAnimationContext currentContext] setDuration: .1];
	NSRect frame = NSMakeRect(.0, .0, slider.frame.size.width, slider.frame.size.height);
	[[slider animator] setFrame: frame];
	[NSAnimationContext endGrouping];
}

- (void) sliderToOffWoStatus {
	[NSAnimationContext beginGrouping];
	[[NSAnimationContext currentContext] setDuration: .1];
	NSRect frame = NSMakeRect(-self.frame.size.width * .6, .0, slider.frame.size.width, slider.frame.size.height);
	[[slider animator] setFrame: frame];
	[NSAnimationContext endGrouping];
}

//       [   switch   ]
// |  on [slider] off | ->
//    <- |  on [slider] off |
- (void) mouseDown: (id) obj {
	if (_disabled) {
		return;
	}

	if ([obj isKindOfClass: [NSEvent class]] || obj == nil) {
		_clicked = YES;
		_status = (_status == GearSwitchStatusAnimatingToOff || _status == GearSwitchStatusOff) ? GearSwitchStatusAnimatingToOn : GearSwitchStatusAnimatingToOff;
	} else if ([obj isKindOfClass: [NSNumber class]]) {
		GearSwitchStatus _s = [obj integerValue];
		_status = (_s == GearSwitchStatusAnimatingToOff || _s == GearSwitchStatusOff) ? GearSwitchStatusAnimatingToOff : GearSwitchStatusAnimatingToOn;
	}

	// off -> on
	if (_status == GearSwitchStatusAnimatingToOn) {
		NSRect frame = NSMakeRect(.0, .0, slider.frame.size.width, slider.frame.size.height);
		_status = GearSwitchStatusAnimatingToOn;
		[NSAnimationContext beginGrouping];
		[[NSAnimationContext currentContext] setDuration: .3];

		[NSAnimationContext currentContext].completionHandler = ^{
			if (_status == GearSwitchStatusAnimatingToOn) {
				_status = GearSwitchStatusOn;
				//NSLog(@"ON !!!");
				_clicked = NO;
				if (_onOffTarget && _onAction) {
					[_onOffTarget performSelector: _onAction withObject: nil afterDelay: .0];
				}
			}
		};
		[[slider animator] setFrame: frame];
		[NSAnimationContext endGrouping];
	// on -> off
	} else if (_status == GearSwitchStatusAnimatingToOff) {
		NSRect frame = NSMakeRect(-self.frame.size.width * .6, .0, slider.frame.size.width, slider.frame.size.height);
		_status = GearSwitchStatusAnimatingToOff;
		[NSAnimationContext beginGrouping];
		[[NSAnimationContext currentContext] setDuration: .3];
		[NSAnimationContext currentContext].completionHandler = ^{
			if (_status == GearSwitchStatusAnimatingToOff) {
				_status = GearSwitchStatusOff;
				_clicked = NO;
				//NSLog(@"OFF !!!");
				if (_onOffTarget && _onAction) {
					[_onOffTarget performSelector: _offAction withObject: nil afterDelay: .0];
				}
			}
		};
		[[slider animator] setFrame: frame];
		[NSAnimationContext endGrouping];
	}
}

@end
