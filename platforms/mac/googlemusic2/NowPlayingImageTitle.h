//
//  NowPlayingImageTitle.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/3/13.
//
//

#import <Cocoa/Cocoa.h>

@protocol ShowHideDelegate <NSObject>
- (NSString *) showHide: (NSString *) title;
@end

@interface NowPlayingImageTitle : NSView

@property (unsafe_unretained) id <ShowHideDelegate> delegate;
@property(nonatomic, assign) CGFloat yOffsetRelativeToCenter;

@end
