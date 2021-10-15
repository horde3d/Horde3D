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

private:
    std::string _shaderName;
    
    std::vector< ShaderSampler > _samplers;
    std::vector< ShaderUniform > _uniforms;

    friend class ShaderResource;
};

} // namespace 
