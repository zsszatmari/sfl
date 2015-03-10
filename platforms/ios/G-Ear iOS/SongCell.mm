//
//  SongCell.m
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "SongCell.h"
#include "StringUtility.h"
#include "IApp.h"
#include "Painter.h"
#include "TextAttributes.h"
#include "Writer.h"
#include "AlbumArtStash.h"
#include "ISession.h"
#include "ThemeManager.h"
#include "ITheme.h"
#include "MainExecutor.h"
#include "IPlayer.h"
#include "OfflineState.h"
#import "CircularIndicator.h"
#import "Debug.h"
#include "NamedImage.h"
#include "SessionManager.h"

using namespace Base;
using namespace Gear;

@implementation SongCell {
    
    SongEntry _songEntry;
    bool _everSet;
    bool _wasPlaying;
    
    SignalConnection _songConnection;
    SignalConnection _playlistConnection;
    shared_ptr<OfflineState> _offlineState;
    SignalConnection _offlineConnection;
    SignalConnection _offlineRatioConnection;
}

@synthesize sourceImage;
@synthesize offlineIndicator;
@synthesize albumLabel;
@synthesize artistLabel;

const CGFloat SourceWidth = 20;

static inline bool playingFromListed()
{
    auto app = IApp::instance();
    auto playing = app->player()->playlistCurrentlyPlaying();
    auto selected = app->selectedPlaylistConnector().value().second;
    bool ret = (playing == selected);
    return ret;
}

