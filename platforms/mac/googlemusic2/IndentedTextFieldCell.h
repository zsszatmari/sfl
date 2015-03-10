//
//  IndentedTextFieldCell.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 10/1/12.
//
//

#import <Foundation/Foundation.h>
#import "ColoredTextFieldCell.h"

@interface IndentedTextFieldCell : ColoredTextFieldCell {
    BOOL editCapable;
}

@property(nonatomic,assign) BOOL selected;
@property(assign, nonatomic) BOOL editCapable;
@property(nonatomic, strong) NSTableView *tableView;
@property(nonatomic, assign) NSUInteger row;

+ (void)enableEditingWhenCapable;
+ (void)disableEditingWhenCapable;

@end
