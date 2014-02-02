// ****************************************************************************************
//
// GameEngine of the University of Augsburg
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
//
// This file is part of the GameEngine developed at the
// Lab for Multimedia Concepts and Applications of the University of Augsburg
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************
//

// ****************************************************************************************
//
// GameEngine Core Library of the University of Augsburg
// ---------------------------------------------------------
// Copyright (C) 2007 Volker Wiendl
//
// ****************************************************************************************

#ifndef GAMEENGINE_CONFIG_H_
#define GAMEENGINE_CONFIG_H_

// Detect platform
#if defined( WINCE )
#	if !defined( PLATFORM_WIN_CE )
#		define PLATFORM_WIN_CE
#	endif
#elif defined( WIN32 ) || defined( _WINDOWS ) || defined( _WIN32 )
#	if !defined( PLATFORM_WIN )
#		define PLATFORM_WIN
#	endif
#elif defined( __APPLE__ ) || defined( __APPLE_CC__ )
#include <TargetConditionals.h>
#	if TARGET_OS_IPHONE
#	  if !defined( PLATFORM_IOS )
#		   define PLATFORM_IOS
#		endif
#	else
#	  if !defined( PLATFORM_MAC )
#		   define PLATFORM_MAC
#		endif
#	endif
#elif defined(__ANDROID__)
#   if !defined( PLATFORM_ANDROID )
#      define PLATFORM_ANDROID
#   endif
#else
#	if !defined( PLATFORM_LINUX )
#		define PLATFORM_LINUX
#	endif
#endif


#if defined(PLATFORM_WIN)
#    include <windows.h>
#	 ifdef GAMEENGINE_EXPORTS
#       define DLLEXP __declspec( dllexport )
#		define GAMEENGINE_API __declspec(dllexport)
#	 else
#       define DLLEXP __declspec( dllimport )
#		define GAMEENGINE_API __declspec(dllimport)
#    endif
#else
#	define DLLEXP
#	define GAMEENGINE_API
#	include <cstddef>
#	include <cstring>
#endif




#if !defined( PLATFORM_WIN ) && !defined( PLATFORM_WIN_CE )
#   define localtime_s localtime_r
#	define _stricmp strcasecmp
#	define _mkdir( name ) mkdir( name, 0755 )
#endif

#if !defined( _MSC_VER ) || (defined( _MSC_VER ) && (_MSC_VER < 1400))
#	define strcpy_s( dst, count, src) strncpy( dst, src, count )
#	define strncpy_s( dst, dstSize, src, count ) strncpy( dst, src, count < dstSize ? count : dstSize )
#	define sprintf_s( buf, size, format, ... ) snprintf( buf, size, format, ... )
#endif
#if defined( _MSC_VER ) && (_MSC_VER < 1400)
#   define vsnprintf _vsnprintf
#endif

#endif
