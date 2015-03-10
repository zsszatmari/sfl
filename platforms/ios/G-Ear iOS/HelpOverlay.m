//
//  HelpOverlay.m
//  NewsBar mobile
//
//  Created by Zsolt Szatmári on 4/16/13.
//  Copyright (c) 2013 Merlin Developments. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "HelpOverlay.h"

#if TARGET_IPHONE_SIMULATOR
#define DEBUG_ALWAYS_SHOW
#endif
#ifdef DEBUG
//#define DEBUG_ALWAYS_SHOW
#endif

NSString * const kDefaultsPrefixOverlay = @"AlreadyShownOverlay";

@implementation HelpOverlay

+ (void)showIfNecessary:(NSString *)identifier fromController:(UIViewController *)controller
{
    if (UIInterfaceOrientationIsLandscape([UIApplication sharedApplication].statusBarOrientation)) {
        return;
    }
    
    CGFloat offset = 0;
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
        
        NSString *version = [[UIDevice currentDevice] systemVersion];
        if ([version compare:@"7.0" options:NSNumericSearch] != NSOrderedAscending) {
            // icon color (ios 7)
        } else {
            offset = -20;  // status bar size returned wrong
        }

        
        if (UIInterfaceOrientationIsLandscape([UIApplication sharedApplication].statusBarOrientation)) {
            
            //static const CGFloat kStatusBarHeight = 20.0f;
            //BOOL ios7 = NO;
            
            // TODO
            //UIView *view = [PadMasterViewController sharedController].view;
            
            //[self showIfNecessary:identifier inView:view offset:offset];
        } else {
            
            [self showIfNecessary:identifier inView:[[[[UIApplication sharedApplication] keyWindow] subviews] lastObject] offset:offset];
        }
        //[self showIfNecessary:identifier inView:[[UIApplication sharedApplication] keyWindow] offset:offset];
    } else {
        //offset = [UIApplication sharedApplication].statusBarFrame.size.height;
        //UIView *view = controller.navigationController.view;
        
        offset = 0;
        UIView *view = [UIApplication sharedApplication].keyWindow;
        
        [self showIfNecessary:identifier inView:view offset:offset];
    }
}

// pieces is a map to piece names to uiviews. they will be shown concentric
+ (void)showIfNecessary:(NSString *)identifier fromController:(UIViewController *)controller pieces:(NSDictionary *)pieces
{
    if (UIInterfaceOrientationIsLandscape([UIApplication sharedApplication].statusBarOrientation)) {
        return;
    }
    
    UIView *view = [UIApplication sharedApplication].keyWindow;
    [self showIfNecessary:identifier inView:view pieces:pieces];
}

static HelpOverlay *overlay;

+ (void)markIdentifier:(NSString *)identifier
{
    if (UIInterfaceOrientationIsLandscape([UIApplication sharedApplication].statusBarOrientation)) {
        return;
    }
    
    NSString *defaultsKey = [kDefaultsPrefixOverlay stringByAppendingString:identifier];
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults setBool:YES forKey:defaultsKey];
}

+ (void)showForIdentifierIfNecessary:(NSString *)identifier block:(dispatch_block_t)block
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *defaultsKey = [kDefaultsPrefixOverlay stringByAppendingString:identifier];
    
#ifndef DEBUG_ALWAYS_SHOW
    if ([defaults boolForKey:defaultsKey]) {
        return;
    }
#endif
    [defaults setBool:YES forKey:defaultsKey];
    
    block();
}

+ (void)showIfNecessary:(NSString *)identifier inView:(UIView *)root offset:(CGFloat)offset
{
    [self showForIdentifierIfNecessary:identifier block:^{
        
        [self show:identifier inView:root offset:offset];
    }];
}

+ (void)showIfNecessary:(NSString *)identifier inView:(UIView *)view pieces:(NSDictionary *)pieces
{
    [self showForIdentifierIfNecessary:identifier block:^{
        
        [self show:identifier inView:view pieces:pieces];
    }];
}

