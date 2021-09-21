#pragma once

#include "egPrerequisites.h"
#include "egShader.h"

namespace Horde3D {
    
class ShaderParser
{
public:
    ShaderParser( const std::string &shaderName );
    ~ShaderParser();
    
    bool parseBinaryShader( char *data, uint32 size );

private:
    bool raiseError( const std::string &msg, int line = -1 );

    bool parseBinarySampler( char *data, uint32 samplerCount );
    
private:
    std::string _shaderName;
    
    std::vector< ShaderSampler > _samplers;
    
    friend class ShaderResource;
};

} // namespace 
