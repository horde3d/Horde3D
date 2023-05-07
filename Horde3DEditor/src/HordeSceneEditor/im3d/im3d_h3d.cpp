#include "im3d_h3d.h"

#include <vector>
#include <cstdio>
#include <cstring>

#include <Horde3D.h>
#include <Horde3DUtils.h>

#include "im3d_math.h"

#include <QFile>

#ifndef __COUNTER__
	#define __COUNTER__ __LINE__
#endif
#define IM3D_TOKEN_CONCATENATE_(_t0, _t1) _t0 ## _t1
#define IM3D_TOKEN_CONCATENATE(_t0, _t1)  IM3D_TOKEN_CONCATENATE_(_t0, _t1)
#define IM3D_UNIQUE_NAME(_base) IM3D_TOKEN_CONCATENATE(_base, __COUNTER__)
#define IM3D_STRINGIFY_(_t) #_t
#define IM3D_STRINGIFY(_t) IM3D_STRINGIFY_(_t)

/*	Code parts from OpenGL 3.1 example
	This example demonstrates a method for integrating Im3d without geometry shaders, instead using the
	vertex shader to expand points/lines into triangle strips. This works by uploading Im3d vertex data
	to a uniform buffer and fetching manually in the vertex shader.
*/

#define IM3D_OPENGL_VSHADER 140

static GLuint g_Im3dVertexArray;
static GLuint g_Im3dVertexBuffer;
static GLuint g_Im3dUniformBuffer;
static GLuint g_Im3dShaderPoints;
static GLuint g_Im3dShaderLines;
static GLuint g_Im3dShaderTriangles;
static GLint g_Im3dUniformBufferMaxSize;
static OpenGLWidget *g_view;

using namespace Im3d;

static const char* StripPath(const char* _path)
{
	int i = 0, last = 0;
	while (_path[i] != '\0') {
		if (_path[i] == '\\' || _path[i] == '/') {
			last = i + 1;
		}
		++i;
	}
	return &_path[last];
}

static void Append(const char* _str, Im3d::Vector<char>& _out_)
{
	while (*_str)
	{
		_out_.push_back(*_str);
		++_str;
	}
}
static void AppendLine(const char* _str, Im3d::Vector<char>& _out_)
{
	Append(_str, _out_);
	_out_.push_back('\n');
}
static bool LoadShader(const char* _path, const char* _defines, Im3d::Vector<char>& _out_)
{
	fprintf(stdout, "Loading shader: '%s'", StripPath(_path));
	if (_defines)
	{
		fprintf(stdout, " ");
		while (*_defines != '\0')
		{
			fprintf(stdout, " %s,", _defines);
			Append("#define ", _out_);
			AppendLine(_defines, _out_);
			_defines = strchr(_defines, 0);
			IM3D_ASSERT(_defines);
			++_defines;
		}
	}
	fprintf(stdout, "\n");

	QFile fin( _path );
	if ( !fin.open( QFile::ReadOnly ) )
	{
		fprintf(stderr, "Error opening '%s'\n", _path);
		return false;
	}

	long fsize = fin.size();

	int srcbeg = _out_.size();
	_out_.resize(srcbeg + fsize, '\0');
	if ( fin.read( _out_.data() + srcbeg, fsize ) != fsize )
	{
		fin.close();
		fprintf(stderr, "Error reading '%s'\n", _path);
		return false;
	}
	fin.close();
	_out_.push_back('\0');

	return true;
}