+ (void)show:(NSString *)identifier inView:(UIView *)view pieces:(NSDictionary *)pieces
{
    overlay = [[HelpOverlay alloc] init];
    [overlay setBackgroundColor:[UIColor colorWithRed:9.0f/255.0f green:90.0f/255.0f blue:159.0f/255.0f alpha:0.70f]];
    CGRect rootFrame = [view convertRect:[view frame] fromView:[view superview]];
    [overlay setFrame:CGRectMake(0, 0, rootFrame.size.width, rootFrame.size.height)];
    
    [pieces enumerateKeysAndObjectsUsingBlock:^(NSString *key, UIView *toCenter, BOOL *stop) {
        
        NSString *imageName = [NSString stringWithFormat:@"overlay-%@-%@", identifier, key];
        //imageName = @"shuffle";
        UIImage *image = [UIImage imageNamed:imageName];
        
        CGRect centerRect = [view convertRect:[toCenter frame] fromView:[toCenter superview]];
        CGPoint center = CGPointMake(CGRectGetMidX(centerRect), CGRectGetMidY(centerRect));
        
        CGSize size = CGSizeMake(image.size.width/2, image.size.height/2);
        CGRect rect = CGRectMake(center.x - size.width/2, center.y - size.height/2, size.width, size.height);
        // rect = CGRectMake(50,50, 140,140);
        
        UIImageView *subview = [[UIImageView alloc] initWithFrame:rect];
        subview.image = image;
        // subview.backgroundColor = [UIColor redColor];
        [subview setContentMode:UIViewContentModeScaleAspectFit];
        [overlay addSubview:subview];
    }];
    
    [view addSubview:overlay];
    [self showAndLetHide:overlay];
}

+ (void)showForced:(NSString *)identifier
{
    [self show:identifier inView:[UIApplication sharedApplication].keyWindow offset:0];
}

+ (void)show:(NSString *)identifier inView:(UIView *)view offset:(CGFloat)offset
{
    overlay = [[HelpOverlay alloc] init];
    [overlay setBackgroundColor:[UIColor colorWithRed:9.0f/255.0f green:90.0f/255.0f blue:159.0f/255.0f alpha:0.70f]];
    UIImage *image = [UIImage imageNamed:[@"overlay-" stringByAppendingString:identifier]];
    overlay.image = image;
    
    // conversion needed for ipad landscape
    CGRect rootFrame = [view convertRect:[view frame] fromView:[view superview]];
    
    [overlay setFrame:CGRectMake(0, 0, rootFrame.size.width, rootFrame.size.height)];
    //NSLog(@"frame: %@", NSStringFromRect([overlay frame]));
    [overlay setContentMode:UIViewContentModeScaleAspectFit];
    [view addSubview:overlay];
    
    [self showAndLetHide:overlay];
    //overlay.layer.borderWidth = 3;
    //overlay.layer.borderColor = [UIColor purpleColor].CGColor;
}

+ (void)showAndLetHide:(UIView *)overlay
{
    UITapGestureRecognizer *recognizer = [[UITapGestureRecognizer alloc] initWithTarget:[self class] action:@selector(tapped:)];
    [overlay addGestureRecognizer:recognizer];
    overlay.userInteractionEnabled = YES;
    
    overlay.alpha = 0.0f;
    [UIView animateWithDuration:0.2f animations:^{
        overlay.alpha = 1.0f;
    }];
    
    [[NSNotificationCenter defaultCenter] addObserver:[self class] selector:@selector(tapped:) name:UIApplicationDidChangeStatusBarOrientationNotification object:nil];
}

+ (void)tapped:(id)sender
{
    [overlay tapped:sender];
}

- (void)tapped:(id)sender
{
    [UIView animateWithDuration:0.2f animations:^{
        self.alpha = 0.0f;
    } completion:^(BOOL finished) {
        [self removeFromSuperview];
    }];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end
