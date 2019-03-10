// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2016 Nicolas Schulz and Horde3D team
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#include "egMaterial.h"
#include "egTexture.h"
#include "egModules.h"
#include "egCom.h"
#include "utXML.h"
#include <cstring>
#include <algorithm>

#include "utDebug.h"


namespace Horde3D {

using namespace std;

void MaterialResource::initializationFunc()
{
	MaterialClassCollection::init();
}


void MaterialResource::releaseFunc()
{
	MaterialClassCollection::release();
}


MaterialResource::MaterialResource( const string &name, int flags ) :
	Resource( ResourceTypes::Material, name, flags )
{
	initDefault();	
}


MaterialResource::~MaterialResource()
{
	release();
}


Resource *MaterialResource::clone()
{
	MaterialResource *res = new MaterialResource( "", _flags );

	*res = *this;
	
	return res;
}


void MaterialResource::initDefault()
{
	_shaderRes = 0x0;
	_combMask = 0;
	_matLink = 0x0;
	_classID = 0;
}


void MaterialResource::release()
{
	_shaderRes = 0x0;
	_matLink = 0x0;
	for( uint32 i = 0; i < _samplers.size(); ++i ) _samplers[i].texRes = 0x0;

	_buffers.clear();
	_samplers.clear();
	_uniforms.clear();
	_shaderFlags.clear();
}


bool MaterialResource::raiseError( const string &msg, int line )
{
	// Reset
	release();
	initDefault();

	if( line < 0 )
		Modules::log().writeError( "Material resource '%s': %s", _name.c_str(), msg.c_str() );
	else
		Modules::log().writeError( "Material resource '%s' in line %i: %s", _name.c_str(), line, msg.c_str() );
	
	return false;
}


bool MaterialResource::load( const char *data, int size )
{
	if( !Resource::load( data, size ) ) return false;
	
	XMLDoc doc;
	doc.parseBuffer( data, size );
	if( doc.hasError() )
		return raiseError( "XML parsing error" );

	XMLNode rootNode = doc.getRootNode();
	if( strcmp( rootNode.getName(), "Material" ) != 0 )
		return raiseError( "Not a material resource file" );

	// Class
	_classID = MaterialClassCollection::addClass( rootNode.getAttribute( "class", "" ) );

	// Link
	if( strcmp( rootNode.getAttribute( "link", "" ), "" ) != 0 )
	{
		uint32 mat = Modules::resMan().addResource(
			ResourceTypes::Material, rootNode.getAttribute( "link" ), 0, false );
		_matLink = (MaterialResource *)Modules::resMan().resolveResHandle( mat );
		if( _matLink == this )
			return raiseError( "Illegal self link in material, causing infinite link loop" );
	}

	// Shader Flags
	XMLNode node1 = rootNode.getFirstChild( "ShaderFlag" );
	while( !node1.isEmpty() )
	{
		if( node1.getAttribute( "name" ) == 0x0 ) return raiseError( "Missing ShaderFlag attribute 'name'" );
		
		_shaderFlags.push_back( node1.getAttribute( "name" ) );
		
		node1 = node1.getNextSibling( "ShaderFlag" );
	}

	// Shader
	node1 = rootNode.getFirstChild( "Shader" );
	if( !node1.isEmpty() )
	{
		if( node1.getAttribute( "source" ) == 0x0 ) return raiseError( "Missing Shader attribute 'source'" );
			
		uint32 shader = Modules::resMan().addResource(
				ResourceTypes::Shader, node1.getAttribute( "source" ), 0, false );
		_shaderRes = (ShaderResource *)Modules::resMan().resolveResHandle( shader );

		_combMask = ShaderResource::calcCombMask( _shaderFlags );
		_shaderRes->preLoadCombination( _combMask );
	}

	// Texture samplers
	node1 = rootNode.getFirstChild( "Sampler" );
	while( !node1.isEmpty() )
	{
		if( node1.getAttribute( "name" ) == 0x0 ) return raiseError( "Missing Sampler attribute 'name'" );
		if( node1.getAttribute( "map" ) == 0x0 ) return raiseError( "Missing Sampler attribute 'map'" );

		MatSampler sampler;
		sampler.name = node1.getAttribute( "name" );

		ResHandle texMap;
		uint32 flags = 0;
		if( !Modules::config().loadTextures ) flags |= ResourceFlags::NoQuery;
		
		if( _stricmp( node1.getAttribute( "allowCompression", "true" ), "false" ) == 0 ||
			_stricmp( node1.getAttribute( "allowCompression", "1" ), "0" ) == 0 )
			flags |= ResourceFlags::NoTexCompression;

		if( _stricmp( node1.getAttribute( "mipmaps", "true" ), "false" ) == 0 ||
			_stricmp( node1.getAttribute( "mipmaps", "1" ), "0" ) == 0 )
			flags |= ResourceFlags::NoTexMipmaps;

		if( _stricmp( node1.getAttribute( "sRGB", "false" ), "true" ) == 0 ||
			_stricmp( node1.getAttribute( "sRGB", "0" ), "1" ) == 0 )
			flags |= ResourceFlags::TexSRGB;

		texMap = Modules::resMan().addResource(
			ResourceTypes::Texture, node1.getAttribute( "map" ), flags, false );

		sampler.texRes = (TextureResource *)Modules::resMan().resolveResHandle( texMap );
		
		_samplers.push_back( sampler );
		
		node1 = node1.getNextSibling( "Sampler" );
	}
		
	// Vector uniforms
	node1 = rootNode.getFirstChild( "Uniform" );
	while( !node1.isEmpty() )
	{
		if( node1.getAttribute( "name" ) == 0x0 ) return raiseError( "Missing Uniform attribute 'name'" );

		MatUniform uniform;
		uniform.name = node1.getAttribute( "name" );

		uniform.values[0] = toFloat( node1.getAttribute( "a", "0" ) );
		uniform.values[1] = toFloat( node1.getAttribute( "b", "0" ) );
		uniform.values[2] = toFloat( node1.getAttribute( "c", "0" ) );
		uniform.values[3] = toFloat( node1.getAttribute( "d", "0" ) );

		_uniforms.push_back( uniform );

		node1 = node1.getNextSibling( "Uniform" );
	}
	
	// Data (compute/texture) buffers
	node1 = rootNode.getFirstChild( "DataBuffer" );
	while ( !node1.isEmpty() )
	{
		if ( node1.getAttribute( "name" ) == 0x0 ) return raiseError( "Missing DataBuffer attribute 'name'" );
		if ( node1.getAttribute( "source" ) == 0x0 ) return raiseError( "Missing DataBuffer attribute 'source'" );

		MatBuffer buf;
		buf.name = node1.getAttribute( "name" );

		uint32 compBuffer = Modules::resMan().addResource(
			ResourceTypes::ComputeBuffer, node1.getAttribute( "source" ), 0, false );
		buf.compBufRes = ( ComputeBufferResource * ) Modules::resMan().resolveResHandle( compBuffer );

		_buffers.push_back( buf );

		node1 = node1.getNextSibling( "DataBuffer" );
	}

	return true;
}


bool MaterialResource::setUniform( const string &name, float a, float b, float c, float d )
{
	for( uint32 i = 0; i < _uniforms.size(); ++i )
	{
		if( _uniforms[i].name == name )
		{
			_uniforms[i].values[0] = a;
			_uniforms[i].values[1] = b;
			_uniforms[i].values[2] = c;
			_uniforms[i].values[3] = d;
			return true;
		}
	}

	return false;
}


bool MaterialResource::isOfClass( int theClassID ) const
{
	return MaterialClassCollection::isOfClass( theClassID, _classID );
}


int MaterialResource::getElemCount( int elem ) const
{
	switch( elem )
	{
	case MaterialResData::MaterialElem:
		return 1;
	case MaterialResData::SamplerElem:
		return (int)_samplers.size();
	case MaterialResData::UniformElem:
		return (int)_uniforms.size();
	default:
		return Resource::getElemCount( elem );
	}
}


int MaterialResource::getElemParamI( int elem, int elemIdx, int param ) const
{
	switch( elem )
	{
	case MaterialResData::MaterialElem:
		switch( param )
		{
		case MaterialResData::MatLinkI:
			return _matLink != 0x0 ? _matLink->getHandle() : 0;		
		case MaterialResData::MatShaderI:
			return _shaderRes != 0x0 ? _shaderRes->getHandle() : 0;
		}
		break;
	case MaterialResData::SamplerElem:
		if( (unsigned)elemIdx < _samplers.size() )
		{
			switch( param )
			{
			case MaterialResData::SampTexResI:
				return _samplers[elemIdx].texRes ? _samplers[elemIdx].texRes->getHandle() : 0;
			}
		}
		break;
	}

	return Resource::getElemParamI( elem, elemIdx, param );
}


void MaterialResource::setElemParamI( int elem, int elemIdx, int param, int value )
{
	switch( elem )
	{
	case MaterialResData::MaterialElem:
		switch( param )
		{
		case MaterialResData::MatLinkI:
			if( value == 0 )
			{	
				_matLink = 0x0;
				return;
			}
			else
			{
				Resource *res = Modules::resMan().resolveResHandle( value );
				if( res != 0x0 && res->getType() == ResourceTypes::Material )
					_matLink = (MaterialResource *)res;
				else
					Modules::setError( "Invalid handle in h3dSetResParamI for H3DMatRes::MatLinkI" );
				return;
			}
			break;
		case MaterialResData::MatShaderI:
			if( value == 0 )
			{	
				_shaderRes = 0x0;
				return;
			}
			else
			{
				Resource *res = Modules::resMan().resolveResHandle( value );
				if( res != 0x0 && res->getType() == ResourceTypes::Shader )
					_shaderRes = (ShaderResource *)res;
				else
					Modules::setError( "Invalid handle in h3dSetResParamI for H3DMatRes::MatShaderI" );
				return;
			}
			break;
		}
		break;
	case MaterialResData::SamplerElem:
		if( (unsigned)elemIdx < _samplers.size() )
		{
			switch( param )
			{
			case MaterialResData::SampTexResI:
				Resource *res = Modules::resMan().resolveResHandle( value );
				if( res == 0 || (res != 0x0 && res->getType() == ResourceTypes::Texture) )
					_samplers[elemIdx].texRes = (TextureResource *)res;
				else
					Modules::setError( "Invalid handle in h3dSetResParamI for H3DMatRes::SampTexResI" );
				return;
			}
		}
		break;
	}

	Resource::setElemParamI( elem, elemIdx, param, value );
}


float MaterialResource::getElemParamF( int elem, int elemIdx, int param, int compIdx ) const
{
	switch( elem )
	{
	case MaterialResData::UniformElem:
		if( (unsigned)elemIdx < _uniforms.size() )
		{
			switch( param )
			{
			case MaterialResData::UnifValueF4:
				if( (unsigned)compIdx < 4 ) return _uniforms[elemIdx].values[compIdx];
				break;
			}
		}
		break;
	}
	
	return Resource::getElemParamF( elem, elemIdx, param, compIdx );
}


void MaterialResource::setElemParamF( int elem, int elemIdx, int param, int compIdx, float value )
{
	switch( elem )
	{
	case MaterialResData::UniformElem:
		if( (unsigned)elemIdx < _uniforms.size() )
		{	
			switch( param )
			{
			case MaterialResData::UnifValueF4:
				if( (unsigned)compIdx < 4 )
				{	
					_uniforms[elemIdx].values[compIdx] = value;
					return;
				}
				break;
			}
		}
		break;
	}
	
	Resource::setElemParamF( elem, elemIdx, param, compIdx, value );
}


const char *MaterialResource::getElemParamStr( int elem, int elemIdx, int param ) const
{
	switch( elem )
	{
	case MaterialResData::MaterialElem:
		switch( param )
		{
		case MaterialResData::MatClassStr:
			return MaterialClassCollection::getClassString( _classID );
		}
		break;
	case MaterialResData::SamplerElem:
		if( (unsigned)elemIdx < _samplers.size() )
		{
			switch( param )
			{
			case MaterialResData::SampNameStr:
				return _samplers[elemIdx].name.c_str();
			}
		}
		break;
	case MaterialResData::UniformElem:
		if( (unsigned)elemIdx < _uniforms.size() )
		{
			switch( param )
			{
			case MaterialResData::UnifNameStr:
				return _uniforms[elemIdx].name.c_str();
			}
		}
		break;
	}
	
	return Resource::getElemParamStr( elem, elemIdx, param );
}


void MaterialResource::setElemParamStr( int elem, int elemIdx, int param, const char *value )
{
	switch( elem )
	{
	case MaterialResData::MaterialElem:
		switch( param )
		{
		case MaterialResData::MatClassStr:
			_classID = MaterialClassCollection::addClass( std::string( value ) );
			return;
		}
		break;
	}
	
	Resource::setElemParamStr( elem, elemIdx, param, value );
}

// =================================================================================================
// MaterialClassCollection
// =================================================================================================
std::vector< MaterialHierarchy > MaterialClassCollection::_matHierarchy;

std::string MaterialClassCollection::_returnedClassString;
//std::vector< std::string > MaterialClassCollection::_classes;

void MaterialClassCollection::init()
{
	_matHierarchy.reserve( 50 );
	_returnedClassString.reserve( 512 );

	// add default class
	MaterialHierarchy hierarchy;
	_matHierarchy.push_back( std::move( hierarchy ) );
}


void MaterialClassCollection::release()
{
	_matHierarchy.clear();
}


void MaterialClassCollection::clear()
{
	_matHierarchy.clear();
	
	// add default class
	MaterialHierarchy hierarchy;
	_matHierarchy.push_back( std::move( hierarchy ) );
}


int MaterialClassCollection::addClass( const std::string &matClass )
{
	if ( matClass.empty() ) return 0; // Most common case - return default class

	// Check level count (number of dots + 1) in a class
	size_t numberOfLevels = ( size_t ) std::count( matClass.begin(), matClass.end(), '.' ) + 1;
	if ( numberOfLevels > H3D_MATERIAL_HIERARCHY_LEVELS )
	{
		Modules::setError( "Number of hierarchy levels in material class exceeds the maximum value. Material class cannot be registered." );
		return 0;
	}

	bool inversed = false;
	if ( !matClass.empty() && matClass[ 0 ] == '~' ) inversed = true; 

	// TODO: implement hashing

	// Split material class levels to separate strings
	constexpr size_t numberOfCharactersInClass = sizeof(MaterialClass::name);
	char tmpStrings[ H3D_MATERIAL_HIERARCHY_LEVELS ][ numberOfCharactersInClass ] = { { '\0' } };
	std::string subString; subString.reserve( 256 );
	int lastDotPosition = 0;

	for ( size_t i = 0; i < numberOfLevels; ++i )
	{
		int dotPos = (int) matClass.find( '.', lastDotPosition );
		if ( i == 0 && inversed ) lastDotPosition = 1; // handle case with ~ character present. It should be removed from the final string
		
		subString = matClass.substr( lastDotPosition, dotPos );
		if ( subString.length() > numberOfCharactersInClass - 1 )
		{
			// Truncate string
			subString.erase( numberOfCharactersInClass - 1, subString.length() - numberOfCharactersInClass - 1 );
		}
		
		strncpy_s( tmpStrings[ i ], numberOfCharactersInClass, subString.c_str(), subString.size() );

		lastDotPosition = dotPos + 1;
	}

	// Try to find class in the collection
	bool exactMatch = true;
	int partialMatchIds[ H3D_MATERIAL_HIERARCHY_LEVELS ] = {};
	for ( int i = 0; i < (int) _matHierarchy.size(); i++ )
	{
		MaterialHierarchy &hierarchy = _matHierarchy[ i ];

		for ( int j = 0; j < H3D_MATERIAL_HIERARCHY_LEVELS; j++ )
		{
			if ( strcmp( hierarchy.value[ j ].name, tmpStrings[ j ] ) != 0 )
			{
				exactMatch = false;
				break;
			}
			else
			{
				// save partial matches in order to save time if new class record would be created
				partialMatchIds[ j ] = hierarchy.value[ j ].index;
			}
		}

		if ( exactMatch )
		{
			return inversed ? i * -1 : i; // minus in index indicates that class should be excluded from processing
		}
		else exactMatch = true; // reset flag for next iteration
	}

	// Class is not found - create new record
	// Hierarchy is created for each level
	for ( size_t levels = 0; levels < numberOfLevels; ++levels )
	{
		if ( partialMatchIds[ levels ] != 0 ) 
		{
			continue;
		}

		MaterialHierarchy hierarchy;

		for ( size_t i = 0; i < H3D_MATERIAL_HIERARCHY_LEVELS; ++i )
		{
			strncpy_s( hierarchy.value[ i ].name, numberOfCharactersInClass, tmpStrings[ i ], numberOfCharactersInClass );

			if ( partialMatchIds[ i ] != 0 ) hierarchy.value[ i ].index = partialMatchIds[ i ];
			else
			{
				if ( i < numberOfLevels ) hierarchy.value[ i ].index = (uint32) _matHierarchy.size();
				else hierarchy.value[ i ].index = 0; // iterator is greater than the number of levels in the class, so use default class for later hierarchy levels
			}
		}

		_matHierarchy.push_back( std::move( hierarchy ) );
	}

	int result = inversed ? ( (int) _matHierarchy.size() - 1 ) * -1 : (int) _matHierarchy.size() - 1;
	return result;
}


const char * MaterialClassCollection::getClassString( int currentMaterialClass )
{
	if ( currentMaterialClass < 0 ) currentMaterialClass *= -1;

	ASSERT( ( size_t ) currentMaterialClass < _matHierarchy.size() )

	_returnedClassString.clear();

	// Combine different hierarchy levels to one string
	MaterialHierarchy &hierarchy = _matHierarchy[ currentMaterialClass ];
	bool previousStepSuccessful = false;

	for ( size_t i = 0; i < H3D_MATERIAL_HIERARCHY_LEVELS; ++i )
	{
		if ( strcmp( hierarchy.value[ i ].name, "" ) != 0 )
		{
			if ( previousStepSuccessful ) _returnedClassString.append( "." );

			_returnedClassString.append( hierarchy.value[ i ].name );
			previousStepSuccessful = true;
		}
		else previousStepSuccessful = false;
	}

	return _returnedClassString.c_str();
}

bool MaterialClassCollection::isOfClass( int requestedMaterialClass, int currentMaterialClass )
{
	if ( requestedMaterialClass == 0 || ( requestedMaterialClass == currentMaterialClass ) )
		return true;

	// Check if index is negative - class exclusion is required
	bool exclusion = false;
	if ( requestedMaterialClass < 0 ) 
	{
		exclusion = true;
		requestedMaterialClass *= -1; // make variable positive as it is used later for array index comparison
	}

	ASSERT( ( size_t ) requestedMaterialClass < _matHierarchy.size() );

	MaterialHierarchy &hierarchy = _matHierarchy[ ( size_t ) currentMaterialClass ];

	// Check if requested class is part of the hierarchy of the current class
	for ( size_t i = 0; i < H3D_MATERIAL_HIERARCHY_LEVELS; ++i )
	{
		if ( hierarchy.value[ i ].index == requestedMaterialClass )
		{
			if ( exclusion ) return false;
			else return true;
		}
	}

	return exclusion ? true : false;
}


// 	static string theClass2;
// 	
// 	if( theClass != "" )
// 	{
// 		if( theClass[0]	!= '~' )
// 		{
// 			if( _class.find( theClass, 0 ) != 0 ) return false;
// 			if( _class.length() > theClass.length() && _class[theClass.length()] != '.' ) return false;
// 		}
// 		else	// Not operator
// 		{
// 			theClass2 = theClass.substr( 1, theClass.length() - 1);
// 			
// 			if( _class.find( theClass2, 0 ) == 0 )
// 			{
// 				if( _class.length() == theClass2.length() )
// 				{
// 					return false;
// 				}
// 				else
// 				{
// 					if( _class[theClass2.length()] == '.' ) return false;
// 				}
// 			}
// 		}
// 	}
// 	else
// 	{
// 		// Special name which is hidden when drawing objects of "all classes"
// 		if( _class == "_DEBUG_" ) return false;
// 	}
// 
// 	return true;

}  // namespace