- (id)init
{
    self = [super initWithFrame:CGRectMake(0, 0, 320, 56)];
    if (self) {
        self.artistLabel = [[UILabel alloc] initWithFrame:CGRectMake(57,27,102,21)];
        self.artistLabel.backgroundColor = [UIColor clearColor];
        self.artistLabel.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleRightMargin;
        
        self.songLabel = [[UILabel alloc] initWithFrame:CGRectMake(57, 10, self.frame.size.width - 57-43-12, 21)];
        _songLabel.backgroundColor = [UIColor clearColor];
        self.songLabel.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleWidth;
        self.albumLabel = [[UILabel alloc] initWithFrame:CGRectMake(160, 27, 131, 21)];
        self.albumLabel.backgroundColor = [UIColor clearColor];
        self.albumLabel.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleRightMargin;
        self.albumArt = [[GearImageView alloc] initWithFrame:CGRectMake(12, 10, 36, 36)];
        self.albumArt.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleRightMargin;
    
        self.offlineIndicator = [[CircularIndicator alloc] initWithFrame:CGRectZero];
        [self.contentView insertSubview:self.offlineIndicator atIndex:0];

        [self addSubview:self.artistLabel];
        [self addSubview:self.songLabel];
        [self addSubview:self.albumLabel];
        [self addSubview:self.albumArt];
        
        
        albumLabel.translatesAutoresizingMaskIntoConstraints = NO;
        
        //static const CGRect SourceRect = CGRectMake(284-5-2, 20-2, SourceWidth, SourceWidth);
        
        self.sourceImage = [[GearImageView alloc] initWithFrame: CGRectZero];
        self.sourceImage.translatesAutoresizingMaskIntoConstraints = NO;
        NSDictionary *views = NSDictionaryOfVariableBindings(sourceImage,offlineIndicator,albumLabel,artistLabel);
        NSDictionary *metrics = @{@"SourceWidth": @(SourceWidth)};
        
        [self.contentView addSubview:self.sourceImage];
        [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"[artistLabel]-10-[albumLabel]-[sourceImage(==SourceWidth)]-23-|" options:0 metrics:metrics views:views]];
        [self.contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[sourceImage(==SourceWidth)]" options:0 metrics:metrics views:views]];
        [self.contentView addConstraint:[NSLayoutConstraint constraintWithItem:sourceImage attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:self.contentView attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
        
        offlineIndicator.translatesAutoresizingMaskIntoConstraints = NO;
        [self.contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"[offlineIndicator(==26)]" options:0 metrics:nil views:views]];
        [self.contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[offlineIndicator(==26)]" options:0 metrics:nil views:views]];
        [self.contentView addConstraint:[NSLayoutConstraint constraintWithItem:offlineIndicator attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:sourceImage attribute:NSLayoutAttributeCenterX multiplier:1 constant:0]];
        [self.contentView addConstraint:[NSLayoutConstraint constraintWithItem:offlineIndicator attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:sourceImage attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
        
        [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|-27-[albumLabel(==21)]" options:0 metrics:nil views:views]];
    }
    return self;
}

- (NSString *)reuseIdentifier
{
    return @"NativeCell";
}

- (void)setSong:(SongEntry)songEntry
{
    if (_songEntry == songEntry) {
        // same as before (don't empty & redraw album art!)
        return;
    }
    
    // a new song has been set
    _everSet = false;
    
    auto song = songEntry.song();
    if (!song) {
        return;
    }
    _songEntry = songEntry;
    auto changeHandler = [self](const SongEntry &playing){
        // this does not work for the queue, that's for sure:
        //bool isPlaying = (playingFromListed()) && (_songEntry == playing);
        bool isPlaying = (_songEntry == playing);
        
        if (_wasPlaying != isPlaying || !_everSet) {
            auto song = _songEntry.song();
            if (song) {
                auto theme = IApp::instance()->themeManager()->current();
                auto attrib = theme->listTitleText(isPlaying);
                Gui::Writer::apply(attrib, self.songLabel);
                attrib = theme->listSubtitleText(isPlaying);
                Gui::Writer::apply(attrib, self.artistLabel);
                attrib = theme->listSubSubtitleText(isPlaying);
                Gui::Writer::apply(attrib, self.albumLabel);
            }
            
            _everSet = true;
        }
        _wasPlaying = isPlaying;
    
    };
    
    auto player = IApp::instance()->player();
    _songConnection = player->songEntryConnector().connect(changeHandler);
    _playlistConnection = player->playlistCurrentlyPlayingConnector().connect([changeHandler](const shared_ptr<IPlaylist> &){
        changeHandler(IApp::instance()->player()->songEntryConnector().value());
    });

    
    [self.songLabel setText:convertString(song->title())];
    [self.artistLabel setText:convertString(song->artist())];
    [self.albumLabel setText:convertString(song->album())];
    NSString *trimmed =
    [self.artistLabel.text stringByTrimmingCharactersInSet:
     [NSCharacterSet whitespaceAndNewlineCharacterSet]];
    


    
    shared_ptr<Gui::IPaintable> simage = IApp::instance()->sessionManager()->sessionIcon(song);
	
#ifdef RANDOMIZE_SOURCEIMAGES
    static int count = 0;
    ++count;
    switch(count % 4) {
        case 0:
            simage = shared_ptr<NamedImage>(new NamedImage("favicon-gplay"));
            break;
        case 1:
            simage = shared_ptr<NamedImage>(new NamedImage("favicon-youtube"));
            break;
        case 2:
            simage = shared_ptr<NamedImage>(new NamedImage("favicon-onedrive"));
            break;
        case 3:
            simage = shared_ptr<NamedImage>(new NamedImage("favicon-dropbox"));
            break;
    };
#endif
    
    self.sourceImage.image = simage;
    
    //self.songLabel.backgroundColor = [UIColor greenColor];
    
    CGSize artistSize = [trimmed sizeWithFont:self.artistLabel.font];
    CGRect artistFrame = self.artistLabel.frame;
    artistFrame.origin.x = self.songLabel.frame.origin.x;
    if (artistSize.width >= self.songLabel.frame.size.width) {
        artistSize.width = self.songLabel.frame.size.width;
    }
    artistFrame.size.width = artistSize.width;
    self.artistLabel.frame = artistFrame;
    //self.artistLabel.backgroundColor = [UIColor redColor];
    
    /*CGRect frame = self.albumLabel.frame;
    frame.origin.x = self.songLabel.frame.origin.x + artistSize.width + kAlbumGap;
    if (frame.origin.x > self.songLabel.frame.origin.x + self.songLabel.frame.size.width) {
        frame.origin.x = self.songLabel.frame.origin.x + self.songLabel.frame.size.width;
    }
    frame.size.width = self.songLabel.frame.size.width - (artistSize.width + kAlbumGap);
    if (frame.size.width < 0) {
        frame.size.width = 0;
    }
    self.albumLabel.frame = frame;*/
    
    [self.albumArt setPromise:IApp::instance()->albumArtStash()->art(song, self.albumArt.frame.size.width*2)];

    _offlineState = song->offlineState();
    __weak SongCell *wself = self;
    _offlineConnection = _offlineState->offlineConnector().connect([wself,songEntry](bool offline){
        
        SongCell *s = wself;
        if (s && songEntry == s->_songEntry) {
            [s.offlineIndicator setHidden:!offline];
        }
    });
    _offlineRatioConnection = _offlineState->ratioConnector().connect([wself,songEntry](float ratio){
        SongCell *s = wself;
        if (s && songEntry == s->_songEntry) {
            [s.offlineIndicator setRatio:ratio];
        }
    });

	UIView *bgv = [[UIView alloc] init];
    [bgv setFrame:self.bounds];
    auto app = IApp::instance();
    auto current = app->themeManager()->current();
    auto color = current->selectedTextBackground();
    bgv.backgroundColor = Gui::Painter::convertColor(color);
	self.selectedBackgroundView = bgv;
}

- (void)setSeparatorThickness:(int)separatorThickness
{
    CGRect sepBounds = self.separator.bounds;
    sepBounds.size.height = separatorThickness;
    self.separator.bounds = sepBounds;
}



@end
