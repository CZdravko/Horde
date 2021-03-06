[[FX]]

// Samplers
sampler2D depthBuf = sampler_state
{
	Address = Clamp;
};

sampler2D gbuf0 = sampler_state
{
	Address = Clamp;
};

sampler2D gbuf1 = sampler_state
{
	Address = Clamp;
};

sampler2D gbuf2 = sampler_state
{
	Address = Clamp;
};

sampler2D gbuf3 = sampler_state
{
	Address = Clamp;
};

samplerCube ambientMap = sampler_state
{
	Address = Clamp;
	Filter = Bilinear;
	MaxAnisotropy = 1;
};

// Contexts
context AMBIENT
{
	VertexShader = compile GLSL VS_FSQUAD;
	PixelShader = compile GLSL FS_AMBIENT;
	
	ZWriteEnable = false;
	BlendMode = Replace;
}

context LIGHTING
{
	VertexShader = compile GLSL VS_VOLUME;
	PixelShader = compile GLSL FS_LIGHTING;
	
	ZWriteEnable = false;
	BlendMode = Add;
}

context COPY_DEPTH
{
	VertexShader = compile GLSL VS_FSQUAD;
	PixelShader = compile GLSL FS_COPY_DEPTH;
}


[[VS_FSQUAD]]
precision mediump float;
uniform mat4 projMat;
attribute vec3 vertPos;
varying vec2 texCoords;
				
void main( void )
{
	texCoords = vertPos.xy; 
	gl_Position = projMat * vec4( vertPos, 1 );
}


[[VS_VOLUME]]
precision mediump float;
uniform mediump  mat4 viewProjMat;
uniform mediump  mat4 worldMat;
attribute mediump  vec3 vertPos;
varying mediump  vec4 vpos;
				
void main( void )
{
	vpos = viewProjMat * worldMat * vec4( vertPos, 1 );
	gl_Position = vpos;
}


[[FS_AMBIENT]]
precision mediump float;
#include "shaders/gles2/utilityLib/fragDeferredRead.glsl"

uniform samplerCube ambientMap;
varying vec2 texCoords;

void main( void )
{
	if( getMatID( texCoords ) == 0.0 )	// Background
	{
		gl_FragColor.rgb = vec3( 0, 0, 0 );
	}
	else if( getMatID( texCoords ) == 2.0 )	// Sky
	{
		gl_FragColor.rgb = getAlbedo( texCoords );
	}
	else
	{
		gl_FragColor.rgb = getAlbedo( texCoords ) * textureCube( ambientMap, getNormal( texCoords ) ).rgb;
	}
}


[[FS_LIGHTING]]
precision mediump float;
#include "shaders/gles2/utilityLib/fragLighting.glsl"
#include "shaders/gles2/utilityLib/fragDeferredRead.glsl"

uniform mediump mat4 viewMat;
varying mediump vec4 vpos;

void main( void )
{
	 mediump vec2 fragCoord = (vpos.xy / vpos.w) * 0.5 + 0.5;
	
	if( getMatID( fragCoord ) == 1.0 )	// Standard phong material
	{
		mediump vec3 pos = getPos( fragCoord ) + viewerPos;
		mediump float vsPos = (viewMat * vec4( pos, 1.0 )).z;
		mediump vec4 specParams = getSpecParams( fragCoord );
		
		gl_FragColor.rgb =
			calcPhongSpotLight( pos, getNormal( fragCoord ),
								getAlbedo( fragCoord ), specParams.rgb, specParams.a, -vsPos, 0.3 );
	}
	else discard;
}


[[FS_COPY_DEPTH]]
precision mediump float;
uniform sampler2D depthBuf;
varying vec2 texCoords;

void main( void )
{
	gl_FragDepth = texture2D( depthBuf, texCoords ).r;
}
