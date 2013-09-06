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

#ifndef _daeLibGeometries_H_
#define _daeLibGeometries_H_

#include "utXML.h"
#include "daeCommon.h"
#include "utils.h"
#include "utMath.h"
#include <string>
#include <vector>

using namespace Horde3D;


struct DaeVSource
{
	std::string  id;
	std::string  posSourceId;
	DaeSource    *posSource;


	bool parse( const XMLNode &verticesNode )
	{
		id = verticesNode.getAttribute( "id", "" );
		if( id == "" ) return false;
		
		XMLNode node1 = verticesNode.getFirstChild( "input" );
		while( !node1.isEmpty() )
		{
			if( strcmp( node1.getAttribute( "semantic", "" ), "POSITION" ) == 0 )
			{
				posSourceId = node1.getAttribute( "source", "" );
				removeGate( posSourceId );
				return true;
			}
			
			node1 = node1.getNextSibling( "input" );
		}
		
		return false;
	}
};


struct IndexEntry
{
	unsigned int  posIndex;     // Required
	int           normIndex;    // Optional, can be -1
	int           texIndex[4];  // Optional, can be -1


	IndexEntry()
	{
		normIndex = -1;
		texIndex[0] = -1; texIndex[1] = -1; texIndex[2] = -1; texIndex[3] = -1;
	}
};


struct DaeTriGroup
{
	std::string                vSourceId;
	DaeVSource                 *vSource;
	std::string                normSourceId;
	DaeSource                  *normSource;
	std::string                texSourceId[4];
	DaeSource                  *texSource[4];
	std::string                matId;
	std::vector< IndexEntry >  indices;


	DaeTriGroup()
	{
		vSource = 0x0;
		normSource = 0x0;
		texSource[0] = 0x0; texSource[1] = 0x0; texSource[2] = 0x0; texSource[3] = 0x0;
	}


	bool parse( const XMLNode &primitiveNode )
	{	
		enum { tTriangles, tPolygons, tPolylist };
		int primType = -1;
		
		if( strcmp( primitiveNode.getName(), "triangles" ) == 0 ) primType = tTriangles;
		else if( strcmp( primitiveNode.getName(), "polygons" ) == 0 ) primType = tPolygons;
		else if( strcmp( primitiveNode.getName(), "polylist" ) == 0 ) primType = tPolylist;
		else log( "Warning: Ignoring unsupported geometry primitive '" +
		          std::string( primitiveNode.getName() ) + "'" );
		if( primType < 0 ) return false;
		
		int vertexOffset = 0, normOffset = -1, texCoordOffset[4] = { -1, -1, -1, -1 };
		int inputsPerVert = 0;
		
		// Find the base mapping channel with the lowest set-id
		int baseChannel = 999999;
		XMLNode node1 = primitiveNode.getFirstChild( "input" );
		while( !node1.isEmpty() )
		{
			if( strcmp( node1.getAttribute( "semantic", "" ), "TEXCOORD" ) == 0 )
			{
				if( node1.getAttribute( "set" ) != 0x0 )
				{
					if( atoi( node1.getAttribute( "set" ) ) < baseChannel )
						baseChannel = atoi( node1.getAttribute( "set" ) );
				}
				else
				{
					baseChannel = 0;
					break;
				}
			}
			node1 = node1.getNextSibling( "input" );
		}
		
		// Parse input mapping
		node1 = primitiveNode.getFirstChild( "input" );
		while( !node1.isEmpty() )
		{
			int offset = atoi( node1.getAttribute( "offset", "0" ) );
			if( offset >= inputsPerVert )
				inputsPerVert = offset + 1;
			
			if( strcmp( node1.getAttribute( "semantic", "" ), "VERTEX" ) == 0 )
			{
				vertexOffset = offset;
				vSourceId = node1.getAttribute( "source", "" );
				removeGate( vSourceId );
				if( vSourceId == "" ) return false;
			}
			if( strcmp( node1.getAttribute( "semantic", "" ), "TEXCOORD" ) == 0 )
			{
				int set = -1;
				if( node1.getAttribute( "set" ) == 0x0 )
					set = 0;
				else
					set = atoi( node1.getAttribute( "set" ) );
				set -= baseChannel;
				
				if( set >= 0 && set < 4 )
				{
					texCoordOffset[set] = offset;
					texSourceId[set] = node1.getAttribute( "source", "" );
					removeGate( texSourceId[set] );
				}
			}
			if( strcmp( node1.getAttribute( "semantic", "" ), "NORMAL" ) == 0 )
			{
				normOffset = offset;
				normSourceId = node1.getAttribute( "source", "" );
				removeGate( normSourceId );
			}
			
			node1 = node1.getNextSibling( "input" );
		}

		matId = primitiveNode.getAttribute( "material", "" );
		
		// Get vertex counts for polylists
		char *vcountStr = 0x0;
		unsigned int numVerts = 0;
		if( primType == tPolylist )
		{	
			if( primitiveNode.getFirstChild( "vcount" ).isEmpty() ) return false;
			vcountStr = (char *)primitiveNode.getFirstChild( "vcount" ).getText();
		}

		// Parse actual primitive data
		node1 = primitiveNode.getFirstChild( "p" );
		while( !node1.isEmpty() )
		{
			char *str = (char *)node1.getText();
			if( str == 0x0 ) return false;
			
			int           si;
			unsigned int  curInput = 0, vertCnt = 0;
			IndexEntry    indexEntry;
			IndexEntry    firstIndex, lastIndex;
			
			while( parseInt( str, si ) )
			{
				// No else-if since offset sharing is possible
				if( (int)curInput == vertexOffset )
					indexEntry.posIndex = (unsigned)si;
				if( (int)curInput == normOffset )
					indexEntry.normIndex = si;
				if( (int)curInput == texCoordOffset[0] )
					indexEntry.texIndex[0] = si;
				if( (int)curInput == texCoordOffset[1] )
					indexEntry.texIndex[1] = si;
				if( (int)curInput == texCoordOffset[2] )
					indexEntry.texIndex[2] = si;
				if( (int)curInput == texCoordOffset[3] )
					indexEntry.texIndex[3] = si;

				if( ++curInput == inputsPerVert )
				{
					if( primType == tPolylist && vertCnt == numVerts )
					{
						vertCnt = 0;
						parseInt( vcountStr, si );
						numVerts = (unsigned)si;
					}
					
					if( primType == tPolygons || primType == tPolylist )
					{
						// Do simple triangulation (assumes convex polygons)
						if( vertCnt == 0 )
						{
							firstIndex = indexEntry;
						}
						else if( vertCnt > 2 )
						{
							// Create new triangle
							indices.push_back( firstIndex );
							indices.push_back( lastIndex );
						}
					}

					indices.push_back( indexEntry );
					lastIndex = indexEntry;
					curInput = 0;
					++vertCnt;
				}
			}

			node1 = node1.getNextSibling( "p" );
		}

		return true;
	}


