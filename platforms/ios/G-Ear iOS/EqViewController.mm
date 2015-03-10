//
//  EqViewController.m
//  G-Ear iOS
//
//  Created by Zsolt Szatmari on 14/02/15.
//  Copyright (c) 2015 Treasure Box. All rights reserved.
//

#import "EqViewController.h"
#import "EqSlider.h"
#include "sfl/Prelude.h"
#include "IApp.h"
#include "IEqualizer.h"
#include "StringUtility.h"
#include "IPreferences.h"
#import "AppDelegate.h"

using std::vector;
using std::string;
using namespace sfl;
using namespace Gear;

@interface TouchingView : UIView

@property (nonatomic,assign) std::function<void(CGPoint)> onTouch;

@end

@implementation TouchingView

- (void)touchesMoved:(NSSet *)touches
           withEvent:(UIEvent *)event
{
    if ([touches count] > 0) {
        function<void(CGPoint)> f = self.onTouch;
        CGPoint p = [[touches anyObject] locationInView:self];
        f(p);
    }
}

@end

@implementation EqViewController {
    UIButton *presetButton;
    UIButton *onButton;
    UIButton *widenButton;
    EqSlider *preSlider;
    std::vector<EqSlider *> sliders;
    std::vector<std::function<void()>> actionSheetSelect;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    [[UIApplication sharedApplication] setStatusBarHidden:YES withAnimation:UIStatusBarAnimationFade];
}

+ (void)showFrom:(UIViewController *)controller;
{
    UIViewController *eq = [[EqViewController alloc] init];

    //[controller.navigationController pushViewController:eq animated:YES];
    
    // does not work
    //controller.modalPresentationStyle = UIModalPresentationOverCurrentContext;
    [controller presentViewController:eq animated:YES completion:^{}];
}

- (NSUInteger)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskLandscape;
}

