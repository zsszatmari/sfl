//
//  PlaylistManager.m
//  G-Ear
//
//  Created by Zsolt Szatmari on 7/12/12.
//
//

#import "PlaylistManager.h"
#import "AppDelegate.h"
#import "MainWindowController.h"
#import "IndentedTextFieldCell.h"
#import "Debug.h"
#import "MainWindowController.h"
#import "CocoaThemeManager.h"
#include "IApp.h"
#include "StringUtility.h"
#import "ColoredTextFieldCell.h"
#include "PlaylistCategory.h"
#include "DragManager.h"
#include "ISongNoIndexIntent.h"
#import "PlaylistWrapper.h"
#include "ISession.h"
#include "IPlaylistIntent.h"
#import "CategoryField.h"
#include "SessionManager.h"
#include "IPlaylist.h"
#import <objc/runtime.h>
#import "TableViewBase.h"
#import "NSView+Recursive.h"

using namespace Gear;
using std::shared_ptr;

@interface PlaylistManager () {
    NSTableView *cachedTableView;
    NSMutableArray *itemsToRetain;
    
    Song *displaySongRequest;
    NSMutableDictionary *playlistsByCategories;
    NSMutableArray *categories;
	NSMutableDictionary *categoriesDict;
    // an array of treenodes
    NSMutableArray *tree;
    
    NSTimeInterval lastSelectRow;
    
    SignalConnection categoriesConnection;
    SignalConnection selectedPlaylistConnection;
    
    vector<shared_ptr<IPlaylistIntent>> playlistIntents;
	NSMutableDictionary *hiddenCategories;
//    NSIndexSet *lastMultiSelect;
	NSMutableIndexSet *lastMultiSelect;
}
@end

@interface ClickableTextField : NSTextField

@property (nonatomic, assign) std::shared_ptr<IPlaylist> playlist;

@end

@interface CustomTableRow : NSTableRowView

@property (nonatomic, strong) NSColor *customBackground;
@property (nonatomic, strong) NSGradient *customBackgroundGradient;
@property (nonatomic, strong) NSColor *topColor;
@property (nonatomic, strong) NSColor *bottomColor;
@property (nonatomic, strong) NSColor *selectionBorder;
@property (nonatomic, strong) NSGradient *selectedGradient;
@property (nonatomic, assign) BOOL omitTop;


@end


@implementation PlaylistManager

// datasource

- (id)init
{
    self = [super init];
    if (self) {
        [self setupCategories];
    }
    return self;
}

+ (BOOL)isSelectable:(id)item
{
    return [item conformsToProtocol:@protocol(PlaylistProtocol)];
}

- (void)applyTheme
{
    if (yosemiteOrBetter() && [CocoaThemeManager usingCocoaTheme] && vibrancyEnabled()) {
		[cachedTableView setBackgroundColor: [NSColor colorWithCatalogName: @"System" colorName:@"_sourceListBackgroundColor"]];
        [cachedTableView setSelectionHighlightStyle: NSTableViewSelectionHighlightStyleSourceList];
	} else {
        dispatch_async(dispatch_get_main_queue(),^{
            [cachedTableView setBackgroundColor:[[CocoaThemeManager sharedManager] playlistsBackgroundColor]];
        });
		[cachedTableView setSelectionHighlightStyle: NSTableViewSelectionHighlightStyleRegular];
	}
    if (lionOrBetter()) {
        [self.scrollView setScrollerKnobStyle:[[CocoaThemeManager sharedManager] knobStyle]];
    }
    [cachedTableView setNeedsDisplay:YES];
    [cachedTableView reloadData];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	//NSLog(@"ROWW1");
    if (cachedTableView == nil) {
        cachedTableView = aTableView;
        
        [cachedTableView setAllowsMultipleSelection:YES];
        [cachedTableView setColumnAutoresizingStyle: NSTableViewUniformColumnAutoresizingStyle];

        // this seems to do harm, at least on snow leopard, and bring no apparent advantages
        /*
        NSScrollView *scrollView = cachedTableView.enclosingScrollView;
        scrollView.wantsLayer = YES;
        id documentView = scrollView.documentView;
        LayerClipView *clipView = [[LayerClipView alloc] initWithFrame:scrollView.contentView.frame];
        scrollView.contentView = clipView;
        scrollView.documentView = documentView;
        */
        
        [cachedTableView registerForDraggedTypes:@[kPasteBoardTypeSong]];
        [cachedTableView setTarget:self];
        [cachedTableView setDoubleAction:@selector(doubleClick:)];
        [cachedTableView setAction:(@selector(singleClick:))];
//	[cachedTableView setSelectionHighlightStyle: NSTableViewSelectionHighlightStyleRegular];

        //[[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(tableViewScrolled:)
            //name: NSViewBoundsDidChangeNotification object: [[cachedTableView enclosingScrollView] contentView]];


        [self applyTheme];
    }
/* original
    int number = 0;
    for (NSTreeNode *node in tree) {
        number += (1 + [[node childNodes] count]);
    }
*/
//	for (NSArray *arr in tree) {
//		number += [arr count];
//	}
	return [self flattenedArrayCount: tree];
}

