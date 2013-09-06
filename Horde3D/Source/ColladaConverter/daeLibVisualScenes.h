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

#ifndef _daeLibVisualScenes_H_
#define _daeLibVisualScenes_H_

#include "utXML.h"
#include "daeLibAnimations.h"
#include <string>
#include <vector>
#include <map>

using namespace Horde3D;


struct DaeTransformation
{
	enum Type { MATRIX, TRANSLATION, ROTATION, SCALE };
	std::string   sid;
	unsigned int  type;
	float         values[16];
	float         animValues[16];  // Temporary animation values
};


struct DaeInstance
{
	std::string                           url;
	std::map< std::string, std::string >  materialBindings;
};


struct DaeNode
{
	std::string                       id, sid;
	std::string                       name;
	bool                              joint;
	bool                              reference;

	std::vector< DaeTransformation >  transStack;
	std::vector< DaeNode * >          children;
	std::vector< DaeInstance >        instances;

	~DaeNode()
	{
		for( unsigned int i = 0; i < children.size(); ++i ) delete children[i];
	}

	bool parse( const XMLNode &nodeNode )
	{
		reference = false;	
		id = nodeNode.getAttribute( "id", "" );
		name = nodeNode.getAttribute( "name", "" );
		if( name.empty() ) name = id;
		sid = nodeNode.getAttribute( "sid", id.c_str() );
		
		if( strcmp( nodeNode.getAttribute( "type", "" ), "JOINT" ) == 0 ) joint = true;
		else joint = false;
		
		// Parse transformations
		XMLNode node1 = nodeNode.getFirstChild();
		while( !node1.isEmpty() )
		{
			if( node1.getName() == 0x0 ) continue;

			DaeTransformation trans;
			trans.sid = node1.getAttribute( "sid", "" );
			
			if( strcmp( node1.getName(), "matrix" ) == 0 )
			{
				trans.type = DaeTransformation::MATRIX;

				char *str = (char *)node1.getText();
				if( str == 0x0 ) return false;
				for( int i = 0; i < 16; ++i )
				{
					float f;
					parseFloat( str, f );
					trans.values[i] = f;
				}

				memcpy( trans.animValues, trans.values, 16 * sizeof( float ) );
				transStack.push_back( trans );
			}
			else if( strcmp( node1.getName(), "translate" ) == 0 )
			{
				trans.type = DaeTransformation::TRANSLATION;

				char *str = (char *)node1.getText();
				if( str == 0x0 ) continue;
				parseFloat( str, trans.values[0] );
				parseFloat( str, trans.values[1] );
				parseFloat( str, trans.values[2] );

				memcpy( trans.animValues, trans.values, 16 * sizeof( float ) );
				transStack.push_back( trans );
			}
			else if( strcmp( node1.getName(), "rotate" ) == 0 )
			{
				trans.type = DaeTransformation::ROTATION;

				char *str = (char *)node1.getText();
				if( str == 0x0 ) continue;
				parseFloat( str, trans.values[0] );
				parseFloat( str, trans.values[1] );
				parseFloat( str, trans.values[2] );
				parseFloat( str, trans.values[3] );

				memcpy( trans.animValues, trans.values, 16 * sizeof( float ) );
				transStack.push_back( trans );
			}
			else if( strcmp( node1.getName(), "scale" ) == 0 )
			{
				trans.type = DaeTransformation::SCALE;

				char *str = (char *)node1.getText();
				if( str == 0x0 ) continue;
				parseFloat( str, trans.values[0] );
				parseFloat( str, trans.values[1] );
				parseFloat( str, trans.values[2] );

				memcpy( trans.animValues, trans.values, 16 * sizeof( float ) );
				transStack.push_back( trans );
			}
			else if( strcmp( node1.getName(), "skew" ) == 0 )
			{
				log( "Warning: Unsupported transformation type" );
			}

			node1 = node1.getNextSibling();
		}

		// Parse instances
		node1 = nodeNode.getFirstChild();
		while( !node1.isEmpty() && node1.getName() != 0x0 )
		{
			if( strcmp( node1.getName(), "instance_node" ) == 0 )
			{
				std::string url = node1.getAttribute( "url", "" );
				removeGate( url );
				if( !url.empty() )
				{
					DaeNode *n = new DaeNode();
					n->name = url;
					n->reference = true;
					children.push_back( n );
				}
			}
			else if( strcmp( node1.getName(), "instance_geometry" ) == 0 ||
			         strcmp( node1.getName(), "instance_controller" ) == 0 )
			{
				std::string url = node1.getAttribute( "url", "" );
				removeGate( url );

				if( url != "" )
				{
					instances.push_back( DaeInstance() );
					DaeInstance &inst = instances.back();
					
					inst.url = url;

					// Find material bindings
					XMLNode node2 = node1.getFirstChild( "bind_material" );
					if( !node2.isEmpty() )
					{
						XMLNode node3 = node2.getFirstChild( "technique_common" );
						if( !node3.isEmpty() )
						{
							XMLNode node4 = node3.getFirstChild( "instance_material" );
							while( !node4.isEmpty() )
							{
								std::string s = node4.getAttribute( "target", "" );
								removeGate( s );
								inst.materialBindings[node4.getAttribute( "symbol", "" )] = s;

								node4 = node4.getNextSibling( "instance_material" );
							}
						}
					}
				}
			}
			
			node1 = node1.getNextSibling();
		}

		// Parse children
		node1 = nodeNode.getFirstChild( "node" );
		while( !node1.isEmpty() )
		{
			DaeNode *node = new DaeNode();
			if( node->parse( node1 ) ) children.push_back( node );
			else delete node;

			node1 = node1.getNextSibling( "node" );
		}
		
		return true;
	}

	
	Matrix4f assembleMatrix()
	{
		Matrix4f mat;
		
		for( unsigned int i = 0; i < transStack.size(); ++i )
		{
			switch( transStack[i].type )
			{
			case DaeTransformation::MATRIX:
				mat = mat * Matrix4f( transStack[i].values ).transposed();
				break;
			case DaeTransformation::TRANSLATION:
				mat = mat * Matrix4f::TransMat( transStack[i].values[0], transStack[i].values[1],
				                                transStack[i].values[2] );
				break;
			case DaeTransformation::ROTATION:
				mat = mat * Matrix4f::RotMat( Vec3f( transStack[i].values[0], transStack[i].values[1],
				                              transStack[i].values[2] ), degToRad( transStack[i].values[3] ) );
				break;
			case DaeTransformation::SCALE:
				mat = mat * Matrix4f::ScaleMat( transStack[i].values[0], transStack[i].values[1],
				                                transStack[i].values[2] );
				break;
			}
		}
		
		return mat;
	}


