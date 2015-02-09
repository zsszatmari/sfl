//
//  VerticallyCenteredTextField.h
//  Gear for Google Play
//
//  Created by Zsolt Szatmari on 18/11/14.
//
//

#import <Cocoa/Cocoa.h>

@interface VerticallyCenteredTextField : NSTextField

@property (nonatomic,assign) BOOL disableAntialias;
@property (nonatomic,assign) BOOL enableAntialias;

- (VerticallyCenteredTextField *)alignTop;
- (VerticallyCenteredTextField *)alignBottom;

@end
