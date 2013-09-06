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

#ifndef _daeCommon_H_
#define _daeCommon_H_

#include "utXML.h"
#include "utils.h"
#include <string>
#include <vector>

using namespace Horde3D;


struct DaeSource
{
	std::string                 id;
	std::vector< float >        floatArray;
	std::vector< std::string >  stringArray;
	unsigned int                paramsPerItem;


	bool parse( const XMLNode &sourceNode )
	{
		bool isFloatArray = true;
		paramsPerItem = 1;
		
		id = sourceNode.getAttribute( "id", "" );
		if( id == "" ) return false;

		XMLNode arrayNode = sourceNode.getFirstChild( "float_array" );
		if( arrayNode.isEmpty() )
		{	
			arrayNode = sourceNode.getFirstChild( "Name_array" );
			if( arrayNode.isEmpty() ) arrayNode = sourceNode.getFirstChild( "IDREF_array" );
			if( arrayNode.isEmpty() ) return false;
			isFloatArray = false;
		}
		int count = atoi( arrayNode.getAttribute( "count", "0" ) );
		
		if( count > 0 )
		{
			// Check accessor
			int numItems = count;
			XMLNode node1 = sourceNode.getFirstChild( "technique_common" );
			if( !node1.isEmpty() )
			{
				XMLNode node2 = node1.getFirstChild( "accessor" );
				if( !node2.isEmpty() )
					numItems = atoi( node2.getAttribute( "count", "0" ) );
			}

			paramsPerItem = count / numItems;

			// Parse data
			std::string name;
			char *str = (char *)arrayNode.getText();
			if( str == 0x0 ) return false;
			
			if( isFloatArray )
				floatArray.reserve( count );
			else
				stringArray.reserve( count );
			
			for( int i = 0; i < count; ++i )
			{
				if( isFloatArray )
				{
					float f;
					parseFloat( str, f );
					floatArray.push_back( f );
				}
				else
				{
					parseString( str, name );
					stringArray.push_back( name );
				}
			}
		}

		return true;
	}
};

#endif // _daeCommon_H_
