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

#include "egScene.h"
#include "egSceneGraphRes.h"
#include "egLight.h"
#include "egCamera.h"
#include "egParticle.h"
#include "egModules.h"
#include "egCom.h"
#include "egRenderer.h"

#include "utDebug.h"


namespace Horde3D {

using namespace std;

// *************************************************************************************************
// Class SceneNode
// *************************************************************************************************

SceneNode::SceneNode( const SceneNodeTpl &tpl ) :
	_parent( 0x0 ), _type( tpl.type ), _handle( 0 ), _sgHandle( 0 ), _flags( 0 ), _sortKey( 0 ),
	_dirty( true ), _transformed( true ), _renderable( false ),
	_name( tpl.name ), _attachment( tpl.attachmentString )
{
	_relTrans = Matrix4f::ScaleMat( tpl.scale.x, tpl.scale.y, tpl.scale.z );
	_relTrans.rotate( degToRad( tpl.rot.x ), degToRad( tpl.rot.y ), degToRad( tpl.rot.z ) );
	_relTrans.translate( tpl.trans.x, tpl.trans.y, tpl.trans.z );
}


SceneNode::~SceneNode()
{
}


void SceneNode::getTransform( Vec3f &trans, Vec3f &rot, Vec3f &scale )
{
	if( _dirty ) Modules::sceneMan().updateNodes();
	
	_relTrans.decompose( trans, rot, scale );
	rot.x = radToDeg( rot.x );
	rot.y = radToDeg( rot.y );
	rot.z = radToDeg( rot.z );
}


void SceneNode::setTransform( Vec3f trans, Vec3f rot, Vec3f scale )
{
	// Hack to avoid making setTransform virtual
	if( _type == SceneNodeTypes::Joint )
	{
		((JointNode *)this)->_parentModel->_skinningDirty = true;
	}
	
	_relTrans = Matrix4f::ScaleMat( scale.x, scale.y, scale.z );
	_relTrans.rotate( degToRad( rot.x ), degToRad( rot.y ), degToRad( rot.z ) );
	_relTrans.translate( trans.x, trans.y, trans.z );
	
	markDirty();
}


void SceneNode::setTransform( const Matrix4f &mat )
{
	// Hack to avoid making setTransform virtual
	if( _type == SceneNodeTypes::Joint )
	{
		((JointNode *)this)->_parentModel->_skinningDirty = true;
	}
	
	_relTrans = mat;
	
	markDirty();
}


void SceneNode::getTransMatrices( const float **relMat, const float **absMat ) const
{
	if( relMat != 0x0 )
	{
		if( _dirty ) Modules::sceneMan().updateNodes();
		*relMat = &_relTrans.x[0];
	}
	
	if( absMat != 0x0 )
	{
		if( _dirty ) Modules::sceneMan().updateNodes();
		*absMat = &_absTrans.x[0];
	}
}


void SceneNode::setFlags( int flags, bool recursive )
{
	_flags = flags;

	if( recursive )
	{
		for( size_t i = 0, s = _children.size(); i < s; ++i )
		{
			_children[i]->setFlags( flags, true );
		}
	}
}


int SceneNode::getParamI( int param )
{
	Modules::setError( "Invalid param in h3dGetNodeParamI" );
	return Math::MinInt32;
}

void SceneNode::setParamI( int param, int value )
{
	Modules::setError( "Invalid param in h3dSetNodeParamI" );
}

float SceneNode::getParamF( int param, int compIdx )
{
	Modules::setError( "Invalid param in h3dGetNodeParamF" );
	return Math::NaN;
}

void SceneNode::setParamF( int param, int compIdx, float value )
{
	Modules::setError( "Invalid param in h3dSetNodeParamF" );
}

const char *SceneNode::getParamStr( int param )
{
	switch( param )
	{
	case SceneNodeParams::NameStr:
		return _name.c_str();
	case SceneNodeParams::AttachmentStr:
		return _attachment.c_str();
	}

	Modules::setError( "Invalid param in h3dGetNodeParamStr" );
	return "";
}

void SceneNode::setParamStr( int param, const char *value )
{
	switch( param )
	{
	case SceneNodeParams::NameStr:
		_name = value;
		return;
	case SceneNodeParams::AttachmentStr:
		_attachment = value;
		return;
	}

	Modules::setError( "Invalid param in h3dSetNodeParamStr" );
}


uint32 SceneNode::calcLodLevel( const Vec3f &viewPoint )
{
	return 0;
}


bool SceneNode::canAttach( SceneNode &/*parent*/ )
{
	return true;
}


void SceneNode::markChildrenDirty()
{	
	for( vector< SceneNode * >::iterator itr = _children.begin(),
	     end = _children.end(); itr != end; ++itr )
	{
		if( !(*itr)->_dirty )
		{	
			(*itr)->_dirty = true;
			(*itr)->_transformed = true;
			(*itr)->markChildrenDirty();
		}
	}
}


void SceneNode::markDirty()
{
	_dirty = true;
	_transformed = true;
	
	SceneNode *node = _parent;
	while( node != 0x0 )
	{
		node->_dirty = true;
		node = node->_parent;
	}

	markChildrenDirty();
}


void SceneNode::updateTree()
{
	if( !_dirty ) return;
	
	// Calculate absolute matrix
	if( _parent != 0x0 )
		Matrix4f::fastMult43( _absTrans, _parent->_absTrans, _relTrans );
	else
		_absTrans = _relTrans;
	
	Modules::sceneMan().updateSpatialNode( _sgHandle );

	onPostUpdate();

	_dirty = false;

	// Visit children
	for( uint32 i = 0, s = (uint32)_children.size(); i < s; ++i )
	{
		_children[i]->updateTree();
	}	

	onFinishedUpdate();
}


bool SceneNode::checkIntersection( const Vec3f &/*rayOrig*/, const Vec3f &/*rayDir*/, Vec3f &/*intsPos*/ ) const
{
	return false;
}


// *************************************************************************************************
// Class GroupNode
// *************************************************************************************************

GroupNode::GroupNode( const GroupNodeTpl &groupTpl ) :
	SceneNode( groupTpl )
{
}


SceneNodeTpl *GroupNode::parsingFunc( map< string, string > &attribs )
{
	map< string, string >::iterator itr;
	GroupNodeTpl *groupTpl = new GroupNodeTpl( "" );
	
	return groupTpl;
}


SceneNode *GroupNode::factoryFunc( const SceneNodeTpl &nodeTpl )
{
	if( nodeTpl.type != SceneNodeTypes::Group ) return 0x0;
	
	return new GroupNode( *(GroupNodeTpl *)&nodeTpl );
}


// =================================================================================================
// Class SpatialGraph
// =================================================================================================

SpatialGraph::SpatialGraph()
{
	_lightQueue.reserve( 20 );
	_renderQueue.reserve( 500 );
}


void SpatialGraph::addNode( SceneNode &sceneNode )
{	
	if( !sceneNode._renderable && sceneNode._type != SceneNodeTypes::Light ) return;
	
	if( !_freeList.empty() )
	{
		uint32 slot = _freeList.back();
		ASSERT( _nodes[slot] == 0x0 );
		_freeList.pop_back();

		sceneNode._sgHandle = slot + 1;
		_nodes[slot] = &sceneNode;
	}
	else
	{
		_nodes.push_back( &sceneNode );
		sceneNode._sgHandle = (uint32)_nodes.size();
	}
}


void SpatialGraph::removeNode( uint32 sgHandle )
{
	if( sgHandle == 0 || _nodes[sgHandle - 1] == 0x0 ) return;

	// Reset queues
	_lightQueue.resize( 0 );
	_renderQueue.resize( 0 );
	
	_nodes[sgHandle - 1]->_sgHandle = 0;
	_nodes[sgHandle - 1] = 0x0;
	_freeList.push_back( sgHandle - 1 );
}


void SpatialGraph::updateNode( uint32 sgHandle )
{
	// Since the spatial graph is just a flat list of objects,
	// there is nothing to do at the moment
}


struct RenderQueueItemCompFunc
{
	bool operator()( const RenderQueueItem &a, const RenderQueueItem &b ) const
		{ return a.sortKey < b.sortKey; }
};


void SpatialGraph::updateQueues( const Frustum &frustum1, const Frustum *frustum2, RenderingOrder::List order,
                                 uint32 filterIgnore, bool lightQueue, bool renderQueue )
{
	Modules::sceneMan().updateNodes();
	
	Vec3f camPos( frustum1.getOrigin() );
	if( Modules::renderer().getCurCamera() != 0x0 )
		camPos = Modules::renderer().getCurCamera()->getAbsPos();
	
	// Clear without affecting capacity
	if( lightQueue ) _lightQueue.resize( 0 );
	if( renderQueue ) _renderQueue.resize( 0 );

	// Culling
	for( size_t i = 0, s = _nodes.size(); i < s; ++i )
	{
		SceneNode *node = _nodes[i];
		if( node == 0x0 || (node->_flags & filterIgnore) ) continue;

		if( renderQueue && node->_renderable )
		{
			if( !frustum1.cullBox( node->_bBox ) &&
				(frustum2 == 0x0 || !frustum2->cullBox( node->_bBox )) )
			{
				if( node->_type == SceneNodeTypes::Mesh )  // TODO: Generalize and optimize this
				{
					uint32 curLod = ((MeshNode *)node)->getParentModel()->calcLodLevel( camPos );
					if( ((MeshNode *)node)->getLodLevel() != curLod ) continue;
				}
				
				float sortKey = 0;

				switch( order )
				{
				case RenderingOrder::StateChanges:
					sortKey = node->_sortKey;
					break;
				case RenderingOrder::FrontToBack:
					sortKey = nearestDistToAABB( frustum1.getOrigin(), node->_bBox.min, node->_bBox.max );
					break;
				case RenderingOrder::BackToFront:
					sortKey = -nearestDistToAABB( frustum1.getOrigin(), node->_bBox.min, node->_bBox.max );
					break;
				}
				
				_renderQueue.push_back( RenderQueueItem( node->_type, sortKey, node ) );
			}
		}
		else if( lightQueue && node->_type == SceneNodeTypes::Light )
		{
			_lightQueue.push_back( node );
		}
	}

	// Sort
	if( order != RenderingOrder::None )
		std::sort( _renderQueue.begin(), _renderQueue.end(), RenderQueueItemCompFunc() );
}


// *************************************************************************************************
// Class SceneManager
// *************************************************************************************************

SceneManager::SceneManager()
{
	SceneNode *rootNode = GroupNode::factoryFunc( GroupNodeTpl( "RootNode" ) );
	rootNode->_handle = RootNode;
	_nodes.push_back( rootNode );

	_spatialGraph = new SpatialGraph();
}


SceneManager::~SceneManager()
{
	delete _spatialGraph;

	for( uint32 i = 0; i < _nodes.size(); ++i )
	{
		delete _nodes[i]; _nodes[i] = 0x0;
	}
}


void SceneManager::registerNodeType( int nodeType, const string &typeString, NodeTypeParsingFunc pf,
                                     NodeTypeFactoryFunc ff )
{
	NodeRegEntry entry;
	entry.typeString = typeString;
	entry.parsingFunc = pf;
	entry.factoryFunc = ff;
	_registry[nodeType] = entry;
}


NodeRegEntry *SceneManager::findType( int type )
{
	map< int, NodeRegEntry >::iterator itr = _registry.find( type );
	
	if( itr != _registry.end() ) return &itr->second;
	else return 0x0;
}


NodeRegEntry *SceneManager::findType( const string &typeString )
{
	map< int, NodeRegEntry >::iterator itr = _registry.begin();

	while( itr != _registry.end() )
	{
		if( itr->second.typeString == typeString ) return &itr->second;

		++itr;
	}
	
	return 0x0;
}


void SceneManager::updateNodes()
{
	getRootNode().updateTree();
}


void SceneManager::updateQueues( const Frustum &frustum1, const Frustum *frustum2, RenderingOrder::List order,
                                 uint32 filterIgnore, bool lightQueue, bool renderableQueue )
{
	_spatialGraph->updateQueues( frustum1, frustum2, order, filterIgnore, lightQueue, renderableQueue );
}


NodeHandle SceneManager::parseNode( SceneNodeTpl &tpl, SceneNode *parent )
{
	if( parent == 0x0 ) return 0;
	
	SceneNode *sn = 0x0;

	if( tpl.type == 0 )
	{
		// Reference node
		NodeHandle handle = parseNode( *((ReferenceNodeTpl *)&tpl)->sgRes->getRootNode(), parent );
		sn = Modules::sceneMan().resolveNodeHandle( handle );
		if( sn != 0x0 )
		{	
			sn->_name = tpl.name;
			sn-> setTransform( tpl.trans, tpl.rot, tpl.scale );
			sn->_attachment = tpl.attachmentString;
		}
	}
	else
	{
		map< int, NodeRegEntry >::iterator itr = _registry.find( tpl.type );
		if( itr != _registry.end() ) sn = (*itr->second.factoryFunc)( tpl );
		if( sn != 0x0 ) addNode( sn, *parent );
	}

	if( sn == 0x0 ) return 0;

	// Parse children
	for( uint32 i = 0; i < tpl.children.size(); ++i )
	{
		parseNode( *tpl.children[i], sn );
	}

	return sn->getHandle();
}


NodeHandle SceneManager::addNode( SceneNode *node, SceneNode &parent )
{
	if( node == 0x0 ) return 0;
	
	// Check if node can be attached to parent
	if( !node->canAttach( parent ) )
	{
		Modules::log().writeDebugInfo( "Can't attach node '%s' to parent '%s'", node->_name.c_str(), parent._name.c_str() );
		delete node; node = 0x0;
		return 0;
	}
	
	node->_parent = &parent;
	
	// Attach to parent
	parent._children.push_back( node );

	// Raise event
	node->onAttach( parent );

	// Mark tree as dirty
	node->markDirty();

	// Register node in spatial graph
	_spatialGraph->addNode( *node );
	
	// Insert node in free slot
	if( !_freeList.empty() )
	{
		uint32 slot = _freeList.back();
		ASSERT( _nodes[slot] == 0x0 );
		_freeList.pop_back();

		node->_handle = slot + 1;
		_nodes[slot] = node;
		return slot + 1;
	}
	else
	{
		_nodes.push_back( node );
		node->_handle = (NodeHandle)_nodes.size();
		return node->_handle;
	}
}


NodeHandle SceneManager::addNodes( SceneNode &parent, SceneGraphResource &sgRes )
{
	// Parse root node
	return parseNode( *sgRes.getRootNode(), &parent );
}


void SceneManager::removeNodeRec( SceneNode &node )
{
	NodeHandle handle = node._handle;
	
	// Raise event
	if( handle != RootNode ) node.onDetach( *node._parent );

	// Remove children
	for( uint32 i = 0; i < node._children.size(); ++i )
	{
		removeNodeRec( *node._children[i] );
	}
	
	// Delete node
	if( handle != RootNode )
	{
		_spatialGraph->removeNode( node._sgHandle );
		delete _nodes[handle - 1]; _nodes[handle - 1] = 0x0;
		_freeList.push_back( handle - 1 );
	}
}


void SceneManager::removeNode( SceneNode &node )
{
	SceneNode *parent = node._parent;
	SceneNode *nodeAddr = &node;
	
	removeNodeRec( node );  // node gets deleted if it is not the rootnode
	
	// Remove node from parent
	if( parent != 0x0 )
	{
		// Find child
		for( uint32 i = 0; i < parent->_children.size(); ++i )
		{
			if( parent->_children[i] == nodeAddr )
			{
				parent->_children.erase( parent->_children.begin() + i );
				break;
			}
		}
		parent->markDirty();
	}
	else  // Rootnode
	{
		node._children.clear();
		node.markDirty();
	}
}


bool SceneManager::relocateNode( SceneNode &node, SceneNode &parent )
{
	if( node._handle == RootNode ) return false;
	
	if( !node.canAttach( parent ) )
	{	
		Modules::log().writeDebugInfo( "Can't attach node to parent in h3dSetNodeParent" );
		return false;
	}
	
	// Detach from old parent
	node.onDetach( *node._parent );
	for( uint32 i = 0; i < node._parent->_children.size(); ++i )
	{
		if( node._parent->_children[i] == &node )
		{
			node._parent->_children.erase( node._parent->_children.begin() + i );
			break;
		}
	}

	// Attach to new parent
	parent._children.push_back( &node );
	node._parent = &parent;
	node.onAttach( parent );
	
	parent.markDirty();
	node._parent->markDirty();
	
	return true;
}


int SceneManager::findNodes( SceneNode &startNode, const string &name, int type )
{
	int count = 0;
	
	if( type == SceneNodeTypes::Undefined || startNode._type == type )
	{
		if( name == "" || startNode._name == name )
		{
			_findResults.push_back( &startNode );
			++count;
		}
	}

	for( uint32 i = 0; i < startNode._children.size(); ++i )
	{
		count += findNodes( *startNode._children[i], name, type );
	}

	return count;
}


void SceneManager::castRayInternal( SceneNode &node )
{
	if( !(node._flags & SceneNodeFlags::NoRayQuery) )
	{
		Vec3f intsPos;
		if( node.checkIntersection( _rayOrigin, _rayDirection, intsPos ) )
		{
			float dist = (intsPos - _rayOrigin).length();

			CastRayResult crr;
			crr.node = &node;
			crr.distance = dist;
			crr.intersection = intsPos;

			bool inserted = false;
			for( vector< CastRayResult >::iterator it = _castRayResults.begin(); it != _castRayResults.end(); ++it )
			{
				if( dist < it->distance )
				{
					_castRayResults.insert( it, crr );
					inserted = true;
					break;
				}
			}

			if( !inserted )
			{
				_castRayResults.push_back( crr );
			}

			if( _rayNum > 0 && (int)_castRayResults.size() > _rayNum )
			{
				_castRayResults.pop_back();
			}
		}

		for( size_t i = 0, s = node._children.size(); i < s; ++i )
		{
			castRayInternal( *node._children[i] );
		}
	}
}


int SceneManager::castRay( SceneNode &node, const Vec3f &rayOrig, const Vec3f &rayDir, int numNearest )
{
	_castRayResults.resize( 0 );  // Clear without affecting capacity

	if( node._flags & SceneNodeFlags::NoRayQuery ) return 0;

	_rayOrigin = rayOrig;
	_rayDirection = rayDir;
	_rayNum = numNearest;

	castRayInternal( node );

	return (int)_castRayResults.size();
}


bool SceneManager::getCastRayResult( int index, CastRayResult &crr )
{
	if( (uint32)index < _castRayResults.size() )
	{
		crr = _castRayResults[index];

		return true;
	}

	return false;
}


int SceneManager::checkNodeVisibility( SceneNode &node, CameraNode &cam, bool checkOcclusion, bool calcLod )
{
	// Note: This function is a bit hacky with all the hard-coded node types
	
	if( node._dirty ) updateNodes();

	// Check occlusion
	if( checkOcclusion && cam._occSet >= 0 )
	{
		if( node.getType() == SceneNodeTypes::Mesh && cam._occSet < (int)((MeshNode *)&node)->_occQueries.size() )
		{
			if( gRDI->getQueryResult( ((MeshNode *)&node)->_occQueries[cam._occSet] ) < 1 )
				return -1;
		}
		else if( node.getType() == SceneNodeTypes::Emitter && cam._occSet < (int)((EmitterNode *)&node)->_occQueries.size() )
		{
			if( gRDI->getQueryResult( ((EmitterNode *)&node)->_occQueries[cam._occSet] ) < 1 )
				return -1;
		}
		else if( node.getType() == SceneNodeTypes::Light && cam._occSet < (int)((LightNode *)&node)->_occQueries.size() )
		{
			if( gRDI->getQueryResult( ((LightNode *)&node)->_occQueries[cam._occSet] ) < 1 )
				return -1;
		}
	}
	
	// Frustum culling
	if( cam.getFrustum().cullBox( node.getBBox() ) )
		return -1;
	else if( calcLod )
		return node.calcLodLevel( cam.getAbsPos() );
	else
		return 0;
}

}  // namespace
