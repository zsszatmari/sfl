//
//  ShareManager.m
//  G-Ear
//
//  Created by Zsolt Szatmari on 7/15/12.
//
//

#import "ShareManager.h"
#include "StringUtility.h"
#include "Painter.h"
#include "IApp.h"
#include "AlbumArtStash.h"
#include "PromisedImage.h"

@implementation ShareManager

@synthesize song;

using namespace Gear;

- (void)shareFromView:(NSView *)view
{
    if (song == nil) {
        return;
    }
    
    NSMutableArray *array = [NSMutableArray array];
    
    // TODO: setting to set format?
    NSString *string = [NSString stringWithFormat:@"I am listening to %@ - %@ via Gear Player for Google Play and Youtube", convertString(song->artist()), convertString(song->title())];
    [array addObject:string];
    NSImage *image = nil;
    CGSize viewSize = [view frame].size;
    if ([view respondsToSelector:@selector(convertSizeToBacking:)]) {
        viewSize = [view convertSizeToBacking:viewSize];
    }
    
    auto got = IApp::instance()->albumArtStash()->art(song, viewSize.width)->image();
    if (got) {
        image = Gui::Painter::convertImage(*got);
    }
    if (image != nil) {
        [array addObject:image];
    }
    // this would cripple the picture (multiple items)
    //[array addObject:[NSURL URLWithString:@"http://www.treasurebox.hu"]];
    NSSharingServicePicker *picker = [[NSSharingServicePicker alloc] initWithItems:array];
    
    [picker setDelegate:self];
    [picker showRelativeToRect:[view frame] ofView:[view superview] preferredEdge:NSMinYEdge];
}
    
- (NSArray *)sharingServicePicker:(NSSharingServicePicker *)sharingServicePicker sharingServicesForItems:(NSArray *)items proposedSharingServices:(NSArray *)proposedServices
{
    NSMutableArray *services = [proposedServices mutableCopy];
    NSArray *notWanted = [NSArray arrayWithObjects:NSSharingServiceNameAddToSafariReadingList, NSSharingServiceNameComposeEmail, NSSharingServiceNamePostImageOnFlickr, NSSharingServiceNameAddToAperture, NSSharingServiceNameAddToIPhoto, NSSharingServiceNamePostImageOnFlickr, NSSharingServiceNameSendViaAirDrop, nil];
    [services filterUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary *bindings) {
        
        return !([notWanted containsObject:[evaluatedObject name]]);
    }]];
    return services;
}

@end
