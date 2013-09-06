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

#ifndef _daeLibControllers_H_
#define _daeLibControllers_H_

#include "utXML.h"
#include <string>
#include <vector>

using namespace Horde3D;


struct DaeWeight
{
	int  joint;
	int  weight;
};


typedef std::vector< DaeWeight > DaeVertWeights;


struct DaeSkin
{
	std::string                    id;
	std::string                    ownerId;
	float                          bindShapeMat[16];
	std::vector< DaeSource >       sources;
	DaeSource                      *jointArray;
	DaeSource                      *weightArray;
	DaeSource                      *bindMatArray;
	std::vector< DaeVertWeights >  vertWeights;

	
	DaeSkin()
	{
		jointArray = 0x0;
		weightArray = 0x0;
		bindMatArray = 0x0;
	}
	

	DaeSource *findSource( const std::string &id )
	{
		if( id == "" ) return 0x0;
		
		for( unsigned int i = 0; i < sources.size(); ++i )
		{
			if( sources[i].id == id ) return &sources[i];
		}

		return 0x0;
	}

	
	bool parse( const XMLNode &ctrlNode )
	{
		XMLNode node1 = ctrlNode.getFirstChild( "skin" );
		if( node1.isEmpty() ) return false;
		
		id = ctrlNode.getAttribute( "id", "" );
		if( id == "" ) return false;
		ownerId = node1.getAttribute( "source", "" );
		removeGate( ownerId );

		// Bind shape matrix
		XMLNode node2 = node1.getFirstChild( "bind_shape_matrix" );
		if( node2.isEmpty() ) return false;

		char *str = (char *)node2.getText();
		if( str == 0x0 ) return false;
		for( int i = 0; i < 16; ++i )
		{
			float f;
			parseFloat( str, f );
			bindShapeMat[i] = f;
		}
		
		// Sources
		node2 = node1.getFirstChild( "source" );
		while( !node2.isEmpty() )
		{
			sources.push_back( DaeSource() );
			if( !sources.back().parse( node2 ) ) sources.pop_back();
			
			node2 = node2.getNextSibling( "source" );
		}

		// Joints
		node2 = node1.getFirstChild( "joints" );
		if( !node2.isEmpty() )
		{
			XMLNode node3 = node2.getFirstChild( "input" );
			while( !node3.isEmpty() )
			{
				if( strcmp( node3.getAttribute( "semantic", "" ), "JOINT" ) == 0 )
				{
					std::string id = node3.getAttribute( "source", "" );
					removeGate( id );
					jointArray = findSource( id );
				}
				else if( strcmp( node3.getAttribute( "semantic", "" ), "INV_BIND_MATRIX" ) == 0 )
				{
					std::string sourceId = node3.getAttribute( "source", "" );
					removeGate( sourceId );
					bindMatArray = findSource( sourceId );
				}

				node3 = node3.getNextSibling( "input" );
			}
		}
		
		// Vertex weights
		unsigned int jointOffset = 0, weightOffset = 0;
		unsigned int numInputs = 0;
		
		node2 = node1.getFirstChild( "vertex_weights" );
		int count = atoi( node2.getAttribute( "count", "0" ) );
		XMLNode node3 = node2.getFirstChild( "input" );
		while( !node3.isEmpty() )
		{
			++numInputs;

			if( strcmp( node3.getAttribute( "semantic", "" ), "JOINT" ) == 0 )
			{
				jointOffset = atoi( node3.getAttribute( "offset", "0" ) );
				
				std::string id = node3.getAttribute( "source", "" );
				removeGate( id );
				DaeSource *vertJointArray = findSource( id );
				if( jointOffset != 0 || vertJointArray->stringArray != jointArray->stringArray )
					log( "Warning: Vertex weight joint array doesn't match skin joint array" );
			}
			else if( strcmp( node3.getAttribute( "semantic", "" ), "WEIGHT" ) == 0 )
			{
				weightOffset = atoi( node3.getAttribute( "offset", "0" ) );
				std::string id = node3.getAttribute( "source", "" );
				removeGate( id );
				weightArray = findSource( id );
			}
			
			node3 = node3.getNextSibling( "input" );
		}

		node3 = node2.getFirstChild( "vcount" );
		str = (char *)node3.getText();
		if( str == 0x0 ) return false;
		for( int i = 0; i < count; ++i )
		{
			int si;
			parseInt( str, si );

			DaeVertWeights vertWeight;
			for( unsigned int j = 0; j < (unsigned)si; ++j )
			{
				vertWeight.push_back( DaeWeight() );
			}

			vertWeights.push_back( vertWeight );
		}

		node3 = node2.getFirstChild( "v" );
		str = (char *)node3.getText();
		if( str == 0x0 ) return false;
		for( int i = 0; i < count; ++i )
		{
			for( unsigned int j = 0; j < vertWeights[i].size(); ++j )
			{
				for( unsigned int k = 0; k < numInputs; ++k )
				{
					int si;
					parseInt( str, si );

					if( k == jointOffset ) vertWeights[i][j].joint = si;
					if( k == weightOffset ) vertWeights[i][j].weight = si;
				}
			}
		}
		
		return true;
	}
};


