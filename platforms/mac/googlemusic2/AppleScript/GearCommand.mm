//
//  GearCommand.m
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 30/03/14.
//
//

#import "GearCommand.h"
#include "IApp.h"
#include "IPlayer.h"
#import "ScriptPlaylist.h"
#include "ISession.h"
#import "Bridge.h"
#import "ScriptSong.h"
#import "ScriptSession.h"
#include "PlaylistSession.h"
#include "SessionManager.h"
#include "StringUtility.h"

@implementation GearCommand

using Gear::IApp;
using Gear::IPlaylist;
using Gear::convertString;
using Gear::PlaylistSession;
using Gear::SongEntry;

- (id)performDefaultImplementation
{
    Bridge *bridge = [Bridge sharedBridge];
    NSDictionary *arguments = [self arguments];
    
    switch([[self commandDescription] appleEventCode]) {
        // play suite
        case 'play':
            if (!IApp::instance()->player()->playingConnector().value()) {
                IApp::instance()->player()->play();
            }
            break;
        case 'paus':
            if (IApp::instance()->player()->playingConnector().value()) {
                IApp::instance()->player()->play();
            }
            break;
        case 'togg':
            IApp::instance()->player()->play();
            break;
        case 'next':
            IApp::instance()->player()->next();
            break;
        case 'prev':
            IApp::instance()->player()->prev();
            break;
            
        // manage suite
        case 'goog': {
            auto session = IApp::instance()->sessionManager()->session("gplay");
            auto playlistSession = MEMORY_NS::dynamic_pointer_cast<PlaylistSession>(session);
            if (!playlistSession) {
                return nil;
            }
            return [bridge registerObject:[[ScriptSession alloc] initWithSession:playlistSession]];
        }
        case 'srch': {
            ScriptSession *session = [[Bridge sharedBridge] objectFor:[arguments objectForKey:@"on"]];
            if (![session isKindOfClass:[ScriptSession class]]) {
                return nil;
            }
            [self suspendExecution];
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0), ^{
                SongEntry entry = [session searchFor:[arguments objectForKey:@"title"] artist:[arguments objectForKey:@"artist"] album:[arguments objectForKey:@"album"]];
                
                NSString *logString1 = [NSString stringWithFormat:@"AS Searching for %@/%@/%@...",[arguments objectForKey:@"artist"], [arguments objectForKey:@"album"], [arguments objectForKey:@"title"]];
                NSString *logString2 = @"";
                if (entry.song()) {
                    logString2 = [NSString stringWithFormat:@"Found %@/%@/%@", convertString(entry.song()->artist()), convertString(entry.song()->album()), convertString(entry.song()->title())];
                } else {
                    logString2 = @"Not found";
                }
                NSLog(@"%@ %@", logString1, logString2);
                
                [self resumeExecutionWithResult:[bridge registerObject:[[ScriptSong alloc] initWithSong:entry]]];
            });
            return nil;
        }
        case 'plis': {
            ScriptSession *session = [bridge objectFor:[arguments objectForKey:@""]];
            if (![session isKindOfClass:[ScriptSession class]]) {
                return nil;
            }
            [self suspendExecution];
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0), ^{
                shared_ptr<IPlaylist> playlist = [session playlistNamed:[arguments objectForKey:@"named"]];
                [self resumeExecutionWithResult:[bridge registerObject:[[ScriptPlaylist alloc] initWithPlaylist:playlist]]];
            });
            return nil;
        }
        case 'adds': {
            // 'any' => NSAppleEventDescriptor
            ScriptSong *song = [bridge objectFor:[[arguments objectForKey:@""] stringValue]];
            if (![song isKindOfClass:[ScriptSong class]]) {
                return nil;
            }
            ScriptPlaylist *playlist = [bridge objectFor:[arguments objectForKey:@"toPlaylist"]];
            if (![playlist isKindOfClass:[ScriptPlaylist class]]) {
                return nil;
            }
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0), ^{
                [playlist addSong:song];
            });
            return nil;
        }
    }
    return nil;
}

@end
