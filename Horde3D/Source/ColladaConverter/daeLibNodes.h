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

#ifndef _daeLibNodes_H_
#define _daeLibNodes_H_

#include "utXML.h"
#include <string>
#include <vector>
#include "daeLibVisualScenes.h"

using namespace Horde3D;


struct DaeLibNodes
{
	std::vector< DaeNode * >  nodes;
	std::string               id;
	std::string               name;

	~DaeLibNodes()
	{
		for( unsigned int i = 0; i < nodes.size(); ++i ) delete nodes[i];
	}


	DaeNode *findNode( const std::string &id )
	{
		if( id == "" ) return 0x0;
		
		for( unsigned int i = 0; i < nodes.size(); ++i )
		{
			if( nodes[i]->id == id ) return nodes[i];
		}

		return 0x0;
	}

	
	bool parse( const XMLNode &rootNode )
	{
		XMLNode node1 = rootNode.getFirstChild( "library_nodes" );
		if( node1.isEmpty() ) return true;

		XMLNode node2 = node1.getFirstChild( "node" );
		while( !node2.isEmpty() )
		{
			DaeNode *node = new DaeNode();
			if( node->parse( node2 ) ) nodes.push_back( node );
			else delete node;

			node2 = node2.getNextSibling( "node" );
		}
		
		return true;
	}

};
#endif