- (void) tableViewScrolled: (NSNotification *) notification {
	NSLog(@"Tableview scrolled");
}

- (void)singleClick:(id)sender
{
    [cachedTableView setNeedsDisplay:YES];
    
    [IndentedTextFieldCell disableEditingWhenCapable];
    NSInteger row = [cachedTableView clickedRow];
    NSInteger column = [cachedTableView clickedColumn];
    {
        CategoryField *field = [cachedTableView viewAtColumn:column row:row makeIfNecessary:YES];
        if ([field isKindOfClass:[CategoryField class]]) {
            [field mouseUp:nil];
            return;
        }
    }
    
    id object = [self objectForRow:row];
    NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
    float diff = now - lastSelectRow;
    if (0.1f < diff) {
        
        bool editable = false;
        if ([object isKindOfClass:[PlaylistWrapper class]]) {
            editable = [(PlaylistWrapper *)object playlist]->nameEditable();
        }
        if (editable && 0.55f < diff) {
            if (row == [cachedTableView selectedRow]) {
                [self startEditingObject:object];
            }
        } else {
            [self currentRowClicked];
        }
    }
}

- (void)doubleClick:(id)sender
{
    NSInteger row = [cachedTableView clickedRow];
    id item = [self objectForRow:row];
    if ([item isKindOfClass:[PlaylistWrapper class]]) {
        IApp::instance()->userSelectedPlaylist([(PlaylistWrapper *)item playlist], true);
    }
}

/* original
- (id)objectForRow:(NSInteger)rowIndex
{
    int number = 0;
    for (NSTreeNode *node in tree) {
        if (number == rowIndex) {
            return [node representedObject];
        }
        number++;
        for (id item in [node childNodes]) {
            if (number == rowIndex) {
                return [item representedObject];
            }
            number++;
        }
    }
    return nil;
}

- (NSInteger)rowForObject:(id)object
{
    int number = 0;
    for (NSTreeNode *node in tree) {
        number++;
        for (id item in [node childNodes]) {

            if ([[item representedObject] isEqual:object]) {
                return number;
            }
            number++;
        }
    }
    return NSNotFound;
}
*/

- (id) objectForRow: (NSInteger) rowIndex {
	NSUInteger len = 0;
	for (NSArray *arr in tree) {
		if (rowIndex < len + [arr count]) {
			return arr[rowIndex - len];
		}
		len += [arr count];
	};
	return nil;
}

- (NSInteger) rowForObject: (id) object {
	NSInteger i = 0;
	for (NSArray *arr in tree) {
		for (id item in arr) {
			if ([item isEqual: object]) {
				return i;
			}
			i++;
		}
	}
	return NSNotFound;
}
/* original
- (NSIndexSet *)globalIndexForNode:(NSTreeNode *)aNode
{
    int number = 0;
    for (NSTreeNode *node in tree) {
        if (node == aNode) {
            return [NSIndexSet indexSetWithIndex:number];
        }
        number++;
        for (id item in [node childNodes]) {
            if (item == aNode) {
                return [NSIndexSet indexSetWithIndex:number];
            }
            number++;
        }
    }
    return nil;
}

- (NSIndexSet *)globalIndexForObject:(id)aObject
{
    int number = 0;
    for (NSTreeNode *node in tree) {
        if ([[node representedObject] isEqual:aObject]) {
            return [NSIndexSet indexSetWithIndex:number];
        }
        number++;
        for (NSTreeNode *item in [node childNodes]) {
            if ([[item representedObject] isEqual:aObject]) {
                return [NSIndexSet indexSetWithIndex:number];
            }
            number++;
        }
    }
    return nil;
}
*/
// delegate
/*
- (BOOL)tableView:(NSTableView *)aTableView shouldEditTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{    
    if ([self playlistForRow:rowIndex] != nil) {
        return YES;
    }
    
    [self currentRowClicked];
    return NO;
}*/

