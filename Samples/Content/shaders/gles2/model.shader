[[FX]]

// Supported Flags
/* ---------------
	_F01_Skinning
	_F02_NormalMapping
	_F03_ParallaxMapping
	_F04_EnvMapping
	_F05_AlphaTest
*/


// Samplers
sampler2D albedoMap = sampler_state
{
	Texture = "textures/common/white.tga";
};

sampler2D normalMap = sampler_state
{
	Texture = "textures/common/defnorm.tga";
};

sampler2D lightColorMap = sampler_state
{
	Texture = "textures/common/white.tga";
};

sampler2D lightRrMap = sampler_state
{
	Texture = "textures/common/white.tga";
};

sampler2D tileMap = sampler_state
{
	Texture = "textures/common/white.tga";
};

samplerCube ambientMap = sampler_state
{
	Address = Clamp;
	Filter = Bilinear;
	MaxAnisotropy = 1;
};

samplerCube envMap = sampler_state
{
	Address = Clamp;
	Filter = Bilinear;
	MaxAnisotropy = 1;
};

// Uniforms
float4 matDiffuseCol <
	string desc_abc = "abc: diffuse color";
	string desc_d   = "d: alpha for opacity";
> = {1.0, 1.0, 1.0, 1.0};

float4 matSpecParams <
	string desc_abc = "abc: specular color";
	string desc_d   = "d: gloss";
> = {0.04, 0.04, 0.04, 0.5};

// Contexts
// context ATTRIBPASS
// {
//	VertexShader = compile GLSL VS_GENERAL;
//	PixelShader = compile GLSL FS_ATTRIBPASS;
//}

context SHADOWMAP
{
	VertexShader = compile GLSL VS_SHADOWMAP;
	PixelShader = compile GLSL FS_SHADOWMAP;
}

context LIGHTING
{
	VertexShader = compile GLSL VS_GENERAL;
	PixelShader = compile GLSL FS_LIGHTING2;
	
	ZWriteEnable = false;
	BlendMode = Add;
}

context LIGHTING2
{
	VertexShader = compile GLSL VS_VOLUME;
	PixelShader = compile GLSL FS_VOLUME;
	
	ZWriteEnable = false;
	BlendMode = Add;
}

context AMBIENT
{
	VertexShader = compile GLSL VS_GENERAL;
	PixelShader = compile GLSL FS_AMBIENT;
}

context LIGHT_CLUSTER
{
	VertexShader = compile GLSL VS_FSQUAD;
	PixelShader = compile GLSL FS_COPY_DEPTH;
}

[[VS_FSQUAD]]
precision mediump float;
uniform mat4 projMat;
attribute vec3 vertPos;
varying vec2 texCoords;
varying vec4 vpPos;
				
void main( void )
{
	texCoords = vertPos.xy; 
	vpPos = projMat * vec4( vertPos, 1 );
	gl_Position = projMat * vec4( vertPos, 1 );
}

[[FS_COPY_DEPTH]]
precision mediump float;
uniform sampler2D tileMap;
uniform sampler2D albedoMap;
varying vec2 texCoords;
varying vec4 vpPos;

void main( void )
{
	float x = floor(vpPos.x * 32.0 + 32.0);
	float y = floor(vpPos.y * 32.0 + 32.0);
	float z = floor(y/16.0) * 4.0 + floor(x/16.0);
	x = floor(fract(x/16.0) * 16.0);
	y = floor(fract(y/16.0) * 16.0);
	
	
	
	float Xtc = (16.0 * x)/256.0;
	float Ytc = (16.0 * z + y)/256.0;
	vec4 numLightColor = texture2D(tileMap,  vec2(0.5/256.0 + Xtc, 0.5/256.0 + Ytc));//vec2(0.5/256.0 + Xtc, 0.5/256.0 + Ytc) vpPos.xy*0.5 + vec2(0.5,0.5)
	float numLights =  numLightColor.r * 256.0;
	
	
	
	if(numLights > 0.0){
		gl_FragColor.rgb = vec3(1.0,0.0,(16.0*y+x)/256.0);
	}else{
		gl_FragColor.rgb = vec3(0.0,1.0,(16.0*y+x)/256.0);
	}
	
	//gl_FragColor.r = numLightColor.r * 50.0; //vec3(x/16.0,y/16.0,z/16.0);
	//gl_FragColor.g = y/16.0;
	
}

[[VS_VOLUME]]

uniform mat4 viewProjMat;
uniform mat4 worldMat;
attribute vec3 vertPos;
varying vec4 vpos;
				
