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

#ifndef _egExtensions_H_
#define _egExtensions_H_

#include "egPrerequisites.h"
#include <string>
#include <vector>


namespace Horde3D {

// =================================================================================================
// IExtension
// =================================================================================================

class IExtension
{
public:
    virtual ~IExtension() {}
	virtual const char *getName() = 0;
	virtual bool init() = 0;
	virtual void release() = 0;
};


// =================================================================================================
// Extension Manager
// =================================================================================================

class ExtensionManager
{
public:
	~ExtensionManager();
	
	bool installExtension( IExtension *extension );
	bool checkExtension( const std::string &name );

protected:
	std::vector< IExtension * >  _extensions;
};

}
#endif // _egExtensions_H_
