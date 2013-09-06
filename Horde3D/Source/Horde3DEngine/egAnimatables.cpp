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

#include "egModel.h"
#include "egMaterial.h"
#include "egModules.h"
#include "egRenderer.h"

#include "utDebug.h"


namespace Horde3D {

using namespace std;


// *************************************************************************************************
// Class MeshNode
// *************************************************************************************************

MeshNode::MeshNode( const MeshNodeTpl &meshTpl ) :
	SceneNode( meshTpl ),
	_materialRes( meshTpl.matRes ), _batchStart( meshTpl.batchStart ), _batchCount( meshTpl.batchCount ),
	_vertRStart( meshTpl.vertRStart ), _vertREnd( meshTpl.vertREnd ), _lodLevel( meshTpl.lodLevel ),
	_parentModel( 0x0 )
{
	_renderable = true;
	
	if( _materialRes != 0x0 )
		_sortKey = (float)_materialRes->getHandle();
}


MeshNode::~MeshNode()
{
	_materialRes = 0x0;
	for( uint32 i = 0; i < _occQueries.size(); ++i )
	{
		if( _occQueries[i] != 0 )
			gRDI->destroyQuery( _occQueries[i] );
	}
}


SceneNodeTpl *MeshNode::parsingFunc( map< string, string > &attribs )
{
	bool result = true;
	
	map< string, string >::iterator itr;
	MeshNodeTpl *meshTpl = new MeshNodeTpl( "", 0x0, 0, 0, 0, 0 );

	itr = attribs.find( "material" );
	if( itr != attribs.end() )
	{
		uint32 res = Modules::resMan().addResource( ResourceTypes::Material, itr->second, 0, false );
		if( res != 0 )
			meshTpl->matRes = (MaterialResource *)Modules::resMan().resolveResHandle( res );
	}
	else result = false;
	itr = attribs.find( "batchStart" );
	if( itr != attribs.end() ) meshTpl->batchStart = atoi( itr->second.c_str() );
	else result = false;
	itr = attribs.find( "batchCount" );
	if( itr != attribs.end() ) meshTpl->batchCount = atoi( itr->second.c_str() );
	else result = false;
	itr = attribs.find( "vertRStart" );
	if( itr != attribs.end() ) meshTpl->vertRStart = atoi( itr->second.c_str() );
	else result = false;
	itr = attribs.find( "vertREnd" );
	if( itr != attribs.end() ) meshTpl->vertREnd = atoi( itr->second.c_str() );
	else result = false;

	itr = attribs.find( "lodLevel" );
	if( itr != attribs.end() ) meshTpl->lodLevel = atoi( itr->second.c_str() );

	if( !result )
	{
		delete meshTpl; meshTpl = 0x0;
	}
	
	return meshTpl;
}


SceneNode *MeshNode::factoryFunc( const SceneNodeTpl &nodeTpl )
{
	if( nodeTpl.type != SceneNodeTypes::Mesh ) return 0x0;
	
	return new MeshNode( *(MeshNodeTpl *)&nodeTpl );
}


IAnimatableNode *MeshNode::getANParent()
{
	switch( _parent->getType() )
	{
	case SceneNodeTypes::Joint:
		return (IAnimatableNode *)((JointNode *)_parent);
	case SceneNodeTypes::Mesh:
		return (IAnimatableNode *)((MeshNode *)_parent);
	default:
		return 0x0;
	}
}


bool MeshNode::canAttach( SceneNode &parent )
{
	// Important: Meshes may not live outside of models
	return (parent.getType() == SceneNodeTypes::Model) ||
		   (parent.getType() == SceneNodeTypes::Mesh) ||
		   (parent.getType() == SceneNodeTypes::Joint);
}


int MeshNode::getParamI( int param )
{
	switch( param )
	{
	case MeshNodeParams::MatResI:
		if( _materialRes != 0x0 ) return _materialRes->getHandle();
		else return 0;
	case MeshNodeParams::BatchStartI:
		return _batchStart;
	case MeshNodeParams::BatchCountI:
		return _batchCount;
	case MeshNodeParams::VertRStartI:
		return _vertRStart;
	case MeshNodeParams::VertREndI:
		return _vertREnd;
	case MeshNodeParams::LodLevelI:
		return _lodLevel;
	}

	return SceneNode::getParamI( param );
}


void MeshNode::setParamI( int param, int value )
{
	Resource *res;
	
	switch( param )
	{
	case MeshNodeParams::MatResI:
		res = Modules::resMan().resolveResHandle( value );
		if( res != 0x0 && res->getType() == ResourceTypes::Material )
		{
			_materialRes = (MaterialResource *)res;
			_sortKey = (float)_materialRes->getHandle();
		}
		else
		{
			Modules::setError( "Invalid handle in h3dSetNodeParamI for H3DMesh::MatResI" );
		}
		return;
	case MeshNodeParams::LodLevelI:
		_lodLevel = value;
		return;
	}

	SceneNode::setParamI( param, value );
}


bool MeshNode::checkIntersection( const Vec3f &rayOrig, const Vec3f &rayDir, Vec3f &intsPos ) const
{
	// Collision check is only done for base LOD
	if( _lodLevel != 0 ) return false;

	if( !rayAABBIntersection( rayOrig, rayDir, _bBox.min, _bBox.max ) ) return false;
	
	GeometryResource *geoRes = _parentModel->getGeometryResource();
	if( geoRes == 0x0 || geoRes->getIndexData() == 0x0 || geoRes->getVertPosData() == 0x0 ) return false;
	
	// Transform ray to local space
	Matrix4f m = _absTrans.inverted();
	Vec3f orig = m * rayOrig;
	Vec3f dir = m * (rayOrig + rayDir) - orig;

	Vec3f nearestIntsPos = Vec3f( Math::MaxFloat, Math::MaxFloat, Math::MaxFloat );
	bool intersection = false;
	
	// Check triangles
	for( uint32 i = _batchStart; i < _batchStart + _batchCount; i += 3 )
	{
		Vec3f *vert0, *vert1, *vert2;
		
		if( geoRes->_16BitIndices )
		{
			vert0 = &geoRes->getVertPosData()[((uint16 *)geoRes->_indexData)[i + 0]];
			vert1 = &geoRes->getVertPosData()[((uint16 *)geoRes->_indexData)[i + 1]];
			vert2 = &geoRes->getVertPosData()[((uint16 *)geoRes->_indexData)[i + 2]];
		}
		else
		{
			vert0 = &geoRes->getVertPosData()[((uint32 *)geoRes->_indexData)[i + 0]];
			vert1 = &geoRes->getVertPosData()[((uint32 *)geoRes->_indexData)[i + 1]];
			vert2 = &geoRes->getVertPosData()[((uint32 *)geoRes->_indexData)[i + 2]];
		}
		
		if( rayTriangleIntersection( orig, dir, *vert0, *vert1, *vert2, intsPos ) )
		{
			intersection = true;
			if( (intsPos - orig).length() < (nearestIntsPos - orig).length() )
				nearestIntsPos = intsPos;
		}
	}

	intsPos = _absTrans * nearestIntsPos;
	
	return intersection;
}


void MeshNode::onAttach( SceneNode &parentNode )
{
	// Find parent model node
	SceneNode *node = &parentNode;
	while( node->getType() != SceneNodeTypes::Model ) node = node->getParent();
	_parentModel = (ModelNode *)node;
	_parentModel->markNodeListDirty();
}


void MeshNode::onDetach( SceneNode &/*parentNode*/ )
{
	if( _parentModel != 0x0 ) _parentModel->markNodeListDirty();
}


void MeshNode::onPostUpdate()
{
	_bBox = _localBBox;
	_bBox.transform( _absTrans );
}


// *************************************************************************************************
// Class JointNode
// *************************************************************************************************

JointNode::JointNode( const JointNodeTpl &jointTpl ) :
	SceneNode( jointTpl ), _jointIndex( jointTpl.jointIndex ), _parentModel( 0x0 )
{
}


SceneNodeTpl *JointNode::parsingFunc( map< string, string > &attribs )
{
	bool result = true;
	
	map< string, string >::iterator itr;
	JointNodeTpl *jointTpl = new JointNodeTpl( "", 0 );

	itr = attribs.find( "jointIndex" );
	if( itr != attribs.end() ) jointTpl->jointIndex = atoi( itr->second.c_str() );
	else result = false;

	if( !result )
	{
		delete jointTpl; jointTpl = 0x0;
	}

	return jointTpl;
}


SceneNode *JointNode::factoryFunc( const SceneNodeTpl &nodeTpl )
{
	if( nodeTpl.type != SceneNodeTypes::Joint ) return 0x0;
	
	return new JointNode( *(JointNodeTpl *)&nodeTpl );
}


IAnimatableNode *JointNode::getANParent()
{
	switch( _parent->getType() )
	{
	case SceneNodeTypes::Joint:
		return (IAnimatableNode *)((JointNode *)_parent);
	case SceneNodeTypes::Mesh:
		return (IAnimatableNode *)((MeshNode *)_parent);
	default:
		return 0x0;
	}
}


bool JointNode::canAttach( SceneNode &parent )
{
	// Important: Joints may not live outside of models
	return (parent.getType() == SceneNodeTypes::Model) ||
		   (parent.getType() == SceneNodeTypes::Joint);
}


int JointNode::getParamI( int param )
{
	switch( param )
	{
	case JointNodeParams::JointIndexI:
		return (int)_jointIndex;
	}

	return SceneNode::getParamI( param );
}


void JointNode::onPostUpdate()
{
	if( _parentModel->getGeometryResource() == 0x0 ) return;
	
	if( _parent->getType() != SceneNodeTypes::Joint )
		_relModelMat = _relTrans;
	else
		Matrix4f::fastMult43( _relModelMat, ((JointNode *)_parent)->_relModelMat, _relTrans );

	if( _parentModel->jointExists( _jointIndex ) )
	{
		Matrix4f mat( Math::NO_INIT );
		Matrix4f::fastMult43( mat, _relModelMat, _parentModel->getGeometryResource()->getInvBindMat( _jointIndex ) );

		_parentModel->setSkinningMat( _jointIndex, mat );
	}
}


void JointNode::onAttach( SceneNode &parentNode )
{
	// Find parent model node
	SceneNode *node = &parentNode;
	while( node->getType() != SceneNodeTypes::Model ) node = node->getParent();
	_parentModel = (ModelNode *)node;
	
	_parentModel->markNodeListDirty();
}


void JointNode::onDetach( SceneNode &/*parentNode*/ )
{
	if( _parentModel != 0x0 ) _parentModel->markNodeListDirty();
}

}  // namespace
