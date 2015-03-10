//
//  Debug.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef G_Ear_iOS_Debug_h
#define G_Ear_iOS_Debug_h

//#define DEBUG_DONT_CLEAR_LOGIN_COOKIES
//#define DEBUG_DISABLE_PLAYBACK

// this macro is for creating screenshots to use as a splash screen
//#define SPLASH_SCREEN
//#define STATUSBAR_HIDDEN
//#define PINCH_IS_HELP
//#define RANDOMIZE_SOURCEIMAGES

#ifdef SPLASH_SCREEN
#define STATUSBAR_HIDDEN
#endif

#ifdef DEBUG


#define PLDEBUG 1 // playlist
#define SLDEBUG 2 // songlist
#define SVDEBUG 4 // songsview
#define PVDEBUG 8 // playview
#define LSDEBUG 16 // lockscreen
#define TBDEBUG 32 // tabbar

#define DEBUGLEVEL 0
//#define DEBUGLEVEL (SLDEBUG | SVDEBUG)
//#define DEBUGLEVEL TBDEBUG
//#define DEBUGLEVEL SVDEBUG
//#define DEBUGLEVEL (PLDEBUG | SVDEBUG)
//#define DEBUGLEVEL (PLDEBUG | SLDEBUG | SVDEBUG | PVDEBUG | LSDEBUG | TBDEBUG)

#define PLNSLOG(...) do {if (DEBUGLEVEL & PLDEBUG) NSLog(@"Playlist: " __VA_ARGS__);} while (0)
#define SLNSLOG(...) do {if (DEBUGLEVEL & SLDEBUG) NSLog(@"Songlist: " __VA_ARGS__);} while (0)
#define SVNSLOG(...) do {if (DEBUGLEVEL & SVDEBUG) NSLog(@"Songsview: " __VA_ARGS__);} while (0)
#define PVNSLOG(...) do {if (DEBUGLEVEL & PVDEBUG) NSLog(@"Playview: " __VA_ARGS__);} while (0)
#define LSNSLOG(...) do {if (DEBUGLEVEL & LSDEBUG) NSLog(@"Lockscreen: " __VA_ARGS__);} while (0)
#define TBNSLOG(...) do {if (DEBUGLEVEL & TBDEBUG) NSLog(@"Tabbar: " __VA_ARGS__);} while (0)


#else


#define PLNSLOG(...)
#define SLNSLOG(...)
#define SVNSLOG(...)
#define PVNSLOG(...)
#define LSNSLOG(...)
#define TBNSLOG(...)


#endif

#endif