GLuint Im3d::LoadCompileShader(QOpenGLFunctions_3_1* glf, GLenum _stage, const char* _path, const char* _defines)
{
    Vector<char> src;
    AppendLine("#version " IM3D_STRINGIFY(IM3D_OPENGL_VSHADER), src);
    if (!LoadShader(_path, _defines, src))
    {
        return 0;
    }

    GLuint ret = 0;
    ret = glf->glCreateShader(_stage);
    const GLchar* pd = src.data();
    GLint ps = src.size();
    glf->glShaderSource(ret, 1, &pd, &ps);

    glf->glCompileShader(ret);
    GLint compileStatus = GL_FALSE;
    glf->glGetShaderiv(ret, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE)
    {
        fprintf(stderr, "Error compiling '%s':\n\n", _path);
        GLint len;
        glf->glGetShaderiv(ret, GL_INFO_LOG_LENGTH, &len);
        char* log = new GLchar[len];
        glf->glGetShaderInfoLog(ret, len, 0, log);
        fprintf(stderr, "%s\n", log);
        delete[] log;

        glf->glDeleteShader(ret);

        return 0;
    }

    return ret;
}

bool Im3d::LinkShaderProgram( QOpenGLFunctions_3_1* glf, GLuint _handle)
{
    IM3D_ASSERT(_handle != 0);

    glf->glLinkProgram(_handle);
    GLint linkStatus = GL_FALSE;
    glf->glGetProgramiv(_handle, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE)
    {
        fprintf(stderr, "Error linking program:\n\n");
        GLint len;
        glf->glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &len);
        GLchar* log = new GLchar[len];
        glf->glGetProgramInfoLog(_handle, len, 0, log);
        fprintf(stderr, "%s\n", log);
        delete[] log;

        return false;
    }

    return true;
}


// Resource init/shutdown will be app specific. In general you'll need one shader for each of the 3
// draw primitive types (points, lines, triangles), plus some number of vertex buffers.
bool Im3d_Init( QOpenGLFunctions_3_1 *glf, OpenGLWidget *view )
{
 // OpenGL uniform buffers require 16 byte alignment for structs - set IM3D_VERTEX_ALIGNMENT in im3d_config.h
	IM3D_ASSERT(sizeof(Im3d::VertexData) % 16 == 0);

	glGetIntegerv( GL_MAX_UNIFORM_BLOCK_SIZE, &g_Im3dUniformBufferMaxSize );
	if ( g_Im3dUniformBufferMaxSize <= 0 ) return false;

	{	GLuint vs = LoadCompileShader(glf, GL_VERTEX_SHADER,   ":/Shaders/im3d/im3d.glsl", "VERTEX_SHADER\0POINTS\0");
		GLuint fs = LoadCompileShader(glf, GL_FRAGMENT_SHADER, ":/Shaders/im3d/im3d.glsl", "FRAGMENT_SHADER\0POINTS\0");
		if (vs && fs)
		{
			g_Im3dShaderPoints = glf->glCreateProgram();
			glf->glAttachShader(g_Im3dShaderPoints, vs);
			glf->glAttachShader(g_Im3dShaderPoints, fs);
			bool ret = LinkShaderProgram(glf, g_Im3dShaderPoints);
			glf->glDeleteShader(vs);
			glf->glDeleteShader(fs);
			if (!ret)
			{
				return false;
			}
		}
		else
		{
			return false;
		}
		GLuint blockIndex;
		blockIndex = glf->glGetUniformBlockIndex(g_Im3dShaderPoints, "VertexDataBlock");
		glf->glUniformBlockBinding(g_Im3dShaderPoints, blockIndex, 0);
	}

	{	GLuint vs = LoadCompileShader(glf, GL_VERTEX_SHADER,   ":/Shaders/im3d/im3d.glsl", "VERTEX_SHADER\0LINES\0");
		GLuint fs = LoadCompileShader(glf, GL_FRAGMENT_SHADER, ":/Shaders/im3d/im3d.glsl", "FRAGMENT_SHADER\0LINES\0");
		if (vs && fs)
		{
			g_Im3dShaderLines = glf->glCreateProgram();
			glf->glAttachShader(g_Im3dShaderLines, vs);
			glf->glAttachShader(g_Im3dShaderLines, fs);
			bool ret = LinkShaderProgram(glf, g_Im3dShaderLines);
			glf->glDeleteShader(vs);
			glf->glDeleteShader(fs);
			if (!ret)
			{
				return false;
			}
		}
		else
		{
			return false;
		}

		GLuint blockIndex;
		blockIndex = glf->glGetUniformBlockIndex(g_Im3dShaderLines, "VertexDataBlock");
		glf->glUniformBlockBinding(g_Im3dShaderLines, blockIndex, 0);
	}

	{	GLuint vs = LoadCompileShader(glf, GL_VERTEX_SHADER,   ":/Shaders/im3d/im3d.glsl", "VERTEX_SHADER\0TRIANGLES\0");
		GLuint fs = LoadCompileShader(glf, GL_FRAGMENT_SHADER, ":/Shaders/im3d/im3d.glsl", "FRAGMENT_SHADER\0TRIANGLES\0");
		if (vs && fs)
		{
			g_Im3dShaderTriangles = glf->glCreateProgram();
			glf->glAttachShader(g_Im3dShaderTriangles, vs);
			glf->glAttachShader(g_Im3dShaderTriangles, fs);
			bool ret = LinkShaderProgram(glf, g_Im3dShaderTriangles);
			glf->glDeleteShader(vs);
			glf->glDeleteShader(fs);
			if (!ret)
			{
				return false;
			}
		}
		else
		{
			return false;
		}

		GLuint blockIndex;
		blockIndex = glf->glGetUniformBlockIndex(g_Im3dShaderTriangles, "VertexDataBlock");
		glf->glUniformBlockBinding(g_Im3dShaderTriangles, blockIndex, 0);
	}

 // In this example we're using a static buffer as the vertex source with a uniform buffer to provide the shader with the Im3d vertex data.
	Im3d::Vec4 vertexData[] =
		{
			Im3d::Vec4(-1.0f, -1.0f, 0.0f, 1.0f),
			Im3d::Vec4( 1.0f, -1.0f, 0.0f, 1.0f),
			Im3d::Vec4(-1.0f,  1.0f, 0.0f, 1.0f),
			Im3d::Vec4( 1.0f,  1.0f, 0.0f, 1.0f)
		};

	glf->glGenBuffers(1, &g_Im3dVertexBuffer);
	glf->glGenVertexArrays(1, &g_Im3dVertexArray);
	glf->glBindVertexArray(g_Im3dVertexArray);
	glf->glBindBuffer(GL_ARRAY_BUFFER, g_Im3dVertexBuffer);
	glf->glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), (GLvoid*)vertexData, GL_STATIC_DRAW);
	glf->glEnableVertexAttribArray(0);
	glf->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Im3d::Vec4), (GLvoid*)0);
	glf->glBindVertexArray(0);

	glf->glGenBuffers(1, &g_Im3dUniformBuffer);

	g_view = view;
	return true;
}