- (std::shared_ptr<IPlaylist>)playlistForControl:(NSControl *)control
{
    if ([control isKindOfClass:[ClickableTextField class]]) {
        
        ClickableTextField *textField = (ClickableTextField *)control;
        std::shared_ptr<IPlaylist> playlist = textField.playlist;
        return playlist;
    }
    return nil;
}

- (BOOL)control:(NSControl *)control textShouldBeginEditing:(NSText *)fieldEditor
{
    if (control == cachedTableView) {
        // snow leo situation
        return YES;
    }
    
    if ([self playlistForControl:control] != nil) {
        return YES;
    }
    
    [self currentRowClicked];
    return NO;
}

- (BOOL)control:(NSControl *)control textShouldEndEditing:(NSText *)fieldEditor
{
    auto playlist = [self playlistForControl:control];
    if (![control isKindOfClass:[NSTextField class]]) {
        return YES;
    }
    NSTextField *textField = (NSTextField *)control;
    playlist->setName(convertString(textField.stringValue));
    //[[playlist managedObjectContext] save:nil];
    return YES;
}

- (void)currentRowClicked
{
    auto playlist = IApp::instance()->selectedPlaylist();
    if (playlist) {
        playlist->selectGrouping(SongGrouping(), false);
        
        [[[AppDelegate sharedDelegate] mainWindowController] adjustCollection];
    }
}

- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(NSInteger)rowIndex
{
    lastSelectRow = [NSDate timeIntervalSinceReferenceDate];
    
    id item = [self objectForRow:rowIndex];
    if ([item isKindOfClass:[PlaylistWrapper class]]) {
        IApp::instance()->userSelectedPlaylist([(PlaylistWrapper *)item playlist]);
    }
    return NO;
 //   return [self selectRow:rowIndex];
}

- (NSIndexSet *)tableView:(NSTableView *)tableView selectionIndexesForProposedSelection:(NSIndexSet *)proposedSelectionIndexes
{
    auto count = [proposedSelectionIndexes count];
    if (count == 0) {
        return lastMultiSelect;
    }
    if (count == 1) {
        // always returns NO
        [self tableView:tableView shouldSelectRow:[proposedSelectionIndexes firstIndex]];
        return lastMultiSelect;
    }
    
    // prevent name editing
    lastSelectRow = [NSDate timeIntervalSinceReferenceDate];
    
    // ensure that the same actions apply to all the selected playlists, then merge those into one
    __block vector<std::shared_ptr<IPlaylist>> playlists;
    __block BOOL allow = YES;
    [proposedSelectionIndexes enumerateIndexesUsingBlock:^(NSUInteger rowIndex, BOOL *stop) {
        id item = [self objectForRow:rowIndex];
        if ([item isKindOfClass:[PlaylistWrapper class]]) {
            playlists.push_back([(PlaylistWrapper *)item playlist]);
        } else {
            *stop = YES;
            allow = NO;
        }
    }];
    if (!allow) {
        return lastMultiSelect;
    }
    
    auto intents = IPlaylist::playlistIntents(playlists);
    if (intents.empty()) {
        // different actions
        return lastMultiSelect;
    } else {
        playlistIntents = intents;
        lastMultiSelect = [proposedSelectionIndexes mutableCopy];
    }
    
    return lastMultiSelect;
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    id item = [self objectForRow:rowIndex];
	if ([item isKindOfClass: [NSString class]] && [item hasPrefix: @"gear_spacer_"]) {
		return nil;
	}
    if ([item respondsToSelector:@selector(name)]) {
        return [item name];
    }
    //return [[item description] uppercaseString];
    return [item description];
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    id object = [self objectForRow:row];
    if ([object isKindOfClass:[NSString class]] && ![object hasPrefix: @"gear_spacer_"]) {
        
        CategoryField *field = [[CategoryField alloc] init];
        field.layerContentsRedrawPolicy = NSViewLayerContentsRedrawDuringViewResize;
        field.topSpacing = (row == 0) ? [self topSpacing] : 0;
        field.actionDelegate = self;
        return field;
    }
    
    ClickableTextField *field = [[ClickableTextField alloc] init];
    field.layerContentsRedrawPolicy = NSViewLayerContentsRedrawDuringViewResize;

    field.cell = [self tableView:tableView dataCellForTableColumn:tableColumn row:row];
     if ([object isKindOfClass:[PlaylistWrapper class]]) {
        
        field.playlist = [(PlaylistWrapper *)object playlist];
    }
    
    // for detecting click on an already selected row
    [field setDelegate:self];

    return field;
}

