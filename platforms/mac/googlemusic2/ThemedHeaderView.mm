//
//  ThemedHeaderView.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 3/29/13.
//
//

#import "ThemedHeaderView.h"
#import "CocoaThemeManager.h"
#import "ThemedHeaderCell.h"

@implementation ThemedHeaderView

- (void)awakeFromNib
{
    // this is not used at all, but needed because of xib
    // so we will remove this at the first suitable moment
    for (NSTableColumn *column in [self.tableView tableColumns]) {
        ThemedHeaderCell *cell = [[ThemedHeaderCell alloc]
                                  initTextCell:[[column headerCell] stringValue]];
        [cell setIdentifier:column.identifier];
        [column setHeaderCell:cell];
    }
}

@end
