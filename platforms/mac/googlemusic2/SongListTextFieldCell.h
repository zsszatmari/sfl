//
//  SongListTextFieldCell.h
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 09/06/14.
//
//

#import "ColoredTextFieldCell.h"
#import "SongsTableView.h"

@interface SongListTextFieldCell : ColoredTextFieldCell

@property(nonatomic,strong) SongsTableView *tableView;
@property(nonatomic,assign) NSUInteger row;
@property(nonatomic,assign) NSUInteger column;
@property(nonatomic,assign) BOOL mayHighlightOnHover;

@end
