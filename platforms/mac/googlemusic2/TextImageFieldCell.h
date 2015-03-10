//
//  TextImageFieldCell.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 2/6/13.
//
//

#import <Cocoa/Cocoa.h>

@interface TextImageFieldCell : NSTextFieldCell

@property(nonatomic,strong) NSTableView *tableView;
@property(nonatomic,assign) NSUInteger row;

@end
