//
//  Grouping.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 10/15/12.
//
//

#import "Grouping.h"
#import "AlbumViewController.h"
#import "AppDelegate.h"
#include "SongGrouping.h"
#include "AlbumInfo.h"
#include "StringUtility.h"
#include "Painter.h"
#include "IApp.h"
#include "AlbumArtStash.h"
#include "PromisedImage.h"
#import "MainWindowController.h"

@interface Grouping () {
    int imageVersion;
    
    int artDownloadInProgress;
    BOOL alreadyTried;
    std::vector<Base::SignalConnection> _promiseConnections;
    std::vector<shared_ptr<Gear::PromisedImage>> _promises;
    
    Gear::SongGrouping songGrouping;
    float oldDimension;
    dispatch_block_t refreshHandler;
}
@end

@implementation Grouping

@synthesize title;
@synthesize subtitle;
@synthesize predicate;
@synthesize controller;
@synthesize multiImage;

using namespace Gear;
using namespace Gui;

- (int)imageVersion
{
    return imageVersion;
}

- (NSString *)imageRepresentationType
{
#ifndef DISABLE_IMAGEKIT
    return IKImageBrowserNSImageRepresentationType;
#endif
}

- (Gear::SongGrouping)songGrouping
{
    return songGrouping;
}

- (void)setSongGrouping:(Gear::SongGrouping)value
{
    songGrouping = value;
    _promises.clear();
}

- (void)subscribe
{
    oldDimension = [self cellDimension];
    _promises = songGrouping.images([self cellDimension]);
    _promiseConnections.clear();
    for (auto &promise : _promises) {
        auto conn = promise->connector().connect([self](const shared_ptr<IPaintable> &rhs){
            imageVersion++;
            
            // this would be slow:
            //[self.controller reload];
            // instead:
            refreshHandler();
        });
        _promiseConnections.push_back(conn);
    }
}

- (void)refresh
{
    ++imageVersion;
}

- (NSImage *)imageRepresentationWithRefresh:(dispatch_block_t)aRefreshHandler
{
    refreshHandler = aRefreshHandler;
    
    /*
    if (![self.controller isVisible:self]) {
        return nil;
    }*/
    
    bool subscribed = (_promises.size() > 0);
    CGFloat cellSize = [self cellDimension];
    if (cellSize > oldDimension) {
        
        if (subscribed) {
            // when we enlarge, we don't need a continuum flow of images
            double delayInSeconds = 1.0;
            dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
            dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
                [self subscribe];
            });
        } else {
            [self subscribe];
            subscribed = true;
        }
    }
    
    if (!subscribed) {
        [self subscribe];
        subscribed = true;
    }
    
    NSArray *images = self.imageArray;
    if ([images count] == 0) {
        return nil;
    }
    if ([images count] == 1) {
        return [images objectAtIndex:0];
    }
    static const int kMaxImages = 3;
    if ([images count] > kMaxImages) {
        images = [images subarrayWithRange:NSMakeRange(0, kMaxImages)];
    }
    
    NSImage *bigImage = [[NSImage alloc] initWithSize:NSMakeSize(cellSize, cellSize)];
    [bigImage lockFocus];
    NSGraphicsContext *context = [NSGraphicsContext currentContext];
    NSMutableArray *angles = [[NSMutableArray alloc] init];
    float minRotationScale = 1;
    
    // pseudorandom, so that it won't change when new data rrives 
    int random = 0;
    int sign = 0;
    
    for (NSImage *image in images) {
        if (image.size.width == 0) {
            // avoid divide by 0
            continue;
        }
        const int kMaxAngle = 20;
        for (int i = 0 ; i < [title length] ; i++) {
            random += [title characterAtIndex:i];
        }
        random %= (2*kMaxAngle);
        //random = (random + [title hash] + [subtitle hash]) % (2*kMaxAngle);
        
        float angle = random - kMaxAngle;
        if (sign == 0) {
            if (angle!=0) {
                sign = abs(angle)/angle;
            } else {
                sign = 1;
            }
        } else {
            sign *= (-1);
            angle = abs(angle) * sign;
        }
        
        float cosine = cos(angle * M_PI/180);
        float sine = sin(angle * M_PI/180);
        float rotationScale = 1.0f / (fabsf(cosine) + fabsf(sine));
        [angles addObject:@(angle)];
        if (rotationScale < minRotationScale) {
            minRotationScale = rotationScale;
        }
    }
    for (NSImage *image in images) {
        
        if (image.size.width == 0) {
            // avoid divide by 0
            continue;
        }
        
        [context saveGraphicsState];
        
        float angle = [[angles lastObject] floatValue];
        [angles removeLastObject];
        float scale = (cellSize / image.size.width);
        
        NSAffineTransform *transform = [[NSAffineTransform alloc] init];
        //reverse order because of matrix multiplication
        [transform translateXBy:(cellSize)/2 yBy:(cellSize)/2];
        [transform rotateByDegrees:angle];
        [transform translateXBy:-(cellSize*minRotationScale)/2 yBy:-(cellSize*minRotationScale)/2];
        [transform scaleBy:scale*minRotationScale];
        
        [transform concat];
        
        
        [image drawAtPoint:NSMakePoint(0, 0) fromRect:NSMakeRect(0, 0, image.size.width, image.size.height) operation:NSCompositeCopy fraction:1.0f];
        [context restoreGraphicsState];
    }
    [bigImage unlockFocus];
    
    return bigImage;
}

