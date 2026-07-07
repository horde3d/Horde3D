#pragma once

#include "im3d.h"
#include <QOpenGLFunctions_3_1>
#include "../OpenGLWidget.h"

#define glAssert(call) \
    do { \
        (call); \
        GLenum err = glGetError(); \
        if (err != GL_NO_ERROR) { \
            Im3d::Assert(#call, __FILE__, __LINE__, Im3d::GetGlEnumString(err)); \
            IM3D_BREAK(); \
        } \
    } while (0)

namespace Im3d {
    // Return 0 on failure (prints log info to stderr). _defines is a list of null-separated strings e.g. "DEFINE1 1\0DEFINE2 1\0"
    GLuint LoadCompileShader( QOpenGLFunctions_3_1* glf, GLenum _stage, const char* _path, const char* _defines = 0);
    // Return false on failure (prints log info to stderr).
    bool LinkShaderProgram( QOpenGLFunctions_3_1* glf, GLuint _handle);

    const char* GetGlEnumString(GLenum _enum);
    const char* GlGetString(GLenum _name);
}

// per-example implementations (in the example .cpp)
extern bool Im3d_Init( QOpenGLFunctions_3_1* glf, OpenGLWidget *view );
void Im3d_Shutdown( QOpenGLFunctions_3_1* glf );
extern void Im3d_NewFrame( QOpenGLFunctions_3_1* glf, int h3d_camera );
extern void Im3d_EndFrame( QOpenGLFunctions_3_1* glf, int h3d_camera );
