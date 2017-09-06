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

#include "egTextureEx.h"

#include <egModules.h>
#include <egCom.h>
#include <egRenderer.h>
#include <utOpenGL.h>

#include "egRendererBaseGL2.h"
#include "egRendererBaseGL4.h"

namespace  Horde3DExternalTexture {

using namespace Horde3D;

TextureResourceEx::TextureResourceEx(const std::string &name, int flags) : TextureResource(name,flags), m_imported(false)
{
}

void TextureResourceEx::release()
{
    if( m_imported )
        replaceTexObj(0);
    TextureResource::release();
}

bool TextureResourceEx::importTexGL(uint32 texGL, int width, int height )
{
    _loaded = true;
    _width = width;
    _height = height;

    if( _texObject == defTex2DObject )
    {
        m_imported = false;
        RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();
        _texObject = rdi->createTexture( _texType, _width, _height, _depth == 0 ? 1 : _depth, _texFormat,
                                         false, false, false, _sRGB );
    }
    replaceTexObj( texGL );
    m_imported = true;
}

uint32 TextureResourceEx::getGLTexID()
{
    if( !_texObject )
        return 0;
    switch ( Modules::renderer().getRenderDeviceType() )
    {
    case RenderBackendType::OpenGL2:
    {
        RDI_GL2::RenderDeviceGL2 *rdi = ( RDI_GL2::RenderDeviceGL2 * ) Modules::renderer().getRenderDevice();
        RDI_GL2::RDITextureGL2& tex = rdi->getTexture( _texObject );
        return tex.glObj;
    }
    case RenderBackendType::OpenGL4:
    {
        RDI_GL4::RenderDeviceGL4 *rdi = ( RDI_GL4::RenderDeviceGL4 * ) Modules::renderer().getRenderDevice();
        RDI_GL4::RDITextureGL4& tex = rdi->getTexture( _texObject );
        return tex.glObj;
    }
    default:
        Modules::log().writeError("Render backend not supported");
        return 0;
    }
}

void TextureResourceEx::replaceTexObj( uint32 texObj )
{
    switch ( Modules::renderer().getRenderDeviceType() )
    {
    case RenderBackendType::OpenGL2:
    {
        RDI_GL2::RenderDeviceGL2 *rdi = ( RDI_GL2::RenderDeviceGL2 * ) Modules::renderer().getRenderDevice();
        RDI_GL2::RDITextureGL2& tex = rdi->getTexture( _texObject );
        if( !m_imported && tex.glObj )
            glDeleteTextures( 1, &tex.glObj );
        tex.glObj = texObj;
        tex.memSize = 0;
        return;
    }
    case RenderBackendType::OpenGL4:
    {
        RDI_GL4::RenderDeviceGL4 *rdi = ( RDI_GL4::RenderDeviceGL4 * ) Modules::renderer().getRenderDevice();
        RDI_GL4::RDITextureGL4& tex = rdi->getTexture( _texObject );
        if( !m_imported && tex.glObj )
            glDeleteTextures( 1, &tex.glObj );
        tex.glObj = texObj;
        tex.memSize = 0;
        return;
    }
    default:
        Modules::log().writeError("Render backend not supported");
        return;
    }
}

} // Namespace
