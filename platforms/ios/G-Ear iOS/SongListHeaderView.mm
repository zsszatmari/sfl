//
//  SongListHeaderView.m
//  G-Ear iOS
//
//  Created by Gál László on 8/7/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "SongListHeaderView.h"

@implementation SongListHeaderView

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        self.searchBar = [[UISearchBar alloc] initWithFrame:frame];
        [self addSubview:self.searchBar];

    }
    return self;
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    // Drawing code
}
*/

@end
