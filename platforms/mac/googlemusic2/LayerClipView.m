//
//  LayerClipView.m
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 09/06/14.
//
//

#import <QuartzCore/QuartzCore.h>
#import "LayerClipView.h"
#import "AppDelegate.h"

@implementation LayerClipView

+ (NSView *)enable:(NSTableView *)tableView
{
    // might cause problems on snow leo
    if (lionOrBetter()) {
    
        NSScrollView *scrollView = tableView.enclosingScrollView;
        scrollView.wantsLayer = YES;
        id documentView = scrollView.documentView;
        LayerClipView *clipView = [[LayerClipView alloc] initWithFrame:scrollView.contentView.frame];
        scrollView.contentView = clipView;
        scrollView.documentView = documentView;
        return clipView;
    }
    return nil;
}

- (BOOL)isOpaque {
	return self.layer.opaque;
}

- (void)setOpaque:(BOOL)opaque {
	self.layer.opaque = opaque;
}

- (id)initWithFrame:(NSRect)frame {
	self = [super initWithFrame:frame];
	if (self == nil) return nil;
    
	self.layer = [CAScrollLayer layer];
	self.wantsLayer = YES;
    
	self.layerContentsRedrawPolicy = NSViewLayerContentsRedrawNever;
    
	// Matches default NSClipView settings.
	self.backgroundColor = NSColor.clearColor;
	self.opaque = NO;
    
	return self;
}

@end
