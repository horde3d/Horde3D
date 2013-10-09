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

#ifndef _daeLibImages_H_
#define _daeLibImages_H_

#include "utXML.h"
#include <string>
#include <vector>

using namespace Horde3D;


struct DaeImage
{
	std::string  id;
	std::string  name;
	std::string  fileName;


	bool parse( const XMLNode &imageNode )
	{
		id = imageNode.getAttribute( "id", "" );
		if( id == "" ) return false;
		name = imageNode.getAttribute( "name", "" );
		if( name.empty() ) name = id;

		if( !imageNode.getFirstChild( "init_from" ).isEmpty() &&
		    imageNode.getFirstChild( "init_from" ).getText() != 0x0 )
		{
			fileName = imageNode.getFirstChild( "init_from" ).getText();
		}
		else
		{
			return false;
		}

		// Extract filename from path
		fileName = extractFileName( decodeURL( fileName ), true );
		
		return true;
	}
};


struct DaeLibImages
{
	std::vector< DaeImage * >	images;


	~DaeLibImages()
	{
		for( unsigned int i = 0; i < images.size(); ++i ) delete images[i];
	}


	DaeImage *findImage( const std::string &id )
	{
		if( id == "" ) return 0x0;
		
		for( unsigned int i = 0; i < images.size(); ++i )
		{
			if( images[i]->id == id ) return images[i];
		}

		return 0x0;
	}

	
	bool parse( const XMLNode &rootNode )
	{
		XMLNode node1 = rootNode.getFirstChild( "library_images" );
		if( node1.isEmpty() ) return true;

		XMLNode node2 = node1.getFirstChild( "image" );
		while( !node2.isEmpty() )
		{
			DaeImage *image = new DaeImage();
			if( image->parse( node2 ) ) images.push_back( image );
			else delete image;

			node2 = node2.getNextSibling( "image" );
		}
		
		return true;
	}
};

#endif // _daeLibImages_H_