- (NSTableRowView *)tableView:(NSTableView *)tableView rowViewForRow:(NSInteger)rowNum
{
    id item = [self objectForRow:rowNum];
    if ([item isKindOfClass:[NSString class]]) {
        if ([item hasPrefix: @"gear_spacer_"]) {
            return nil;
        }

        if ([item length] > 0) {
            CustomTableRow *row = [[CustomTableRow alloc] init];
            
            row.customBackground = [[CocoaThemeManager sharedManager] playlistsRowColor];
            row.topColor = [[CocoaThemeManager sharedManager] playlistsRowColorTop];
            row.omitTop = NO;
            if ([[CocoaThemeManager sharedManager] themePrefix] != nil) {
                if (rowNum == 0) {
                    row.omitTop = YES;
                }
                // some special treatment (design touch) for modern dark theme...
                row.topColor = [NSColor colorWithDeviceWhite:8.0f/255.0f alpha:1.0f];
            }
            row.bottomColor = [[CocoaThemeManager sharedManager] playlistsRowColorBottom];
            row.selectedGradient = [[CocoaThemeManager sharedManager] playlistsRowSelectedGradient];
            row.selectionBorder = [[CocoaThemeManager sharedManager] playlistsRowSelectionBorder];
            row.customBackgroundGradient = [[CocoaThemeManager sharedManager] playlistCategoryGradient];
            //row.selectionHighlightStyle = NSTableViewSelectionHighlightStyleRegular;
            //row.backgroundColor = [NSColor yellowColor];
            return row;
        }
        
        return nil;
    } else {
        CustomTableRow *row = [[CustomTableRow alloc] init];
        
        row.customBackground = [[CocoaThemeManager sharedManager] playlistsRowColor];
        row.topColor = [[CocoaThemeManager sharedManager] playlistsRowColorTop];
        row.bottomColor = [[CocoaThemeManager sharedManager] playlistsRowColorBottom];
        row.selectedGradient = [[CocoaThemeManager sharedManager] playlistsRowSelectedGradient];
        row.selectionBorder = [[CocoaThemeManager sharedManager] playlistsRowSelectionBorder];
        //row.selectionHighlightStyle = NSTableViewSelectionHighlightStyleRegular;
        //row.backgroundColor = [NSColor yellowColor];
        return row;
    }

}

//- (BOOL) tableView: (NSTableView *) tableView shouldEditTableColumn: (NSTableColumn *) column row: (NSInteger) row {
//	return NO;
//}

- (NSCell *)tableView:(NSTableView *)tableView dataCellForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
//	NSLog(@"tablecol");
    if (tableColumn == nil) {
        return nil;
    }
    id item = [self objectForRow:row];
    if ([item isKindOfClass:[NSString class]]) {
        return nil;
    } else {
        IndentedTextFieldCell *playlistCell = [[IndentedTextFieldCell alloc] init];
        playlistCell.row = row;
        playlistCell.tableView = tableView;

        [playlistCell setFont:[[CocoaThemeManager sharedManager] playlistsTextFont]];
        [playlistCell setTextColor:[[CocoaThemeManager sharedManager] playlistsTextColor]];        
        
        //[playlistCell setTextColor:[NSColor redColor]];
        
        // only editable for real playlists
        if ([item isKindOfClass:[PlaylistWrapper class]]) {
            auto pl = [(PlaylistWrapper *)item playlist];
            if (pl->nameEditable()) {
                [playlistCell setEditCapable:YES];
            }
        }
        
        return playlistCell;
    }
}

