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

#ifndef _egSceneGraphRes_H_
#define _egSceneGraphRes_H_

#include "egPrerequisites.h"
#include "egScene.h"
#include "utMath.h"
#include "egParticle.h"


namespace Horde3D {

class XMLNode;


// =================================================================================================
// SceneGraph Resource
// =================================================================================================

class SceneGraphResource : public Resource
{
public:
	static Resource *factoryFunc( const std::string &name, int flags )
		{ return new SceneGraphResource( name, flags ); }
	
	SceneGraphResource( const std::string &name, int flags );
	~SceneGraphResource();
	
	void initDefault();
	void release();
	bool load( const char *data, int size );

	SceneNodeTpl *getRootNode() { return _rootNode; }

private:
	void parseBaseAttributes( XMLNode &xmlNode, SceneNodeTpl &nodeTpl );
	void parseNode( XMLNode &xmlNode, SceneNodeTpl *parentTpl );

private:
	SceneNodeTpl	*_rootNode;

	friend class SceneManager;
};

typedef SmartResPtr< SceneGraphResource > PSceneGraphResource;

// =================================================================================================

struct ReferenceNodeTpl : public SceneNodeTpl
{
	PSceneGraphResource		sgRes;

	ReferenceNodeTpl( const std::string &name, SceneGraphResource *sgRes ) :
		SceneNodeTpl( 0, name ), sgRes( sgRes )
	{
	}
};

}
#endif // _egSceneGraphRes_H_