	Vec3f getPos( int posIndex )
	{
		Vec3f v;
		
		// Assume the float buffer has at least 3 values per element
		DaeSource *source = vSource->posSource;
		v.x = source->floatArray[posIndex * source->paramsPerItem + 0];
		v.y = source->floatArray[posIndex * source->paramsPerItem + 1];
		v.z = source->floatArray[posIndex * source->paramsPerItem + 2];
		
		return v;
	}

	Vec3f getNormal( int normIndex )
	{
		Vec3f v;
		
		DaeSource *source = normSource;
		if( source != 0x0 && normIndex >= 0 )
		{
			// Assume the float buffer has at least 3 values per element
			v.x = source->floatArray[normIndex * source->paramsPerItem + 0];
			v.y = source->floatArray[normIndex * source->paramsPerItem + 1];
			v.z = source->floatArray[normIndex * source->paramsPerItem + 2];
		}
		
		return v;
	}

	Vec3f getTexCoords( int texIndex, unsigned int set )
	{
		Vec3f v;
		
		if( set < 4 )
		{
			DaeSource *source = texSource[set];
			if( source != 0x0 && texIndex >= 0 )
			{
				// Assume the float buffer has at least 2 values per element
				v.x = source->floatArray[texIndex * source->paramsPerItem + 0];
				v.y = source->floatArray[texIndex * source->paramsPerItem + 1];
				
				if( source->paramsPerItem >= 3 )
					v.z = source->floatArray[texIndex * source->paramsPerItem + 2];
			}
		}
		
		return v;
	}
};


struct DaeGeometry
{
	std::string                 id;
	std::string                 name;
	std::vector< DaeSource >    sources;
	std::vector< DaeVSource >   vsources;
	std::vector< DaeTriGroup >  triGroups;


	DaeSource *findSource( const std::string &id )
	{
		if( id == "" ) return 0x0;
		
		for( unsigned int i = 0; i < sources.size(); ++i )
		{
			if( sources[i].id == id ) return &sources[i];
		}

		return 0x0;
	}


