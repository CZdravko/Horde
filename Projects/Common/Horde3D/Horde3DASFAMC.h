// *************************************************************************************************
//
// Horde3D Terrain Extension
// --------------------------------------------------------
// Copyright (C) 2006-2009 Nicolas Schulz and Volker Wiendl
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

/*	Title: Horde3D Terrain Extension */

#pragma once

#include "Horde3D.h"

#ifndef DLL
#	if defined( WIN32 ) || defined( _WINDOWS )
#		define DLL extern "C" __declspec( dllimport )
#	else
#		if defined( __GNUC__ ) && __GNUC__ >= 4
#		  define DLLEXP extern "C" __attribute__ ((visibility("default")))
#   	else
#		  define DLLEXP extern "C"
#   	endif
#	endif
#endif


/*	Topic: Introduction
		Some words about the Terrain Extension.
	
	The Terrain Extension extends Horde3D with the capability to render large landscapes. A special
	level of detail algorithm adapts the resolution of the terrain mesh so that near regions get
	more details than remote ones. The algorithm also considers the geometric complexity of the
	terrain to increase the resoultion solely where this is really required. This makes the
	rendering fast and provides a high quality with a minimum of popping artifacts.
	
	A height map is used to define the altitude of the terrain. The height map is a usual texture
	map that encodes 16 bit height information in two channels. The red channel of the texture
	contains the coarse height, while the green channel encodes finer graduations. The encoding of
	the information is usually done with an appropriate tool. If you just want to use 8 bit
	height information, you can simply copy the greyscale image to the red channel of the height
	map and leave the green channel black.

	The extension defines the uniform *terBlockParams* and the attribute *terHeight* that can be used
	in a shader to render the terrain. To see how this is working in detail, have a look at the included
	sample shader.
*/


/*
	Constants: Predefined constants
	H3DEXT_NodeType_Terrain  - Type identifier of Terrain scene node
*/
const int H3DEXT_AMC_resType = 150;
const int H3DEXT_ASF_resType = 151;



