//
//  OutputsController.m
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 28/01/14.
//
//

#include <vector>
#import <CoreAudio/CoreAudio.h>
#import <QuartzCore/QuartzCore.h>
#import "OutputsController.h"
#include "AudioDevice.h"
#include "StringUtility.h"
#import "PlaybackController.h"
#import "MainWindowController.h"
#import "AppDelegate.h"
#include "IApp.h"
#include "IPlaybackWorker.h"

using namespace Gear;
using namespace std;

@interface FlippedView : NSView
@end

@implementation FlippedView

- (BOOL)isFlipped
{
    return YES;
}

@end

@implementation OutputsController {
    NSPopover *popover;
    NSWindow *window;
    NSButton *outputsButton;
    vector<Gear::AudioDevice> _availableDevices;
    //vector<pair<AudioDevice, bool>> devicesSelected;
    BOOL multiMode;
    //AudioObjectID multiDevice;
    //NSMutableArray *multiDevices;
}

static NSString * const kUserDefaultsOutputDevice = @"SelectedOutputDevice";
static NSString * const kUserDefaultsMultiOutputDevices = @"SelectedMultiOutputDevice";

+ (OutputsController *)sharedController
{
    static dispatch_once_t onceToken = 0;
    static OutputsController *controller = nil;
    dispatch_once(&onceToken, ^{
        controller = [[OutputsController alloc] init];
    });
    return controller;
}
 
- (id)init
{
    self = [super init];
    if (self) {
        
        //[self setupOutputs];
        
        if (!PlaybackController::instance()->resetVolume()) {
            
            float sliderValue = [[[[NSUserDefaultsController sharedUserDefaultsController] values] valueForKey:@"volume"] floatValue];
            
            MainWindowController *controller = [[AppDelegate sharedDelegate] mainWindowController];
            [controller.volumeSlider setFloatValue:sliderValue];
            [controller volumeChange:controller.volumeSlider];
        }
        
    }
    return self;
}

- (void)show:(NSButton *)sender
{
    outputsButton = sender;
    
    //[self setupOutputs];
    
    CGRect viewRect = CGRectMake(0, 0, 265, 100);
    NSView *view = [[FlippedView alloc] initWithFrame:viewRect];
    view.autoresizesSubviews = YES;
    
    self.view = view;
    [self populateView];
    
    BOOL popoverPossible = NSClassFromString(@"NSPopover") != nil;
//#define DEBUG_SIMULATE_SNOWLEO
#ifdef DEBUG_SIMULATE_SNOWLEO
    popoverPossible = NO;
#endif
    
    if (!popoverPossible) {
        
        CGRect buttonRectInWindow = [[sender superview] convertRect:[sender frame] toView:nil];
        // this is not yet available in snow leopard:
        //CGRect buttonRect = [[sender window] convertRectToScreen:buttonRectInWindow];
        CGRect buttonRect = {[[sender window]convertBaseToScreen:buttonRectInWindow.origin], buttonRectInWindow.size};
        CGRect windowRect = CGRectMake(buttonRect.origin.x - 264/2 + 40, buttonRect.origin.y + 20, 265,view.frame.size.height);
        window = [[NSWindow alloc] initWithContentRect:windowRect styleMask:NSTitledWindowMask | NSClosableWindowMask backing:NSBackingStoreBuffered defer:NO];
        window.title = @"Outputs";
        window.contentView = view;
        [window setDelegate:self];
        [window orderFront:nil];
        [window setReleasedWhenClosed:NO];
        
    } else {
        popover = [[NSPopover alloc] init];
        popover.contentViewController = self;
        popover.behavior = NSPopoverBehaviorTransient;
        popover.animates = YES;
        popover.delegate = self;
        
        [popover showRelativeToRect:[sender frame] ofView:[sender superview] preferredEdge:CGRectMaxYEdge];
    }
    
}

- (void)popoverDidClose:(NSNotification *)notification
{
    [outputsButton setIntValue:0];
}

- (void)windowWillClose:(NSNotification *)notification
{
    [outputsButton setIntValue:0];
    window = nil;
}

