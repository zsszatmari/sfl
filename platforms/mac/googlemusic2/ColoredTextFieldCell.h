//
//  ColoredTextFieldCell.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/23/13.
//
//

#import <Cocoa/Cocoa.h>

@interface ColoredTextFieldCell : NSTextFieldCell

@property(nonatomic,assign) CGFloat verticalOffset;
@property(nonatomic,strong) NSColor *superBackgroundColor;

@end
