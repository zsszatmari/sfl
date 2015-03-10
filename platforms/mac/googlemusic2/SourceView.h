//
//  SourceView.h
//  Gear for Google Play
//
//  Created by Zsolt Szatmari on 20/11/14.
//
//

#import <Cocoa/Cocoa.h>
#include <memory>
#include "stdplus.h"

namespace Gear
{
    class ISong;
}

@interface SourceView : NSView

- (void)setSong:(std::shared_ptr<Gear::ISong>) song;

@end
