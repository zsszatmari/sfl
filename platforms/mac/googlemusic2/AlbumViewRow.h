//
//  AlbumViewRow.h
//  Gear for Google Play and YouTube
//
//  Created by Zsolt Szatmari on 12/01/15.
//
//

#import <Cocoa/Cocoa.h>

@interface AlbumViewRow : NSTableRowView

@property (nonatomic,assign) CGSize intercellSpacing;

- (id)initWithClickHandler:(void (^)(NSUInteger,int))clickHandler;
- (void)setGroupings:(NSArray *)groupings itemsPerRow:(NSInteger)items startIndex:(NSUInteger)startIndex selectedIndex:(NSUInteger)selectedIndex;

@end
