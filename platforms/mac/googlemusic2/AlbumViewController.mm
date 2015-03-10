//
//  AlbumViewController.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 10/16/12.
//
//

#include <vector>
#import "Debug.h"
#import "AlbumViewController.h"
#import "AppDelegate.h"
#import "Grouping.h"
#import "MainWindowController.h"
#import "CocoaThemeManager.h"
#include "ValueSignal.h"
#include "IPlaylist.h"
#import "ResizingImageBrowserView.h"
#import "AlbumViewRow.h"

NSString *const kDefaultsKeyAlbumCellSize = @"AlbumCellSize";

using std::vector;
using namespace Gear;
using namespace Base;

@interface AlbumViewController () {
    vector<SongGrouping> visibleGroupings;
    NSMutableArray *nativeGroupings;
    Gear::SongPredicate filterPredicate;
    BOOL selfInducedSelection;
    
    SignalConnection groupingsConnection;
    SignalConnection selectedConnection;
    NSUInteger selectedIndex;
    NSUInteger lastItemsPerRow;
}
@end

// not a class variable but static to workaround a strange crash
static shared_ptr<IPlaylist> _browserPlaylist;
static const int intercellSpacing = 40;

@implementation AlbumViewController

- (void)awakeFromNib
{
#ifdef DISABLE_IMAGEKIT
    return;
#endif
    selectedIndex = NSNotFound;
    
    NSTableView *realImageBrowser = [[NSTableView alloc] init];
    realImageBrowser.frame = self.imageBrowserView.frame;
    realImageBrowser.autoresizingMask = self.imageBrowserView.autoresizingMask;
    [(NSClipView *)[self.imageBrowserView superview] setDocumentView:realImageBrowser];
    [self.imageBrowserView removeFromSuperview];
    self.imageBrowserView = realImageBrowser;
    self.imageBrowserView.delegate = self;
    self.imageBrowserView.dataSource = self;
    self.imageBrowserView.headerView = nil;
    self.imageBrowserView.backgroundColor = [NSColor clearColor];
    self.imageBrowserView.selectionHighlightStyle = NSTableViewSelectionHighlightStyleNone;
    [self.imageBrowserView.enclosingScrollView addObserver:self forKeyPath:@"frame" options:NSKeyValueObservingOptionNew context:nil];
    [[AppDelegate sharedDelegate] mainWindowController].imageBrowserView = self.imageBrowserView;
    
    [self setZoomValue];
    
    NSUserDefaultsController *defaultsContoller = [NSUserDefaultsController sharedUserDefaultsController];
    [defaultsContoller addObserver:self forKeyPath:[NSString stringWithFormat:@"values.%@",kDefaultsKeyAlbumCellSize] options:0 context:nil];
    
    [self applyTheme];
}

- (void)applyTheme
{
    for (Grouping *grouping in nativeGroupings) {
        [grouping refresh];
    }
    [self.imageBrowserView reloadData];
}

- (void)setZoomValue
{
    [self.imageBrowserView reloadData];
    
    // resize left panel
    [[[AppDelegate sharedDelegate] mainWindowController] adjustCollection];
}

- (CGFloat)cellTotalHeight
{
    return floor([[[[NSUserDefaultsController sharedUserDefaultsController] values] valueForKey:kDefaultsKeyAlbumCellSize] floatValue] + intercellSpacing);
}

- (CGFloat)tableView:(NSTableView *)tableView
         heightOfRow:(NSInteger)row
{
    return [self cellTotalHeight];
}

- (CGSize)cellSize
{
    CGFloat netSize = [self cellTotalHeight];
    return CGSizeMake(netSize, netSize);
}

- (Gear::SongPredicate)filterPredicate
{
    return filterPredicate;
}

- (void)setFilterPredicate:(Gear::SongPredicate)value
{
    if (!(value == filterPredicate)) {
        filterPredicate = value;
        [self reload];
    }
}

#ifndef DISABLE_IMAGEKIT



- (NSUInteger) numberOfItemsInImageBrowser:(IKImageBrowserView *)aBrowser
{
    auto ret = [nativeGroupings count];
    return ret;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    NSInteger ret = ([nativeGroupings count]+ [self itemsPerRow]-1) / [self itemsPerRow];
    return ret;
}

