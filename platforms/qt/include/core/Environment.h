//
//  Environment.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/29/13.
//
//

#ifndef G_Ear_Environment_h
#define G_Ear_Environment_h

#ifndef __APPLE__
#undef NON_APPLE
#define NON_APPLE
#endif

// need this on vs2012
#if defined(_MSVC_VER) && _MSVC_VER < 1800
#define final
#endif

#ifdef BUILDING_CORE_DLL
	#define CORE_IS_DLL
#endif

#ifdef _WIN32
typedef int ssize_t;
	#ifdef CORE_IS_DLL
		#ifdef BUILDING_CORE_DLL
			#define core_export __declspec(dllexport)
		#else
			#define core_export __declspec(dllimport)
		#endif
	#else
		#define core_export
	#endif
#else
	#define core_export
#endif

#ifndef NON_APPLE
#import "TargetConditionals.h"
#else

#ifdef TIZEN
#define nullptr NULL
#define BOOST_NO_DEFAULTED_FUNCTIONS
#define NO_FRIEND_TEMPLATE
#define NO_EXCEPTION_PTR
#endif

#endif

#ifdef ANDROID
//#include "boost/config.hpp"
//#undef BOOST_NO_CXX11_SMART_PTR
#define NO_EXCEPTION_PTR
#endif

#ifdef TARGET_OS_IPHONE
#if !TARGET_OS_IPHONE
#ifndef NON_APPLE
//#define USE_BOOST_INSTEAD_OF_CPP11
#define TARGET_OS_MACDESKTOP 1
#endif
#endif
#endif

#ifdef NON_APPLE
#undef TARGET_OS_IPHONE
#endif

#ifndef NON_APPLE
#if TARGET_IPHONE_SIMULATOR
#define DISABLE_DECODERAV
#endif
#endif

#endif
