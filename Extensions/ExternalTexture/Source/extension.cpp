// *************************************************************************************************
//
// Horde3D ExternalTexture Extension
// --------------------------------------------------------
// Copyright (C) 2017 Volker Vogelhuber
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************


#include "utPlatform.h"
#include "egModules.h"
#include "egCom.h"
#include "egRenderer.h"
#include "extension.h"
#include "egTextureEx.h"
#include "utOpenGL.h"
#include "utDebug.h"


// Internal extension interface
namespace Horde3DExternalTexture {

using namespace Horde3D;

bool ExternalTexture::init()
{
	//Modules::log().writeDebugInfo( "ExternalTexture extension: overwriting TextureResource with inherited implementation TextureResourceEx" );
	Modules::resMan().registerResType( Horde3D::ResourceTypes::Texture, "Texture", 0x0, TextureResource::releaseFunc, TextureResourceEx::factoryFunc );
	return true;
}

void ExternalTexture::release()
{
	//Modules::log().writeDebugInfo( "Reenable default implementation for texture resources" );
	Modules::resMan().registerResType( Horde3D::ResourceTypes::Texture, "Texture", 0x0, TextureResource::releaseFunc, TextureResource::factoryFunc );
}

// Public C API
H3D_IMPL bool h3dextImportTexture( Horde3D::ResHandle texRes, int texID )
{
	Resource *res = Modules::resMan().resolveResHandle(texRes);
	if( res == 0x0 || res->getType() != ResourceTypes::Texture )
	{
		Modules::log().writeError("Error adding texture resource");
		return false;
	}

	TextureResourceEx* texEx = static_cast<TextureResourceEx*>(res);
	texEx->importTexGL( texID, 0, 0 );
	return true;
}

// Public C API
H3D_IMPL int h3dextGetGLTextureID( Horde3D::ResHandle texRes )
{
	Resource *res = Modules::resMan().resolveResHandle(texRes);
	if( res == 0x0 || res->getType() != ResourceTypes::Texture )
	{
		Modules::log().writeError("Error getting texture resource");
		return 0;
	}

	TextureResourceEx* texEx = static_cast<TextureResourceEx*>(res);
	return texEx->getGLTexID();
}


}  // namespace
