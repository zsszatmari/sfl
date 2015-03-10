//
//  AlbumViewRow.m
//  Gear for Google Play and YouTube
//
//  Created by Zsolt Szatmari on 12/01/15.
//
//

#import "AlbumViewRow.h"
#import "Grouping.h"
#import "CocoaThemeManager.h"
#import "NSDictionary+F.h"
#import "VerticallyCenteredTextField.h"
#import "NSView+Recursive.h"
#include <algorithm>


@interface GroupingImageView : NSView

- (void)setGrouping:(Grouping *)grouping;

@end

@implementation GroupingImageView {
    __weak Grouping *grouping;
    NSImage *image;
    int currentIndex;
}

- (void)drawRect:(NSRect)dirtyRect
{
    if (image == nil) {
        __weak GroupingImageView *w = self;
        int index = currentIndex;
        
        image = [grouping imageRepresentationWithRefresh:^{
            GroupingImageView *s = w;
            if (s && index == currentIndex) {
                s->image = nil;
                [s setNeedsDisplay:YES];
            }
        }];
    }
    
    //NSLog(@"image bounds: %@", NSStringFromRect(self.bounds));
    [image drawInRect:self.bounds fromRect:CGRectMake(0,0,image.size.width,image.size.height) operation:NSCompositeSourceOver fraction:1.0f];
}

- (void)setGrouping:(Grouping *)aGrouping
{
    if (grouping != aGrouping) {
        grouping = aGrouping;
        ++currentIndex;
        image = nil;
        [self setNeedsDisplay:YES];
    }
}

@end

@interface AlbumViewCell : NSView

- (void)setGrouping:(Grouping *)grouping selected:(BOOL)selected;

@property(nonatomic,copy) void (^clickHandler)(int);

@end

@implementation AlbumViewCell {
    GroupingImageView *imageView;
    VerticallyCenteredTextField *title;
    VerticallyCenteredTextField *subtitle;
    NSView *titleBorder;
    CALayer *borderLayer;
}

//static int count = 0;

