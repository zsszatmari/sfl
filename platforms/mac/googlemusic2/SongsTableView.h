//
//  SongsTableView.h
//  G-Ear
//
//  Created by Zsolt Szatmari on 4/30/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TableViewBase.h"

@interface SongsTableView : TableViewBase

@property(nonatomic, readonly) int hoverRow;
@property(nonatomic, readonly) int hoverColumn;
@property(nonatomic, strong) NSString *sortKey;

+ (NSArray *)sortDescriptorsForKey:(NSString *)protoKey ascending:(BOOL)ascending;
+ (NSString *)defaultKey;
- (void)resetSortDescriptors;

@end