- (void)loadView
{
    self.view = [[UIView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.view.backgroundColor = [UIColor whiteColor];

    auto slider = [self](const std::string &label, const std::string &key){
        EqSlider *s = [[EqSlider alloc] init];
        [self.view addSubview:s];
        s.translatesAutoresizingMaskIntoConstraints = NO;
        [self.view addConstraint:[NSLayoutConstraint constraintWithItem:s attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeTop multiplier:1 constant:50]];
        [self.view addConstraint:[NSLayoutConstraint constraintWithItem:s attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeBottom multiplier:1 constant:-30]];
        
        s.labelText = label;
        s.key = key;
        [s update:NO];
        s.onUpdate = ^{
            [self updateTitle];
            [self updateOnButton];
        };
        return s;
    };

    preSlider = slider("Pre", "eqPre");

    vector<string> texts = IApp::instance()->equalizer()->bandLabels();
    auto keys = map([](int i){return std::string("eq") + std::to_string(i);}, sequence(0,1,texts.size()-1));
    sliders = map([&](const std::pair<string,string> &p){return slider(p.first,p.second);}, zip(texts,keys));
    
    auto addGapBetween= [self](UIView *left, UIView *right, float gapWidth){
        [self.view addConstraint:[NSLayoutConstraint constraintWithItem:left attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:right attribute:NSLayoutAttributeLeft multiplier:1 constant:-gapWidth]];
    };
    
    UIButton *cancelButton = [UIButton buttonWithType: UIButtonTypeCustom];
    [cancelButton setTitleColor: [UIColor colorWithRed: 66. / 255. green: 127. / 255. blue: 237. / 255. alpha: 1.] forState: UIControlStateNormal];
    cancelButton.frame = CGRectMake(0, 17, 80, 22);
    [cancelButton setTitle: @"Back" forState: UIControlStateNormal];
    cancelButton.titleLabel.font = [UIFont systemFontOfSize:15.0f];
    [cancelButton addTarget:self action: @selector(back:) forControlEvents: UIControlEventTouchUpInside];
    [self.view addSubview: cancelButton];
    
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:preSlider attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:cancelButton attribute:NSLayoutAttributeLeft multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:preSlider attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:cancelButton attribute:NSLayoutAttributeRight multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:last(sliders) attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeRight multiplier:1 constant:-20]];
    
    addGapBetween(preSlider, sliders[0], 30);
    for (auto &p : zip(sliders,tail(sliders))) {
        [self.view addConstraint:[NSLayoutConstraint constraintWithItem:p.first attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:p.second attribute:NSLayoutAttributeWidth multiplier:1 constant:0]];
        addGapBetween(p.first,p.second,0);
    }
    
    
    // we should be able to draw an eq curve continously...
    TouchingView *touchingView = [[TouchingView alloc] init];
    //touchingView.backgroundColor = [[UIColor redColor] colorWithAlphaComponent:0.3f];
    touchingView.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:touchingView];
    
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:touchingView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:preSlider attribute:NSLayoutAttributeLeft multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:touchingView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:preSlider attribute:NSLayoutAttributeTop multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:touchingView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:preSlider attribute:NSLayoutAttributeBottom multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:touchingView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:last(sliders) attribute:NSLayoutAttributeRight multiplier:1 constant:0]];
 
    touchingView.onTouch = [=](CGPoint point){
        for (auto slider : snoc(sliders,preSlider)) {
            CGPoint inside = [touchingView convertPoint:point toView:slider];
            if (CGRectContainsPoint(slider.bounds, inside)) {
                [slider userInput:inside];
            }
        }
    };
    
    
    presetButton = [UIButton buttonWithType: UIButtonTypeCustom];
    presetButton.translatesAutoresizingMaskIntoConstraints = NO;
    [presetButton setTitleColor:[cancelButton titleColorForState:UIControlStateNormal] forState:UIControlStateNormal];
    presetButton.titleLabel.font = cancelButton.titleLabel.font;
    [presetButton addTarget:self action:@selector(presetTapped:) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:presetButton];
    
    onButton = [UIButton buttonWithType: UIButtonTypeCustom];
    onButton.translatesAutoresizingMaskIntoConstraints = NO;
    [onButton setTitleColor:[cancelButton titleColorForState:UIControlStateNormal] forState:UIControlStateNormal];
    onButton.titleLabel.font = cancelButton.titleLabel.font;
    [onButton addTarget:self action:@selector(onTapped:) forControlEvents:UIControlEventTouchUpInside];
    [self updateOnButton];
    [self.view addSubview:onButton];
    
    widenButton = [UIButton buttonWithType: UIButtonTypeCustom];
    widenButton.translatesAutoresizingMaskIntoConstraints = NO;
    [widenButton setTitle:@"Widen" forState:UIControlStateNormal];
    [widenButton setTitleColor:[cancelButton titleColorForState:UIControlStateNormal] forState:UIControlStateNormal];
    widenButton.titleLabel.font = cancelButton.titleLabel.font;
    [widenButton addTarget:self action:@selector(widenTapped:) forControlEvents:UIControlEventTouchUpInside];
    [self updateWidenButton];
    [self.view addSubview:widenButton];
    
    NSDictionary *views = NSDictionaryOfVariableBindings(onButton,presetButton,widenButton);
    [self.view addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"[onButton][presetButton][widenButton(==55)]" options:NSLayoutFormatAlignAllCenterY metrics:nil views:views]];
    
    // reach to the top for easier pressing (looks strange)
    //[self.view addConstraint:[NSLayoutConstraint constraintWithItem:presetButton attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeTop multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:onButton attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:cancelButton attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:onButton attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:head(sliders) attribute:NSLayoutAttributeCenterX multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:widenButton attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:last(sliders) attribute:NSLayoutAttributeCenterX multiplier:1 constant:0]];
    
    [self updateTitle];
}

- (void)back:(id)sender
{
    [self dismissViewControllerAnimated:YES completion:^{}];
}