- (id)init
{
    self = [super init];
    if (self) {
        //++count;
        //NSLog(@"inc: %d",count);
        
        imageView = [[GroupingImageView alloc] init];
        title = [[VerticallyCenteredTextField alloc] init];
        subtitle = [[VerticallyCenteredTextField alloc] init];
        title.disableAntialias = YES;
        subtitle.disableAntialias = YES;
        
        title.bezeled = NO;
        title.editable = NO;
        title.drawsBackground = NO;
        title.alignment = NSCenterTextAlignment;
        
        titleBorder = [[NSView alloc] init];
        borderLayer = [CALayer layer];
        borderLayer.cornerRadius = 3.0f;
        titleBorder.layer = borderLayer;
        //title.layer = borderLayer;
        
        subtitle.bezeled = NO;
        subtitle.editable = NO;
        subtitle.drawsBackground = NO;
        subtitle.alignment = NSCenterTextAlignment;
        
        imageView.translatesAutoresizingMaskIntoConstraints = NO;
        title.translatesAutoresizingMaskIntoConstraints = NO;
        titleBorder.translatesAutoresizingMaskIntoConstraints = NO;
        subtitle.translatesAutoresizingMaskIntoConstraints = NO;
        
        [self addSubview:imageView];
        [self addSubview:titleBorder];
        [self addSubview:title];
        [self addSubview:subtitle];
        
        NSDictionary *views = NSDictionaryOfVariableBindings(imageView,title,subtitle);
        
        [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[title]-0-[subtitle]|" options:NSLayoutFormatAlignAllCenterX metrics:nil views:views]];
        [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[imageView]-6-[title]" options:NSLayoutFormatAlignAllCenterX metrics:nil views:views]];
        //[self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[imageView]|" options:NSLayoutFormatAlignAllCenterX metrics:nil views:views]];
        
        [self addConstraint:[NSLayoutConstraint constraintWithItem:imageView attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeCenterX multiplier:1 constant:0]];
        [self addConstraint:[NSLayoutConstraint constraintWithItem:titleBorder attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:title attribute:NSLayoutAttributeTop multiplier:1 constant:3]];
        [self addConstraint:[NSLayoutConstraint constraintWithItem:titleBorder attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:title attribute:NSLayoutAttributeLeft multiplier:1 constant:-5]];
        [self addConstraint:[NSLayoutConstraint constraintWithItem:titleBorder attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:title attribute:NSLayoutAttributeRight multiplier:1 constant:+3]];
        [self addConstraint:[NSLayoutConstraint constraintWithItem:titleBorder attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:title attribute:NSLayoutAttributeBottom multiplier:1 constant:0]];
        
        
        [self addConstraint:[NSLayoutConstraint constraintWithItem:imageView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:imageView attribute:NSLayoutAttributeHeight multiplier:1 constant:0]];
    }
    return self;
}

/*
- (void)dealloc
{
    --count;
    NSLog(@"def: %d",count);
}*/

- (void)setBorderColor
{
    NSColor *color = [NSColor colorWithDeviceRed:81.0f/255.0f green:121.0f/255.0f blue:198.0f/255.0f alpha:0.7f];
    
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGFloat components[] = {color.redComponent, color.greenComponent, color.blueComponent, color.alphaComponent};
    CGColorRef cgColor = CGColorCreate(space, components);
    borderLayer.backgroundColor = cgColor;
    CGColorSpaceRelease(space);
    CGColorRelease(cgColor);
}

- (NSDictionary *)titleAttributes
{
    return [[CocoaThemeManager sharedManager] albumViewTitleAttributes];
    //return [[[CocoaThemeManager sharedManager] albumViewTitleAttributes] addObject:[self paragraphStyle] forKey:NSParagraphStyleAttributeName];
}

- (NSDictionary *)subtitleAttributes
{
    return [[CocoaThemeManager sharedManager] albumViewSubtitleAttributes];
    //return [[[CocoaThemeManager sharedManager] albumViewSubtitleAttributes] addObject:[self paragraphStyle] forKey:NSParagraphStyleAttributeName];
}

- (void)setGrouping:(Grouping *)grouping selected:(BOOL)selected
{
    imageView.grouping = grouping;
    
    title.attributedStringValue = [[NSAttributedString alloc] initWithString:grouping.title attributes:[self titleAttributes]];
    subtitle.attributedStringValue = [[NSAttributedString alloc] initWithString:grouping.subtitle attributes:[self subtitleAttributes]];

    titleBorder.hidden = !selected;
    if (selected) {
        [self setBorderColor];
    }
    
    //[self applyTheme];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    if (self.clickHandler) {
        self.clickHandler((int)[theEvent clickCount]);
    }
}

@end

@implementation AlbumViewRow {
    NSMutableArray *cells;
    void (^clickHandler)(NSUInteger,int);
}

- (id)initWithClickHandler:(void(^)(NSUInteger,int))aClickHandler
{
    self = [super init];
    if (self) {
        clickHandler = aClickHandler;
    }
    return self;
}


- (void)setFrame:(CGRect)frame
{
    CGRect oldFrame = [super frame];
    [super setFrame:frame];
    //NSLog(@"setting frame: %@", NSStringFromRect(frame));
    
    if (oldFrame.size.width != frame.size.width || oldFrame.size.height != frame.size.height) {
        [self setupViews];
    }
}

- (void)setGroupings:(NSArray *)groupings itemsPerRow:(NSInteger)items startIndex:(NSUInteger)startIndex selectedIndex:(NSUInteger)selectedIndex
{
    if (cells == nil) {
        cells = [NSMutableArray array];
    }
    if ([cells count] != items) {
        while ([cells count] < items) {
            AlbumViewCell *cell = [[AlbumViewCell alloc] init];
            //cell.translatesAutoresizingMaskIntoConstraints = NO;
            
            [cells addObject:cell];
            [self addSubview:cell];
        }
        while ([cells count] > items) {
            AlbumViewCell *cell = [cells lastObject];
            [cell removeFromSuperview];
            [cells removeLastObject];
        }
        
        [self setupViews];
        //[self resetConstraints];
    }
    for (int i = 0 ; i < items ; ++i) {
        AlbumViewCell *cell = [cells objectAtIndex:i];
        if (i < [groupings count]) {
            NSUInteger index = startIndex + i;
            [cell setGrouping:[groupings objectAtIndex:i] selected:index == selectedIndex];
            [cell setHidden:NO];
            cell.clickHandler = ^(int count){
                clickHandler(index, count);
            };
        } else {
            [cell setHidden:YES];
        }
    }
}

- (void)setupViews
{
    int size = self.frame.size.width / std::max<NSUInteger>(1,[cells count]);
    int x = 0;
    
    for (NSView *cell : cells) {
        CGRect frame;
        frame.origin.y = self.intercellSpacing.height/2;
        frame.size.height = self.frame.size.height - self.intercellSpacing.height;
        frame.origin.x = x;
        frame.size.width = size;
        x += size;
        cell.frame = frame;
        
        // oddly, an autoresizing mask (or autolayout!) makes it unmanageable
        //cell.autoresizingMask = NSViewMinXMargin | NSViewMaxXMargin | NSViewWidthSizable | NSViewHeightSizable;
        //[cell setNeedsDisplay:YES];
    }
}

@end
