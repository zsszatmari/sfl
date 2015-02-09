//
//  ResizingImageBrowserView.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 10/21/12.
//
//

#import "Debug.h"

#ifndef DISABLE_IMAGEKIT
#import <Quartz/Quartz.h>
//#import <ImageKit/ImageKit.h>

@interface ResizingImageBrowserView : IKImageBrowserView

@end

#else

@interface ResizingImageBrowserView : NSView
@end

#endif
