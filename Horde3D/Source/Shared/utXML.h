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

#ifndef _utXML_H_
#define _utXML_H_

#if defined( _MSC_VER ) && _MSC_VER >= 1400
#	pragma warning( push )
#	pragma warning( disable : 4996 )  // Disable deprecation warnings
#endif

#include <stdio.h>
#include <cstring>
#include "rapidxml.h"


inline void rapidxml::parse_error_handler( const char *what, void *where )
{
}


namespace Horde3D {

class XMLAttribute
{
public:
	XMLAttribute() : attrib( 0x0 ) {}
	XMLAttribute( rapidxml::xml_attribute<> *attrib ) : attrib( attrib ) {}

	bool isEmpty() const { return attrib == 0x0; }
	const char *getName() const { return attrib->name(); }
	const char *getValue() const { return attrib->value(); }

	XMLAttribute getNextAttrib() { return XMLAttribute( attrib->next_attribute( 0x0 ) ); }

protected:
	rapidxml::xml_attribute<>  *attrib;
};


class XMLNode
{
public:
	XMLNode() : node( 0x0 ) {}
	XMLNode( rapidxml::xml_node<> *node ) : node( node ) {}

	rapidxml::xml_node<> *getRapidXMLNode() { return node; }
	
	bool isEmpty() const { return node == 0x0; }
	const char *getName() const { return node->name(); }
	const char *getText() const { return node->value(); }
	XMLNode getFirstChild( const char *name = 0x0 ) const { return XMLNode( node->first_node( name ) ); }
	XMLNode getNextSibling( const char *name = 0x0 ) const { return XMLNode( node->next_sibling( name ) ); }

	XMLAttribute getFirstAttrib() { return XMLAttribute( node->first_attribute( 0x0 ) ); }
	const char *getAttribute( const char *name, const char *defValue = "" ) const
	{
		rapidxml::xml_attribute<> *attrib = node->first_attribute( name );
		return attrib != 0x0 ? (const char *)attrib->value() : defValue;
	}

	int countChildNodes( const char *name = 0x0 ) const
	{
		int numNodes = 0;
		rapidxml::xml_node<> *node1 = node->first_node( name );
		while( node1 )
		{
			++numNodes;
			node1 = node1->next_sibling( name );
		}
		return numNodes;
	}

protected:
	rapidxml::xml_node<>  *node;
};


class XMLDoc
{
public:
	XMLDoc() : buf( 0x0 ) {}
	~XMLDoc() { delete[] buf; }
	
	bool hasError() const { return doc.first_node() == 0x0; }
	XMLNode getRootNode() const { return XMLNode( doc.first_node() ); }
	
	void parseString( char *text )
	{
		doc.parse< rapidxml::parse_validate_closing_tags >( text );
	}

	void parseBuffer( const char *charbuf, int size )
	{
		buf = new char[size + 1];
		memcpy( buf, charbuf, size );
		buf[size] = '\0';

		parseString( buf );
	}

	bool parseFile( const char *fileName )
	{
		FILE *f = fopen( fileName, "rb" );
		if( f == 0x0 ) return false;

		fseek( f, 0, SEEK_END );
		int size = ftell( f );
		fseek( f, 0, SEEK_SET );
		
		buf = new char[size + 1];
		fread( buf, 1, size, f );
		buf[size] = '\0';

		fclose( f );

		parseString( buf );
		return true;
	}

private:
	rapidxml::xml_document<>  doc;
	char                      *buf;
};


#if defined( _MSC_VER ) && _MSC_VER >= 1400
#	pragma warning( pop )
#endif

}
#endif // _utXML_H_