- (void)populateView
{
    for (NSView *view in [[self.view subviews] copy]) {
        if ([view isKindOfClass:[NSButton class]] || [view isKindOfClass:[NSTextField class]]) {
            [view removeFromSuperview];
        }
    }
    
    int y = 0;
    static const CGFloat kRowHeight = 22;
    static const CGFloat kCheckmarkWidth = 20;
    CGRect viewRect = self.view.frame;
    int i = 0;
    
    auto addButton = [&](const string &text, bool checkmark){
        NSButton *button = [[NSButton alloc] initWithFrame:CGRectMake(0, y, viewRect.size.width - kCheckmarkWidth, kRowHeight)];
        [button setBordered:NO];
        [button setTitle:convertString(text)];
        [button setAlignment:NSLeftTextAlignment];
        [[button cell] setHighlightsBy:0];
        [button setTarget:self];
        [self.view addSubview:button];
        
        if (checkmark) {
            NSTextField *checkmark = [[NSTextField alloc] initWithFrame:CGRectMake(viewRect.size.width - kCheckmarkWidth, y, kCheckmarkWidth, kRowHeight)];
            [checkmark setStringValue:@"\u2713"];
            checkmark.bezeled = NO;
            checkmark.drawsBackground = NO;
            checkmark.editable = NO;
            checkmark.selectable = NO;
            [self.view addSubview:checkmark];
        }
        y += kRowHeight;
        
        return button;
    };
    
    auto allDevices = IApp::instance()->playbackWorker()->availableDevices();
    auto devicesSelected = IApp::instance()->playbackWorker()->devices();
    _availableDevices = allDevices;
    if (devicesSelected.size() > 1) {
        multiMode = YES;
    }
    for (auto &device : allDevices) {
        
        NSButton *button = addButton(device.name(), find(devicesSelected.begin(), devicesSelected.end(), device) != devicesSelected.end());
        /*if (multiMode && device.deviceId() == 0) {
            [button setTarget:nil];
            button.enabled = NO;
        }*/
        [button setAction:@selector(outputPressed:)];
        [button setTag:i];
        ++i;
    }
    NSButton *button = addButton("Multiple Outputs", multiMode);
    [button setAction:@selector(multiPressed:)];
    
    viewRect.size.height = y;
    self.view.frame = viewRect;
    popover.contentSize = viewRect.size;
}

- (void)flash:(NSView *)sender
{
    CGRect frame = [sender frame];
    frame.size.width = [[sender superview] frame].size.width;
    NSView *flash = [[NSView alloc] initWithFrame:frame];
    flash.autoresizingMask = NSViewMinYMargin;
    [flash setWantsLayer:YES];
    CGColorRef blueColor = CGColorCreateGenericRGB(31.0f/255.0f,107.0f/255.0f,218.0f/255.0f, 1);
    CGColorRef clearColor = CGColorCreateGenericRGB(0, 0, 0, 0);
    CALayer *layer = flash.layer;
    [layer setBackgroundColor:clearColor];
    
    CABasicAnimation *flashAnim = [CABasicAnimation animationWithKeyPath:@"backgroundColor"];
    flashAnim.fromValue = (__bridge id)blueColor;
    flashAnim.toValue = (__bridge id)clearColor;
    flashAnim.duration = 0.2f;        // 1 second
    flashAnim.autoreverses = NO;    // Back
    flashAnim.repeatCount = 1;       // Or whatever
    
    [layer addAnimation:flashAnim forKey:@"flashAnimation"];
    
    [[sender superview] addSubview:flash];
    CGColorRelease(blueColor);
    CGColorRelease(clearColor);
    
    double delayInSeconds = 0.2f;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        [flash removeFromSuperview];
    });
}

- (BOOL)multipleAirplayPossible
{
    if (floor(NSFoundationVersionNumber) > NSFoundationVersionNumber10_8_4) {
        return NO;
    }

    return YES;
}

- (void)outputPressed:(id)sender
{
    [self flash:sender];
    
    AudioDevice device = _availableDevices.at([sender tag]);
    auto deviceId = device.deviceId();
    
    auto worker = IApp::instance()->playbackWorker();
    if (multiMode) {
        
        auto selected = worker->devices();
        id object = @(deviceId);
        
        auto it = find(selected.begin(), selected.end(), device);
        if (it != selected.end()) {
            if (selected.size() > 1) {
                selected.erase(it);
            }
        } else {
            // it is important to insert to beginning, so that the newly selected item would take precedence            
            selected.insert(selected.begin(), device);
        }
        
        worker->setDevices(selected);
        
        //[[NSUserDefaults standardUserDefaults] setObject:multiDevices forKey:kUserDefaultsMultiOutputDevices];
        //[self setAggregateDevices];
        //PlaybackController::instance()->selectDeviceId(multiDevice);
    } else {
        //IApp::instance()->playbackWorker().setSources(vector<uint32_t>());
        
        worker->setDevices(vector<AudioDevice>(1, device));
    }
        
    //[self setupOutputs];
    [self populateView];
}

- (void)multiPressed:(id)sender
{
    [self flash:sender];
    
    if (multiMode) {
        multiMode = NO;
        auto worker = IApp::instance()->playbackWorker();
        auto devices = worker->devices();
        if (devices.size() == 0) {
            auto availableDevices = worker->availableDevices();
            if (availableDevices.size() > 0) {
                [self setDevice:availableDevices.front()];
            }
        } else if (devices.size() == 1) {
            // nada
        } else {
            [self setDevice:devices.front()];
        }
    } else {
        multiMode = YES;
    }
    
    [self populateView];
}

- (void)setDevice:(AudioDevice)device
{
    IApp::instance()->playbackWorker()->setDevices(vector<AudioDevice>(1, device));
    //PlaybackController::instance()->selectDeviceId(deviceId);
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults setObject:@(device.deviceId()) forKey:kUserDefaultsOutputDevice];
}

@end