void Im3d_Shutdown( QOpenGLFunctions_3_1 *glf )
{
	glf->glDeleteVertexArrays(1, &g_Im3dVertexArray);
	glf->glDeleteBuffers(1, &g_Im3dUniformBuffer);
	glf->glDeleteBuffers(1, &g_Im3dVertexBuffer);
	glf->glDeleteProgram(g_Im3dShaderPoints);
	glf->glDeleteProgram(g_Im3dShaderLines);
	glf->glDeleteProgram(g_Im3dShaderTriangles);
}

// At the top of each frame, the application must fill the Im3d::AppData struct and then call Im3d::NewFrame().
// The example below shows how to do this, in particular how to generate the 'cursor ray' from a mouse position
// which is necessary for interacting with gizmos.
void Im3d_NewFrame( QOpenGLFunctions_3_1* glf, int h3d_camera )
{
	AppData& ad = GetAppData();

	if ( !h3d_camera ) return;

	int width = h3dGetNodeParamI( h3d_camera, H3DCamera::ViewportWidthI );
	int height = h3dGetNodeParamI( h3d_camera, H3DCamera::ViewportHeightI );
	int ortho = h3dGetNodeParamI( h3d_camera, H3DCamera::OrthoI );
	float frustTop = h3dGetNodeParamF( h3d_camera, H3DCamera::TopPlaneF, 0 );
	float frustNear = h3dGetNodeParamF( h3d_camera, H3DCamera::NearPlaneF, 0 );

	// Normalize viewport coordinates

	// float scaledWidth = g_view->width() * scale;
	// float scaledHeight = g_view->height() * scale;
 //
	// float normalized_x( float(event->pos().x()) / scaledWidth );
	// float normalized_y( float((height() - event->pos().y())) / scaledHeight );
 //
	// h3dutPickRay( h3d_camera, );
	const float* camera = 0;
    // Retrieve camera position...
    h3dGetNodeTransMats(h3d_camera, 0, &camera);

    // In case of an invalid camera (e.g. pipeline not set) return
    if ( !camera ) return;

	Im3d::Mat4 vm( camera[ 0 ], camera[ 4 ], camera[ 8 ], camera[ 12 ],
				   camera[ 1 ], camera[ 5 ], camera[ 9 ], camera[ 13 ],
				   camera[ 2 ], camera[ 6 ], camera[ 10 ], camera[ 14 ],
				   camera[ 3 ], camera[ 7 ], camera[ 11 ], camera[ 15 ] );

	// calculate direction`
	Im3d::Mat4 m = vm;
	m.m[3] = 0; m.m[7] = 0; m.m[11] = 0;
	Im3d::Vec3 camDir = m * Im3d::Vec3( 0, 0, -1 );
	camDir = Im3d::Normalize( camDir );

    // ... and projection matrix
    float projMat[16];
    h3dGetCameraProjMat( h3d_camera, projMat );
	Im3d::Mat4 pm( projMat[ 0 ], projMat[ 4 ], projMat[ 8 ], projMat[ 12 ],
				   projMat[ 1 ], projMat[ 5 ], projMat[ 9 ], projMat[ 13 ],
				   projMat[ 2 ], projMat[ 6 ], projMat[ 10 ], projMat[ 14 ],
				   projMat[ 3 ], projMat[ 7 ], projMat[ 11 ], projMat[ 15 ] );

	float cameraFieldOfView = atanf( frustTop / frustNear ) * 360.0f / 3.1415926;

//	ad.m_deltaTime     = g_Example->m_deltaTime;
	ad.m_viewportSize  = Vec2((float)width, (float)height);
	ad.m_viewOrigin    = Vec3( camera[ 12], camera[ 13 ], camera[ 14 ] ); // for VR use the head position
	ad.m_viewDirection = camDir;
	ad.m_worldUp       = Vec3(0.0f, 1.0f, 0.0f); // used internally for generating orthonormal bases
	ad.m_projOrtho     = (bool) ortho;

 // m_projScaleY controls how gizmos are scaled in world space to maintain a constant screen height
	ad.m_projScaleY = ad.m_projOrtho
		? 2.0f / pm(1, 1) // use far plane height for an ortho projection
		: tanf( Im3d::Radians( cameraFieldOfView ) * 0.5f) * 2.0f // or vertical fov for a perspective projection
		;

 // World space cursor ray from mouse position; for VR this might be the position/orientation of the HMD or a tracked controller.
	const qreal display_scale = g_view->devicePixelRatio(); // support scaled display
	QPoint p = g_view->mapFromGlobal( QCursor::pos() ) * display_scale;
	Vec2 cursorPos = Vec2( p.x(), p.y() );
	cursorPos = (cursorPos / ad.m_viewportSize) * 2.0f - 1.0f;
	cursorPos.y = -cursorPos.y; // window origin is top-left, ndc is bottom-left
	Vec3 rayOrigin, rayDirection;
	if (ortho)
	{
		rayOrigin.x  = cursorPos.x / pm(0, 0);
		rayOrigin.y  = cursorPos.y / pm(1, 1);
		rayOrigin.z  = 0.0f;
		rayOrigin    = vm * Vec4(rayOrigin, 1.0f);
		rayDirection = vm * Vec4(0.0f, 0.0f, -1.0f, 0.0f);
	}
	else
	{
		rayOrigin = ad.m_viewOrigin;
		rayDirection.x  = cursorPos.x / pm(0, 0);
		rayDirection.y  = cursorPos.y / pm(1, 1);
		rayDirection.z  = -1.0f;
		rayDirection    = vm * Vec4(Normalize(rayDirection), 0.0f);
	}
	ad.m_cursorRayOrigin = rayOrigin;
	ad.m_cursorRayDirection = rayDirection;

 // Set cull frustum planes. This is only required if IM3D_CULL_GIZMOS or IM3D_CULL_PRIMTIIVES is enable via im3d_config.h, or if any of the IsVisible() functions are called.
//	ad.setCullFrustum(g_Example->m_camViewProj, true);

 // Fill the key state array; using GetAsyncKeyState here but this could equally well be done via the window proc.
 // All key states have an equivalent (and more descriptive) 'Action_' enum.
	ad.m_keyDown[Im3d::Mouse_Left/*Im3d::Action_Select*/] = g_view->selectButtonPressed();

 // The following key states control which gizmo to use for the generic Gizmo() function. Here using the left ctrl key as an additional predicate.
	// bool ctrlDown = (GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
	// ad.m_keyDown[Im3d::Key_L/*Action_GizmoLocal*/]       = ctrlDown && (GetAsyncKeyState(0x4c) & 0x8000) != 0;
	// ad.m_keyDown[Im3d::Key_T/*Action_GizmoTranslation*/] = ctrlDown && (GetAsyncKeyState(0x54) & 0x8000) != 0;
	// ad.m_keyDown[Im3d::Key_R/*Action_GizmoRotation*/]    = ctrlDown && (GetAsyncKeyState(0x52) & 0x8000) != 0;
	// ad.m_keyDown[Im3d::Key_S/*Action_GizmoScale*/]       = ctrlDown && (GetAsyncKeyState(0x53) & 0x8000) != 0;

 // Enable gizmo snapping by setting the translation/rotation/scale increments to be > 0
	// ad.m_snapTranslation = ctrlDown ? 0.1f : 0.0f;
	// ad.m_snapRotation    = ctrlDown ? Im3d::Radians(30.0f) : 0.0f;
	// ad.m_snapScale       = ctrlDown ? 0.5f : 0.0f;

	Im3d::NewFrame();
}