struct DaeMorph
{
	std::string               id;
	std::string               ownerId;
	std::vector< DaeSource >  sources;
	DaeSource                 *targetArray;
	DaeSource                 *weightArray;


	DaeMorph()
	{
		targetArray = 0x0;
		weightArray = 0x0;
	}
		

	DaeSource *findSource( const std::string &id )
	{
		if( id == "" ) return 0x0;
		
		for( unsigned int i = 0; i < sources.size(); ++i )
		{
			if( sources[i].id == id ) return &sources[i];
		}

		return 0x0;
	}

	
	bool parse( const XMLNode &ctrlNode )
	{
		XMLNode node1 = ctrlNode.getFirstChild( "morph" );
		if( node1.isEmpty() ) return false;

		id = ctrlNode.getAttribute( "id", "" );
		if( id == "" ) return false;
		ownerId = node1.getAttribute( "source", "" );
		removeGate( ownerId );
		
		// Sources
		XMLNode node2 = node1.getFirstChild( "source" );
		while( !node2.isEmpty() )
		{
			sources.push_back( DaeSource() );
			if( !sources.back().parse( node2 ) ) sources.pop_back();
			
			node2 = node2.getNextSibling( "source" );
		}

		// Targets
		node2 = node1.getFirstChild( "targets" );
		if( !node2.isEmpty() )
		{
			XMLNode node3 = node2.getFirstChild( "input" );
			while( !node3.isEmpty() )
			{
				if( strcmp( node3.getAttribute( "semantic", "" ), "MORPH_TARGET" ) == 0 )
				{
					std::string sourceId = node3.getAttribute( "source", "" );
					removeGate( sourceId );
					targetArray = findSource( sourceId );
				}
				else if( strcmp( node3.getAttribute( "semantic", "" ), "MORPH_WEIGHT" ) == 0 )
				{
					std::string sourceId = node3.getAttribute( "source", "" );
					removeGate( sourceId );
					weightArray = findSource( sourceId );
				}

				node3 = node3.getNextSibling( "input" );
			}
		}

		return true;
	}
};


struct DaeLibControllers
{
	std::vector< DaeSkin * >   skinControllers;
	std::vector< DaeMorph * >  morphControllers;


	~DaeLibControllers()
	{
		for( unsigned int i = 0; i < skinControllers.size(); ++i ) delete skinControllers[i];
		for( unsigned int i = 0; i < morphControllers.size(); ++i ) delete morphControllers[i];
	}

	
	DaeSkin *findSkin( const std::string &id )
	{
		if( id == "" ) return 0x0;
		
		for( unsigned int i = 0; i < skinControllers.size(); ++i )
		{
			if( skinControllers[i]->id == id ) return skinControllers[i];
		}
		
		return 0x0;
	}


	DaeMorph *findMorph( const std::string &id )
	{
		if( id == "" ) return 0x0;
		
		for( unsigned int i = 0; i < morphControllers.size(); ++i )
		{
			if( morphControllers[i]->id == id ) return morphControllers[i];
		}
		
		return 0x0;
	}
	
	
	bool parse( const XMLNode &rootNode )
	{
		XMLNode node1 = rootNode.getFirstChild( "library_controllers" );
		if( node1.isEmpty() ) return true;

		XMLNode node2 = node1.getFirstChild( "controller" );
		while( !node2.isEmpty() )
		{
			// Skin
			XMLNode node3 = node2.getFirstChild( "skin" );
			if( !node3.isEmpty() )
			{
				DaeSkin *skin = new DaeSkin();
				if( skin->parse( node2 ) ) skinControllers.push_back( skin );
				else delete skin;
			}

			// Morph
			node3 = node2.getFirstChild( "morph" );
			if( !node3.isEmpty() )
			{
				DaeMorph *morph = new DaeMorph();
				if( morph->parse( node2 ) ) morphControllers.push_back( morph );
				else delete morph;
			}

			node2 = node2.getNextSibling( "controller" );
		}
		
		return true;
	}
};

#endif // _daeLibControllers_H_
