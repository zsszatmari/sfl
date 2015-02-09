//
//  SortDescriptorConverter.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/25/13.
//
//

#ifndef __Gear__SortDescriptorConverter__
#define __Gear__SortDescriptorConverter__

#import <Foundation/Foundation.h>

#include "SortDescriptor.h"

namespace Gear
{
    NSArray *arrayFromSortDescriptor(const SortDescriptor &rhs);
    const SortDescriptor sortDescriptorFromArray(NSArray *rhs);
}

#endif