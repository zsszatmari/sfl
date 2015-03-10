//
//  NaviBarView.m
//  Gear for Google Play and YouTube
//
//  Created by Zsolt Szatmari on 20/02/15.
//
//

#import "NaviBarView.h"
#include "sfl/Prelude.h"
#include "StringUtility.h"
#include "IApp.h"

using namespace sfl;
using namespace Gear;

@implementation NaviBarView {
    vector<NSButton *> stackButtons;
    vector<NSButton *> viewButtons;
    std::map<NSButton *,std::function<void()>> actions;
}

- (id)init
{
    if (self = [super init]) {
        
    }
    return self;
}

/*
- (void)drawRect:(NSRect)dirtyRect
{
    [[NSColor redColor] setFill];
    NSRectFill(dirtyRect);
}*/

- (void)setInfo:(const Gear::NaviBarInfo &)info
{
    for (auto b : viewButtons) {
        [b removeFromSuperview];
    }
    for (auto b : stackButtons) {
        [b removeFromSuperview];
    }
    actions.clear();
    
    auto create = [self](auto p){
        NSButton *button = [[NSButton alloc] init];
        button.translatesAutoresizingMaskIntoConstraints = NO;
        button.title = convertString(p.first);
        
        [self addSubview:button];
        [button setTarget:self];
        [button setAction:@selector(action:)];
        actions[button] = [=]{
            IApp::instance()->userSelectedPlaylist(p.second, false);
        };
        
        [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[button]|" options:0 metrics:nil views:NSDictionaryOfVariableBindings(button)]];
        return button;
    };
    
    viewButtons = map(create, info.views);
    stackButtons = map(create, info.stack);
    
    auto glueTogether = [=](auto buttons){
        if (buttons.size() > 0) {
            for (auto &p : zip(buttons, tail(buttons))) {
                [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"[left][right]" options:0 metrics:nil views:@{@"left":p.first,@"right":p.second}]];
            }
        }
    };
    glueTogether(viewButtons);
    glueTogether(stackButtons);
    
    if (!viewButtons.empty()) {
        [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"[button]|" options:0 metrics:nil views:@{@"button":last(viewButtons)}]];
    }
    if (!stackButtons.empty()) {
        [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[button]" options:0 metrics:nil views:@{@"button":head(stackButtons)}]];
    }
}

- (void)action:(id)sender
{
    actions[sender]();
}

@end
