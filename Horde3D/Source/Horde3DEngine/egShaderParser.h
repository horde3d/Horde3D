#pragma once

#include "egPrerequisites.h"
#include "egShader.h"

namespace Horde3D {

class ShaderParser
{
public:
    ShaderParser( const std::string &shaderName );
    ~ShaderParser();
    
    const std::string &getShaderName() { return _shaderName; }
    
    bool parseBinaryShader( char *data, uint32 size );

protected:
    bool raiseError( const std::string &msg, int line = -1 );

    bool parseBinarySampler( char *data, uint32 samplerCount );
    
    bool parseBinaryUniforms( char *data, uint32 variablesCount );

    bool parseBinaryBuffer( char *data, uint32 bufferCount );
    
    bool parseBinaryFlags( char *data, uint32 flagsCount );
    
    bool parseBinaryContexts( char *data, uint32 contextCount );
    
    bool parseBinaryContextShaderCombs( char *data, uint32 shaderCombs );
    
private:
    std::string _shaderName;
    
    std::vector< ShaderSampler >        _samplers;
    std::vector< ShaderUniform >        _uniforms;
    std::vector< ShaderBuffer >         _buffers;
    std::vector< ShaderFlag >           _flags;
    std::vector< ShaderContext >        _contexts;
    std::vector< ShaderBinaryData >     _binaryShaders;
    int                                 _shaderType;
    
    friend class ShaderResource;
};

} // namespace 