- (void)presetTapped:(id)sender
{
    UIActionSheet *options = [[UIActionSheet alloc]
                              initWithTitle:@"EQ"
                              delegate:self cancelButtonTitle:nil
                              destructiveButtonTitle:nil
                              otherButtonTitles: nil];
    
    auto presets = IApp::instance()->equalizer()->queryPresetNames();
    auto currentIndex = IApp::instance()->equalizer()->currentPreset();

    int i = 0;
    actionSheetSelect.clear();
    bool currentModified = IApp::instance()->equalizer()->currentModified();
    for (auto &name : presets) {
        
        NSString *title = convertString(name);
        if (i == currentIndex) {
            title = [NSString stringWithFormat:@"   %@ ✓",title];
        }
        [options addButtonWithTitle:title];
        actionSheetSelect.push_back([=]() mutable {
            if (i == currentIndex) {
                return;
            } else {
                if (i == 0) {
                    IApp::instance()->preferences().setUintForKey("eqEnabled", false);
                }
                IApp::instance()->equalizer()->selectPreset(i);
                [self updateAll];
            }
        });
        i++;
    }
    
    [options addButtonWithTitle:@"Browse factory presets..."];
    actionSheetSelect.push_back([=]{
        [self showFactoryPresets];
    });
    ++i;
    
    if (currentModified) {
        [options addButtonWithTitle:@"Discard changes"];
        actionSheetSelect.push_back([=]{
            IApp::instance()->equalizer()->selectPreset(currentIndex);
            [self updateAll];
        });
        ++i;
        
        [options addButtonWithTitle:@"Save as..."];
        actionSheetSelect.push_back([=]{
            UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Save as..." message:@"" delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"Save", nil];
            alert.alertViewStyle = UIAlertViewStylePlainTextInput;
            [alert textFieldAtIndex:0].placeholder = @"Preset name";
            [alert textFieldAtIndex:0].autocapitalizationType = UITextAutocapitalizationTypeSentences;
            [alert show];
        });
        ++i;
    }

    
    if (currentIndex != 0) {
        if (currentModified) {
            [options addButtonWithTitle:@"Overwrite"];
            actionSheetSelect.push_back([=]{
                IApp::instance()->equalizer()->saveCurrentPresetOverwrite();
                [self updateTitle];
            });
            ++i;
        }
        
        
        [options addButtonWithTitle:@"Delete"];
        // there is a bug in ios 7 which permutes options in a strange way...
        if (![AppDelegate runningPreIOS8]) {
            options.destructiveButtonIndex = i;
        }
        actionSheetSelect.push_back([=]{
            IApp::instance()->equalizer()->deleteCurrentPreset();
            [self updateAll];
        });
        ++i;
    }
    
    options.actionSheetStyle = UIActionSheetStyleBlackOpaque;
    [options showInView:self.view];
}

- (void)showFactoryPresets
{
    using std::make_pair;
    using std::array;
    
    vector<pair<string,array<float,10>>> factory = {
        make_pair(string("Acoustic"),      array<float,10>{{ 4.5, 4.5, 3.8, 0.7, 1.6, 1.5, 3.5, 3.8, 3.5, 1.7}}),
        make_pair(string("Bass Booster"),  array<float,10>{{ 5.4, 4.4, 3.7, 2.7, 1.4, 0.0, 0.0, 0.0, 0.0, 0.0}}),
        make_pair(string("Bass Reducer"),  array<float,10>{{-5.4,-4.4,-3.7,-2.7,-1.4, 0.0, 0.0, 0.0, 0.0, 0.0}}),
        make_pair(string("Classical"),     array<float,10>{{ 4.5, 3.8, 2.9, 2.7,-1.5,-1.4, 0.0, 2.0, 3.1, 3.3}}),
        make_pair(string("Dance"),         array<float,10>{{ 4.0, 6.9, 4.5, 0.0, 1.5, 4.0, 4.8, 4.5, 4.0, 0.0}}),
        make_pair(string("Deep"),          array<float,10>{{ 4.5, 4.0, 1.5, 0.8, 3.0, 2.8, 1.5,-2.0,-4.0,-4.5}}),
        make_pair(string("Electronic"),    array<float,10>{{ 4.3, 4.2, 1.4, 0.0,-2.0, 2.0, 1.0, 1.4, 4.0, 4.5}}),
        make_pair(string("Hip-Hop"),       array<float,10>{{ 4.9, 4.3, 1.5, 3.0,-1.4,-1.4, 1.5,-1.0, 2.0, 2.9}}),
        make_pair(string("Jazz"),          array<float,10>{{ 4.3, 2.9, 1.5, 2.0,-1.5,-1.5, 0.0, 1.5, 2.9, 3.7}}),
        make_pair(string("Latin"),         array<float,10>{{ 4.4, 2.9, 0.0, 0.0,-1.5,-1.5,-1.5, 0.0, 2.9, 4.3}}),
        make_pair(string("Loudness"),      array<float,10>{{ 5.9, 4.0, 0.0, 0.0,-2.0, 0.0,-1.0,-5.0, 4.8, 1.0}}),
        make_pair(string("Lounge"),        array<float,10>{{-3.0,-1.5,-1.0, 1.5, 4.0, 2.2, 0.0,-1.5, 1.7, 1.4}}),
        make_pair(string("Piano"),         array<float,10>{{ 3.0, 2.0, 0.0, 2.8, 3.0, 1.5, 4.0, 4.5, 3.0, 3.7}}),
        make_pair(string("Pop"),           array<float,10>{{-1.5,-1.3, 0.0, 1.6, 4.0, 3.9, 1.7, 0.0,-1.3,-1.5}}),
        make_pair(string("R&B"),           array<float,10>{{ 2.9, 7.0, 5.9, 1.5,-2.0,-1.5, 2.0, 2.9, 3.0, 4.0}}),
        make_pair(string("Rock"),          array<float,10>{{ 5.0, 4.0, 2.9, 1.5,-1.0,-1.3, 0.8, 2.9, 3.2, 4.4}}),
        make_pair(string("Small Speakers"),array<float,10>{{ 5.0, 4.0, 3.3, 2.7, 1.4, 0.0,-1.4,-2.7,-4.0,-4.5}}),
        make_pair(string("Spoken Word"),   array<float,10>{{-4.0,-1.0, 0.0, 1.0, 4.0, 4.5, 4.5, 4.0, 2.7, 0.0}}),
        make_pair(string("Treble Booster"),array<float,10>{{ 0.0, 0.0, 0.0, 0.0, 0.0, 1.4, 2.7, 3.7, 4.4, 5.4}}),
        make_pair(string("Treble Reducer"),array<float,10>{{ 0.0, 0.0, 0.0, 0.0, 0.0,-1.4,-2.7,-3.7,-4.4,-5.4}}),
        make_pair(string("Vocal Booster"), array<float,10>{{-1.5,-3.0,-3.0, 1.5, 4.0, 4.0, 2.9, 1.5, 0.0,-1.5}})
    };
    
    UIActionSheet *options = [[UIActionSheet alloc]
                              initWithTitle:@"EQ"
                              delegate:self cancelButtonTitle:nil
                              destructiveButtonTitle:nil
                              otherButtonTitles: nil];
    
    int i = 0;
    actionSheetSelect.clear();
    for (auto &preset : factory) {
        [options addButtonWithTitle:convertString(preset.first)];
        actionSheetSelect.push_back([=]{
            auto &prefs = IApp::instance()->preferences();
            for (auto &p : zip(sequence(0,1,9),preset.second)) {
                prefs.setFloatForKey(std::string("eq") + std::to_string(p.first), p.second);
            }
            prefs.setUintForKey("eqEnabled", 1);
            prefs.setUintForKey("eqModified", 0);
            IApp::instance()->equalizer()->saveCurrentPreset(preset.first);
            IApp::instance()->equalizer()->notifyChange();
            [self updateAll];
        });
        ++i;
    }
    
    [options addButtonWithTitle:@"Cancel"];
    actionSheetSelect.push_back([=]{
    });
    if (![AppDelegate runningPreIOS8]) {
        options.cancelButtonIndex = i;
    }
    ++i;
    
    options.actionSheetStyle = UIActionSheetStyleBlackOpaque;
    [options showInView:self.view];
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (buttonIndex == 1) {
        std::string title = convertString([alertView textFieldAtIndex:0].text);
        if (!title.empty()) {
            IApp::instance()->equalizer()->saveCurrentPreset(title);
            [self updateTitle];
        }
    }
}


