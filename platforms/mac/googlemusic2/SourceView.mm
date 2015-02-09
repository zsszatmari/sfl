//
//  SourceView.m
//  Gear for Google Play
//
//  Created by Zsolt Szatmari on 20/11/14.
//
//

#import "SourceView.h"
#include "ISong.h"
#include "SignalConnection.h"
#include "OfflineState.h"
#include "Painter.h"
#include "IApp.h"
#include "SessionManager.h"
#import "CocoaThemeManager.h"

using namespace Gear;
using namespace Base;

@implementation SourceView {
    SignalConnection _offlineConnection;
    SignalConnection _offlineRatioConnection;
    shared_ptr<OfflineState> _offlineState;
    shared_ptr<ISong> _song;
    
    BOOL _offline;
    float _ratio;
}

- (void)drawRect:(NSRect)dirtyRect
{
    CGRect cellFrame = self.bounds;
    CGRect bounds = NSRectToCGRect(NSMakeRect(cellFrame.origin.x, cellFrame.origin.y, cellFrame.size.height, cellFrame.size.height));
    bounds = CGRectInset(bounds, 2, 2);
    
    if (_offline) {
        CGRect ellipseBounds = CGRectInset(bounds, 1.25, 1.25);
        CGContextRef ctx = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];
        //bounds = CGRectInset(bounds, 2., 2.);
        
        CGContextAddEllipseInRect(ctx, ellipseBounds);
        NSString *prefix = [[CocoaThemeManager sharedManager] themePrefix];
        CGColorRef color;
        CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
        if (prefix == nil) {
            CGFloat comps[] = {206.0f/255.0f,206.0f/255.0f,206.0f/255.0f,1.0f};
            color = CGColorCreate(space, comps);
        } else {
            CGFloat comps[] = {80.0f/255.0f,80.0f/255.0f,80.0f/255.0f,1.0f};
            color = CGColorCreate(space, comps);
        }
        
        CGContextSetStrokeColorWithColor(ctx, color);
        CGContextSetFillColorWithColor(ctx, color);
        CGColorRelease(color);
        CGColorSpaceRelease(space);
        
        CGContextStrokePath(ctx);
        CGContextBeginPath(ctx);
        CGContextMoveToPoint(ctx, CGRectGetMidX(ellipseBounds), CGRectGetMidY(ellipseBounds));
        CGContextAddArc(ctx, CGRectGetMidX(ellipseBounds), CGRectGetMidY(ellipseBounds), CGRectGetWidth(ellipseBounds) / 2, -M_PI / 2, -M_PI / 2 + 2 * M_PI * _ratio, false);
        CGContextClosePath(ctx);
        CGContextFillPath(ctx);
    }
    
    shared_ptr<Gui::IPaintable> sourceImage = Gear::IApp::instance()->sessionManager()->sessionIcon(_song);
    // auto named = dynamic_pointer_cast<Gui::NamedImage>(sourceImage);
    // NSLog(@"sourceimage: %@ %llu %llu", Gear::convertString(named ? named->imageName() : "nada"), (uint64_t)(_song.get()), (uint64_t)(_song ? _song->session().get() : 0));
    
    if (sourceImage) {
        Gui::Painter::paint(*sourceImage, NSRectToCGRect(NSMakeRect(bounds.origin.x + 3, bounds.origin.y + 3, bounds.size.height - 6, bounds.size.height - 6)));
    }
}

- (void)setSong:(shared_ptr<Gear::ISong>)song
{
    _song = song;
    _offline = _song->offlineState()->offlineConnector().value();
    _ratio = _song->offlineState()->ratioConnector().value();
    
    _offlineState = song->offlineState();

    __weak SourceView *wself = self;
    _offlineConnection = _offlineState->offlineConnector().connect([wself,song](bool offline){
        
        SourceView *s = wself;
        if (s && song == s->_song) {
            s->_offline = offline;
            [s setNeedsDisplay:YES];
        }
    });
    _offlineRatioConnection = _offlineState->ratioConnector().connect([wself,song](float ratio){
        SourceView *s = wself;
        if (s && song == s->_song) {
            s->_ratio = ratio;
            [s setNeedsDisplay:YES];
        }
    });
    [self setNeedsDisplay:YES];
}

- (BOOL)isFlipped
{
    return YES;
}

@end
