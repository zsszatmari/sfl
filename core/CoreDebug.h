//
//  CoreDebug.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/18/13.
//
//

#ifndef G_Ear_Player_CoreDebug_h
#define G_Ear_Player_CoreDebug_h

#include "Environment.h"

//#define DEBUG_FORCE_GENERIC_HTTP
//#define DEBUG_DISABLE_REMOTEFETCHSONGS
//#define DEBUG_DISABLE_REMOTEFETCHPLAYLISTS
//#define DEBUG_DISABLE_AUTOCONNECT
//#define DEBUG_DISABLE_UNICODE_COMPARE
//#define DEBUG_DISABLE_LASTFMART
//#define DEBUG_TEST_SONGCREATED
#if TARGET_OS_MACDESKTOP
//#define DEBUG_DONTLOGOUT
#else
//#define DEBUG_DISABLE_GROUPINGS
#endif

#define DEBUG_INAPP_ENABLE_EVERYTHING

#ifdef TARGET_OS_MACDESKTOP
//#define DISABLE_YOUTUBE
//#define DISABLE_OFFLINE_ON_DESKTOP
#endif



#endif
