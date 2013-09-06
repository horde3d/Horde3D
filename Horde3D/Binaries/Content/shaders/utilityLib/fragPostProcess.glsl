// *************************************************************************************************
// Horde3D Shader Utility Library
// --------------------------------------
//		- Postprocessing functions -
//
// Copyright (C) 2006-2011 Nicolas Schulz
//
// You may use the following code in projects based on the Horde3D graphics engine.
//
// *************************************************************************************************

vec4 getTex2DBilinear( const sampler2D tex, const vec2 texCoord, const vec2 texSize )
{
	// Bilinear filtering function. Useful when hardware filtering is not available, e.g. for
	// floating point textures on ATI 1xx0 cards
	
	vec2 coord0 = texCoord - 0.5 / texSize;
	vec2 coord1 = texCoord + 0.5 / texSize;
	vec2 weight = fract( coord0 * texSize );
	
	vec4 bot = mix( texture2D( tex, coord0 ),
					texture2D( tex, vec2( coord1.x, coord0.y ) ),
					weight.x );
	vec4 top = mix( texture2D( tex, vec2( coord0.x, coord1.y ) ),
					texture2D( tex, coord1 ),
					weight.x );
	
	return mix( bot, top, weight.y );
}

vec4 blurKawase( const sampler2D tex, const vec2 texCoord, const vec2 texSize, const float iteration )
{
	// Function assumes that tex is using bilinear hardware filtering
	
	vec2 dUV = (iteration + 0.5) / texSize;
	
	vec4 col = texture2D( tex, texCoord + vec2( -dUV.x, dUV.y ) );	// Top left
	col += texture2D( tex, texCoord + dUV );						// Top right
	col += texture2D( tex, texCoord + vec2( dUV.x, -dUV.y ) );		// Bottom right
	col += texture2D( tex, texCoord - dUV );						// Bottom left
	
	return col * 0.25;
}