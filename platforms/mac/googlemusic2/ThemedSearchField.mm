//
//  ThemedSearchField.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/1/13.
//
//

#import "ThemedSearchField.h"
#import "CocoaThemeManager.h"
#import "ThemedSearchFieldCell.h"

@implementation ThemedSearchField

- (BOOL)becomeFirstResponder
{
    // by default caret is not blinking after click on Yosemite (most likely a bug)
    // this is a workaround for it:
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        if(self.stringValue.length == 0)
        {
            [self setStringValue:@" "];
            [self setStringValue:@""];
        }
    });

    return [super becomeFirstResponder];
}

@end
