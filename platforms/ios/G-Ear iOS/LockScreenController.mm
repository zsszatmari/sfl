//
//  LockScreenController.m
//  G-Ear iOS
//
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "LockScreenController.h"
#import <MediaPlayer/MPNowPlayingInfoCenter.h>
#import <MediaPlayer/MPMediaItem.h>
#include <stdio.h>

#include "IApp.h"
#include "IPlayer.h"
#include "SignalConnection.h"
#include "StringUtility.h"
#include "Painter.h"
#include "PromisedImage.h"
#include "BitmapImage.h"
#include "Debug.h"

#define ALBUM_ART_SIZE 264.
#define RATIO_SYNC_INTERVAL 15

//enum {AP_PLAYING, AP_STOPPED, AP_PAUSED};

using namespace Gear;
using namespace Gui;

@interface LockScreenController (){
    SignalConnection songEntryConnection;
    SignalConnection elapsedConnection;
    SignalConnection remainingConnection;
    SignalConnection ratioConnection;
    SignalConnection playingConnection;
    SignalConnection repeatConnection;
    SignalConnection shuffleConnection;
    NSMutableDictionary *songInfo;
    // artwork must be handled separately because of rare crashes. take care
    UIImage *songArtwork;

    SignalConnection imageConnection;
    shared_ptr<PromisedImage> imagePromise;
	int ratio_synced;
	float prev_ratio;
//	int player_status;
	BOOL is_playing;
}
@end

@implementation LockScreenController

- (id)init
{
    self = [super init];
    
    if (self) {
        songInfo = [[NSMutableDictionary alloc] init];
        [self connect];
    }
    
    return self;
}


// get elapsed time, it may be false at song switch while the buffer lasts
- (float) get_elapsed_time: (float) ratio {
	if (ratio < 0.)
		ratio = IApp::instance()->player()->songRatioConnector().value();
	SongEntry entry = IApp::instance()->player()->songEntryConnector().value();
	return (entry) ? (entry.song() -> durationMillis() / 1000.) * ratio : -1.;
}

static void scheduleImageUpdate(std::function<bool()> draw)
{
    double delayInSeconds = 1.0;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        if (!draw()) {
            scheduleImageUpdate(draw);
        }
    });
}