void main( void )
{
	vpos = viewProjMat * worldMat * vec4( vertPos, 1 );
	gl_Position = vpos;
}


[[FS_VOLUME]]
precision mediump float;
varying vec4 vpos;
				
void main( void )
{
	gl_FragColor.rgb = vpos.rgb; //vec3(255.0, 153.0, 51.0);
}


[[VS_GENERAL]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/gles2/utilityLib/vertCommon.glsl"

#ifdef _F01_Skinning
	#include "shaders/gles2/utilityLib/vertSkinning.glsl"
#endif

uniform mat4 viewProjMat;
uniform mediump vec3 viewerPos;
attribute vec3 vertPos;
attribute vec2 texCoords0;
attribute vec3 normal;

#ifdef _F02_NormalMapping
	attribute vec4 tangent;
#endif

varying vec4 pos, vsPos, vpPos;
varying vec2 texCoords;

#ifdef _F02_NormalMapping
	varying mat3 tsbMat;
#else
	varying vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	varying vec3 eyeTS;
#endif


void main( void )
{
#ifdef _F01_Skinning
	mat4 skinningMat = calcSkinningMat();
	mat3 skinningMatVec = getSkinningMatVec( skinningMat );
#endif
	
	// Calculate normal
#ifdef _F01_Skinning
	vec3 _normal = normalize( calcWorldVec( skinVec( normal, skinningMatVec ) ) );
#else
	vec3 _normal = normalize( calcWorldVec( normal ) );
#endif

	// Calculate tangent and bitangent
#ifdef _F02_NormalMapping
	#ifdef _F01_Skinning
		vec3 _tangent = normalize( calcWorldVec( skinVec( tangent.xyz, skinningMatVec ) ) );
	#else
		vec3 _tangent = normalize( calcWorldVec( tangent.xyz ) );
	#endif
	
	vec3 _bitangent = cross( _normal, _tangent ) * tangent.w;
	tsbMat = calcTanToWorldMat( _tangent, _bitangent, _normal );
#else
	tsbNormal = _normal;
#endif

	// Calculate world space position
#ifdef _F01_Skinning	
	pos = calcWorldPos( skinPos( vec4( vertPos, 1.0 ), skinningMat ) );
#else
	pos = calcWorldPos( vec4( vertPos, 1.0 ) );
#endif

	vsPos = calcViewPos( pos );

	// Calculate tangent space eye vector
#ifdef _F03_ParallaxMapping
	eyeTS = calcTanVec( viewerPos - pos.xyz, _tangent, _bitangent, _normal );
#endif
	
	// Calculate texture coordinates and clip space position
	texCoords = texCoords0;
	vpPos = viewProjMat * pos;
	gl_Position = vpPos;
	//vpPos.x /= vpPos.w;
	//vpPos.y /= vpPos.w;
}

[[FS_LIGHTING2]]
// =================================================================================================
precision mediump float;
uniform mediump	vec3 viewerPos;
uniform highp mat4 viewProjMat;//added
uniform mediump vec4 matDiffuseCol;
uniform mediump vec4 matSpecParams;
uniform sampler2D albedoMap;
uniform sampler2D tileMap;


varying mediump vec4 pos, vsPos, vpPos;
varying mediump vec2 texCoords;


varying mediump vec3 tsbNormal;


void main( void )
{
	mediump vec3 newCoords = vec3( texCoords, 0 );
	
	// Flip texture vertically to match the GL coordinate system
	newCoords.t *= -1.0;

	mediump vec4 albedo = texture2D( albedoMap, newCoords.st ) * matDiffuseCol;
	
	mediump vec3 normal = tsbNormal;

	mediump vec3 newPos = (viewProjMat * vec4(pos.xyz, 1.0)).xyz;
	//LOOKUP TEXTURE AS ARRAY
	//We have a 256 x 256 texture (
	
	//Calculate z index
	mediump float logZ = log2( -vsPos.z / 5.0 );
	logZ = max(logZ, 0.0);
	mediump float z = floor(logZ/log2(1.36)) + 1.0;
	//Calculate x and y indexes
	mediump float x = floor(vpPos.x/vpPos.w * 8.0  + 8.0);
	mediump float y = floor(vpPos.y/vpPos.w * 8.0  + 8.0);
	
	
	//16(z) x 16(y) x 16(x) x 16(lights)
	//Texture is 256x256 so 1 row means 16 Xes times 16 Lights
	// Y = 16xZ + Y
	// X = 16xX
	float Xtc = (16.0 * x)/256.0;
	float Ytc = (16.0 * z + y)/256.0;
	vec4 numLightColor = texture2D(tileMap, vec2(Xtc, Ytc) );
	float numLights =  numLightColor.r * 256.0;
	
	float clusterIndex = 256.0*z + 16.0*y + x;
	
	mediump int m3 = int(floor(fract(clusterIndex/3.0) * 3.0));
	mediump vec3 fragC;
	if(int(numLights) == 0){
		fragC = vec3(1.0,0.0,0.0);
	}else if(m3 == 1){
		fragC = vec3(0.0,1.0,0.0);
	}else if(m3 == 2){
		fragC = vec3(0.0,0.0,1.0);
	}else{
	fragC = vec3(1.0,1.0,1.0);
	}
	
	if(numLights > 0.0){
	gl_FragColor.rgb = vec3(1.0,0.0,0.0);
	}else{
	gl_FragColor.rgb = vec3(0.0,1.0,0.0);
	}
	
	
	
	
	//gl_FragColor.rgb = fragC; //vec3(clusterIndex/4096.0, 0.0, 0.0);// fragC; //numLightColor.rga; //vec3(Xtc,Ytc,0.0); //numLightColor; //fragC;
		//calcPhongSpotLight( newPos, normalize( normal ), albedo.rgb, matSpecParams.rgb,
		//                  matSpecParams.a, -vsPos.z, 0.3 );
}



[[FS_ATTRIBPASS]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/gles2/utilityLib/fragDeferredWrite.glsl" 

uniform mediump vec3 viewerPos;
uniform mediump vec4 matDiffuseCol;
uniform mediump vec4 matSpecParams;
uniform sampler2D albedoMap;

#ifdef _F02_NormalMapping
	uniform sampler2D normalMap;
#endif

varying mediump vec4 pos;
varying mediump vec2 texCoords;

#ifdef _F02_NormalMapping
	varying mediump mat3 tsbMat;
#else
	varying mediump vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	varying mediump vec3 eyeTS;
#endif

void main( void )
{
	mediump vec3 newCoords = mediump vec3( texCoords, 0 );
	
#ifdef _F03_ParallaxMapping	
	const mediump float plxScale = 0.03;
	const mediump float plxBias = -0.015;
	
	// Iterative parallax mapping
	mediump vec3 eye = normalize( eyeTS );
	for( int i = 0; i < 4; ++i )
	{
		mediump vec4 nmap = texture2D( normalMap, newCoords.st * vec2( 1, -1 ) );
		mediump float height = nmap.a * plxScale + plxBias;
		newCoords += (height - newCoords.p) * nmap.z * eye;
	}
#endif

	// Flip texture vertically to match the GL coordinate system
	newCoords.t *= -1.0;

	mediump vec4 albedo = texture2D( albedoMap, newCoords.st ) * matDiffuseCol;
	
#ifdef _F05_AlphaTest
	if( albedo.a < 0.01 ) discard;
#endif
	
#ifdef _F02_NormalMapping
	mediump vec3 normalMap = texture2D( normalMap, newCoords.st ).rgb * 2.0 - 1.0;
	mediump vec3 normal = tsbMat * normalMap;
#else
	mediump vec3 normal = tsbNormal;
#endif

	mediump vec3 newPos = pos.xyz;

#ifdef _F03_ParallaxMapping
	newPos += mediump vec3( 0.0, newCoords.p, 0.0 );
#endif
	
	setMatID( 1.0 );
	setPos( newPos - viewerPos );
	setNormal( normalize( normal ) );
	setAlbedo( albedo.rgb );
	setSpecParams( matSpecParams.rgb, matSpecParams.a );
}

	
[[VS_SHADOWMAP]]
// =================================================================================================
	
#include "shaders/gles2/utilityLib/vertCommon.glsl"
#include "shaders/gles2/utilityLib/vertSkinning.glsl"

uniform mat4 viewProjMat;
uniform mediump vec4 lightPos;
attribute vec3 vertPos;
varying mediump vec3 lightVec;

#ifdef _F05_AlphaTest
	attribute vec2 texCoords0;
	varying mediump vec2 texCoords;
#endif

void main( void )
{
#ifdef _F01_Skinning	
	vec4 pos = calcWorldPos( skinPos( vec4( vertPos, 1.0 ) ) );
#else
	vec4 pos = calcWorldPos( vec4( vertPos, 1.0 ) );
#endif

#ifdef _F05_AlphaTest
	texCoords = texCoords0;
#endif

	lightVec = lightPos.xyz - pos.xyz;
	gl_Position = viewProjMat * pos;
}
	
	
[[FS_SHADOWMAP]]
// =================================================================================================

uniform mediump vec4 lightPos;
varying mediump vec3 lightVec;

#ifdef _F05_AlphaTest
	uniform mediump vec4 matDiffuseCol;
	uniform sampler2D albedoMap;
	varying mediump vec2 texCoords;
#endif

void main( void )
{
#ifdef _F05_AlphaTest
	mediump vec4 albedo = texture2D( albedoMap, texCoords * vec2( 1, -1 ) ) * matDiffuseCol;
	if( albedo.a < 0.01 ) discard;
#endif
	
	gl_FragColor = vec4( 1.0, 1.0, 1.0, 1.0 );
	
	// Clearly better bias but requires SM 3.0
	// gl_FragDepth = dist + abs( dFdx( dist ) ) + abs( dFdy( dist ) ) + shadowBias;
}


[[FS_LIGHTING]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/gles2/utilityLib/fragLighting.glsl" 

uniform mediump vec4 matDiffuseCol;
uniform mediump vec4 matSpecParams;
uniform sampler2D albedoMap;

#ifdef _F02_NormalMapping
	uniform sampler2D normalMap;
#endif

varying mediump vec4 pos, vsPos;
varying mediump vec2 texCoords;

#ifdef _F02_NormalMapping
	varying mediump mat3 tsbMat;
#else
	varying mediump vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	varying mediump vec3 eyeTS;
#endif

void main( void )
{
	mediump vec3 newCoords = vec3( texCoords, 0 );
	
#ifdef _F03_ParallaxMapping	
	const mediump float plxScale = 0.03;
	const mediump float plxBias = -0.015;
	
	// Iterative parallax mapping
	mediump vec3 eye = normalize( eyeTS );
	for( int i = 0; i < 4; ++i )
	{
		mediump vec4 nmap = texture2D( normalMap, newCoords.st * vec2( 1, -1 ) );
		mediump float height = nmap.a * plxScale + plxBias;
		newCoords += (height - newCoords.p) * nmap.z * eye;
	}
#endif

	// Flip texture vertically to match the GL coordinate system
	newCoords.t *= -1.0;

	mediump vec4 albedo = texture2D( albedoMap, newCoords.st ) * matDiffuseCol;
	
#ifdef _F05_AlphaTest
	if( albedo.a < 0.01 ) discard;
#endif
	
#ifdef _F02_NormalMapping
	mediump vec3 _normalMap = texture2D( normalMap, newCoords.st ).rgb * 2.0 - 1.0;
	mediump vec3 normal = tsbMat * _normalMap;
#else
	mediump vec3 normal = tsbNormal;
#endif

	mediump vec3 newPos = pos.xyz;

#ifdef _F03_ParallaxMapping
	newPos += vec3( 0.0, newCoords.p, 0.0 );
#endif
	
	gl_FragColor.rgb =
		calcPhongSpotLight( newPos, normalize( normal ), albedo.rgb, matSpecParams.rgb,
		                    matSpecParams.a, -vsPos.z, 0.3 );
}



[[FS_AMBIENT]]	
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/gles2/utilityLib/fragLighting.glsl" 

uniform sampler2D albedoMap;
uniform samplerCube ambientMap;

#ifdef _F02_NormalMapping
	uniform sampler2D normalMap;
#endif

#ifdef _F04_EnvMapping
	uniform samplerCube envMap;
#endif

varying mediump vec4 pos;
varying mediump vec2 texCoords;

#ifdef _F02_NormalMapping
	varying mediump mat3 tsbMat;
#else
	varying mediump vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	varying mediump vec3 eyeTS;
#endif

void main( void )
{
	highp vec3 newCoords = vec3( texCoords, 0 );
	


	// Flip texture vertically to match the GL coordinate system
	newCoords.t *= -1.0;

	mediump vec4 albedo = texture2D( albedoMap, newCoords.st );
	
#ifdef _F05_AlphaTest
	if( albedo.a < 0.01 ) discard;
#endif
	
#ifdef _F02_NormalMapping
	mediump vec3 _normalMap = texture2D( normalMap, texCoords.xy ).rgb * 2.0 - 1.0;
	mediump vec3 normal = tsbMat * _normalMap;
#else
	mediump vec3 normal = tsbNormal;
#endif
	
	gl_FragColor.rgb = albedo.rgb * textureCube( ambientMap, normal ).rgb;
	
#ifdef _F04_EnvMapping
	mediump vec3 refl = textureCube( envMap, reflect( pos.xyz - viewerPos, normalize( normal ) ) ).rgb;
	gl_FragColor.rgb = refl * 1.5;
#endif
}
