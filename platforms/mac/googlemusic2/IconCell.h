//
//  IconCell.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 9/23/12.
//
//

#import <Cocoa/Cocoa.h>
#include <boost/shared_ptr.hpp>
#include "ISong.h"

@interface IconCell : NSCell

// song or playlist entry
@property (nonatomic, assign) std::shared_ptr<Gear::ISong> song;
@property(nonatomic,strong) NSTableView *tableView;
@property(nonatomic,assign) NSUInteger row;

@end
