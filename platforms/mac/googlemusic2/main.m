//
//  main.m
//  googlemusic2
//
//  Created by Zsolt Szatmári on 2/29/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "Config.h"
#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"
#import "receigen.h"
#import "Carbon/Carbon.h"
#import "Debug.h"


//#define SEND_PREVIEW_BY_EMAIL

#ifdef ENABLE_PADDLE
#define DISABLE_RECEIPT
#endif

int main(int argc, char *argv[])
{
    @autoreleasepool {
        // mimics LSUIPresentationMode
        unsigned int presentationMode = (unsigned int)[[NSUserDefaults standardUserDefaults] integerForKey:@"PresentationMode"];
        if (presentationMode > 0) {
            SetSystemUIMode(presentationMode, 0);
        }
        [AppDelegate setupDefaults];
        
#ifdef DISABLE_PADDLE
        NSLog(@"WARNING: disabled");
#endif
    }
  
#if DEBUG
    return NSApplicationMain(argc, (const char **)argv);
#else
#ifdef DISABLE_RECEIPT
    return NSApplicationMain(argc, (const char **)argv);
#else
    return CheckReceiptAndRun(argc, (const char **)argv);
#endif
#endif
}