- (void)connect
{
    auto player = IApp::instance()->player();

    ratioConnection = player->songRatioConnector().connect([self](const float ratio){
        float elapsed_secs = [self get_elapsed_time: ratio];

        // a counter at song start would be a better idea to sync with ?
        // we can't read the lockscreen autocounter, but can be faked with our own counter
        if (is_playing) {
            // ratio is synced at song start, exact 0.0 might be false (???)
            if (elapsed_secs > .0 && elapsed_secs < 1. && ratio_synced <= 0) {
                LSNSLOG(@"ratio connector synced: %f ", elapsed_secs);
                [songInfo setObject: @(elapsed_secs) forKey: MPNowPlayingInfoPropertyElapsedPlaybackTime];
                [songInfo setObject: @1. forKey: MPNowPlayingInfoPropertyPlaybackRate];
                ratio_synced = 1;
                [self updateInfo];
            }
            // buffer is playing after song switch
            if (! ratio_synced) {
                LSNSLOG(@"ratio connector not synced: %f ", elapsed_secs);
                [songInfo setObject: @.0 forKey: MPNowPlayingInfoPropertyPlaybackRate];
                ratio_synced--;
                [self updateInfo];
            }
            // just to make sure, ratio is synced sometimes
    //		if (((int) elapsed_secs == 15 && ratio_synced == 1) || ((int) elapsed_secs == 30 && ratio_synced == 2)) {
            int elapsed_int = (int) rintf(elapsed_secs);
            if (! (elapsed_int % RATIO_SYNC_INTERVAL) && (ratio_synced == elapsed_int / RATIO_SYNC_INTERVAL) && ratio_synced >= 1) {
                LSNSLOG(@"ratio connector synced at: %f ", elapsed_secs);
                [songInfo setObject: @(elapsed_secs) forKey: MPNowPlayingInfoPropertyElapsedPlaybackTime];
                ratio_synced++;
                [self updateInfo];
            }
        }
    ////	is_playing = playing_;
        prev_ratio = ratio;
    });

    songEntryConnection = player->songEntryConnector().connect([self](const SongEntry &songEntry) {
        LSNSLOG(@"songentry connector");

        ratio_synced = 0;
        auto song = songEntry.song();
        if (song) {

           // on iphone 5 I am fairly sure that the album art spans the entire width of the device (320), x2 for retina
            static const float coverSize = 640;

            imagePromise = IApp::instance()->player()->albumArt(coverSize);
            

            imageConnection = imagePromise->connector().connect([self](const shared_ptr<Gui::IPaintable> &paintable) {
                
                auto draw = [self]{
                    
                    // don't retain the previous image, it might have changed
                    auto paintable = imagePromise->image();
                    
                    //LSNSLOG(@"image connector %x", (unsigned int)bitmap.get());
                    
                    UIImage *image = Painter::convertImage(paintable, CGSizeMake(coverSize, coverSize));
                    
                    
                    if (image) {
                        //LSNSLOG(@"image converted %@ %@ %@", NSStringFromCGSize(image.size), NSStringFromCGRect(artw.bounds), NSStringFromCGRect(artw.imageCropRect));
                        songArtwork = image;
                        
                    } else {
                        LSNSLOG(@"image not yet converted, retry later");
                        songArtwork = nil;
                        // retry at a later moment
                    }
                    [songInfo setObject: @(ratio_synced >= 1 ? [self get_elapsed_time: -1.] : .0) forKey: MPNowPlayingInfoPropertyElapsedPlaybackTime];
                    [self updateInfo];
                    
                    return image != nil;
                };
            
                if (!draw()) {
                    scheduleImageUpdate(draw);
                }
            });

            [songInfo setObject:convertString(song->title()) forKey:MPMediaItemPropertyTitle];
            [songInfo setObject:convertString(song->artist()) forKey:MPMediaItemPropertyArtist];
            [songInfo setObject:convertString(song->album()) forKey:MPMediaItemPropertyAlbumTitle];
            [songInfo setObject:convertString(song->albumArtist()) forKey:MPMediaItemPropertyAlbumArtist];
//           [songInfo setObject:convertString(song->albumArtUrl()) forKey: MPMediaItemPropertyArtwork];
            [songInfo setObject:@(song->track()) forKey:MPMediaItemPropertyAlbumTrackNumber];
            [songInfo setObject:@(song->durationMillis() / 1000) forKey:MPMediaItemPropertyPlaybackDuration];
            [songInfo setObject:@(song->playCount()) forKey:MPMediaItemPropertyPlayCount];
            [songInfo setObject:@(song->lastPlayed()) forKey:MPMediaItemPropertyLastPlayedDate];
            [songInfo setObject:@(song->rating()) forKey:MPMediaItemPropertyRating];
            [songInfo setObject: @.0 forKey: MPNowPlayingInfoPropertyElapsedPlaybackTime];
            [songInfo setObject: @1. forKey: MPNowPlayingInfoPropertyPlaybackRate];

        }
    
        [self updateInfo];
    });

    playingConnection = player->playingConnector().connect([self](const bool playing) {
        LSNSLOG(@"playing connector");

        [songInfo setObject: @(ratio_synced ? [self get_elapsed_time: -1.] : .0) forKey: MPNowPlayingInfoPropertyElapsedPlaybackTime];
        [songInfo setObject: @(playing) forKey: MPNowPlayingInfoPropertyPlaybackRate];

        is_playing = playing;

        [self updateInfo];
    });

}

- (void)updateInfo
{
    // should only set (and more importantly, discard!) MPMediaItemPropertyArtwork in a controlled manner
    NSMutableDictionary *dictionary = [songInfo mutableCopy];
    if (songArtwork) {
        [dictionary setObject:[[MPMediaItemArtwork alloc] initWithImage:songArtwork] forKey:MPMediaItemPropertyArtwork];
    }
    
    [[MPNowPlayingInfoCenter defaultCenter] setNowPlayingInfo:dictionary];
}
@end