- (CGFloat)topSpacing
{
    return [[CocoaThemeManager sharedManager] themePrefix] == nil ? 7 : -1;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    id item = [self objectForRow:row];
    
    CGFloat defaultHeight = [tableView rowHeight];
    //if (item == nil) {
    //    return defaultHeight - 14;
    //} else
    if ([item isKindOfClass:[NSString class]]) {
        if ([item length] == 0) {
            return (int)((defaultHeight - 8) * 1.1f);
        } else {
            return (int)((defaultHeight - 3) * 1.1f) + (row == 0 ? [self topSpacing] : 0);
        }
    } else {
        return (int)(defaultHeight * 1.1f);
    }
}

- (NSString *) cellCurrRow {
	id obj = [self objectForRow: [(TableViewBase *) cachedTableView currRow]];
	return ([obj isKindOfClass: [NSString class]]) ? obj : nil;
}

- (NSString *) cellPrevRow {
	id obj = [self objectForRow: [(TableViewBase *) cachedTableView prevRow]];
	return ([obj isKindOfClass: [NSString class]]) ? obj : nil;
}

- (NSString *)showHideLabel:(NSString *)cellTitle
{
    if (hiddenCategories[cellTitle] != nil) {
		return @"Show";
	} else {
		return @"Hide";
	}
}

- (NSString *)cellAction: (NSString *) cellTitle
{
	//NSLog(@"CELLTITLE: %@", cellTitle);
	if (hiddenCategories == nil) {
		hiddenCategories = [[NSMutableDictionary alloc] init];
	}

	if (hiddenCategories[cellTitle] != nil) {
		[hiddenCategories removeObjectForKey: cellTitle];
	} else {
		hiddenCategories[cellTitle] = @(1);
	}

	[self reloadTable];

	//NSLog(@"hidden: %@", hiddenCategories);

	return [self showHideLabel:cellTitle];
}

//- (BOOL) tableView: (NSTableView *) tableView shouldTrackCell: (NSCell *) cell forTableColumn: (NSTableColumn *) tableColumn row: (NSInteger) row {
//	return [cell isKindOfClass: [CategoryCell class]];
//}
    
+ (void)purgeInstantMixesWithContext:(NSManagedObjectContext *)context
{
    // maybe we should delete candidate playlists here?
}

- (BOOL)deletePossible:(std::shared_ptr<IPlaylist>)realPlaylist
{
    if (!realPlaylist) {
        return NO;
    }
    return realPlaylist->removable();
}

- (IBAction)menuItem:(NSMenuItem *)sender
{
    auto intent = playlistIntents.at([sender tag]);
    
    bool confirmationNeeded = intent->confirmationNeeded();
    
    if (confirmationNeeded) {
        NSInteger result = NSRunAlertPanel(@"Alert", convertString(intent->confirmationText()), @"Sure", @"No", nil);
        if (result != NSAlertDefaultReturn) {
            return;
        }
        //[self reloadTable];
    }
    
    intent->apply();
}

- (NSMenu *)tableView:(NSTableView *)tableView menuForRows:(NSIndexSet *)rows column:(int)column
{
    if ([rows count] == 0) {
        return nil;
    }
    if ([rows count] > 1) {
        // we already have playlistintents
    } else {
        id object = [self objectForRow:[rows firstIndex]];
        playlistIntents.clear();
        if ([object isKindOfClass:[PlaylistWrapper class]]) {
            
            std::shared_ptr<IPlaylist> realPlaylist = [(PlaylistWrapper *)object playlist];
            if (!realPlaylist) {
                return nil;
            }
            
            playlistIntents = realPlaylist->playlistIntents();
        }
    }
    if (playlistIntents.empty()) {
        return nil;
    }

    NSMenu *menu = [[NSMenu alloc] init];
    
    int i = 0;
    
    for (auto &intent : playlistIntents) {
        NSMenuItem *item = [menu addItemWithTitle:convertString(intent->menuText()) action:@selector(menuItem:) keyEquivalent:@""];
        [item setTarget:self];
        item.tag = i;
        ++i;
    }
    return menu;
}

- (void)createInstantMixForSong:(Song *)song
{
    NSAssert(NO, @"should implement this");
}

- (NSArray *) flattenArray: (NSArray *) array {
	NSMutableArray *flatArray = [NSMutableArray array];
	for (id item in array) {
		if ([item isKindOfClass: [NSArray class]]) {
			[flatArray addObjectsFromArray: [self flattenArray: item]];
		} else {
			[flatArray addObject: item];
		}
	}
//	return [flatArray copy];
	return flatArray;
}

