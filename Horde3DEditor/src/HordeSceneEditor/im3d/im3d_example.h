#pragma once

#include "im3d.h"

// Compiler
#if defined(__GNUC__)
	#define IM3D_COMPILER_GNU
#elif defined(_MSC_VER)
	#define IM3D_COMPILER_MSVC
#else
	#error im3d: Compiler not defined
#endif

// Platform 
#if defined(_WIN32) || defined(_WIN64)
 // Windows
	#define IM3D_PLATFORM_WIN
	
	#define NOMINMAX 1
	#define WIN32_LEAN_AND_MEAN 1
	#define VC_EXTRALEAN 1
	#include <Windows.h>

	#define winAssert(e) IM3D_VERIFY_MSG(e, Im3d::GetPlatformErrorString(GetLastError()))
	
	namespace Im3d {
		const char* GetPlatformErrorString(DWORD _err);
	}
	
#else
	#error im3d: Platform not defined
#endif

// Graphics API
#if defined(IM3D_OPENGL)
 // OpenGL
	//#define IM3D_OPENGL_VMAJ    3
	//#define IM3D_OPENGL_VMIN    3
	//#define IM3D_OPENGL_VSHADER "#version 150"
 
	#include "GL/glew.h"
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
		GLuint LoadCompileShader(GLenum _stage, const char* _path, const char* _defines = 0);
		// Return false on failure (prints log info to stderr).
		bool LinkShaderProgram(GLuint _handle);
		
		const char* GetGlEnumString(GLenum _enum);
		const char* GlGetString(GLenum _name);
	}
	
#elif defined(IM3D_DX11)
 // DirectX 11
	#include <d3d11.h>

	#define IM3D_DX11_VSHADER "4_0"

	#define dxAssert(call) \
		do { \
			HRESULT err = (call); \
			if (err != S_OK) { \
				Im3d::Assert(#call, __FILE__, __LINE__, Im3d::GetPlatformErrorString((DWORD)err)); \
				IM3D_BREAK(); \
			} \
		} while (0)
	
	namespace Im3d {
		// Return 0 on failure (prints log info to stderr). _defines is a list of null-separated strings e.g. "DEFINE1 1\0DEFINE2 1\0"
		ID3DBlob* LoadCompileShader(const char* _target, const char* _path, const char* _defines = 0);

		// Resource helpers.
		ID3D11Buffer* CreateBuffer(UINT _size, D3D11_USAGE _usage, UINT _bind, const void* _data = nullptr);
		ID3D11Buffer* CreateConstantBuffer(UINT _size, D3D11_USAGE _usage, const void* _data = nullptr);
		ID3D11Buffer* CreateVertexBuffer(UINT _size, D3D11_USAGE _usage, const void* _data = nullptr);
		ID3D11Buffer* CreateIndexBuffer(UINT _size, D3D11_USAGE _usage, const void* _data = nullptr);
		void* MapBuffer(ID3D11Buffer* _buffer, D3D11_MAP _mapType);
		void  UnmapBuffer(ID3D11Buffer* _buffer);
		ID3D11Texture2D* CreateTexture2D(UINT _width, UINT _height, DXGI_FORMAT _format, ID3D11ShaderResourceView** resView_ = nullptr, const void* _data = nullptr);
		ID3D11DepthStencilView* CreateDepthStencil(UINT _width, UINT _height, DXGI_FORMAT _format);
	}

#else
	#error im3d: Graphics API not defined
#endif

#define IM3D_UNUSED(x) do { (void)sizeof(x); } while(0)
#ifdef IM3D_COMPILER_MSVC
	#define IM3D_BREAK() __debugbreak()
#else
	#include <cstdlib>
	#define IM3D_BREAK() abort()
#endif

#define IM3D_ASSERT_MSG(e, msg, ...) \
	do { \
		if (!(e)) { \
			Im3d::Assert(#e, __FILE__, __LINE__, msg, ## __VA_ARGS__); \
			IM3D_BREAK(); \
		} \
	} while (0)

#undef  IM3D_ASSERT
#define IM3D_ASSERT(e)                 IM3D_ASSERT_MSG(e, 0, 0)
#define IM3D_VERIFY_MSG(e, msg, ...)   IM3D_ASSERT_MSG(e, msg, ## __VA_ARGS__)
#define IM3D_VERIFY(e)                 IM3D_VERIFY_MSG(e, 0, 0)

#ifndef __COUNTER__
	#define __COUNTER__ __LINE__
#endif
#define IM3D_TOKEN_CONCATENATE_(_t0, _t1) _t0 ## _t1
#define IM3D_TOKEN_CONCATENATE(_t0, _t1)  IM3D_TOKEN_CONCATENATE_(_t0, _t1)
#define IM3D_UNIQUE_NAME(_base) IM3D_TOKEN_CONCATENATE(_base, __COUNTER__)
#define IM3D_STRINGIFY_(_t) #_t
#define IM3D_STRINGIFY(_t) IM3D_STRINGIFY_(_t)

#include "im3d_math.h"

#include "imgui/imgui.h"

namespace Im3d {

void  Assert(const char* _e, const char* _file, int _line, const char* _msg, ...);

void  RandSeed(int _seed);
int   RandInt(int _min, int _max);
float RandFloat(float _min, float _max);
Vec3  RandVec3(float _min, float _max);
Mat3  RandRotation();
Color RandColor(float _min, float _max);

void  DrawNdcQuad();
void  DrawTeapot(const Mat4& _world, const Mat4& _viewProj);

struct Example
{
	bool init(int _width, int _height, const char* _title);
	void shutdown();
	bool update();
	void draw();
 
 // window 
	int m_width, m_height;
	const char* m_title;
	Vec2  m_prevCursorPos;
	
	bool hasFocus() const;
	Vec2 getWindowRelativeCursor() const;
	
 // 3d camera
	bool  m_camOrtho;
    Vec3  m_camPos;
	Vec3  m_camDir;
	float m_camFovDeg;
	float m_camFovRad;
	Mat4  m_camWorld;
	Mat4  m_camView;
	Mat4  m_camProj;
	Mat4  m_camViewProj;
	
	float m_deltaTime;

 // platform/graphics specifics
	#if defined(IM3D_PLATFORM_WIN)
		HWND m_hwnd;
		LARGE_INTEGER m_currTime, m_prevTime;
		
		#if defined(IM3D_OPENGL)
			HDC   m_hdc;
			HGLRC m_hglrc;
		
		#elif defined(IM3D_DX11)
			ID3D11Device*           m_d3dDevice;
			ID3D11DeviceContext*    m_d3dDeviceCtx;
			IDXGISwapChain*         m_dxgiSwapChain;
			ID3D11RenderTargetView* m_d3dRenderTarget;
			ID3D11DepthStencilView* m_d3dDepthStencil;
		#endif
	#endif

 // text rendering
	void drawTextDrawListsImGui(const Im3d::TextDrawList _textDrawLists[], U32 _count);

}; // struct Example

extern Example* g_Example;

} // namespace Im3d


// per-example implementations (in the example .cpp)
extern bool Im3d_Init();
extern void Im3d_Shutdown();
extern void Im3d_NewFrame();
extern void Im3d_EndFrame();
