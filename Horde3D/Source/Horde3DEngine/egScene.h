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

#ifndef _egScene_H_
#define _egScene_H_

#include "egPrerequisites.h"
#include "utMath.h"
#include "egPrimitives.h"
#include "egPipeline.h"
#include <map>


namespace Horde3D {

struct SceneNodeTpl;
class CameraNode;
class SceneGraphResource;


const int RootNode = 1;


// =================================================================================================
// Scene Node
// =================================================================================================

struct SceneNodeTypes
{
	enum List
	{
		Undefined = 0,
		Group,
		Model,
		Mesh,
		Joint,
		Light,
		Camera,
		Emitter
	};
};

struct SceneNodeParams
{
	enum List
	{
		NameStr = 1,
		AttachmentStr
	};
};

struct SceneNodeFlags
{
	enum List
	{
		NoDraw = 0x1,
		NoCastShadow = 0x2,
		NoRayQuery = 0x4,
		Inactive = 0x7  // NoDraw | NoCastShadow | NoRayQuery
	};
};

// =================================================================================================

struct SceneNodeTpl
{
	int                            type;
	std::string                    name;
	Vec3f                          trans, rot, scale;
	std::string                    attachmentString;
	std::vector< SceneNodeTpl * >  children;

	SceneNodeTpl( int type, const std::string &name ) :
		type( type ), name( name ), scale( Vec3f ( 1, 1, 1 ) )
	{
	}
	
	virtual ~SceneNodeTpl()
	{
		for( uint32 i = 0; i < children.size(); ++i ) delete children[i];
	}
};

// =================================================================================================

class SceneNode
{
public:
	SceneNode( const SceneNodeTpl &tpl );
	virtual ~SceneNode();

	void getTransform( Vec3f &trans, Vec3f &rot, Vec3f &scale );	// Not virtual for performance
	void setTransform( Vec3f trans, Vec3f rot, Vec3f scale );	// Not virtual for performance
	void setTransform( const Matrix4f &mat );
	void getTransMatrices( const float **relMat, const float **absMat ) const;

	int getFlags() { return _flags; }
	void setFlags( int flags, bool recursive );

	virtual int getParamI( int param );
	virtual void setParamI( int param, int value );
	virtual float getParamF( int param, int compIdx );
	virtual void setParamF( int param, int compIdx, float value );
	virtual const char *getParamStr( int param );
	virtual void setParamStr( int param, const char* value );

	virtual uint32 calcLodLevel( const Vec3f &viewPoint );

	virtual bool canAttach( SceneNode &parent );
	void markDirty();
	void updateTree();
	virtual bool checkIntersection( const Vec3f &rayOrig, const Vec3f &rayDir, Vec3f &intsPos ) const;

	virtual void setCustomInstData( float *data, uint32 count ) {}

	int getType() { return _type; };
	NodeHandle getHandle() { return _handle; }
	SceneNode *getParent() { return _parent; }
	const std::string &getName() { return _name; }
	std::vector< SceneNode * > &getChildren() { return _children; }
	Matrix4f &getRelTrans() { return _relTrans; }
	Matrix4f &getAbsTrans() { return _absTrans; }
	BoundingBox &getBBox() { return _bBox; }
	const std::string &getAttachmentString() { return _attachment; }
	void setAttachmentString( const char* attachmentData ) { _attachment = attachmentData; }
	bool checkTransformFlag( bool reset )
		{ bool b = _transformed; if( reset ) _transformed = false; return b; }

protected:
	void markChildrenDirty();

	virtual void onPostUpdate() {}  // Called after absolute transformation has been updated
	virtual void onFinishedUpdate() {}  // Called after children have been updated
	virtual void onAttach( SceneNode &parentNode ) {}  // Called when node is attached to parent
	virtual void onDetach( SceneNode &parentNode ) {}  // Called when node is detached from parent

protected:
	Matrix4f                    _relTrans, _absTrans;  // Transformation matrices
	SceneNode                   *_parent;  // Parent node
	int                         _type;
	NodeHandle                  _handle;
	uint32                      _sgHandle;  // Spatial graph handle
	uint32                      _flags;
	float                       _sortKey;
	bool                        _dirty;  // Does the node need to be updated?
	bool                        _transformed;
	bool                        _renderable;

	BoundingBox                 _bBox;  // AABB in world space

	std::vector< SceneNode * >  _children;  // Child nodes
	std::string                 _name;
	std::string                 _attachment;  // User defined data

	friend class SceneManager;
	friend class SpatialGraph;
	friend class Renderer;
};


// =================================================================================================
// Group Node
// =================================================================================================

struct GroupNodeTpl : public SceneNodeTpl
{
	GroupNodeTpl( const std::string &name ) :
		SceneNodeTpl( SceneNodeTypes::Group, name )
	{
	}
};

// =================================================================================================

class GroupNode : public SceneNode
{
public:
	static SceneNodeTpl *parsingFunc( std::map< std::string, std::string > &attribs );
	static SceneNode *factoryFunc( const SceneNodeTpl &nodeTpl );

