// *************************************************************************************************
// Horde3D Shader Utility Library
// --------------------------------------
//		- Deferred shading functions -
//
// Copyright (C) 2006-2011 Nicolas Schulz
//
// You may use the following code in projects based on the Horde3D graphics engine.
//
// *************************************************************************************************

void setMatID( const mediump float id ) { gl_FragData[0].a = id; }
void setPos( const mediump vec3 pos ) { gl_FragData[0].rgb = pos; }
void setNormal( const mediump vec3 normal ) { gl_FragData[1].rgb = normal; }
void setAlbedo( const mediump vec3 albedo ) { gl_FragData[2].rgb = albedo; }
void setSpecParams( const mediump vec3 specCol, const mediump float gloss ) { gl_FragData[3].rgb = specCol; gl_FragData[3].a = gloss; }