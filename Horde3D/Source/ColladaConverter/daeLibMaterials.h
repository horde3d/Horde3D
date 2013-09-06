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

#ifndef _daeLibMaterials_H_
#define _daeLibMaterials_H_

#include "utXML.h"
#include "daeLibEffects.h"
#include <string>
#include <vector>

using namespace Horde3D;


struct DaeMaterial
{
	std::string  id;
	std::string  name;
	std::string  effectId;
	DaeEffect    *effect;
	bool         used;
	

	DaeMaterial()
	{
		used = false;
		effect = 0x0;
	}
	

	bool parse( const XMLNode &matNode )
	{
		id = matNode.getAttribute( "id", "unnamed" );
		name = matNode.getAttribute( "name", "" );
		if( name.empty() ) name = id;
		
		XMLNode node1 = matNode.getFirstChild( "instance_effect" );
		if( !node1.isEmpty() )
		{
			effectId = node1.getAttribute( "url", "" );
			removeGate( effectId );
		}
		
		return true;
	}
};


struct DaeLibMaterials
{
	std::vector< DaeMaterial * >  materials;


	~DaeLibMaterials()
	{
		for( unsigned int i = 0; i < materials.size(); ++i ) delete materials[i];
	}


	DaeMaterial *findMaterial( const std::string &id )
	{
		if( id == "" ) return 0x0;
		
		for( unsigned int i = 0; i < materials.size(); ++i )
		{
			if( materials[i]->id == id ) return materials[i];
		}

		return 0x0;
	}


	bool parse( const XMLNode &rootNode )
	{
		XMLNode node1 = rootNode.getFirstChild( "library_materials" );
		if( node1.isEmpty() ) return true;

		XMLNode node2 = node1.getFirstChild( "material" );
		while( !node2.isEmpty() )
		{
			DaeMaterial *material = new DaeMaterial();
			if( material->parse( node2 ) ) materials.push_back( material );
			else delete material;

			node2 = node2.getNextSibling( "material" );
		}
		
		return true;
	}
};

#endif // _daeLibMaterials_H_
