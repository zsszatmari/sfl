//
//  ElasticFlowLayout.m
//  G-Ear iOS
//
//  Created by Szabo Attila on 12/22/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "ElasticFlowLayout.h"

@implementation ElasticFlowLayout

- (id) init {
	if (self = [super init]) {
		NSLog(@"init");
		_animator = [[UIDynamicAnimator alloc] initWithCollectionViewLayout: self];
		_gravity = [[UIGravityBehavior alloc] init];
		_gravity.angle = M_PI_2;
		_gravity.magnitude = 1.;
		[_animator addBehavior: _gravity];
		_collision = [[UICollisionBehavior alloc] init];
		_collision.translatesReferenceBoundsIntoBoundary = YES;
		[_collision addBoundaryWithIdentifier: @"bottom" fromPoint: CGPointMake(-100000, 55) toPoint: CGPointMake(100000, 55)];
		[_animator addBehavior: _collision];
	}
	return self;
}

- (void) prepareLayout {
	//[UIView setAnimationsEnabled: NO];
	[super prepareLayout];
	CGRect rect = self.collectionView.bounds;
	rect.origin.x -= 100.;
	rect.size.width += 200;
	NSArray *items = [super layoutAttributesForElementsInRect: rect];

	[items enumerateObjectsUsingBlock: ^(id <UIDynamicItem> item, NSUInteger i, BOOL *stop) {

		__block BOOL found = NO;
		// step over the first behavior (gravity)
		[[_animator.behaviors subarrayWithRange: NSMakeRange(2, _animator.behaviors.count - 2)] enumerateObjectsUsingBlock: ^(id behavior, NSUInteger j, BOOL *stop) {
			if (((UICollectionViewLayoutAttributes *) ((UIAttachmentBehavior *) behavior).items.firstObject).indexPath.item == ((UICollectionViewLayoutAttributes *) item).indexPath.item)
				found = *stop = YES;
		}];

		if (! found) {
			UIAttachmentBehavior *behaviour = [[UIAttachmentBehavior alloc] initWithItem: item attachedToAnchor: item.center];
//			behaviour.length = .0;
//			behaviour.damping = .8;
//			behaviour.frequency = 1.;
			behaviour.length = .0;
			behaviour.damping = .8;
			behaviour.frequency = 3.;

			//[_gravity addItem: item];
			//[_collision addItem: item];

			[_animator addBehavior: behaviour];
		}
	}];
}

- (void) prepareForCollectionViewUpdates: (NSArray *) uitems {
	CGPoint center;
	for (UICollectionViewUpdateItem *uitem in uitems) {
		//NSLog(@"ITEM: %@", uitem);
		if (uitem.updateAction == UICollectionUpdateActionInsert) {
			for (UICollectionViewLayoutAttributes *mitem in [_animator itemsInRect: CGRectMake(200, 0, 600, 50)]) {
				//NSLog(@"MITEM: %@", mitem.indexPath);

				center = mitem.center;
				center.x -= 70;
				mitem.center = center;

				[_animator updateItemUsingCurrentState: mitem];
			}

			UICollectionViewLayoutAttributes *attr = [super initialLayoutAttributesForAppearingItemAtIndexPath: uitem.indexPathAfterUpdate];
			center = attr.center;
////			center.y += 50;
			center.y -= 50;
			center.x += 60;

			UICollectionViewLayoutAttributes *insert_attr = [self layoutAttributesForItemAtIndexPath: uitem.indexPathAfterUpdate];
			//center = insert_attr.center;
			insert_attr.center = center;
			[_animator updateItemUsingCurrentState: insert_attr];
//			center.y += 50;
//			insert_attr.center = center;
//			[_animator updateItemUsingCurrentState: insert_attr];
		//	NSLog(@"UITEM: %@", uitem);
		}
	}
}

- (NSArray *) layoutAttributesForElementsInRect: (CGRect) rect {
	return [_animator itemsInRect: rect];
}

- (UICollectionViewLayoutAttributes *) layoutAttributesForItemAtIndexPath: (NSIndexPath *) index_path {
	return [_animator layoutAttributesForCellAtIndexPath: index_path];
}

- (BOOL) shouldInvalidateLayoutForBoundsChange: (CGRect) new_bounds {
	// scroll delta
	CGFloat delta = new_bounds.origin.x - self.collectionView.bounds.origin.x;
	CGFloat touch_x = [self.collectionView.panGestureRecognizer locationInView: self.collectionView].x;

	for (UIAttachmentBehavior *behavior in [_animator.behaviors subarrayWithRange: NSMakeRange(2, _animator.behaviors.count - 2)]) {
		UICollectionViewLayoutAttributes *attr = [behavior.items firstObject];
		CGPoint center = attr.center;
		//NSLog(@"CENTER: %@", NSStringFromCGPoint(attr.center));
		center.x += delta * MIN(1, fabsf(behavior.anchorPoint.x - touch_x) / 400.);
		attr.center = center;

		[_animator updateItemUsingCurrentState: attr];
	}

	return NO;
}

@end
