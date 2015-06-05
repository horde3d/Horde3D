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

#include "egCamera.h"
#include "egModules.h"
#include "egRenderer.h"

#include "utDebug.h"


namespace Horde3D {

using namespace std;


CameraNode::CameraNode( const CameraNodeTpl &cameraTpl ) :
	SceneNode( cameraTpl )
{
	_pipelineRes = cameraTpl.pipeRes;
	_outputTex = cameraTpl.outputTex;
	_outputBufferIndex = cameraTpl.outputBufferIndex;
	_vpX = 0; _vpY = 0; _vpWidth = 320; _vpHeight = 240;
	_frustLeft = cameraTpl.leftPlane;
	_frustRight = cameraTpl.rightPlane;
	_frustBottom = cameraTpl.bottomPlane;
	_frustTop = cameraTpl.topPlane;
	_frustNear = cameraTpl.nearPlane;
	_frustFar = cameraTpl.farPlane;
	_orthographic = cameraTpl.orthographic;
	_occSet = cameraTpl.occlusionCulling ? Modules::renderer().registerOccSet() : -1;
	_manualProjMat = false;
}


CameraNode::~CameraNode()
{
	_pipelineRes = 0x0;
	_outputTex = 0x0;
	if( _occSet >= 0 ) Modules::renderer().unregisterOccSet( _occSet );
}


SceneNodeTpl *CameraNode::parsingFunc( map< string, string > &attribs )
{
	bool result = true;
	
	map< string, string >::iterator itr;
	CameraNodeTpl *cameraTpl = new CameraNodeTpl( "", 0x0 );

	itr = attribs.find( "pipeline" );
	if( itr != attribs.end() )
	{
		uint32 res = Modules::resMan().addResource( ResourceTypes::Pipeline, itr->second, 0, false );
		cameraTpl->pipeRes = (PipelineResource *)Modules::resMan().resolveResHandle( res );
	}
	else result = false;
	itr = attribs.find( "outputTex" );
	if( itr != attribs.end() )
	{	
		cameraTpl->outputTex = (TextureResource *)Modules::resMan().findResource(
			ResourceTypes::Texture, itr->second );
	}
	itr = attribs.find( "outputBufferIndex" );
	if( itr != attribs.end() ) cameraTpl->outputBufferIndex = atoi( itr->second.c_str() );
	itr = attribs.find( "leftPlane" );
	if( itr != attribs.end() ) cameraTpl->leftPlane = (float)atof( itr->second.c_str() );
	itr = attribs.find( "rightPlane" );
	if( itr != attribs.end() ) cameraTpl->rightPlane = (float)atof( itr->second.c_str() );
	itr = attribs.find( "bottomPlane" );
	if( itr != attribs.end() ) cameraTpl->bottomPlane = (float)atof( itr->second.c_str() );
	itr = attribs.find( "topPlane" );
	if( itr != attribs.end() ) cameraTpl->topPlane = (float)atof( itr->second.c_str() );
	itr = attribs.find( "nearPlane" );
	if( itr != attribs.end() ) cameraTpl->nearPlane = (float)atof( itr->second.c_str() );
	itr = attribs.find( "farPlane" );
	if( itr != attribs.end() ) cameraTpl->farPlane = (float)atof( itr->second.c_str() );
	itr = attribs.find( "orthographic" );
	if( itr != attribs.end() ) 
	{
		if ( _stricmp( itr->second.c_str(), "true" ) == 0 || _stricmp( itr->second.c_str(), "1" ) == 0 )
			cameraTpl->orthographic = true;
		else
			cameraTpl->orthographic = false;
	}
	itr = attribs.find( "occlusionCulling" );
	if( itr != attribs.end() ) 
	{
		if ( _stricmp( itr->second.c_str(), "true" ) == 0 || _stricmp( itr->second.c_str(), "1" ) == 0 )
			cameraTpl->occlusionCulling = true;
		else
			cameraTpl->occlusionCulling = false;
	}

	if( !result )
	{
		delete cameraTpl; cameraTpl = 0x0;
	}
	
	return cameraTpl;
}


SceneNode *CameraNode::factoryFunc( const SceneNodeTpl &nodeTpl )
{
	if( nodeTpl.type != SceneNodeTypes::Camera ) return 0x0;

	return new CameraNode( *(CameraNodeTpl *)&nodeTpl );
}


int CameraNode::getParamI( int param )
{
	switch( param )
	{
	case CameraNodeParams::PipeResI:
		return _pipelineRes != 0x0 ? _pipelineRes->getHandle() : 0;
	case CameraNodeParams::OutTexResI:
		return _outputTex != 0x0 ? _outputTex->getHandle() : 0;
	case CameraNodeParams::OutBufIndexI:
		return _outputBufferIndex;
	case CameraNodeParams::ViewportXI:
		return _vpX;
	case CameraNodeParams::ViewportYI:
		return _vpY;
	case CameraNodeParams::ViewportWidthI:
		return _vpWidth;
	case CameraNodeParams::ViewportHeightI:
		return _vpHeight;
	case CameraNodeParams::OrthoI:
		return _orthographic ? 1 : 0;
	case CameraNodeParams::OccCullingI:
		return _occSet >= 0 ? 1 : 0;
	}

	return SceneNode::getParamI( param );
}


void CameraNode::setParamI( int param, int value )
{
	Resource *res;
	
	switch( param )
	{
	case CameraNodeParams::PipeResI:
		res = Modules::resMan().resolveResHandle( value );
		if( res != 0x0 && res->getType() == ResourceTypes::Pipeline )
			_pipelineRes = (PipelineResource *)res;
		else
			Modules::setError( "Invalid handle in h3dSetNodeParamI for H3DCamera::PipeResI" );
		return;
	case CameraNodeParams::OutTexResI:
		res = Modules::resMan().resolveResHandle( value );
		if( res == 0x0 || (res->getType() == ResourceTypes::Texture &&
		    ((TextureResource *)res)->getTexType() == TextureTypes::Tex2D) )
			_outputTex = (TextureResource *)res;
		else
			Modules::setError( "Invalid 2D texture resource in h3dSetNodeParamI for H3DCamera::OutTexResI" );
		return;
	case CameraNodeParams::OutBufIndexI:
		_outputBufferIndex = value;
		return;
	case CameraNodeParams::ViewportXI:
		_vpX = value;
		return;
	case CameraNodeParams::ViewportYI:
		_vpY = value;
		return;
	case CameraNodeParams::ViewportWidthI:
		_vpWidth = value;
		return;
	case CameraNodeParams::ViewportHeightI:
		_vpHeight = value;
		return;
	case CameraNodeParams::OrthoI:
		_orthographic = (value == 1);
		markDirty();
		return;
	case CameraNodeParams::OccCullingI:
		if( _occSet < 0 && value != 0 )
		{		
			_occSet = Modules::renderer().registerOccSet();
		}
		else if( _occSet >= 0 && value == 0 )
		{
			Modules::renderer().unregisterOccSet( _occSet );
			_occSet = -1;
		}
		return;
	}

	SceneNode::setParamI( param, value );
}


float CameraNode::getParamF( int param, int compIdx )
{
	switch( param )
	{
	case CameraNodeParams::LeftPlaneF:
		return _frustLeft;
	case CameraNodeParams::RightPlaneF:
		return _frustRight;
	case CameraNodeParams::BottomPlaneF:
		return _frustBottom;
	case CameraNodeParams::TopPlaneF:
		return _frustTop;
	case CameraNodeParams::NearPlaneF:
		return _frustNear;
	case CameraNodeParams::FarPlaneF:
		return _frustFar;
	}

	return SceneNode::getParamF( param, compIdx );
}


void CameraNode::setParamF( int param, int compIdx, float value )
{
	switch( param )
	{
	case CameraNodeParams::LeftPlaneF:
		_frustLeft = value;
		markDirty();
		return;
	case CameraNodeParams::RightPlaneF:
		_frustRight = value;
		markDirty();
		return;
	case CameraNodeParams::BottomPlaneF:
		_frustBottom = value;
		markDirty();
		return;
	case CameraNodeParams::TopPlaneF:
		_frustTop = value;
		markDirty();
		return;
	case CameraNodeParams::NearPlaneF:
		_frustNear = value;
		markDirty();
		return;
	case CameraNodeParams::FarPlaneF:
		_frustFar = value;
		markDirty();
		return;
	}

	SceneNode::setParamF( param, compIdx, value );
}


void CameraNode::setupViewParams( float fov, float aspect, float nearPlane, float farPlane )
{
	float ymax = nearPlane * tanf( degToRad( fov / 2 ) );
	float xmax = ymax * aspect;

	_frustLeft = -xmax;
	_frustRight = xmax;
	_frustBottom = -ymax;
	_frustTop = ymax;
	_frustNear = nearPlane;
	_frustFar = farPlane;

	// setting view params implicitly disables the manual projection matrix
	_manualProjMat = false;
	
	markDirty();
}

void CameraNode::setProjectionMatrix( float* projMat ) 
{
	memcpy( _projMat.x, projMat, 16 * sizeof( float ) );
	_manualProjMat = true;

	markDirty();
}


void CameraNode::onPostUpdate()
{
	// Get position
	_absPos = Vec3f( _absTrans.c[3][0], _absTrans.c[3][1], _absTrans.c[3][2] );
	
	// Calculate view matrix
	_viewMat = _absTrans.inverted();
	
	// Calculate projection matrix if not using a manually set one
	if( !_manualProjMat )
	{
		if( !_orthographic )
			_projMat = Matrix4f::PerspectiveMat( _frustLeft, _frustRight, _frustBottom, _frustTop, _frustNear, _frustFar );
		else
			_projMat = Matrix4f::OrthoMat( _frustLeft, _frustRight, _frustBottom, _frustTop, _frustNear, _frustFar );
	}

	// Update frustum
	_frustum.buildViewFrustum( _viewMat, _projMat );
}

}  // namespace
