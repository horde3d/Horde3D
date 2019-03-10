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

#ifndef _Horde3DExternalTexture_extension_H_
#define _Horde3DExternalTexture_extension_H_

#include "egPrerequisites.h"
#include "egExtensions.h"


namespace Horde3DExternalTexture {


class ExternalTexture : public Horde3D::IExtension
{
public:
	virtual const char *getName() const { return "ExternalTexture"; }
	virtual bool init();
	virtual void release();
};

}

#endif	// _Horde3DExternalTexture_extension_H_