	DaeVSource *findVSource( const std::string &id )
	{
		if( id == "" ) return 0x0;
		
		for( unsigned int i = 0; i < vsources.size(); ++i )
		{
			if( vsources[i].id == id ) return &vsources[i];
		}

		return 0x0;
	}


	bool parse( const XMLNode &geometryNode )
	{
		XMLNode node1 = geometryNode.getFirstChild( "mesh" );
		if( node1.isEmpty() )
		{	
			log( "Warning: Ignoring unsupported geometry '" +
			     std::string( geometryNode.getAttribute( "id", "" ) ) + "'" );
			return false;
		}

		id = geometryNode.getAttribute( "id", "" );
		if( id == "" ) return false;
		
		name = geometryNode.getAttribute( "name", "" );
		if( name.empty() ) name = id;

		// Parse sources
		XMLNode node2 = node1.getFirstChild( "source" );
		while( !node2.isEmpty() )
		{
			sources.push_back( DaeSource() );
			if( !sources.back().parse( node2 ) ) sources.pop_back();
			
			node2 = node2.getNextSibling( "source" );
		}

		// Parse vertex data
		node2 = node1.getFirstChild( "vertices" );
		while( !node2.isEmpty() )
		{
			vsources.push_back( DaeVSource() );
			if( vsources.back().parse( node2 ) )
			{
				vsources.back().posSource = findSource( vsources.back().posSourceId );
				if( vsources.back().posSource == 0x0 ) vsources.pop_back();
			}
			else vsources.pop_back();
			
			node2 = node2.getNextSibling( "vertices" );
		}

		// Parse primitives
		node2 = node1.getFirstChild();
		while( !node2.isEmpty() )
		{
			if( strcmp( node2.getName(), "triangles" ) == 0 ||
			    strcmp( node2.getName(), "polygons" ) == 0 ||
			    strcmp( node2.getName(), "polylist" ) == 0 ||
			    strcmp( node2.getName(), "trifans" ) == 0 ||
			    strcmp( node2.getName(), "tristrips" ) == 0 ||
			    strcmp( node2.getName(), "lines" ) == 0 ||
			    strcmp( node2.getName(), "linestrips" ) == 0 )
			{
				triGroups.push_back( DaeTriGroup() );
				if( triGroups.back().parse( node2 ) )
				{
					DaeTriGroup &triGroup = triGroups.back();
					
					triGroup.vSource = findVSource( triGroup.vSourceId );
					triGroup.normSource = findSource( triGroup.normSourceId );
					triGroup.texSource[0] = findSource( triGroup.texSourceId[0] );
					triGroup.texSource[1] = findSource( triGroup.texSourceId[1] );
					triGroup.texSource[2] = findSource( triGroup.texSourceId[2] );
					triGroup.texSource[3] = findSource( triGroup.texSourceId[3] );

					if( triGroup.vSource == 0x0 )
					{
						log( "Warning: Mesh '" + id + "' has no vertex coordinates and is ignored" );
						triGroups.pop_back();
					}
				}
				else triGroups.pop_back();
			}
			
			node2 = node2.getNextSibling();
		}

		return true;
	}


	Vec3f getPos( int posIndex )
	{
		Vec3f v;
		
		// Assume the float buffer has at least 3 values per element
		DaeSource *source = vsources[0].posSource;
		v.x = source->floatArray[posIndex * source->paramsPerItem + 0];
		v.y = source->floatArray[posIndex * source->paramsPerItem + 1];
		v.z = source->floatArray[posIndex * source->paramsPerItem + 2];
		
		return v;
	}
};


struct DaeLibGeometries
{
public:

	std::vector< DaeGeometry* >  geometries;

	
	~DaeLibGeometries()
	{
		for( unsigned int i = 0; i < geometries.size(); ++i ) delete geometries[i];
	}
	

	DaeGeometry *findGeometry( const std::string &id )
	{
		if( id == "" ) return 0x0;
		
		for( unsigned int i = 0; i < geometries.size(); ++i )
		{
			if( geometries[i]->id == id ) return geometries[i];
		}

		return 0x0;
	}
	
	
	bool parse( const XMLNode &rootNode )
	{
		XMLNode node1 = rootNode.getFirstChild( "library_geometries" );
		if( node1.isEmpty() ) return true;

		XMLNode node2 = node1.getFirstChild( "geometry" );
		while( !node2.isEmpty() )
		{
			DaeGeometry *geometry = new DaeGeometry();
			if( geometry->parse( node2 ) ) geometries.push_back( geometry );
			else delete geometry;

			node2 = node2.getNextSibling( "geometry" );
		}
		
		return true;
	}
};

#endif // _daeLibGeometries_H_