- (NSUInteger) flattenedArrayCount: (NSArray *) array {
	NSUInteger flatCount = 0;

	for (id item in array) {
		if ([item isKindOfClass: [NSArray class]]) {
			flatCount += [self flattenedArrayCount: item];
		} else {
			flatCount++;
		}
	}
	return flatCount;
}

- (BOOL) reloadTable {
	NSMutableArray *oldTree;

	if (NSAppKitVersionNumber >= NSAppKitVersionNumber10_7) {
		oldTree = [NSMutableArray arrayWithArray: tree];
	}

	[tree removeAllObjects];
	if (tree == nil) {
		tree = [NSMutableArray array];
	}

	[categories enumerateObjectsUsingBlock: ^(id object, NSUInteger i, BOOL *stop) {
		NSMutableArray *categoriesAndPlaylists = [NSMutableArray arrayWithObject: object];
	//	NSLog(@"OBJ: %@", object);
		if (hiddenCategories[object] == nil) {
			[categoriesAndPlaylists addObjectsFromArray: playlistsByCategories[object]];
		}
		[categoriesAndPlaylists addObject: [NSString stringWithFormat: @"gear_spacer_%@", object]];
	//	NSLog(@"catpl: %@", categoriesAndPlaylists);
		[tree addObject: categoriesAndPlaylists];
	}];

	//NSLog(@"tree: %@", tree);

    if (NSAppKitVersionNumber >= NSAppKitVersionNumber10_7) {
		NSMutableIndexSet *indexesToRemove = [NSMutableIndexSet indexSet];
		NSMutableIndexSet *indexesToAdd = [NSMutableIndexSet indexSet];

		NSArray *oldFlatTree = [self flattenArray: oldTree];
		NSArray *flatTree = [self flattenArray: tree];

		// remove stuff
		[oldFlatTree enumerateObjectsUsingBlock: ^(id object, NSUInteger i, BOOL *stop) {
			if (! [flatTree containsObject: object]) {
				[indexesToRemove addIndex: i];
			}
            // this leads to ugly animation and possibly crash
            /*else {
				if ([object isKindOfClass: [NSString class]] && [flatTree indexOfObject: object] != [oldFlatTree indexOfObject: object]) {
					[indexesToRemove addIndex: i];
				}
			}*/
		}];
        // this is bad, don't even think about it
        //[oldFlatTree removeObjectsAtIndexes:indexesToRemove];
        
		// add stuff
		[flatTree enumerateObjectsUsingBlock: ^(id object, NSUInteger i, BOOL *stop) {
			if (! [oldFlatTree containsObject: object]) {
				[indexesToAdd addIndex: i];
			} /*else {
				if ([object isKindOfClass: [NSString class]] && [flatTree indexOfObject: object] != [oldFlatTree indexOfObject: object]) {
					[indexesToAdd addIndex: i];
				}
			}*/
		}];
		[indexesToRemove enumerateIndexesUsingBlock: ^(NSUInteger i, BOOL *stop) {
			if ([lastMultiSelect containsIndex: i]) {
				[lastMultiSelect removeIndex: i];
			}
		}];
        
        if (([oldFlatTree count] - [indexesToRemove count] + [indexesToAdd count]) != [flatTree count] || [cachedTableView numberOfRows] != [oldFlatTree count]) {
            
            NSLog(@"playlist update inconsistency total rows: %ld (%lu,%lu) remove: %@ add: %@", (long)[cachedTableView numberOfRows], (unsigned long)[oldFlatTree count], (unsigned long)[flatTree count], indexesToRemove, indexesToAdd);
            [cachedTableView reloadData];
        } else {
            [cachedTableView beginUpdates];
            [cachedTableView removeRowsAtIndexes: indexesToRemove withAnimation: NSTableViewAnimationSlideUp];
            [cachedTableView insertRowsAtIndexes: indexesToAdd withAnimation: NSTableViewAnimationSlideUp];
            [cachedTableView endUpdates];
        }
	} else {
		[cachedTableView reloadData];
	}

	auto playlistToSelect = IApp::instance()->selectedPlaylistConnector().value().second;
	if (playlistToSelect) {
		[self selectPlaylist: [[PlaylistWrapper alloc] initWithPlaylist: playlistToSelect] withScroll: NO];
	}

	return YES;
}

#pragma mark -- making order in chaos

