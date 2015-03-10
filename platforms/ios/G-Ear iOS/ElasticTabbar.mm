//
//  ElasticTabbar.mm
//  G-Ear iOS
//
//  Created by Szabo Attila on 12/22/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "ElasticTabbar.h"
#import "ElasticFlowLayout.h"
//#import "GearImageView.h"
#import "GearTabBarItem.h"

#include "IApp.h"
#include "Painter.h"
#include "TextAttributes.h"
#include "TextAttributes.h"
#include "Writer.h"
#include "Debug.h"
#import "GearTabBarItemCell.h"
#import "F.h"
#import "NSArray+F.h"
#import "FK/FKPrelude.h"
#import "SongsViewController.h"

@implementation ElasticTabbar {
    NSArray *slots;
    NSArray *widthConstraints;
    NSLayoutConstraint *leftSideConstraint;
    NSLayoutConstraint *rightSideConstraint;
    NSArray *items;
}

static const int kTotalSlots = SETTINGS_TAG+1;

- (CGFloat)defaultCellSize
{
    // should be 80 for an iphone 4, so that there are 4 visible items in portrait
    CGSize size = [UIScreen mainScreen].applicationFrame.size;
    CGFloat width = fminf(size.width, size.height);
    
    return width/4;
}

- (id) initWithFrame: (CGRect) frame
{
	if (self = [super initWithFrame: frame]) {
        
        _collection_view = [[UIScrollView alloc] initWithFrame: self.bounds];
        _collection_view.showsHorizontalScrollIndicator = NO;
		_collection_view.scrollsToTop = NO;
        _collection_view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
		_collection_view.backgroundColor = [UIColor clearColor];

		[self addSubview: _collection_view];
        slots = [F mapRangeFrom:0 To:kTotalSlots withBlock:^(NSInteger i) {
            UIView *view = [[UIView alloc] init];
            
            view.translatesAutoresizingMaskIntoConstraints = NO;
            view.backgroundColor = [UIColor greenColor];
            view.alpha = 0.0f;
            
            [_collection_view addSubview:view];
            return view;
        }];
        widthConstraints = [slots map:^(UIView *view) {
            return [NSLayoutConstraint constraintWithItem:view attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:0 constant:0];
        }];
        [_collection_view addConstraints:[slots bind:^(UIView *view){
            return @[[NSLayoutConstraint constraintWithItem:view attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:_collection_view attribute:NSLayoutAttributeTop multiplier:1 constant:0],
                     [NSLayoutConstraint constraintWithItem:view attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:_collection_view attribute:NSLayoutAttributeBottom multiplier:1 constant:0]];
        }]];
        [_collection_view addConstraints:widthConstraints];
        leftSideConstraint = [NSLayoutConstraint constraintWithItem:[slots firstObject] attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:_collection_view attribute:NSLayoutAttributeLeft multiplier:1 constant:0];
        [_collection_view addConstraint:leftSideConstraint];
        
        // right constraint is unneeded and makes tab bar 'nervous'
        //rightSideConstraint = [NSLayoutConstraint constraintWithItem:[slots lastObject] attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:_collection_view attribute:NSLayoutAttributeRight multiplier:1 constant:0];
        //[_collection_view addConstraint:rightSideConstraint];
        
        NSArray *neighborConstraints = [[slots zip:[slots tail]] map:^(FKP2 *pair){
            return [NSLayoutConstraint constraintWithItem:pair._1 attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:pair._2 attribute:NSLayoutAttributeLeft multiplier:1 constant:0];
        }];
        [_collection_view addConstraints:neighborConstraints];
        
        UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tabTapped:)];
        [_collection_view addGestureRecognizer:tap];
	}
	return self;
}

- (NSArray *)itemCells
{
    return [[slots filter:^BOOL(UIView *view){return [[view subviews] count] > 0;}] map:^(UIView *view){return [view subviews][0];}];
}

- (void)recolor
{
    for (GearTabBarItemCell *cell in [self itemCells]) {
        cell.selected_color = self.selected_color;
        cell.unselected_color = self.unselected_color;
        [cell setSelectedTag:_selected_tag];
    }
}

- (void) drawRect: (CGRect) rect
{
    if (_background) {
		Gui::Painter::paint(_background, true);
    }
}