// After all Im3d calls have been made for a frame, the user must call Im3d::EndFrame() to finalize draw data, then
// access the draw lists for rendering. Draw lists are only valid between calls to EndFrame() and NewFrame().
// The example below shows the simplest approach to rendering draw lists; variations on this are possible. See the
// shader source file for more details.
void Im3d_EndFrame( QOpenGLFunctions_3_1* glf, int h3d_camera )
{
	if ( !glf || !h3d_camera ) return;
	Im3d::EndFrame();

	// Primitive rendering.
	AppData& ad = GetAppData();

 	// Typical pipeline state: enable alpha blending, disable depth test and backface culling.
	glf->glEnable(GL_BLEND);
	glf->glBlendEquation(GL_FUNC_ADD);
	glf->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glf->glEnable(GL_PROGRAM_POINT_SIZE);
	glf->glDisable(GL_DEPTH_TEST);
	glf->glDisable(GL_CULL_FACE);

	glf->glViewport(0, 0, (GLsizei)ad.m_viewportSize.x, (GLsizei)ad.m_viewportSize.y);

	for (U32 i = 0, n = Im3d::GetDrawListCount(); i < n; ++i)
	{
		auto& drawList = Im3d::GetDrawLists()[i];

		if (drawList.m_layerId == Im3d::MakeId("NamedLayer"))
		{
		 // The application may group primitives into layers, which can be used to change the draw state (e.g. enable depth testing, use a different shader)
		}

		GLenum prim;
		GLuint sh;
		int primVertexCount;
		switch (drawList.m_primType)
		{
			case Im3d::DrawPrimitive_Points:
				prim = GL_TRIANGLE_STRIP;
				primVertexCount = 1;
				sh = g_Im3dShaderPoints;
				glf->glDisable(GL_CULL_FACE); // points are view-aligned
				break;
			case Im3d::DrawPrimitive_Lines:
				prim = GL_TRIANGLE_STRIP;
				primVertexCount = 2;
				sh = g_Im3dShaderLines;
				glf->glDisable(GL_CULL_FACE); // lines are view-aligned
				break;
			case Im3d::DrawPrimitive_Triangles:
				prim = GL_TRIANGLES;
				primVertexCount = 3;
				sh = g_Im3dShaderTriangles;
				//glAssert(glEnable(GL_CULL_FACE)); // culling valid for triangles, but optional
				break;
			default:
				IM3D_ASSERT(false);
				return;
		};

		glf->glBindVertexArray(g_Im3dVertexArray);
		glf->glBindBuffer(GL_ARRAY_BUFFER, g_Im3dVertexBuffer);

		glf->glUseProgram(sh);

		// calc view proj matrix
		float projMat[16];
		const float *viewMat;
		h3dGetCameraProjMat( h3d_camera, projMat );
		h3dGetNodeTransMats( h3d_camera, nullptr, &viewMat );

		Im3d::Mat4 vm( viewMat[ 0 ], viewMat[ 4 ], viewMat[ 8 ], viewMat[ 12 ],
				   viewMat[ 1 ], viewMat[ 5 ], viewMat[ 9 ], viewMat[ 13 ],
				   viewMat[ 2 ], viewMat[ 6 ], viewMat[ 10 ], viewMat[ 14 ],
				   viewMat[ 3 ], viewMat[ 7 ], viewMat[ 11 ], viewMat[ 15 ] );

		vm = Im3d::Inverse( vm );

		Im3d::Mat4 pm( projMat[ 0 ], projMat[ 4 ], projMat[ 8 ], projMat[ 12 ],
				   projMat[ 1 ], projMat[ 5 ], projMat[ 9 ], projMat[ 13 ],
				   projMat[ 2 ], projMat[ 6 ], projMat[ 10 ], projMat[ 14 ],
				   projMat[ 3 ], projMat[ 7 ], projMat[ 11 ], projMat[ 15 ] );

		Im3d::Mat4 viewProj = pm * vm;

		glf->glUniform2f( glf->glGetUniformLocation(sh, "uViewport"), ad.m_viewportSize.x, ad.m_viewportSize.y );
		glf->glUniformMatrix4fv( glf->glGetUniformLocation(sh, "uViewProjMatrix"), 1, false, (const GLfloat*) &viewProj.m[ 0 ] );

	 // Uniform buffers have a size limit; split the vertex data into several passes.
		const int kMaxBufferSize = g_Im3dUniformBufferMaxSize; //64 * 1024; // assuming 64kb here but the application should check the implementation limit
	 	const int kPrimsPerPass = kMaxBufferSize / (sizeof (Im3d::VertexData) * primVertexCount);

		int remainingPrimCount = drawList.m_vertexCount / primVertexCount;
		const Im3d::VertexData* vertexData = drawList.m_vertexData;
		while (remainingPrimCount > 0)
		{
			int passPrimCount = remainingPrimCount < kPrimsPerPass ? remainingPrimCount : kPrimsPerPass;
			int passVertexCount = passPrimCount * primVertexCount;

			glf->glBindBuffer(GL_UNIFORM_BUFFER, g_Im3dUniformBuffer);
			glf->glBufferData(GL_UNIFORM_BUFFER, (GLsizeiptr)passVertexCount * sizeof(Im3d::VertexData), (GLvoid*)vertexData, GL_DYNAMIC_DRAW);

		 // instanced draw call, 1 instance per prim
			glf->glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_Im3dUniformBuffer);
			glf->glDrawArraysInstanced(prim, 0, prim == GL_TRIANGLES ? 3 : 4, passPrimCount); // for triangles just use the first 3 verts of the strip

			vertexData += passVertexCount;
			remainingPrimCount -= passPrimCount;
		}
	}

	// Disable used states
	glf->glDisable(GL_BLEND);
	glf->glDisable(GL_PROGRAM_POINT_SIZE);
}
