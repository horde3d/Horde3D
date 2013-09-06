// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2011 Nicolas Schulz
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#include "egLight.h"
#include "egMaterial.h"
#include "egModules.h"
#include "egRenderer.h"

#include "utDebug.h"


namespace Horde3D {

using namespace std;


LightNode::LightNode( const LightNodeTpl &lightTpl ) :
	SceneNode( lightTpl )
{
	_materialRes = lightTpl.matRes;
	_lightingContext = lightTpl.lightingContext;
	_shadowContext = lightTpl.shadowContext;
	_radius = lightTpl.radius; _fov = lightTpl.fov;
	_diffuseCol = Vec3f( lightTpl.col_R, lightTpl.col_G, lightTpl.col_B );
	_diffuseColMult = lightTpl.colMult;
	_shadowMapCount = lightTpl.shadowMapCount;
	_shadowSplitLambda = lightTpl.shadowSplitLambda;
	_shadowMapBias = lightTpl.shadowMapBias;
}


LightNode::~LightNode()
{
	for( uint32 i = 0; i < _occQueries.size(); ++i )
	{
		if( _occQueries[i] != 0 )
			gRDI->destroyQuery( _occQueries[i] );
	}
}


SceneNodeTpl *LightNode::parsingFunc( map< string, string > &attribs )
{
	bool result = true;
	
	map< string, string >::iterator itr;
	LightNodeTpl *lightTpl = new LightNodeTpl( "", 0x0, "", "" );

	itr = attribs.find( "material" );
	if( itr != attribs.end() )
	{
		uint32 res = Modules::resMan().addResource( ResourceTypes::Material, itr->second, 0, false );
		if( res != 0 )
			lightTpl->matRes = (MaterialResource *)Modules::resMan().resolveResHandle( res );
	}
	itr = attribs.find( "lightingContext" );
	if( itr != attribs.end() ) lightTpl->lightingContext = itr->second;
	else result = false;
	itr = attribs.find( "shadowContext" );
	if( itr != attribs.end() ) lightTpl->shadowContext = itr->second;
	else result = false;
	itr = attribs.find( "radius" );
	if( itr != attribs.end() ) lightTpl->radius = (float)atof( itr->second.c_str() );
	itr = attribs.find( "fov" );
	if( itr != attribs.end() ) lightTpl->fov = (float)atof( itr->second.c_str() );
	itr = attribs.find( "col_R" );
	if( itr != attribs.end() ) lightTpl->col_R = (float)atof( itr->second.c_str() );
	itr = attribs.find( "col_G" );
	if( itr != attribs.end() ) lightTpl->col_G = (float)atof( itr->second.c_str() );
	itr = attribs.find( "col_B" );
	if( itr != attribs.end() ) lightTpl->col_B = (float)atof( itr->second.c_str() );
	itr = attribs.find( "colMult" );
	if( itr != attribs.end() ) lightTpl->colMult = (float)atof( itr->second.c_str() );
	itr = attribs.find( "shadowMapCount" );
	if( itr != attribs.end() ) lightTpl->shadowMapCount = atoi( itr->second.c_str() );
	itr = attribs.find( "shadowSplitLambda" );
	if( itr != attribs.end() ) lightTpl->shadowSplitLambda = (float)atof( itr->second.c_str() );
	itr = attribs.find( "shadowMapBias" );
	if( itr != attribs.end() ) lightTpl->shadowMapBias = (float)atof( itr->second.c_str() );
	
	if( !result )
	{
		delete lightTpl; lightTpl = 0x0;
	}
	
	return lightTpl;
}


SceneNode *LightNode::factoryFunc( const SceneNodeTpl &nodeTpl )
{
	if( nodeTpl.type != SceneNodeTypes::Light ) return 0x0;

	return new LightNode( *(LightNodeTpl *)&nodeTpl );
}


int LightNode::getParamI( int param )
{
	switch( param )
	{
	case LightNodeParams::MatResI:
		if( _materialRes != 0x0 ) return _materialRes->getHandle();
		else return 0;
	case LightNodeParams::ShadowMapCountI:
		return _shadowMapCount;
	}

	return SceneNode::getParamI( param );
}


void LightNode::setParamI( int param, int value )
{
	Resource *res;
	
	switch( param )
	{
	case LightNodeParams::MatResI:
		res = Modules::resMan().resolveResHandle( value );
		if( res == 0x0 || res->getType() == ResourceTypes::Material )
			_materialRes = (MaterialResource *)res;
		else
			Modules::setError( "Invalid handle in h3dSetNodeParamI for H3DLight::MatResI" );
		return;
	case LightNodeParams::ShadowMapCountI:
		if( value == 0 || value == 1 || value == 2 || value == 3 || value == 4 )
			_shadowMapCount = (uint32)value;
		else
			Modules::setError( "Invalid value in h3dSetNodeParamI for H3DLight::ShadowMapCountI" );
		return;
	}

	return SceneNode::setParamI( param, value );
}


float LightNode::getParamF( int param, int compIdx )
{
	switch( param )
	{
	case LightNodeParams::RadiusF:
		return _radius;
	case LightNodeParams::ColorF3:
		if( (unsigned)compIdx < 3 ) return _diffuseCol[compIdx];
		break;
	case LightNodeParams::ColorMultiplierF:
		return _diffuseColMult;
	case LightNodeParams::FovF:
		return _fov;
	case LightNodeParams::ShadowSplitLambdaF:
		return _shadowSplitLambda;
	case LightNodeParams::ShadowMapBiasF:
		return _shadowMapBias;
	}

	return SceneNode::getParamF( param, compIdx );
}


void LightNode::setParamF( int param, int compIdx, float value )
{
	switch( param )
	{
	case LightNodeParams::RadiusF:
		_radius = value;
		markDirty();
		return;
	case LightNodeParams::FovF:
		_fov = value;
		markDirty();
		return;
	case LightNodeParams::ColorF3:
		if( (unsigned)compIdx < 3 )
		{
			_diffuseCol[compIdx] = value;
			return;
		}
		break;
	case LightNodeParams::ColorMultiplierF:
		_diffuseColMult = value;
		return;
	case LightNodeParams::ShadowSplitLambdaF:
		_shadowSplitLambda = value;
		return;
	case LightNodeParams::ShadowMapBiasF:
		_shadowMapBias = value;
		return;
	}

	SceneNode::setParamF( param, compIdx, value );
}


const char *LightNode::getParamStr( int param )
{
	switch( param )
	{
	case LightNodeParams::LightingContextStr:
		return _lightingContext.c_str();
	case LightNodeParams::ShadowContextStr:
		return _shadowContext.c_str();
	}

	return SceneNode::getParamStr( param );
}


void LightNode::setParamStr( int param, const char *value )
{
	switch( param )
	{
	case LightNodeParams::LightingContextStr:
		_lightingContext = value;
		return;
	case LightNodeParams::ShadowContextStr:
		_shadowContext = value;
		return;
	}

	SceneNode::setParamStr( param, value );
}


void LightNode::calcScreenSpaceAABB( const Matrix4f &mat, float &x, float &y, float &w, float &h )
{
	uint32 numPoints = 0;
	Vec3f points[8];
	Vec4f pts[8];

	float min_x = Math::MaxFloat, min_y = Math::MaxFloat;
	float max_x = -Math::MaxFloat, max_y = -Math::MaxFloat;
	
	if( _fov < 180 )
	{
		// Generate frustum for spot light
		numPoints = 5;
		float val = 1.0f * tanf( degToRad( _fov / 2 ) );
		points[0] = _absTrans * Vec3f( 0, 0, 0 );
		points[1] = _absTrans * Vec3f( -val * _radius, -val * _radius, -_radius );
		points[2] = _absTrans * Vec3f(  val * _radius, -val * _radius, -_radius );
		points[3] = _absTrans * Vec3f(  val * _radius,  val * _radius, -_radius );
		points[4] = _absTrans * Vec3f( -val * _radius,  val * _radius, -_radius );
	}
	else
	{
		// Generate sphere for point light
		numPoints = 8;
		points[0] = _absPos + Vec3f( -_radius, -_radius, -_radius );
		points[1] = _absPos + Vec3f(  _radius, -_radius, -_radius );
		points[2] = _absPos + Vec3f(  _radius,  _radius, -_radius );
		points[3] = _absPos + Vec3f( -_radius,  _radius, -_radius );
		points[4] = _absPos + Vec3f( -_radius, -_radius,  _radius );
		points[5] = _absPos + Vec3f(  _radius, -_radius,  _radius );
		points[6] = _absPos + Vec3f(  _radius,  _radius,  _radius );
		points[7] = _absPos + Vec3f( -_radius,  _radius,  _radius );
	}

	// Project points to screen-space and find extents
	for( uint32 i = 0; i < numPoints; ++i )
	{
		pts[i] = mat * Vec4f( points[i].x, points[i].y, points[i].z, 1 );
		
		if( pts[i].w != 0 )
		{
			pts[i].x = (pts[i].x / pts[i].w) * 0.5f + 0.5f;
			pts[i].y = (pts[i].y / pts[i].w) * 0.5f + 0.5f;
		}

		if( pts[i].x < min_x ) min_x = pts[i].x;
		if( pts[i].y < min_y ) min_y = pts[i].y;
		if( pts[i].x > max_x ) max_x = pts[i].x;
		if( pts[i].y > max_y ) max_y = pts[i].y;
	}
	
	// Clamp values
	if( min_x < 0 ) min_x = 0; if( min_x > 1 ) min_x = 1;
	if( max_x < 0 ) max_x = 0; if( max_x > 1 ) max_x = 1;
	if( min_y < 0 ) min_y = 0; if( min_y > 1 ) min_y = 1;
	if( max_y < 0 ) max_y = 0; if( max_y > 1 ) max_y = 1;
	
	x = min_x; y = min_y;
	w = max_x - min_x; h = max_y - min_y;

	// Check if viewer is inside bounding box
	if( pts[0].w < 0 || pts[1].w < 0 || pts[2].w < 0 || pts[3].w < 0 || pts[4].w < 0 )
	{
		x = 0; y = 0; w = 1; h = 1;
	}
	else if( numPoints == 8 && (pts[5].w < 0 || pts[6].w < 0 || pts[7].w < 0) )
	{
		x = 0; y = 0; w = 1; h = 1;
	}
}


void LightNode::onPostUpdate()
{
	// Calculate view matrix
	_viewMat = _absTrans.inverted();
	
	// Get position and spot direction
	Matrix4f m = _absTrans;
	m.c[3][0] = 0; m.c[3][1] = 0; m.c[3][2] = 0;
	_spotDir = m * Vec3f( 0, 0, -1 );
	_spotDir.normalize();
	_absPos = Vec3f( _absTrans.c[3][0], _absTrans.c[3][1], _absTrans.c[3][2] );

	// Generate frustum
	if( _fov < 180 )
		_frustum.buildViewFrustum( _absTrans, _fov, 1.0f, 0.1f, _radius );
	else
		_frustum.buildBoxFrustum( _absTrans, -_radius, _radius, -_radius, _radius, _radius, -_radius );
}

}  // namespace