	friend class Renderer;
	friend class SceneManager;

protected:
	GroupNode( const GroupNodeTpl &groupTpl );
};


// =================================================================================================
// Spatial Graph
// =================================================================================================

struct RenderQueueItem
{
	SceneNode  *node;
	int        type;  // Type is stored explicitly for better cache efficiency when iterating over list
	float      sortKey;

	RenderQueueItem() {}
	RenderQueueItem( int type, float sortKey, SceneNode *node )
		: node( node ), type( type ), sortKey( sortKey ) {}
};

typedef std::vector< RenderQueueItem > RenderQueue;


class SpatialGraph
{
public:
	SpatialGraph();
	
	void addNode( SceneNode &sceneNode );
	void removeNode( uint32 sgHandle );
	void updateNode( uint32 sgHandle );

	void updateQueues( const Frustum &frustum1, const Frustum *frustum2,
	                   RenderingOrder::List order, uint32 filterIgnore, bool lightQueue, bool renderQueue );

	std::vector< SceneNode * > &getLightQueue() { return _lightQueue; }
	RenderQueue &getRenderQueue() { return _renderQueue; }

protected:
	std::vector< SceneNode * >     _nodes;		// Renderable nodes and lights
	std::vector< uint32 >          _freeList;
	std::vector< SceneNode * >     _lightQueue;
	RenderQueue                    _renderQueue;
};


// =================================================================================================
// Scene Manager
// =================================================================================================

typedef SceneNodeTpl *(*NodeTypeParsingFunc)( std::map< std::string, std::string > &attribs );
typedef SceneNode *(*NodeTypeFactoryFunc)( const SceneNodeTpl &tpl );

struct NodeRegEntry
{
	std::string          typeString;
	NodeTypeParsingFunc  parsingFunc;
	NodeTypeFactoryFunc  factoryFunc;
};

struct CastRayResult
{
	SceneNode  *node;
	float      distance;
	Vec3f      intersection;
};

// =================================================================================================

class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	void registerNodeType( int nodeType, const std::string &typeString, NodeTypeParsingFunc pf,
	                       NodeTypeFactoryFunc ff );
	NodeRegEntry *findType( int type );
	NodeRegEntry *findType( const std::string &typeString );
	
	void updateNodes();
	void updateSpatialNode( uint32 sgHandle ) { _spatialGraph->updateNode( sgHandle ); }
	void updateQueues( const Frustum &frustum1, const Frustum *frustum2,
	                   RenderingOrder::List order, uint32 filterIgnore, bool lightQueue, bool renderableQueue );
	
	NodeHandle addNode( SceneNode *node, SceneNode &parent );
	NodeHandle addNodes( SceneNode &parent, SceneGraphResource &sgRes );
	void removeNode( SceneNode &node );
	bool relocateNode( SceneNode &node, SceneNode &parent );
	
	int findNodes( SceneNode &startNode, const std::string &name, int type );
	void clearFindResults() { _findResults.resize( 0 ); }
	SceneNode *getFindResult( int index ) { return (unsigned)index < _findResults.size() ? _findResults[index] : 0x0; }
	
	int castRay( SceneNode &node, const Vec3f &rayOrig, const Vec3f &rayDir, int numNearest );
	bool getCastRayResult( int index, CastRayResult &crr );

	int checkNodeVisibility( SceneNode &node, CameraNode &cam, bool checkOcclusion, bool calcLod );

	SceneNode &getRootNode() { return *_nodes[0]; }
	SceneNode &getDefCamNode() { return *_nodes[1]; }
	std::vector< SceneNode * > &getLightQueue() { return _spatialGraph->getLightQueue(); }
	RenderQueue &getRenderQueue() { return _spatialGraph->getRenderQueue(); }
	
	SceneNode *resolveNodeHandle( NodeHandle handle )
		{ return (handle != 0 && (unsigned)(handle - 1) < _nodes.size()) ? _nodes[handle - 1] : 0x0; }

protected:
	NodeHandle parseNode( SceneNodeTpl &tpl, SceneNode *parent );
	void removeNodeRec( SceneNode &node );

	void castRayInternal( SceneNode &node );

protected:
	std::vector< SceneNode *>      _nodes;  // _nodes[0] is root node
	std::vector< uint32 >          _freeList;  // List of free slots
	std::vector< SceneNode * >     _findResults;
	std::vector< CastRayResult >   _castRayResults;
	SpatialGraph                   *_spatialGraph;

	std::map< int, NodeRegEntry >  _registry;  // Registry of node types

	Vec3f                          _rayOrigin;  // Don't put these values on the stack during recursive search
	Vec3f                          _rayDirection;  // Ditto
	int                            _rayNum;  // Ditto

	friend class Renderer;
};

}
#endif // _egScene_H_