- (NSArray *)nonEmptyCategories
{
    return [categories filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary *bindings) {
        
        return [[playlistsByCategories objectForKey:evaluatedObject] count] > 0;
    }]];
}

- (void)setupCategories
{
    categories = [NSMutableArray array];
    playlistsByCategories = [NSMutableDictionary dictionary];
	categoriesDict = [NSMutableDictionary dictionary];
    auto c = [self]{
        
        auto rhs = IApp::instance()->sessionManager()->categories();
            
        [categories removeAllObjects];
        [playlistsByCategories removeAllObjects];
        for (auto &category : *rhs) {
            
            NSString *key = convertString(category.title());
//            [categories addObject:key];
		categoriesDict[@(category.tag())] = key;
            NSMutableArray *playlists = [NSMutableArray array];
            for (auto &playlist : category.playlists()) {
                [playlists addObject:[[PlaylistWrapper alloc] initWithPlaylist:playlist]];
            }
            
            [playlistsByCategories setObject:playlists forKey:key];
        }

	for (id i in  [[categoriesDict allKeys] sortedArrayUsingSelector: @selector(compare:)]) {
		[categories addObject: categoriesDict[i]];
	}

        [self reloadTable];
        
        auto playlistToSelect = IApp::instance()->selectedPlaylistConnector().value().second;
        if (playlistToSelect) {
            [self selectPlaylist:[[PlaylistWrapper alloc] initWithPlaylist:playlistToSelect] withScroll: NO];
        }
    };
    
    c();
    categoriesConnection = IApp::instance()->sessionManager()->playlistsChangeEvent().connect(c);
    
    selectedPlaylistConnection = IApp::instance()->selectedPlaylistConnector().connect([self](const pair<PlaylistCategory,std::shared_ptr<IPlaylist>> &p){
        
        auto &playlistToSelect = p.second;
        
        if (playlistToSelect) {
            [self selectPlaylist:[[PlaylistWrapper alloc] initWithPlaylist:playlistToSelect] withScroll: NO];
        }
    });
}

- (void)selectPlaylist: (id<PlaylistProtocol>) playlist withScroll: (BOOL) scroll
{
	NSInteger index = [self rowForObject: playlist];
    
	if (index != NSNotFound) {
		NSIndexSet *row = [NSIndexSet indexSetWithIndex: index];
		if (row != nil) {
            lastMultiSelect = [row mutableCopy];
            
			dispatch_async(dispatch_get_main_queue(), ^{
				[cachedTableView selectRowIndexes: row byExtendingSelection: NO];
				if (scroll) {
					[cachedTableView scrollRowToVisible: [row firstIndex]];
				}
			});
		}
	}
}

- (void)startEditing:(std::shared_ptr<IPlaylist>)playlist
{
    dispatch_block_t block = ^{
    
        [playlistsByCategories enumerateKeysAndObjectsUsingBlock:^(id key, NSArray *arr, BOOL *stop){
            
            for (id obj in arr) {
            
                if ([obj isKindOfClass:[PlaylistWrapper class]]) {
                    PlaylistWrapper *wrapper = obj;
                    std::shared_ptr<IPlaylist> ePlaylist = [wrapper playlist];
                    if (equals(ePlaylist, playlist)) {
                        
                        [self startEditingObject:wrapper];
                        *stop = YES;
                    }
                }
            }
        }];
    };
    
    if ([self reloadTable]) {
        // wait because otherwise editing will stop...
        double delayInSeconds = 0.3f;
        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
        dispatch_after(popTime, dispatch_get_main_queue(), block);
    } else {
        block();
    }
}

- (void)startEditingObject:(id)playlist
{
    [IndentedTextFieldCell enableEditingWhenCapable];
    NSInteger row = [self rowForObject:playlist];
    if (row == NSNotFound) {
        return;
    }
    
    [cachedTableView editColumn:0 row:row withEvent:nil select:YES];
}

- (NSDragOperation)tableView:(NSTableView *)aTableView validateDrop:(id < NSDraggingInfo >)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)operation
{
    id playlistCandidate = [self objectForRow:row];
    
    auto &dragManager = Gear::IApp::instance()->dragManager();
    auto draggedSongs = dragManager.draggedEntries();
    
    std::shared_ptr<IPlaylist> destination;
    
    if ([playlistCandidate isKindOfClass:[PlaylistWrapper class]]) {
        destination = [(PlaylistWrapper *)playlistCandidate playlist];
    }
        
    if (destination) {
        auto intent = destination->dragIntentTo(draggedSongs);
        if (intent) {
            
            [aTableView setDropRow:row dropOperation:NSTableViewDropOn];
            return NSDragOperationCopy;
        }
    }
    
    return 0;
}