- (void)set_items:(NSArray *)aItems
{
    //NSLog(@"setting items: %@", aItems);
    
    static BOOL firstTime = YES;
    dispatch_block_t changes = ^{
        
        _collection_view.contentSize = CGSizeMake(fmax(self.frame.size.width, [aItems count] * self.defaultCellSize), self.frame.size.height);
        
        NSArray *tags = [aItems map:^(GearTabBarItem *item){return @(item.tag);}];
        NSDictionary *tagsToItems = [NSDictionary dictionaryWithObjects:aItems forKeys:tags];
        
        const CGFloat inset = [self insetForCount:[tags count]];
        const CGFloat itemWidth = [self widthForCount:[tags count]];
        
#ifdef DEBUG
        /*
        NSLog(@"tabbar size: %@ content size: %f inset: %@", NSStringFromCGRect(_collection_view.frame), _collection_view.contentSize.width, NSStringFromUIEdgeInsets(_collection_view.contentInset));
        for (int i = 0 ; i < kTotalSlots ; ++i) {
            NSLog(@"slot %d: %@", i, NSStringFromCGRect([slots[i] frame]));
        }*/
#endif
        
        leftSideConstraint.constant = inset;
        //rightSideConstraint.constant = -inset;
        
        int i = 0;
        for (NSLayoutConstraint *constraint in widthConstraints) {
            BOOL present = [tags containsObject:@(i)];
            
            constraint.constant = present ? itemWidth : 0;
            UIView *slot = slots[i];
            slot.alpha = present ? 1.0f : 0.0f;
            if (present) {
                UIView *cell = slots[i];
                
                // removing then readding makes tabbar "nervous"
                /*for (UIView *sub in [[cell subviews] copy]) {
                    [sub removeFromSuperview];
                }*/
                
                GearTabBarItemCell *subview = [[[cell subviews] toOption] orNil];
                if (subview == nil) {
                    subview = [self createCell];
                    subview.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
                    subview.frame = cell.bounds;
                    [cell addSubview:subview];
                }
                
                GearTabBarItem *item = [tagsToItems objectForKey:@(i)];
                
                subview.tag = item.tag;
                [subview setTabBarItem:item];
                [subview setSelectedTag:_selected_tag];
            }
            ++i;
        }
        [self layoutIfNeeded];
    };
    
    if (firstTime) {
        changes();
        firstTime = NO;
    } else {
        [self layoutIfNeeded];
        [UIView animateWithDuration:0.2f animations:changes];
    }
    items = aItems;
}

- (void) select_item_by_tag: (int) tag
{
    _selected_tag = tag;
    //TBNSLOG(@"reload tag: %d", _selected_tag);
	[self recolor];
}

#pragma mark collectionview stuff

- (CGFloat)insetForCount:(NSInteger)count
{
    return count <= 2
            ? 30
            : (count <= 3
               ? 10
               : 0);
}

- (CGFloat)widthForCount:(NSInteger)totalCount
{
    if (totalCount >= 4) {
        return self.defaultCellSize;
    }
    
    const int visibleSlots = self.frame.size.width / self.defaultCellSize;
    const int count = std::min(totalCount, visibleSlots);
    
    const CGFloat insets = [self insetForCount:count];
    const CGFloat sides = 2*insets;
    
    return (self.frame.size.width - sides) /count;
}

- (GearTabBarItemCell *)createCell
{
    GearTabBarItemCell *cell = [[GearTabBarItemCell alloc] initWithFrame:CGRectMake(0,0,self.defaultCellSize,self.frame.size.height)];
    cell.selected_color = self.selected_color;
    cell.unselected_color = self.unselected_color;
    [cell setSelectedTag:_selected_tag];
    
	return cell;
}

- (void)didRotateInterface
{
    [self reloadData];
}

- (void)reloadData
{
    [self set_items:items];
}

- (void)tabTapped:(UITapGestureRecognizer *)gesture
{
    NSArray *activeSlots = [slots filter:^BOOL(UIView *view) {return view.frame.size.width > 0;}];
    float tapX = [gesture locationInView:_collection_view].x;
    
    auto distance = [&](const UIView *view){
        float viewCenter = CGRectGetMidX(view.frame);
        return fabs(viewCenter - tapX);
    };
    
    /*NSLog(@"tap: %@", NSStringFromCGPoint([gesture locationInView:self]));
    for (id view in activeSlots) {
        NSLog(@"slot: %@ tag: %d distance: %f", NSStringFromCGRect([view frame]), [[[view subviews] firstObject] tag], distance(view));
    }*/
    
    UIView *closestSlot = [activeSlots min:^NSComparisonResult(id a, id b) {
        return [@(distance(a)) compare:@(distance(b))];
    }];
    [[[closestSlot subviews] toOption] foreach:^(GearTabBarItemCell *cell) {
        //NSLog(@"selected tag: %d", cell.tag);
        [_didselect_delegate tabBarDidSelectTag:cell.tag];
    }];
}

- (void)applyTheme
{
    [self recolor];
    [self reloadData];
    [self setNeedsDisplay];
}

@end
