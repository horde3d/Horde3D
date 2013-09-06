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

#ifndef _egAnimatables_H_
#define _egAnimatables_H_

#include "egPrerequisites.h"
#include "egScene.h"
#include "egAnimation.h"
#include "utMath.h"


namespace Horde3D {

class MaterialResource;
class ModelNode;


// =================================================================================================
// Mesh Node
// =================================================================================================

struct MeshNodeParams
{
	enum List
	{
		MatResI = 300,
		BatchStartI,
		BatchCountI,
		VertRStartI,
		VertREndI,
		LodLevelI
	};
};

// =================================================================================================

struct MeshNodeTpl : public SceneNodeTpl
{
	PMaterialResource  matRes;
	uint32             batchStart, batchCount;
	uint32             vertRStart, vertREnd;
	uint32             lodLevel;

	MeshNodeTpl( const std::string &name, MaterialResource *materialRes, uint32 batchStart,
	             uint32 batchCount, uint32 vertRStart, uint32 vertREnd ) :
		SceneNodeTpl( SceneNodeTypes::Mesh, name ), matRes( materialRes ), batchStart( batchStart ),
		batchCount( batchCount ), vertRStart( vertRStart ), vertREnd( vertREnd ), lodLevel( 0 )
	{
	}
};

// =================================================================================================

class MeshNode : public SceneNode, public IAnimatableNode
{
public:
	static SceneNodeTpl *parsingFunc( std::map< std::string, std::string > &attribs );
	static SceneNode *factoryFunc( const SceneNodeTpl &nodeTpl );

	// IAnimatableNode
	const std::string &getANName() { return _name; }
	Matrix4f &getANRelTransRef() { return _relTrans; }
	IAnimatableNode *getANParent();
	
	bool canAttach( SceneNode &parent );
	int getParamI( int param );
	void setParamI( int param, int value );
	bool checkIntersection( const Vec3f &rayOrig, const Vec3f &rayDir, Vec3f &intsPos ) const;

	void onAttach( SceneNode &parentNode );
	void onDetach( SceneNode &parentNode );
	void onPostUpdate();

	MaterialResource *getMaterialRes() { return _materialRes; }
	uint32 getBatchStart() { return _batchStart; }
	uint32 getBatchCount() { return _batchCount; }
	uint32 getVertRStart() { return _vertRStart; }
	uint32 getVertREnd() { return _vertREnd; }
	uint32 getLodLevel() { return _lodLevel; }
	ModelNode *getParentModel() { return _parentModel; }

protected:
	MeshNode( const MeshNodeTpl &meshTpl );
	~MeshNode();

protected:
	PMaterialResource   _materialRes;
	uint32              _batchStart, _batchCount;
	uint32              _vertRStart, _vertREnd;
	uint32              _lodLevel;
	
	ModelNode           *_parentModel;
	BoundingBox         _localBBox;

	std::vector< uint32 >  _occQueries;
	std::vector< uint32 >  _lastVisited;

	friend class SceneManager;
	friend class SceneNode;
	friend class ModelNode;
	friend class Renderer;
};


// =================================================================================================
// Joint Node
// =================================================================================================

struct JointNodeParams
{
	enum List
	{
		JointIndexI = 400
	};
};

// =================================================================================================

struct JointNodeTpl : public SceneNodeTpl
{
	uint32  jointIndex;

	JointNodeTpl( const std::string &name, uint32 jointIndex ) :
		SceneNodeTpl( SceneNodeTypes::Joint, name ), jointIndex( jointIndex )
	{
	}
};

// =================================================================================================

class JointNode : public SceneNode, public IAnimatableNode
{
public:
	static SceneNodeTpl *parsingFunc( std::map< std::string, std::string > &attribs );
	static SceneNode *factoryFunc( const SceneNodeTpl &nodeTpl );
	
	// IAnimatableNode
	const std::string &getANName() { return _name; }
	Matrix4f &getANRelTransRef() { return _relTrans; }
	IAnimatableNode *getANParent();
	
	bool canAttach( SceneNode &parent );
	int getParamI( int param );

	void onPostUpdate();
	void onAttach( SceneNode &parentNode );
	void onDetach( SceneNode &parentNode );

protected:
	JointNode( const JointNodeTpl &jointTpl );

protected:
	uint32     _jointIndex;
	
	ModelNode  *_parentModel;
	Matrix4f   _relModelMat;  // Transformation relative to parent model

	friend class SceneNode;
	friend class ModelNode;
};

}
#endif // _egAnimatables_H_