- (BOOL)tableView:(NSTableView *)aTableView acceptDrop:(id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation
{
    id playlistCandidate = [self objectForRow:row];
    
    auto &dragManager = Gear::IApp::instance()->dragManager();
    auto draggedSongs = dragManager.draggedEntries();
    
    std::shared_ptr<IPlaylist> destination;
    if ([playlistCandidate isKindOfClass:[PlaylistWrapper class]]) {
        destination = [(PlaylistWrapper *)playlistCandidate playlist];
    }
    
    if (destination) {
        auto intent = destination->dragIntentTo(draggedSongs);
        if (intent) {
            
            intent->apply(draggedSongs);
            return YES;
        }
    }

    return 0;
}

@end


@implementation ClickableTextField

@synthesize playlist;
/*
- (void)mouseDown:(NSEvent *)theEvent
{
    [[self delegate] control:self textShouldBeginEditing:nil];
}

- (BOOL)becomeFirstResponder
{
    return NO;
}*/

/*
- (void)selectText:(id)sender
{
    return;
}

- (void)selectTextForced
{
    [super selectText:nil];
}*/

- (void) setBackgroundStyle: (NSBackgroundStyle) style
{
    // this causes problems here, makes it difficult to set color in cell!
	//self.textColor = (style == NSBackgroundStyleDark) ? [NSColor colorWithDeviceWhite: .9 alpha: 1.] : [NSColor blackColor];
	//self.backgroundColor= [NSColor redColor];
}

- (BOOL) drawsBackground {
	return NO;
}

@end


@implementation CustomTableRow

- (void)drawBackgroundInRect:(NSRect)dirtyRect
{
    if (self.customBackgroundGradient != nil) {
        [self.customBackgroundGradient drawInRect:[self bounds] angle:270.0f];
    } else if (self.customBackground != nil) {
        [self.customBackground setFill];
        NSRectFill(dirtyRect);
    } else {
        [super drawBackgroundInRect:dirtyRect];
        return;
    }

    if (dirtyRect.origin.y == 0 && !self.omitTop) {
        [self.topColor setFill];
        NSRect topRect = CGRectMake(dirtyRect.origin.x, 0, dirtyRect.size.width, 1);
        NSRectFill(topRect);
    }
    
    if (dirtyRect.origin.y + dirtyRect.size.height >= self.frame.size.height-1) {
        
        [self.bottomColor setFill];
        NSRect bottomRect = CGRectMake(dirtyRect.origin.x, self.frame.size.height-1, dirtyRect.size.width, 1);
        NSRectFill(bottomRect);
    }
}

- (void)drawSelectionInRect:(NSRect)dirtyRect
{
    if (self.selectedGradient == nil) {
        [super drawSelectionInRect:dirtyRect];
        return;
    }
 
    CGRect bounds = self.bounds;
    
    [self.selectedGradient drawInRect:bounds angle:270.0f];
    
    [self.selectionBorder setFill];

    NSRect leftRect = CGRectMake(0, bounds.origin.y, 1, bounds.size.height);
    NSRect rightRect = CGRectMake(bounds.size.width-1, bounds.origin.y, 1, bounds.size.height);
    NSRect bottomRect = CGRectMake(bounds.origin.x, bounds.origin.y + bounds.size.height-1, bounds.size.width, 1);
    NSRectFill(bottomRect);
    NSRectFill(leftRect);
    NSRectFill(rightRect);
}

- (void)setSelected:(BOOL)value
{
    BOOL change = (value != self.selected);
    
    [super setSelected:value];
    
    if (change) {
        if (self.numberOfColumns > 0) {
            [self refreshSelected];
        } else {
            dispatch_async(dispatch_get_main_queue(), ^{
                [self refreshSelected];
            });
        }
    }
}

- (void)refreshSelected
{
    NSControl *control = [self viewAtColumn:0];
    IndentedTextFieldCell *cell = control.cell;
    [cell setSelected:self.selected];
    [self setNeedsDisplayRecursive];
}

@end
