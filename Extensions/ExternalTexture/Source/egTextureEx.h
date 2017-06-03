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

#ifndef EGTEXTUREEX_H
#define EGTEXTUREEX_H

#include <egTexture.h>

namespace Horde3DExternalTexture {

class TextureResourceEx : public Horde3D::TextureResource
{
public:
    static Resource *factoryFunc( const std::string &name, int flags )
        { return new TextureResourceEx( name, flags ); }

    TextureResourceEx( const std::string &name, int flags );

    void release();

    bool importTexGL(int texGL , int width, int height);

private:    
    void replaceTexObj( int texObj );

    bool  m_imported;

};

}

#endif // EGTEXTUREEX_H
