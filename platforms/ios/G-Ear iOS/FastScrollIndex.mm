//
//  FastScrollIndex.m
//  G-Ear iOS
//
//  Created by Zsolt Szatmari on 10/01/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#import "FastScrollIndex.h"
#include <vector>
#include <string>
#include "TextAttributes.h"
#include "Writer.h"
#include "SongEntry.h"
#include "StringUtility.h"
#include "MusicSort.h"
#include "SongsViewController.h"
#include "IPlaylist.h"

//#define DEBUG_SCROLLINDEX

using std::vector;
using std::string;

@implementation FastScrollIndex {
    vector<uint32_t> positionToScroll;
    NSMutableArray *alphabetToDraw;
}

NSString * const kNotificationFastScroll = @"NotificationFastScroll";

- (id)init
{
    self = [super init];
    if (self) {
        [self reset];
    }
    return self;
}

- (id)initWithFrame:(CGRect)aRect
{
    self = [super initWithFrame:aRect];
    if (self) {
        [self reset];
    }
    return self;
}

- (void)reset
{
    [self setBackgroundColor:[UIColor clearColor]];
    alphabetToDraw = [NSMutableArray array];
    [self setAlpha:0.0f];
}

- (BOOL)calculateAlphabet:(int)num nameAt:(std::function<string(int)>)nameAt
{
    const int kSlots = 'Z' - 'A' + 1;
    
    [alphabetToDraw removeAllObjects];
    positionToScroll.clear();
    positionToScroll.reserve(kSlots);

    int diffs = 0;
    // if we have only a few songs this doesn't makes sense and is just confusing
    NSString *lastLetter = @"";
    if (num > 0) {
        for (int i = 0 ; i < kSlots ; ++i) {
            int position = (num-1) * i / (kSlots-1);
            
            positionToScroll.push_back(position);
            if (i % 2 == 1) {
                [alphabetToDraw addObject:@"\u25CF"];
            } else {
                std::string title = Gear::MusicSort::strip(nameAt(position));
                
                int len = title.length();
                // first 4 bytes = unicode character
                if (len == 0) {
                    [alphabetToDraw addObject:@" "];
                } else {
                    //std::cout << i << ": " << title << std::endl;
                    NSString *letter = [[Gear::convertString(title) substringToIndex:1] uppercaseString];
                    if (![lastLetter isEqualToString:letter]) {
                        ++diffs;
                    }
                    lastLetter = letter;
                    [alphabetToDraw addObject:letter];
                }
            }
        }
    }
    
#ifdef DEBUG_SCROLLINDEX
    //NSLog(@"alphabet to draw: %@", alphabetToDraw);
#endif
    
    return YES;
}

- (void)setAlphabet:(const std::map<int,std::string> &)mapping
{
    [alphabetToDraw removeAllObjects];
    positionToScroll.clear();
    positionToScroll.reserve(kFastScrollSlots);
    
    if (mapping.size() > 0) {
        
        int i = 0;
        
        for (const auto &p : mapping) {
            
            positionToScroll.push_back(p.first);
            if (i % 2 == 1) {
                [alphabetToDraw addObject:@"\u25CF"];
            } else {
                std::string title = Gear::MusicSort::strip(p.second);
                
                int len = title.length();
                // first 4 bytes = unicode character
                if (len == 0) {
                    [alphabetToDraw addObject:@" "];
                } else {
                    //std::cout << i << ": " << title << std::endl;
                    NSString *letter = [[Gear::convertString(title) substringToIndex:1] uppercaseString];
                    
                    [alphabetToDraw addObject:letter];
                }
            }

            ++i;
        }
        
        [UIView animateWithDuration:0.3f animations:^{
            [self setAlpha:1.0f];
        }];
        [self setNeedsDisplay];
        
    } else {
        [UIView animateWithDuration:0.3f animations:^{
            [self setAlpha:0.0f];
        }];
    }
}

- (void)setMapping:(const std::map<int,std::string> &)mapping
{
    [self setAlphabet:mapping];
}

- (void)setNames:(int)num nameAt:(std::function<string(int)>)nameAt
{
    BOOL result = [self calculateAlphabet:num nameAt:nameAt];
    
    if (result) {
        
        [UIView animateWithDuration:0.3f animations:^{
            [self setAlpha:1.0f];
        }];
        [self setNeedsDisplay];
    } else {
        [UIView animateWithDuration:0.3f animations:^{
            [self setAlpha:0.0f];
        }];
    }
}

- (void)setPlaylists:(const vector<shared_ptr<Gear::IPlaylist>> &)playlists
{
#ifdef DEBUG_SCROLLINDEX
     NSLog(@"setting index (artists): %zu", playlists.size());
#endif
    [self setNames:playlists.size() nameAt:[&](int num){
        return playlists.at(num)->name();
    }];
}

- (void)drawRect:(CGRect)rect
{
    if ([alphabetToDraw count] == 0) {
        return;
    }
    CGFloat height = self.frame.size.height / [alphabetToDraw count];
    for (int i = 0 ; i < [alphabetToDraw count] ; ++i) {
        CGRect letterRect = rect;
        letterRect.origin.y = i * height;
        letterRect.size.height = height;
        NSString *s = [alphabetToDraw objectAtIndex:i];

        Gui::TextAttributes attributes = Gui::TextAttributes();
        UIFont *font = Gui::Writer::convertFont(attributes);
        UIColor *color;
        if (i % 2 == 1) {
            color = [UIColor colorWithWhite:0.7f alpha:1.0f];
            //color = [UIColor grayColor];
            font = [font fontWithSize:font.pointSize -4];
            letterRect.origin.y += 2;
        } else {
            color = [UIColor whiteColor];
        }
        
        CGContextSetFillColorWithColor(UIGraphicsGetCurrentContext(), color.CGColor);
        // deprecated in ios 7.0, but the alternative is not yet present in 6.x
        
        
        [s drawInRect:letterRect withFont:font lineBreakMode:NSLineBreakByClipping alignment:NSTextAlignmentCenter];
    }
}

- (void)touches:(NSSet *)touches
{
    UITouch *touch = [touches anyObject];
    if (touch) {
        if (positionToScroll.empty()) {
            return;
        }
        int index = [touch locationInView:self].y / (self.frame.size.height / positionToScroll.size());
        if (index < 0) {
            index = 0;
        } else if (index >= positionToScroll.size()) {
            index = positionToScroll.size() -1;
        }
    
        auto pos = positionToScroll.at(index);
        [[NSNotificationCenter defaultCenter] postNotificationName:kNotificationFastScroll object:self userInfo:@{@"position":@(pos)}];
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    [self touches:touches];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    [self touches:touches];
}

@end
