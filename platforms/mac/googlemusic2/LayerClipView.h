//
//  LayerClipView.h
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 09/06/14.
//
//

#import <Foundation/Foundation.h>

@interface LayerClipView : NSClipView

// The backing layer for this view.
//@property (nonatomic, strong) CAScrollLayer *layer;

// Whether the content in this view is opaque.
//
// Defaults to NO.
@property (nonatomic, getter = isOpaque) BOOL opaque;

+ (NSView *)enable:(NSTableView *)tableView;

@end