	Matrix4f assembleAnimMatrix()
	{
		Matrix4f mat;
		
		for( unsigned int i = 0; i < transStack.size(); ++i )
		{
			switch( transStack[i].type )
			{
			case DaeTransformation::MATRIX:
				mat = mat * Matrix4f( transStack[i].animValues ).transposed();
				break;
			case DaeTransformation::TRANSLATION:
				mat = mat * Matrix4f::TransMat( transStack[i].animValues[0], transStack[i].animValues[1],
				                                transStack[i].animValues[2] );
				break;
			case DaeTransformation::ROTATION:
				mat = mat * Matrix4f::RotMat( Vec3f( transStack[i].animValues[0], transStack[i].animValues[1],
				                              transStack[i].animValues[2] ), degToRad( transStack[i].animValues[3] ) );
				break;
			case DaeTransformation::SCALE:
				mat = mat * Matrix4f::ScaleMat( transStack[i].animValues[0], transStack[i].animValues[1],
				                                transStack[i].animValues[2] );
				break;
			}
		}
		
		return mat;
	}
};


struct DaeVisualScene
{
	std::string               id;
	std::vector< DaeNode * >  nodes;

	
	~DaeVisualScene()
	{
		for( unsigned int i = 0; i < nodes.size(); ++i ) delete nodes[i];
	}


	bool parse( const XMLNode &visSceneNode )
	{
		id = visSceneNode.getAttribute( "id", "" );
		if( id == "" ) return false;
		
		XMLNode node1 = visSceneNode.getFirstChild( "node" );
		while( !node1.isEmpty() )
		{
			DaeNode *node = new DaeNode();
			if( node->parse( node1 ) ) nodes.push_back( node );
			else delete node;

			node1 = node1.getNextSibling( "node" );
		}
		
		return true;
	}
};


struct DaeLibVisScenes
{
	std::vector< DaeVisualScene * >	visScenes;


	~DaeLibVisScenes()
	{
		for( unsigned int i = 0; i < visScenes.size(); ++i ) delete visScenes[i];
	}
	

	DaeVisualScene *findVisualScene( const std::string &id )
	{
		if( id == "" ) return 0x0;
		
		for( unsigned int i = 0; i < visScenes.size(); ++i )
		{
			if( visScenes[i]->id == id ) return visScenes[i];
		}

		return 0x0;
	}
	
	
	bool parse( const XMLNode &rootNode )
	{
		XMLNode node1 = rootNode.getFirstChild( "library_visual_scenes" );
		if( node1.isEmpty() ) return true;

		XMLNode node2 = node1.getFirstChild( "visual_scene" );
		while( !node2.isEmpty() )
		{
			DaeVisualScene *visScene = new DaeVisualScene();

			if( visScene->parse( node2 ) ) visScenes.push_back( visScene );
			else delete visScene;

			node2 = node2.getNextSibling( "visual_scene" );
		}
		
		return true;
	}
};

#endif // _daeLibVisualScenes_H_
