// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2020 Nicolas Schulz and Horde3D team
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#include "egComputeNode.h"
#include "egModules.h"
#include "egCom.h"

namespace Horde3D {

using namespace std;

// =================================================================================================
// Compute Node
// =================================================================================================

ComputeNode::ComputeNode( const ComputeNodeTpl &computeTpl ) : SceneNode( computeTpl )
{
	_compBufferRes = computeTpl.compBufRes;
	_materialRes = computeTpl.matRes;
	_drawType = computeTpl.drawType;
	_elementsCount = computeTpl.elementsCount;

	_renderable = true;

	_localBBox.min = computeTpl.aabbMin;
	_localBBox.max = computeTpl.aabbMax;
}


ComputeNode::~ComputeNode()
{

}


SceneNodeTpl *ComputeNode::parsingFunc( map< string, string > &attribs )
{
	bool result = true;

	map< string, string >::iterator itr;
    ComputeNodeTpl *computeTpl = new ComputeNodeTpl( "", 0x0, 0x0, 0, 0 );

	itr = attribs.find( "computeBuffer" );
	if ( itr != attribs.end() )
	{
		uint32 res = Modules::resMan().addResource( ResourceTypes::ComputeBuffer, itr->second, 0, false );
		if ( res != 0 )
			computeTpl->compBufRes = ( ComputeBufferResource * ) Modules::resMan().resolveResHandle( res );
	}
	else result = false;

	itr = attribs.find( "material" );
	if ( itr != attribs.end() )
	{
		uint32 res = Modules::resMan().addResource( ResourceTypes::Material, itr->second, 0, false );
		if ( res != 0 )
			computeTpl->matRes = ( MaterialResource * ) Modules::resMan().resolveResHandle( res );
	}
	else result = false;

	itr = attribs.find( "drawType" );
	if ( itr != attribs.end() )
	{
		if ( _stricmp( itr->second.c_str(), "triangles" ) == 0 ) computeTpl->drawType = 0; // triangles
		else if ( _stricmp( itr->second.c_str(), "lines" ) == 0 ) computeTpl->drawType = 1; // lines
		else if ( _stricmp( itr->second.c_str(), "points" ) == 0 ) computeTpl->drawType = 2; // points
		else result = false;
	}
	else result = false;
	
	itr = attribs.find( "elementsCount" );
	if ( itr != attribs.end() ) computeTpl->elementsCount = atoi( itr->second.c_str() );
	else result = false;
	
	// AABB
	itr = attribs.find( "aabbMinX" );
	if ( itr != attribs.end() ) computeTpl->aabbMin.x = toFloat( itr->second.c_str() );
	else result = false;

	itr = attribs.find( "aabbMinY" );
	if ( itr != attribs.end() ) computeTpl->aabbMin.y = toFloat( itr->second.c_str() );
	else result = false;

	itr = attribs.find( "aabbMinZ" );
	if ( itr != attribs.end() ) computeTpl->aabbMin.z = toFloat( itr->second.c_str() );
	else result = false;

	itr = attribs.find( "aabbMaxX" );
	if ( itr != attribs.end() ) computeTpl->aabbMax.x = toFloat( itr->second.c_str() );
	else result = false;

	itr = attribs.find( "aabbMaxY" );
	if ( itr != attribs.end() ) computeTpl->aabbMax.y = toFloat( itr->second.c_str() );
	else result = false;

	itr = attribs.find( "aabbMaxZ" );
	if ( itr != attribs.end() ) computeTpl->aabbMax.z = toFloat( itr->second.c_str() );
	else result = false;

	if ( !result )
	{
		delete computeTpl; computeTpl = 0x0;
	}

	return computeTpl;
}


SceneNode *ComputeNode::factoryFunc( const SceneNodeTpl &nodeTpl )
{
	if ( nodeTpl.type != SceneNodeTypes::Compute ) return 0;

	return new ComputeNode( *( ComputeNodeTpl * ) &nodeTpl );
}


void ComputeNode::onPostUpdate()
{
	_bBox = _localBBox;
	_bBox.transform( _absTrans );
}


int ComputeNode::getParamI( int param ) const
{
	switch ( param )
	{
		case ComputeNodeParams::CompBufResI:
			if ( _compBufferRes ) return _compBufferRes->getHandle();
			else return 0;
		case ComputeNodeParams::MatResI:
			if ( _materialRes ) return _materialRes->getHandle();
			else return 0;
		case ComputeNodeParams::DrawTypeI:
			return _drawType;
		case ComputeNodeParams::ElementsCountI:
			return _elementsCount;
		default:
			break;
	}

	return SceneNode::getParamI( param );
}


void ComputeNode::setParamI( int param, int value )
{
	Resource *res;
	
	switch ( param )
	{
		case ComputeNodeParams::CompBufResI:
			res = Modules::resMan().resolveResHandle( value );
			if ( res == 0x0 || res->getType() == ResourceTypes::ComputeBuffer )
				_compBufferRes = ( ComputeBufferResource * ) res;
			else
				Modules::setError( "Invalid handle in h3dSetNodeParamI for H3DComputeNode::CompBufResI" );
			return;
		case ComputeNodeParams::MatResI:
			res = Modules::resMan().resolveResHandle( value );
			if ( res == 0x0 || res->getType() == ResourceTypes::Material )
				_materialRes = ( MaterialResource * ) res;
			else
				Modules::setError( "Invalid handle in h3dSetNodeParamI for H3DComputeNode::MatResI" );
			return;
		case ComputeNodeParams::DrawTypeI:
			if ( value < 0 || value > 3 ) // Triangles - 0, Lines - 1, Patches - 2, Points - 3
			{
				Modules::log().writeError( "Invalid value specified in h3dSetNodeParamI for H3DComputeNode::DrawTypeI" );
				return;
			}

			_drawType = value;
			return;
		case ComputeNodeParams::ElementsCountI:
			if ( value < 0 )
			{
				Modules::log().writeError( "Invalid number of draw elements specified in h3dSetNodeParamI for H3DComputeNode::ElementsCountI" );
				return;
			}

			_elementsCount = value;
			return;
		default:
			break;
	}

	SceneNode::setParamI( param, value );
}


float ComputeNode::getParamF( int param, int compIdx ) const
{
	switch ( param )
	{
		case ComputeNodeParams::AABBMinF:
			if ( compIdx < 0 || compIdx > 2 ) 
			{
				Modules::setError( "Invalid compIdx specified in h3dGetNodeParamF for H3DComputeNode::AABBMinF" );
				return Math::NaN;
			}
			
			return _localBBox.min[ compIdx ];
		case ComputeNodeParams::AABBMaxF:
			if ( compIdx < 0 || compIdx > 2 )
			{
				Modules::setError( "Invalid compIdx specified in h3dGetNodeParamF for H3DComputeNode::AABBMaxF" );
				return Math::NaN;
			}

			return _localBBox.max[ compIdx ];
		default:
			break;
	}

	return SceneNode::getParamF( param, compIdx );
}


void ComputeNode::setParamF( int param, int compIdx, float value )
{
	switch ( param )
	{
		case ComputeNodeParams::AABBMinF:
			if ( compIdx < 0 || compIdx > 2 )
			{
				Modules::setError( "Invalid compIdx specified in h3dSetNodeParamF for H3DComputeNode::AABBMinF" );
				return;
			}

			_localBBox.min[ compIdx ] = value;
			markDirty();
	
			return;
		case ComputeNodeParams::AABBMaxF:
			if ( compIdx < 0 || compIdx > 2 )
			{
				Modules::setError( "Invalid compIdx specified in h3dSetNodeParamF for H3DComputeNode::AABBMaxF" );
				return;
			}

			_localBBox.max[ compIdx ] = value;
			markDirty();

			return;
		default:
			break;
	}
	
	SceneNode::setParamF( param, compIdx, value );
}

} // namespace
