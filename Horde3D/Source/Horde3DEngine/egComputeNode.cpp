// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2016 Nicolas Schulz and Horde3D team
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

	_localBBox.min = Vec3f( 0, 0, 0 );
	_localBBox.max = Vec3f( 1, 1, 1 );
}


ComputeNode::~ComputeNode()
{

}


SceneNodeTpl *ComputeNode::parsingFunc( map< string, string > &attribs )
{
// 	map< string, string >::iterator itr;
// 	ComputeNodeTpl *computeTpl = new ComputeNodeTpl( "", 0, 0 );

	return 0;
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
			if ( value < 0 || value > 2 ) // Triangles - 0, Lines - 1, Points - 2
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
