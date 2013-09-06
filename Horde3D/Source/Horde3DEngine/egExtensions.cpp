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

#include "egExtensions.h"
#include <cstring>

#include "utDebug.h"


namespace Horde3D {

ExtensionManager::~ExtensionManager()
{
	for( uint32 i = 0; i < _extensions.size(); ++i )
	{
		_extensions[i]->release();
		delete _extensions[i];
	}

	_extensions.clear();
}


bool ExtensionManager::checkExtension( const std::string &name )
{
	for( uint32 i = 0; i < _extensions.size(); ++i )
	{
		if( strcmp( _extensions[i]->getName(), name.c_str() ) == 0 ) return true;
	}
	return false;
}


bool ExtensionManager::installExtension( IExtension *extension )
{
	_extensions.push_back( extension );
	return _extensions.back()->init();
}

}  // namespace
