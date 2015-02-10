//
//  GradientView.m
//  Gear for Google Play
//
//  Created by Zsolt Szatmari on 14/11/14.
//
//

#import "GradientView.h"
#import "CocoaThemeManager.h"
#import "MainWindowController.h"
#import "AppDelegate.h"


@implementation GradientView

- (void)drawRect:(NSRect)dirtyRect
{
    
    CocoaThemeManager *manager = [CocoaThemeManager sharedManager];
    NSString *prefix = [manager themePrefix];
    if (prefix == nil) {
        return;
    }
    
    if (self.drawTop) {
        
        NSImage *top = [NSImage imageNamed:[prefix stringByAppendingString:@"-topgradient"]];
        CGRect topRect = [[AppDelegate sharedDelegate] mainWindowController].topView.frame;
        CGFloat height = topRect.size.height;
        CGRect drawRect = CGRectMake(dirtyRect.origin.x, 0, dirtyRect.size.width, height);
        if (CGRectIntersectsRect(drawRect, dirtyRect)) {
            
            [top drawInRect:drawRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0f respectFlipped:YES hints:nil];
        }
    } else {
        
        NSImage *bottom = [NSImage imageNamed:[prefix stringByAppendingString:@"-bottomgradient"]];
        // +2 is needed because the image is a little too small
        CGRect drawRect = CGRectMake(dirtyRect.origin.x, 0, dirtyRect.size.width, bottom.size.height+3);
        
        if (CGRectIntersectsRect(drawRect, dirtyRect)) {
            
            [bottom drawInRect:drawRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0f respectFlipped:YES hints:nil];
        }
    }
    
    
    // disable title
    /*
     // in full screen there is no title
     if (([[self window] styleMask] & NSFullScreenWindowMask) != NSFullScreenWindowMask) {
     
     NSString *title = @"G-Ear";
     NSMutableParagraphStyle *style = [[NSMutableParagraphStyle defaultParagraphStyle] mutableCopy];
     [style setAlignment:NSCenterTextAlignment];
     
     [title drawInRect:CGRectMake(0, self.frame.size.height-20.0f, self.frame.size.width, 20.0f) withAttributes:
     @{NSFontAttributeName : [NSFont fontWithName:@"HelveticaNeue-Bold" size:13.0f],
     NSForegroundColorAttributeName: [manager textColor],
     NSParagraphStyleAttributeName: style }];
     }*/
    
}


@end
