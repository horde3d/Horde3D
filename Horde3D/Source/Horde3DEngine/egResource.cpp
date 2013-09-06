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

#include "egResource.h"
#include "egModules.h"
#include "egCom.h"
#include <sstream>
#include <cstring>

#include "utDebug.h"


namespace Horde3D {

using namespace std;

// **********************************************************************************
// Class Resource
// **********************************************************************************

Resource::Resource( int type, const string &name, int flags )
{
	_type = type;
	_name = name;
	_handle = 0;
	_loaded = false;
	_refCount = 0;
	_userRefCount = 0;
	_flags = flags;
	
	if( (flags & ResourceFlags::NoQuery) == ResourceFlags::NoQuery ) _noQuery = true;
	else _noQuery = false;
}


Resource::~Resource()
{
	// Remove all references
	// Nothing to do here
}


Resource *Resource::clone()
{
	Modules::log().writeDebugInfo( "Resource cloning not implemented for type %i", _type );
	return 0x0;
}


void Resource::initDefault()
{
}


void Resource::release()
{
}


bool Resource::load( const char *data, int size )
{	
	// Resources can only be loaded once
	if( _loaded ) return false;
	
	// A NULL pointer can be used if the file could not be loaded
	if( data == 0x0 || size <= 0 )
	{	
		Modules::log().writeWarning( "Resource '%s' of type %i: No data loaded (file not found?)", _name.c_str(), _type );
		_noQuery = true;
		return false;
	}

	_loaded = true;
	
	return true;
}


void Resource::unload()
{
	release();
	initDefault();
	_loaded = false;
}


int Resource::findElem( int elem, int param, const char *value )
{
	for( int i = 0, s = getElemCount( elem ); i < s; ++i )
	{
		if( strcmp( getElemParamStr( elem, i, param ), value ) == 0 )
			return i;
	}

	return -1;
}


int Resource::getElemCount( int elem )
{
	Modules::setError( "Invalid elem in h3dGetResElemCount" );
	return 0;
}

int Resource::getElemParamI( int elem, int elemIdx, int param )
{
	Modules::setError( "Invalid elem or param in h3dGetResParamI" );
	return Math::MinInt32;
}

void Resource::setElemParamI( int elem, int elemIdx, int param, int value )
{
	Modules::setError( "Invalid elem or param in h3dSetResParamI" );
}

float Resource::getElemParamF( int elem, int elemIdx, int param, int compIdx )
{
	Modules::setError( "Invalid elem, param or component in h3dGetResParamF" );
	return Math::NaN;
}

void Resource::setElemParamF( int elem, int elemIdx, int param, int compIdx, float value )
{
	Modules::setError( "Invalid elem, param or component in h3dSetResParamF" );
}

const char *Resource::getElemParamStr( int elem, int elemIdx, int param )
{
	Modules::setError( "Invalid elem or param in h3dGetResParamStr" );
	return "";
}

void Resource::setElemParamStr( int elem, int elemIdx, int param, const char *value )
{
	Modules::setError( "Invalid elem or param in h3dSetResParamStr" );
}

void *Resource::mapStream( int elem, int elemIdx, int stream, bool read, bool write )
{
	Modules::setError( "Invalid operation in h3dMapResStream" );
	return 0x0;
}

void Resource::unmapStream()
{
	Modules::setError( "Invalid operation by h3dUnmapResStream" );
}


// **********************************************************************************
// Class ResourceManager
// **********************************************************************************

ResourceManager::ResourceManager()
{
	_resources.reserve( 100 );
}


ResourceManager::~ResourceManager()
{
	clear();

	// Release resource types
	map< int, ResourceRegEntry >::const_iterator itr = _registry.begin();
	while( itr != _registry.end() )
	{
		if( itr->second.releaseFunc != 0x0 )
			(*itr->second.releaseFunc)();

		++itr;
	}
}


void ResourceManager::registerResType( int resType, const string &typeString, ResTypeInitializationFunc inf,
									   ResTypeReleaseFunc rf, ResTypeFactoryFunc ff )
{
	ResourceRegEntry entry;
	entry.typeString = typeString;
	entry.initializationFunc = inf;
	entry.releaseFunc = rf;
	entry.factoryFunc = ff;
	_registry[resType] = entry;

	// Initialize resource type
	if( inf != 0 ) (*inf)();
}


Resource *ResourceManager::findResource( int type, const string &name )
{
	for( size_t i = 0, s = _resources.size(); i < s; ++i )
	{
		if( _resources[i] != 0x0 && _resources[i]->_type == type && _resources[i]->_name == name )
		{
			return _resources[i];
		}
	}
	
	return 0x0;
}


Resource *ResourceManager::getNextResource( int type, ResHandle start )
{
	for( size_t i = start, s = _resources.size(); i < s; ++i )
	{
		if( _resources[i] != 0x0 &&
		    (type == ResourceTypes::Undefined || _resources[i]->_type == type) )
		{
			return _resources[i];
		}
	}
	
	return 0x0;
}


ResHandle ResourceManager::addResource( Resource &resource )
{
	// Try to insert resource in free slot
	for( uint32 i = 0; i < _resources.size(); ++i )
	{
		if( _resources[i] == 0x0 )
		{
			resource._handle = i + 1;
			_resources[i] = &resource;
			return i + 1;
		}
	}
	
	// If there is no free slot, add resource to end
	resource._handle = (ResHandle)_resources.size() + 1;
	_resources.push_back( &resource );
	return resource._handle;
}


ResHandle ResourceManager::addResource( int type, const string &name, int flags, bool userCall )
{
	if( name == "" )
	{	
		Modules::log().writeDebugInfo( "Invalid name for added resource of type %i", type );
		return 0;
	}
	
	// Check if resource is already in list and return index
	for( uint32 i = 0; i < _resources.size(); ++i )
	{
		if( _resources[i] != 0x0 && _resources[i]->_name == name )
		{
			if( _resources[i]->_type == type )
			{
				if( userCall ) ++_resources[i]->_userRefCount;
				return i + 1;
			}
		}
	}
	
	// Create resource
	Resource *resource = 0x0;
	map< int, ResourceRegEntry >::iterator itr = _registry.find( type );
	if( itr != _registry.end() ) resource = (*itr->second.factoryFunc)( name, flags );
	if( resource == 0x0 ) return 0;

	//Modules::log().writeInfo( "Adding %s resource '%s'", itr->second.typeString.c_str(), name.c_str() );
	
	if( userCall ) resource->_userRefCount = 1;
	
	return addResource( *resource );
}


ResHandle ResourceManager::addNonExistingResource( Resource &resource, bool userCall )
{
	if( resource._name == "" ) return 0;

	// Check that name does not yet exist
	for( uint32 i = 0; i < _resources.size(); ++i )
	{
		if( _resources[i] != 0x0 && _resources[i]->_name == resource._name ) return 0;
	}

	if( userCall ) resource._userRefCount += 1;
	return addResource( resource );
}


ResHandle ResourceManager::cloneResource( Resource &sourceRes, const string &name )
{
	// Check that name does not yet exist
	if( name != "" )
	{
		for( uint32 i = 0; i < _resources.size(); ++i )
		{
			if( _resources[i] != 0x0 && _resources[i]->_name == name )
			{	
				Modules::log().writeDebugInfo( "Name '%s' used for h3dCloneResource already exists", name.c_str() );
				return 0;
			}
		}
	}

	Resource *newRes = sourceRes.clone();
	if( newRes == 0x0 ) return 0;

	newRes->_name = name != "" ? name : "|tmp|";
	newRes->_userRefCount = 1;
	newRes->_refCount = 0;
	int handle = addResource( *newRes );
	
	if( name == "" )
	{
		stringstream ss;
		ss << sourceRes._name << "|" << handle;
		newRes->_name = ss.str();
	}

	return handle;
}


int ResourceManager::removeResource( Resource &resource, bool userCall )
{
	// Decrease reference count
	if( userCall && resource._userRefCount > 0 ) --resource._userRefCount;

	return (signed)resource._userRefCount;
}


void ResourceManager::clear()
{
	// Release resources and remove dependencies
	for( uint32 i = 0; i < _resources.size(); ++i )
	{
		if( _resources[i] != 0x0 ) _resources[i]->release();
	}

	// Delete resources
	for( uint32 i = 0; i < _resources.size(); ++i )
	{
		if( _resources[i] != 0x0 )
		{
			delete _resources[i]; _resources[i] = 0x0;
		}
	}
}


ResHandle ResourceManager::queryUnloadedResource( int index )
{
	int j = 0;

	for( uint32 i = 0; i < _resources.size(); ++i )
	{
		if( _resources[i] != 0x0 && !_resources[i]->_loaded && !_resources[i]->_noQuery )
		{	
			if( j == index ) return _resources[i]->_handle;
			else ++j;
		}
	}

	return 0;
}


void ResourceManager::releaseUnusedResources()
{
	vector< uint32 > killList;
	
	// Find unused resources and release dependencies
	for( uint32 i = 0; i < _resources.size(); ++i )
	{
		if( _resources[i] != 0x0 && _resources[i]->_userRefCount == 0 && _resources[i]->_refCount == 0 )
		{
			killList.push_back( i );
			_resources[i]->release();
		}
	}
	
	// Delete unused resources
	for( uint32 i = 0; i < killList.size(); ++i )
	{
		Modules::log().writeInfo( "Removed resource '%s'", _resources[killList[i]]->_name.c_str() );
		delete _resources[killList[i]]; _resources[killList[i]] = 0x0;
	}

	// Releasing a resource can remove dependencies from other resources which can also be released
	if( !killList.empty() ) releaseUnusedResources();
}

}  // namespace
