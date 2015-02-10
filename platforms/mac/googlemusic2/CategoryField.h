//
//  CategoryCell.h
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 29/03/14.
//
//

#import <Cocoa/Cocoa.h>

@protocol cellActionDelegate <NSObject>
- (NSString *) cellAction: (NSString *) cellTitle;
- (NSString *) showHideLabel:(NSString *)cellTitle;
- (NSString *) cellCurrRow;
- (NSString *) cellPrevRow;
@end

@interface CategoryField : NSTextField

@property (unsafe_unretained) id <cellActionDelegate> actionDelegate;
@property (nonatomic, assign) NSInteger row;
@property (nonatomic, assign) CGFloat topSpacing;

@end