- (void)updateTitle
{
    auto presets = IApp::instance()->equalizer()->queryPresetNames();
    auto index = IApp::instance()->equalizer()->currentPreset();
    [presetButton setTitle:convertString(presets[index]) forState:UIControlStateNormal];
}

- (void)updateOnButton
{
    auto on = IApp::instance()->preferences().boolForKey("eqEnabled");
    
    [onButton setTitleColor:on ? [presetButton titleColorForState:UIControlStateNormal] : [UIColor grayColor] forState:UIControlStateNormal];
    [onButton setTitle:on ? @"On" : @"Off" forState:UIControlStateNormal];
}

- (void)onTapped:(id)sender
{
    auto &prefs = IApp::instance()->preferences();
    prefs.setUintForKey("eqEnabled", !prefs.boolForKey("eqEnabled"));
    IApp::instance()->equalizer()->notifyChange();

    [self updateOnButton];
}

- (void)updateWidenButton
{
    [widenButton setTitleColor:IApp::instance()->preferences().boolForKey("eqWiden") ? [presetButton titleColorForState:UIControlStateNormal] : [UIColor grayColor] forState:UIControlStateNormal];
}

- (void)widenTapped:(id)sender
{
    IApp::instance()->equalizer()->setWiden(!IApp::instance()->preferences().boolForKey("eqWiden"));
    [self updateWidenButton];
}

- (void)updateAll
{
    [self updateTitle];
    [self updateOnButton];
    [UIView animateWithDuration:0.2f animations:^{
        [preSlider update:YES];
        for (auto slider : sliders) {
            [slider update:YES];
        }
    }];
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (buttonIndex < actionSheetSelect.size()) {
        actionSheetSelect.at(buttonIndex)();
    }
}

@end
