//
//  SortDescriptorConverter.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/25/13.
//
//

#import "SortDescriptorConverter.h"
#include "StringUtility.h"

namespace Gear
{
    NSArray *arrayFromSortDescriptor(const SortDescriptor &rhs)
    {
        NSMutableArray *ret = [[NSMutableArray alloc] init];
        
        for (auto &pair : rhs.keysAndAscending()) {
            [ret addObject:[NSSortDescriptor sortDescriptorWithKey:convertString(pair.first) ascending:pair.second]];
        }
        return ret;
    }
    
    const SortDescriptor sortDescriptorFromArray(NSArray *rhs)
    {
        vector<pair<string, bool>> keys;
        
        keys.reserve([rhs count]);
        
        for (NSSortDescriptor *desc in rhs) {
            keys.push_back(make_pair(convertString([desc key]), [desc ascending]));
        }
        SortDescriptor ret(keys);
        return ret;
    }
}