- (NSTableRowView *)tableView:(NSTableView *)tableView
                rowViewForRow:(NSInteger)row
{
    static NSString * const identifier = @"Row";
    AlbumViewRow *view = [self.imageBrowserView makeViewWithIdentifier:identifier owner:self];
    if (view == nil) {
        view = [[AlbumViewRow alloc] initWithClickHandler:^(NSUInteger index, int clickCount){
            if (index < [nativeGroupings count]) {
                if (clickCount == 1) {
                    [self selectItemAtIndexByUser:index];
                } else if (clickCount == 2) {
                    [self cellWasDoubleClickedAtIndex:index];
                }
            }
        }];
        view.identifier = identifier;
        
        view.intercellSpacing = NSMakeSize(intercellSpacing, intercellSpacing);
        //view.tableView = tableView;
    }
    
    NSArray *groupings = (row * [self itemsPerRow] >= [nativeGroupings count])
                            ? [NSArray array]
                            : [nativeGroupings objectsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:
                                    NSMakeRange(row * [self itemsPerRow],
                                                std::min<int>([nativeGroupings count]-row * [self itemsPerRow],
                                                      [self itemsPerRow]))]];
    [view setGroupings:groupings itemsPerRow:[self itemsPerRow] startIndex:row*[self itemsPerRow] selectedIndex:selectedIndex];
    return view;
}

- (NSView *)tableView:(NSTableView *)tableView
   viewForTableColumn:(NSTableColumn *)tableColumn
                  row:(NSInteger)row
{
    return nil;
}

- (id)imageBrowser:(IKImageBrowserView *)aBrowser itemAtIndex:(NSUInteger)index
{
    return [nativeGroupings objectAtIndex:index];
}

#endif

- (void)refresh
{
    lastItemsPerRow = [self itemsPerRow];
    
    //NSLog(@"reload");
    [self reload];
    [self.imageBrowserView scrollRowToVisible:selectedIndex/[self itemsPerRow]];
}

- (int)itemsPerRow
{
    return std::min<int>(10,fmaxl(1,floor(self.imageBrowserView.enclosingScrollView.frame.size.width / [self cellTotalHeight])));
}

/*
- (BOOL)isVisible:(Grouping *)grouping
{
    NSIndexSet *visible = [self.imageBrowserView visibleItemIndexes];
    NSUInteger index = [nativeGroupings indexOfObject:grouping];
    
    return [visible containsIndex:index];
}*/

- (void)reload
{
    [self.imageBrowserView reloadData];
}

- (void)selectItemAtIndex:(NSUInteger)index
{
    selectedIndex = index;
    [[AppDelegate sharedDelegate].mainWindowController search:nil];
    [self refresh];
}

- (void)selectItemAtIndexByUser:(NSUInteger)index
{
    if (index < visibleGroupings.size()) {
        auto selected = visibleGroupings.at(index);
        _browserPlaylist->selectGrouping(selected,false);
        
        [self selectItemAtIndex:index];
    }
}

- (void)setBrowserPlaylist:(shared_ptr<IPlaylist>)value
{
    if (value == _browserPlaylist) {
        return;
    }
    _browserPlaylist = value;
    
    if (value) {
        groupingsConnection = _browserPlaylist->availableGroupingsConnector().connect([self](const vector<SongGrouping> &groupings){
            
            visibleGroupings = groupings;
            
            nativeGroupings = [NSMutableArray arrayWithCapacity:visibleGroupings.size()];
            for (auto &g : visibleGroupings) {
                [nativeGroupings addObject:[Grouping groupingWith:g]];
            }
            
            [self reload];
        });
        selectedConnection = _browserPlaylist->selectedGroupingConnector().connect([self](const SongGrouping &selected){
            
            [self selectGrouping:selected];
        });
    }
}

- (shared_ptr<IPlaylist>)browserPlaylist
{
    return _browserPlaylist;
}

- (void)selectGrouping:(const SongGrouping &)grouping
{
    if (grouping) {
        auto it = find(visibleGroupings.begin(), visibleGroupings.end(), grouping);
        
        if (it != visibleGroupings.end()) {
            auto index = std::distance(visibleGroupings.begin(), it);
            
            [self selectItemAtIndex:index];
            [self.imageBrowserView scrollRowToVisible:index/[self itemsPerRow]];
        }
    }
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath hasSuffix:kDefaultsKeyAlbumCellSize]) {
        
        [self setZoomValue];
        //[self.imageBrowserView reloadData];
    } else if ([keyPath isEqualToString:@"frame"]) {
        //CGRect newFrame;
        //[[change objectForKey:NSKeyValueChangeNewKey] getValue:&newFrame];
        //NSLog(@"newFrame: %@ %@", NSStringFromRect(newFrame), NSStringFromRect(self.imageBrowserView.frame));
        
        NSUInteger value = [self itemsPerRow];
        if (value != lastItemsPerRow) {
            [self refresh];
        }
    }
}

- (void)cellWasDoubleClickedAtIndex:(NSUInteger) index
{
    // it does not really matter which item is for the second click...
    //[self selectItemAtIndex:index];
    
    float delayInSeconds = 0.5;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        [[AppDelegate sharedDelegate].mainWindowController playFirstForced:YES];
    });
}

- (void)dealloc
{
    self.browserPlaylist = nil;
}

@end