- (NSString *)imageUID
{
    return [NSString stringWithFormat:@"%@.%@", title, subtitle];
}

- (NSString *)imageTitle
{
    return title;
}


- (NSString *)imageSubtitle
{
    return subtitle;
}

- (int)maxImageCount
{
    return multiImage ? 3 : 1;
}

- (CGFloat)cellDimension
{
    CGSize cellSize = [controller cellSize];
    CGRect cellRect = CGRectMake(0,0,cellSize.width, cellSize.height);
    if ([[controller.imageBrowserView window] respondsToSelector:@selector(convertRectToBacking:)]) {
        cellRect = [[controller.imageBrowserView window] convertRectToBacking:cellRect];
    }
    return MAX(cellRect.size.width, cellRect.size.height);
}

- (NSUInteger)hashForArtist:(NSString *)artist album:(NSString *)album
{
    return [[[artist stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] lowercaseString] hash] + [[[album stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] lowercaseString] hash];
}

- (NSArray *)imageArray
{
    NSMutableArray *ret = [NSMutableArray array];
    
    for (auto &promise : _promises) {
        auto current = promise->image();
        if (current) {
            NSImage *image = Painter::convertImage(*current);
            if (image) {
                [ret addObject:image];
            }
        }
    }
    return ret;
}

- (BOOL)isEqual:(id)other {
    if (other == self) {
        return YES;
    }
    if (!other || ![other isKindOfClass:[self class]]) {
        return NO;
    }
    BOOL ret = [title isEqualToString:[other title]] && [subtitle isEqualToString:[other subtitle]];
    //NSLog(@"title: %@ subtitle: %@ othertitle: %@, othersubtitle: %@ %d",title, subtitle, [other title], [other subtitle], ret);
    return ret;
}

- (NSUInteger)hash {
    NSUInteger hash = [title hash] + [subtitle hash];
    return hash;
}

- (void)dealloc
{
}

+ (Grouping *)groupingWith:(Gear::SongGrouping &)s
{
    Grouping *grouping = [[Grouping alloc] init];
    
    grouping.title = convertString(s.title());
    grouping.subtitle = convertString(s.subtitle());
    
    SongPredicate predicate(s.firstField(), s.firstValue(), SongPredicate::Matches());
    
    
    grouping.multiImage = s.multiImage();
    grouping.predicate = predicate;
    grouping.controller = [[[AppDelegate sharedDelegate] mainWindowController]albumViewController];
    grouping.songGrouping = s;
    
    return grouping;
}

@end
