#include "im3d_example.h"

#include "teapot.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace Im3d;

#ifdef IM3D_COMPILER_MSVC
	#pragma warning(disable: 4996) // vsnprintf

	#pragma warning(disable: 4311) // typecast
	#pragma warning(disable: 4302) //    "
	#pragma warning(disable: 4312) //    "
#endif

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

/******************************************************************************/
#if defined(IM3D_PLATFORM_WIN)
	// force Nvidia/AMD drivers to use the discrete GPU
	extern "C" {
		__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
		__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
	}

	static LARGE_INTEGER g_SysTimerFreq;

	const char* Im3d::GetPlatformErrorString(DWORD _err)
	{
		const int kErrMsgMax = 1024;
		static char buf[kErrMsgMax];
		buf[0] = '\0';
		IM3D_VERIFY(
			FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
				nullptr, 
				_err,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)buf, 
				kErrMsgMax, 
				nullptr
			) != 0
		);
		return buf;
	}
	
	static LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _umsg, WPARAM _wparam, LPARAM _lparam)
	{
		ImGuiIO& imgui = ImGui::GetIO();
		Example* im3d = g_Example;
		
		switch (_umsg)
		{
			case WM_SIZE:
			{
				int w = (int)LOWORD(_lparam), h = (int)HIWORD(_lparam);
				if (im3d->m_width != w || im3d->m_height != h)
				{
					im3d->m_width = w;
					im3d->m_height = h;
				}
				#if defined(IM3D_DX11)
				 // DX requires that we reset the backbuffer when the window resizes
					if (g_Example->m_d3dRenderTarget)
					{
						g_Example->m_d3dRenderTarget->Release();
						g_Example->m_d3dDepthStencil->Release();
						dxAssert(g_Example->m_dxgiSwapChain->ResizeBuffers(0, (UINT)w, (UINT)h, DXGI_FORMAT_UNKNOWN, 0));
						ID3D11Texture2D* backBuffer;
						dxAssert(g_Example->m_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer));
						dxAssert(g_Example->m_d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &g_Example->m_d3dRenderTarget));
						g_Example->m_d3dDepthStencil = CreateDepthStencil((UINT)w, (UINT)h, DXGI_FORMAT_D24_UNORM_S8_UINT);
						g_Example->m_d3dDeviceCtx->OMSetRenderTargets(1, &g_Example->m_d3dRenderTarget, g_Example->m_d3dDepthStencil);
						backBuffer->Release();
					}

				#endif
				break;
			}
			case WM_SIZING:
			{
				RECT* r = (RECT*)_lparam;
				int w = (int)(r->right - r->left);
				int h = (int)(r->bottom - r->top);
				if (im3d->m_width != w || im3d->m_height != h)
				{
					im3d->m_width = w;
					im3d->m_height = h;
				}
				break;
			}
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
				imgui.MouseDown[0] = _umsg == WM_LBUTTONDOWN;
				break;
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
				imgui.MouseDown[2] = _umsg == WM_MBUTTONDOWN;
				break;
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
				imgui.MouseDown[1] = _umsg == WM_RBUTTONDOWN;
				break;
			case WM_MOUSEWHEEL:
				imgui.MouseWheel = (float)(GET_WHEEL_DELTA_WPARAM(_wparam)) / (float)(WHEEL_DELTA); 
				break;
			case WM_MOUSEMOVE:
				imgui.MousePos.x = LOWORD(_lparam);
				imgui.MousePos.y = HIWORD(_lparam);
				break;
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				WPARAM vk = _wparam;
				UINT sc = (_lparam & 0x00ff0000) >> 16;
				bool e0 = (_lparam & 0x01000000) != 0;
				if (vk == VK_SHIFT)
				{
					vk = MapVirtualKey(sc, MAPVK_VSC_TO_VK_EX);
				}
				switch (vk)
				{
					case VK_CONTROL:
						imgui.KeyCtrl = _umsg == WM_KEYDOWN;
						break;
					case VK_MENU:
						imgui.KeyAlt = _umsg == WM_KEYDOWN;
						break;
					case VK_LSHIFT:
					case VK_RSHIFT:
						imgui.KeyShift = _umsg == WM_KEYDOWN;
						break;
					case VK_ESCAPE:
						PostQuitMessage(0);
						break;
					default:
						if (vk < 512)
						{
							imgui.KeysDown[vk] = _umsg == WM_KEYDOWN;
						}
						break;
				};
				return 0;
			}
			case WM_CHAR:
				if (_wparam > 0 && _wparam < 0x10000)
				{
					imgui.AddInputCharacter((unsigned short)_wparam);
				}
				return 0;
			case WM_PAINT:
				//IM3D_ASSERT(false); // should be suppressed by calling ValidateRect()
				break;
			case WM_CLOSE:
				PostQuitMessage(0);
				return 0; // prevent DefWindowProc from destroying the window
			default:
				break;
		};

		return DefWindowProc(_hwnd, _umsg, _wparam, _lparam);
	}
	
	static bool InitWindow(int& _width_, int& _height_, const char* _title)
	{
		static ATOM wndclassex = 0;
		if (wndclassex == 0)
		{
			WNDCLASSEX wc;
			memset(&wc, 0, sizeof(wc));
			wc.cbSize = sizeof(wc);
			wc.style = CS_OWNDC;// | CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc = WindowProc;
			wc.hInstance = GetModuleHandle(0);
			wc.lpszClassName = "Im3dTestApp";
			wc.hCursor = LoadCursor(0, IDC_ARROW);
			winAssert(wndclassex = RegisterClassEx(&wc));
		}
	
		DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_MINIMIZEBOX | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	
		if (_width_ == -1 || _height_ == -1)
		{
		 // auto size; get the dimensions of the primary screen area and subtract the non-client area
			RECT r;
			winAssert(SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0));
			_width_  = r.right - r.left;
			_height_ = r.bottom - r.top;
	
			RECT wr = {};
			winAssert(AdjustWindowRectEx(&wr, dwStyle, FALSE, dwExStyle));
			_width_  -= wr.right - wr.left;
			_height_ -= wr.bottom - wr.top;
		}
	
		RECT r; r.top = 0; r.left = 0; r.bottom = _height_; r.right = _width_;
		winAssert(AdjustWindowRectEx(&r, dwStyle, FALSE, dwExStyle));
		g_Example->m_hwnd = CreateWindowEx(
			dwExStyle, 
			MAKEINTATOM(wndclassex), 
			_title, 
			dwStyle, 
			0, 0, 
			r.right - r.left, r.bottom - r.top, 
			nullptr, 
			nullptr, 
			GetModuleHandle(0), 
			nullptr
			);
		IM3D_ASSERT(g_Example->m_hwnd);
		ShowWindow(g_Example->m_hwnd, SW_SHOW);

		return true;
	}
	
	static void ShutdownWindow()
	{
		if (g_Example->m_hwnd)
		{
			winAssert(DestroyWindow(g_Example->m_hwnd));
		}
	}
	
	#if defined(IM3D_OPENGL)
		#include "GL/wglew.h"
		static PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormat    = 0;
		static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs = 0;

		
		static bool InitOpenGL(int _vmaj, int _vmin)
		{
			HWND hwnd = g_Example->m_hwnd;
			winAssert(g_Example->m_hdc = GetDC(hwnd));
			HDC hdc = g_Example->m_hdc;

		 // set the window pixel format
			PIXELFORMATDESCRIPTOR pfd = {};
			pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
			pfd.nVersion     = 1;
			pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_GENERIC_ACCELERATED;
			pfd.iPixelType   = PFD_TYPE_RGBA;
			pfd.cColorBits   = 24;
			pfd.cDepthBits   = 24;
			pfd.dwDamageMask = 8;
			int pformat = 0;
			winAssert(pformat = ChoosePixelFormat(hdc, &pfd));
			winAssert(SetPixelFormat(hdc, pformat, &pfd));
			
		 // create dummy context to load wgl extensions
			HGLRC hglrc = 0;
			winAssert(hglrc = wglCreateContext(hdc));
			winAssert(wglMakeCurrent(hdc, hglrc));
		
		 // check the platform supports the requested GL version
			GLint platformVMaj, platformVMin;
			glAssert(glGetIntegerv(GL_MAJOR_VERSION, &platformVMaj));
			glAssert(glGetIntegerv(GL_MINOR_VERSION, &platformVMin));
			_vmaj = _vmaj < 0 ? platformVMaj : _vmaj;
			_vmin = _vmin < 0 ? platformVMin : _vmin;
			if (platformVMaj < _vmaj || (platformVMaj >= _vmaj && platformVMin < _vmin))
			{
				fprintf(stderr, "OpenGL version %d.%d is not available (available version is %d.%d).", _vmaj, _vmin, platformVMaj, platformVMin);
				fprintf(stderr, "This error may occur if the platform has an integrated GPU.");

				return false;
			}
			
		 // load wgl extensions for true context creation
			static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs;
			winAssert(wglCreateContextAttribs = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB"));
		
		 // delete the dummy context
			winAssert(wglMakeCurrent(0, 0));
			winAssert(wglDeleteContext(hglrc));
		
		 // create true context
			int profileBit = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
			//profileBit = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
			int attr[] =
				{
					WGL_CONTEXT_MAJOR_VERSION_ARB,	_vmaj,
					WGL_CONTEXT_MINOR_VERSION_ARB,	_vmin,
					WGL_CONTEXT_PROFILE_MASK_ARB,	profileBit,
					0
				};
			winAssert(g_Example->m_hglrc = wglCreateContextAttribs(hdc, 0, attr));
			hglrc = g_Example->m_hglrc;
		
		// load extensions
			if (!wglMakeCurrent(hdc, hglrc))
			{
				fprintf(stderr, "wglMakeCurrent failed");
				return false;
			}
			glewExperimental = GL_TRUE;
			GLenum err = glewInit();
			IM3D_ASSERT(err == GLEW_OK);
			glGetError(); // clear any errors caused by glewInit()

			winAssert(wglSwapIntervalEXT(0)); // example uses FPS as a rough perf measure, hence disable vsync

			fprintf(stdout, "OpenGL context:\n\tVersion: %s\n\tGLSL Version: %s\n\tVendor: %s\n\tRenderer: %s\n",
				GlGetString(GL_VERSION),
				GlGetString(GL_SHADING_LANGUAGE_VERSION),
				GlGetString(GL_VENDOR),
				GlGetString(GL_RENDERER)
				);

			if (_vmaj == 3 && _vmin == 1)
			{
			 // check that the uniform blocks size is at least 64kb
				GLint maxUniformBlockSize; 
				glAssert(glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize));
				if (maxUniformBlockSize < (64*1024))
				{
					IM3D_ASSERT(false);
					fprintf(stderr, "GL_MAX_UNIFORM_BLOCK_SIZE is less than 64kb (%dkb)", maxUniformBlockSize / 1024);
					return false;
				}
			}

			return true;
		}
		
		static void ShutdownOpenGL()
		{
			winAssert(wglMakeCurrent(0, 0));
			winAssert(wglDeleteContext(g_Example->m_hglrc));
			winAssert(ReleaseDC(g_Example->m_hwnd, g_Example->m_hdc) != 0);
		}
		
	#elif defined(IM3D_DX11)
		#include <d3dcompiler.h>	

		static bool InitDx11()
		{
			g_Example->m_dxgiSwapChain   = nullptr;
			g_Example->m_d3dDevice       = nullptr;
			g_Example->m_d3dDeviceCtx    = nullptr;
			g_Example->m_d3dRenderTarget = nullptr;
			g_Example->m_d3dDepthStencil = nullptr;

			DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
			swapChainDesc.OutputWindow = g_Example->m_hwnd;
			swapChainDesc.Windowed = TRUE;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapChainDesc.BufferCount = 2;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
			swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			swapChainDesc.SampleDesc.Count = 1;

			UINT createDeviceFlags = 0;
			//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
			D3D_FEATURE_LEVEL featureLevel;
			const D3D_FEATURE_LEVEL featureLevelArray[1] = { D3D_FEATURE_LEVEL_11_0, };
			dxAssert(D3D11CreateDeviceAndSwapChain(
				nullptr, 
				D3D_DRIVER_TYPE_HARDWARE, 
				nullptr, 
				createDeviceFlags, 
				featureLevelArray, 
				1, 
				D3D11_SDK_VERSION, 
				&swapChainDesc,
				&g_Example->m_dxgiSwapChain,
				&g_Example->m_d3dDevice, 
				&featureLevel, 
				&g_Example->m_d3dDeviceCtx
				));
			if (!g_Example->m_dxgiSwapChain || !g_Example->m_d3dDevice || !g_Example->m_d3dDeviceCtx)
			{
				fprintf(stderr, "Error initializing DirectX");
				return false;
			}

			ID3D11Texture2D* backBuffer;
			D3D11_TEXTURE2D_DESC backBufferDesc;
			dxAssert(g_Example->m_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer));
			backBuffer->GetDesc(&backBufferDesc);
			dxAssert(g_Example->m_d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &g_Example->m_d3dRenderTarget));
			g_Example->m_d3dDepthStencil = CreateDepthStencil(backBufferDesc.Width, backBufferDesc.Height, DXGI_FORMAT_D24_UNORM_S8_UINT);
			g_Example->m_d3dDeviceCtx->OMSetRenderTargets(1, &g_Example->m_d3dRenderTarget, g_Example->m_d3dDepthStencil);
			backBuffer->Release();
			
			return true;
		}
		
		static void ShutdownDx11()
		{
			if (g_Example->m_d3dDepthStencil) g_Example->m_d3dDepthStencil->Release();
			if (g_Example->m_d3dRenderTarget) g_Example->m_d3dRenderTarget->Release();
			if (g_Example->m_dxgiSwapChain)   g_Example->m_dxgiSwapChain->Release();
			if (g_Example->m_d3dDeviceCtx)    g_Example->m_d3dDeviceCtx->Release();
			if (g_Example->m_d3dDevice)       g_Example->m_d3dDevice->Release();
		}
		
	#endif // graphics
	
#endif // platform

/******************************************************************************/
static void Append(const char* _str, Vector<char>& _out_)
{
	while (*_str)
	{
		_out_.push_back(*_str);
		++_str;
	}
}
static void AppendLine(const char* _str, Vector<char>& _out_)
{
	Append(_str, _out_);
	_out_.push_back('\n');
}
static bool LoadShader(const char* _path, const char* _defines, Vector<char>& _out_)
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
	
	FILE* fin = fopen(_path, "rb");
	if (!fin)
	{
		fprintf(stderr, "Error opening '%s'\n", _path);
		return false;
	}
	IM3D_VERIFY(fseek(fin, 0, SEEK_END) == 0); // not portable but should work almost everywhere
	long fsize = ftell(fin);
	IM3D_VERIFY(fseek(fin, 0, SEEK_SET) == 0);
	
	int srcbeg = _out_.size();
	_out_.resize(srcbeg + fsize, '\0');
	if (fread(_out_.data() + srcbeg, 1, fsize, fin) != fsize)
	{
		fclose(fin);
		fprintf(stderr, "Error reading '%s'\n", _path);
		return false;
	}
	fclose(fin);
	_out_.push_back('\0');

	return true;
}

#if defined(IM3D_OPENGL)
	GLuint Im3d::LoadCompileShader(GLenum _stage, const char* _path, const char* _defines)
	{
		Vector<char> src;
		AppendLine("#version " IM3D_STRINGIFY(IM3D_OPENGL_VSHADER), src);
		if (!LoadShader(_path, _defines, src))
		{
			return 0;
		}
	
		GLuint ret = 0;
		glAssert(ret = glCreateShader(_stage));
		const GLchar* pd = src.data();
		GLint ps = src.size();
		glAssert(glShaderSource(ret, 1, &pd, &ps));
	
		glAssert(glCompileShader(ret));
		GLint compileStatus = GL_FALSE;
		glAssert(glGetShaderiv(ret, GL_COMPILE_STATUS, &compileStatus));
		if (compileStatus == GL_FALSE)
		{
			fprintf(stderr, "Error compiling '%s':\n\n", _path);
			GLint len;
			glAssert(glGetShaderiv(ret, GL_INFO_LOG_LENGTH, &len));
			char* log = new GLchar[len];
			glAssert(glGetShaderInfoLog(ret, len, 0, log));
			fprintf(stderr, log);
			delete[] log;
	
			//fprintf(stderr, "\n\n%s", src.data());
			fprintf(stderr, "\n");
			glAssert(glDeleteShader(ret));

			return 0;
		}

		return ret;
	}
	
	bool Im3d::LinkShaderProgram(GLuint _handle)
	{
		IM3D_ASSERT(_handle != 0);
	
		glAssert(glLinkProgram(_handle));
		GLint linkStatus = GL_FALSE;
		glAssert(glGetProgramiv(_handle, GL_LINK_STATUS, &linkStatus));
		if (linkStatus == GL_FALSE)
		{
			fprintf(stderr, "Error linking program:\n\n");
			GLint len;
			glAssert(glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &len));
			GLchar* log = new GLchar[len];
			glAssert(glGetProgramInfoLog(_handle, len, 0, log));
			fprintf(stderr, log);
			fprintf(stderr, "\n");
			delete[] log;
	
			return false;
		}

		return true;
	}

	void Im3d::DrawNdcQuad()
	{
		static GLuint vbQuad;
		static GLuint vaQuad;
		if (vbQuad == 0)
		{
			float quadv[8] =
				{
					-1.0f, -1.0f,
					 1.0f, -1.0f,
					-1.0f,  1.0f,
					 1.0f,  1.0f,
				};
			glAssert(glGenBuffers(1, &vbQuad));
			glAssert(glGenVertexArrays(1, &vaQuad));	
			glAssert(glBindVertexArray(vaQuad));
			glAssert(glBindBuffer(GL_ARRAY_BUFFER, vbQuad));
			glAssert(glEnableVertexAttribArray(0));
			glAssert(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (GLvoid*)0));
			glAssert(glBufferData(GL_ARRAY_BUFFER, sizeof(quadv), (GLvoid*)quadv, GL_STATIC_DRAW));
			glAssert(glBindVertexArray(0));	
		}
		glAssert(glBindVertexArray(vaQuad));
		glAssert(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
		glAssert(glBindVertexArray(0));
	}

	void Im3d::DrawTeapot(const Mat4& _world, const Mat4& _viewProj)
	{
		static GLuint shTeapot;
		static GLuint vbTeapot;
		static GLuint ibTeapot;
		static GLuint vaTeapot;
		if (shTeapot == 0)
		{
			GLuint vs = LoadCompileShader(GL_VERTEX_SHADER,   "model.glsl", "VERTEX_SHADER\0");
			GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "model.glsl", "FRAGMENT_SHADER\0");
			if (vs && fs)
			{
				glAssert(shTeapot = glCreateProgram());
				glAssert(glAttachShader(shTeapot, vs));
				glAssert(glAttachShader(shTeapot, fs));
				bool ret = LinkShaderProgram(shTeapot);
				glAssert(glDeleteShader(vs));
				glAssert(glDeleteShader(fs));
				if (!ret)
				{
					return;
				}
			}
			else
			{
				return;
			}

			glAssert(glGenBuffers(1, &vbTeapot));
			glAssert(glGenBuffers(1, &ibTeapot));
			glAssert(glGenVertexArrays(1, &vaTeapot));	
			glAssert(glBindVertexArray(vaTeapot));
			glAssert(glBindBuffer(GL_ARRAY_BUFFER, vbTeapot));
			glAssert(glEnableVertexAttribArray(0));
			glAssert(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3) * 2, (GLvoid*)0));
			glAssert(glEnableVertexAttribArray(1));
			glAssert(glEnableVertexAttribArray(0));
			glAssert(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid*)0));
			glAssert(glEnableVertexAttribArray(1));
			glAssert(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid*)(sizeof(float) * 3)));
			glAssert(glBufferData(GL_ARRAY_BUFFER, sizeof(s_teapotVertices), (GLvoid*)s_teapotVertices, GL_STATIC_DRAW));
			glAssert(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibTeapot));
			glAssert(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(s_teapotIndices), (GLvoid*)s_teapotIndices, GL_STATIC_DRAW));
			glAssert(glBindVertexArray(0));
		}

		glAssert(glUseProgram(shTeapot));
		glAssert(glUniformMatrix4fv(glGetUniformLocation(shTeapot, "uWorldMatrix"), 1, false, _world.m));
		glAssert(glUniformMatrix4fv(glGetUniformLocation(shTeapot, "uViewProjMatrix"), 1, false, _viewProj.m));
		glAssert(glBindVertexArray(vaTeapot));
		glAssert(glEnable(GL_DEPTH_TEST));
		glAssert(glEnable(GL_CULL_FACE));
		glAssert(glDrawElements(GL_TRIANGLES, sizeof(s_teapotIndices) / sizeof(unsigned), GL_UNSIGNED_INT, (GLvoid*)0));
		glAssert(glDisable(GL_DEPTH_TEST));
		glAssert(glDisable(GL_CULL_FACE));
		glAssert(glBindVertexArray(0));
		glAssert(glUseProgram(0));
	}
	
	const char* Im3d::GetGlEnumString(GLenum _enum)
	{
		#define CASE_ENUM(e) case e: return #e
		switch (_enum)
		{
		// errors
			CASE_ENUM(GL_NONE);
			CASE_ENUM(GL_INVALID_ENUM);
			CASE_ENUM(GL_INVALID_VALUE);
			CASE_ENUM(GL_INVALID_OPERATION);
			CASE_ENUM(GL_INVALID_FRAMEBUFFER_OPERATION);
			CASE_ENUM(GL_OUT_OF_MEMORY);
	
			default: return "Unknown GLenum";
		};
		#undef CASE_ENUM
	}
	
	const char* Im3d::GlGetString(GLenum _name)
	{
		const char* ret;
		glAssert(ret = (const char*)glGetString(_name));
		return ret ? ret : "";
	}

#elif defined(IM3D_DX11)
	ID3DBlob* Im3d::LoadCompileShader(const char* _target, const char* _path, const char* _defines)
	{
		Vector<char> src;
		if (!LoadShader(_path, _defines, src))
		{
			return nullptr;
		}
		ID3DBlob* ret = nullptr;
		ID3DBlob* err = nullptr;
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

	 // D3DCompile is not portable - linking with d3dcompiler.lib introdices a dependency on d3dcompiler_XX.lib
	 // \todo get a ptr to D3DCompile at runtime via LoadLibrary/GetProcAddress
		D3DCompile(src.data(), src.size(), nullptr, nullptr, nullptr, "main", _target, flags, 0, &ret, &err);
		if (ret == nullptr)
		{
			fprintf(stderr, "Error compiling '%s':\n\n", _path);
			if (err)
			{
				fprintf(stderr, (char*)err->GetBufferPointer());
				err->Release();
			}
		}

		return ret;
	}

	ID3D11Buffer* Im3d::CreateBuffer(UINT _size, D3D11_USAGE _usage, UINT _bind, const void* _data)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = _size;
		desc.Usage = _usage;
		desc.BindFlags = _bind;
		desc.CPUAccessFlags = _usage == D3D11_USAGE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;

		ID3D11Buffer* ret = nullptr;
		D3D11_SUBRESOURCE_DATA subRes = {};
		subRes.pSysMem = _data;
		dxAssert(g_Example->m_d3dDevice->CreateBuffer(&desc, _data ? &subRes : nullptr, &ret));
		return ret;
	}
	
	ID3D11Buffer* Im3d::CreateConstantBuffer(UINT _size, D3D11_USAGE _usage, const void* _data)
	{
		return CreateBuffer(_size, _usage, D3D11_BIND_CONSTANT_BUFFER, _data);
	}
	ID3D11Buffer* Im3d::CreateVertexBuffer(UINT _size, D3D11_USAGE _usage, const void* _data)
	{
		return CreateBuffer(_size, _usage, D3D11_BIND_VERTEX_BUFFER, _data);
	}
	ID3D11Buffer* Im3d::CreateIndexBuffer(UINT _size, D3D11_USAGE _usage, const void* _data)
	{
		return CreateBuffer(_size, _usage, D3D11_BIND_INDEX_BUFFER, _data);
	}

	void* Im3d::MapBuffer(ID3D11Buffer* _buffer, D3D11_MAP _mapType)
	{
		D3D11_MAPPED_SUBRESOURCE subRes;
		dxAssert(g_Example->m_d3dDeviceCtx->Map(_buffer, 0, _mapType, 0, &subRes));
		return subRes.pData;
	}

	void  Im3d::UnmapBuffer(ID3D11Buffer* _buffer)
	{
		g_Example->m_d3dDeviceCtx->Unmap(_buffer, 0);
	}

	ID3D11Texture2D* Im3d::CreateTexture2D(UINT _width, UINT _height, DXGI_FORMAT _format, ID3D11ShaderResourceView** resView_, const void* _data)
	{
		ID3D11Device* d3d = g_Example->m_d3dDevice;

		D3D11_TEXTURE2D_DESC txDesc = {};
		txDesc.Width = _width;
		txDesc.Height = _height;
		txDesc.MipLevels = 1;
		txDesc.ArraySize = 1;
		txDesc.Format = _format;
		txDesc.SampleDesc.Count = 1;
		txDesc.Usage = D3D11_USAGE_DEFAULT;
		txDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		ID3D11Texture2D* ret = nullptr;
		D3D11_SUBRESOURCE_DATA subRes = {};
		if (_data)
		{
			subRes.pSysMem = _data;
			subRes.SysMemPitch = _width;
			switch (_format) {
				case DXGI_FORMAT_R8_UNORM:       break;
				case DXGI_FORMAT_R8G8B8A8_UNORM:
				default:                         subRes.SysMemPitch *= 4;
			};
		}
		dxAssert(d3d->CreateTexture2D(&txDesc, _data ? &subRes : nullptr, &ret));

		if (resView_)
		{
			dxAssert(d3d->CreateShaderResourceView(ret, nullptr, resView_));
		}

		return ret;
	}

	ID3D11DepthStencilView* Im3d::CreateDepthStencil(UINT _width, UINT _height, DXGI_FORMAT _format)
	{
		ID3D11Device* d3d = g_Example->m_d3dDevice;

		D3D11_TEXTURE2D_DESC txDesc = {};
		txDesc.Width = _width;
		txDesc.Height = _height;
		txDesc.MipLevels = 1;
		txDesc.ArraySize = 1;
		txDesc.Format = _format;
		txDesc.SampleDesc.Count = 1;
		txDesc.Usage = D3D11_USAGE_DEFAULT;
		txDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		ID3D11Texture2D* tx = nullptr;
		ID3D11DepthStencilView* ret;
		dxAssert(d3d->CreateTexture2D(&txDesc, nullptr, &tx));
		dxAssert(d3d->CreateDepthStencilView(tx, nullptr, &ret));
		return ret;
	}
	
	void Im3d::DrawNdcQuad()
	{
	}
	
	void Im3d::DrawTeapot(const Mat4& _world, const Mat4& _viewProj)
	{
		static ID3DBlob*              s_vsBlob;
		static ID3D11VertexShader*    s_vs;
		static ID3DBlob*              s_psBlob;
		static ID3D11PixelShader*     s_ps;
		static ID3D11InputLayout*     s_inputLayout;
		static ID3D11Buffer*          s_vb;
		static ID3D11Buffer*          s_ib;
		static ID3D11Buffer*          s_cb;
		static ID3D11RasterizerState* s_rasterizerState;

		ID3D11Device* d3d = g_Example->m_d3dDevice;
		ID3D11DeviceContext* ctx = g_Example->m_d3dDeviceCtx;

		if (s_vsBlob == 0)
		{
			s_vsBlob = LoadCompileShader("vs_" IM3D_DX11_VSHADER, "model.hlsl", "VERTEX_SHADER\0");
			dxAssert(d3d->CreateVertexShader((DWORD*)s_vsBlob->GetBufferPointer(), s_vsBlob->GetBufferSize(), nullptr, &s_vs));
			s_psBlob = LoadCompileShader("ps_" IM3D_DX11_VSHADER, "model.hlsl", "PIXEL_SHADER\0");
			dxAssert(d3d->CreatePixelShader((DWORD*)s_psBlob->GetBufferPointer(), s_psBlob->GetBufferSize(), nullptr, &s_ps));

			s_vb = CreateVertexBuffer(sizeof(s_teapotVertices), D3D11_USAGE_IMMUTABLE, s_teapotVertices);
			s_ib = CreateIndexBuffer(sizeof(s_teapotIndices), D3D11_USAGE_IMMUTABLE, s_teapotIndices); 
		
			D3D11_INPUT_ELEMENT_DESC inputDesc[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,  0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,  0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
				};
			dxAssert(d3d->CreateInputLayout(inputDesc, 2, s_vsBlob->GetBufferPointer(), s_vsBlob->GetBufferSize(), &s_inputLayout));
		
			s_cb = CreateConstantBuffer(sizeof(Mat4) * 2, D3D11_USAGE_DYNAMIC);

			D3D11_RASTERIZER_DESC rasterizerDesc = {};
			rasterizerDesc.FillMode = D3D11_FILL_SOLID;
			rasterizerDesc.CullMode = D3D11_CULL_BACK;
			rasterizerDesc.FrontCounterClockwise = true;
			dxAssert(d3d->CreateRasterizerState(&rasterizerDesc, &s_rasterizerState));
		}

		Mat4* cbData = (Mat4*)MapBuffer(s_cb, D3D11_MAP_WRITE_DISCARD);
		cbData[0] = _world;
		cbData[1] = _viewProj;
		UnmapBuffer(s_cb);

		unsigned int stride = 4 * 3 * 2;
		unsigned int offset = 0;
		ctx->IASetInputLayout(s_inputLayout);
		ctx->IASetVertexBuffers(0, 1, &s_vb, &stride, &offset);
		ctx->IASetIndexBuffer(s_ib, DXGI_FORMAT_R32_UINT, 0);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ctx->VSSetShader(s_vs, nullptr, 0);
		ctx->VSSetConstantBuffers(0, 1, &s_cb);
		ctx->PSSetShader(s_ps, nullptr, 0);

		ctx->OMSetBlendState(nullptr, nullptr, 0xffffffff);
		ctx->OMSetDepthStencilState(nullptr, 0);
		ctx->RSSetState(s_rasterizerState);

		ctx->DrawIndexed(sizeof(s_teapotIndices) / sizeof(unsigned), 0, 0);
	}
	
#endif // graphics

/******************************************************************************/
void Im3d::Assert(const char* _e, const char* _file, int _line, const char* _msg, ...)
{
	const int kAssertMsgMax = 1024;

	char buf[kAssertMsgMax];
	if (_msg != nullptr)
	{
		va_list args;
		va_start(args, _msg);
		vsnprintf(buf, kAssertMsgMax, _msg, args);
		va_end(args);
	}
	else
	{
		buf[0] = '\0';
	}
	fprintf(stderr, "Assert (%s, line %d)\n\t'%s' %s", StripPath(_file), _line, _e ? _e : "", buf);
}

void Im3d::RandSeed(int _seed)
{
	srand(_seed);
}
int Im3d::RandInt(int _min, int _max)
{
	return _min + (int)rand() % (_max - _min);
}
float Im3d::RandFloat(float _min, float _max)
{
	return _min + (float)rand() / (float)RAND_MAX * (_max - _min);
}
Mat3 Im3d::RandRotation()
{
	return Rotation(Normalize(RandVec3(-1.0f, 1.0f)), RandFloat(-Pi, Pi));
}
Vec3 Im3d::RandVec3(float _min, float _max)
{
	return Im3d::Vec3(
		RandFloat(_min, _max),
		RandFloat(_min, _max),
		RandFloat(_min, _max)
		);
}
Color Im3d::RandColor(float _min, float _max)
{
	Vec3 v = RandVec3(_min, _max);
	return Color(v.x, v.y, v.z);
}

/******************************************************************************/
#if defined(IM3D_OPENGL)
	static GLuint g_ImGuiVertexArray;
	static GLuint g_ImGuiVertexBuffer;
	static GLuint g_ImGuiIndexBuffer;
	static GLuint g_ImGuiShader;
	static GLuint g_ImGuiFontTexture;

	static void ImGui_Draw(ImDrawData* _drawData)
	{
		ImGuiIO& io = ImGui::GetIO();
	
		int fbX, fbY;
		fbX = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
		fbY = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
		if (fbX == 0  || fbY == 0)
		{
			return;
		}
		_drawData->ScaleClipRects(io.DisplayFramebufferScale);
	
		glAssert(glViewport(0, 0, (GLsizei)fbX, (GLsizei)fbY));
		glAssert(glEnable(GL_BLEND));
		glAssert(glBlendEquation(GL_FUNC_ADD));
		glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		glAssert(glDisable(GL_CULL_FACE));
		glAssert(glDisable(GL_DEPTH_TEST));
		glAssert(glEnable(GL_SCISSOR_TEST));
		glAssert(glActiveTexture(GL_TEXTURE0));
		
		Mat4 ortho = Mat4(
			2.0f/io.DisplaySize.x, 0.0f,                   0.0f, -1.0f,
			0.0f,                  2.0f/-io.DisplaySize.y, 0.0f,  1.0f,
			0.0f,                  0.0f,                   1.0f,  0.0f
			);
		glAssert(glUseProgram(g_ImGuiShader));
	
		bool transpose = false;
		#ifdef IM3D_MATRIX_ROW_MAJOR
			transpose = true;
		#endif
		glAssert(glUniformMatrix4fv(glGetUniformLocation(g_ImGuiShader, "uProjMatrix"), 1, transpose, (const GLfloat*)ortho));
		glAssert(glBindVertexArray(g_ImGuiVertexArray));
	
		for (int i = 0; i < _drawData->CmdListsCount; ++i)
		{
			const ImDrawList* drawList = _drawData->CmdLists[i];
			const ImDrawIdx* indexOffset = 0;
	
			glAssert(glBindBuffer(GL_ARRAY_BUFFER, g_ImGuiVertexBuffer));
			glAssert(glBufferData(GL_ARRAY_BUFFER, drawList->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid*)&drawList->VtxBuffer.front(), GL_STREAM_DRAW));
			glAssert(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ImGuiIndexBuffer));
			glAssert(glBufferData(GL_ELEMENT_ARRAY_BUFFER, drawList->IdxBuffer.Size * sizeof(ImDrawIdx), (GLvoid*)drawList->IdxBuffer.Data, GL_STREAM_DRAW));
	
			for (const ImDrawCmd* pcmd = drawList->CmdBuffer.begin(); pcmd != drawList->CmdBuffer.end(); ++pcmd)
			{
				if (pcmd->UserCallback)
				{
					pcmd->UserCallback(drawList, pcmd);
				}
				else
				{
					glAssert(glBindTexture(GL_TEXTURE_2D, (GLuint)pcmd->TextureId));
					glAssert(glScissor((int)pcmd->ClipRect.x, (int)(fbY - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y)));
					glAssert(glDrawElements(GL_TRIANGLES, pcmd->ElemCount, GL_UNSIGNED_SHORT, (GLvoid*)indexOffset));
				}
				indexOffset += pcmd->ElemCount;
			}
		}
	
		glAssert(glDisable(GL_SCISSOR_TEST));
		glAssert(glDisable(GL_BLEND));
		glAssert(glUseProgram(0));
	}

	static bool ImGui_Init()
	{
		GLuint vs = LoadCompileShader(GL_VERTEX_SHADER,   "imgui.glsl", "VERTEX_SHADER\0");
		GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "imgui.glsl", "FRAGMENT_SHADER\0");
		if (vs && fs)
		{
			glAssert(g_ImGuiShader = glCreateProgram());
			glAssert(glAttachShader(g_ImGuiShader, vs));
			glAssert(glAttachShader(g_ImGuiShader, fs));
			bool ret = LinkShaderProgram(g_ImGuiShader);
			glAssert(glDeleteShader(vs));
			glAssert(glDeleteShader(fs));
			if (!ret)
			{
				return false;
			}
		}
		else
		{
			return false;
		}

		glAssert(glUseProgram(g_ImGuiShader));
		glAssert(glUniform1i(glGetUniformLocation(g_ImGuiShader, "txTexture"), 0));
		glAssert(glUseProgram(0));

		glAssert(glGenBuffers(1, &g_ImGuiVertexBuffer));
		glAssert(glGenBuffers(1, &g_ImGuiIndexBuffer));
		glAssert(glGenVertexArrays(1, &g_ImGuiVertexArray));	
		glAssert(glBindVertexArray(g_ImGuiVertexArray));
		glAssert(glBindBuffer(GL_ARRAY_BUFFER, g_ImGuiVertexBuffer));
		glAssert(glEnableVertexAttribArray(0));
		glAssert(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, pos)));
		glAssert(glEnableVertexAttribArray(1));
		glAssert(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, uv)));
		glAssert(glEnableVertexAttribArray(2));
		glAssert(glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, col)));
		glAssert(glBindVertexArray(0));
	
		unsigned char* txbuf;
		int txX, txY;
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->GetTexDataAsAlpha8(&txbuf, &txX, &txY);
		glAssert(glGenTextures(1, &g_ImGuiFontTexture));
		glAssert(glBindTexture(GL_TEXTURE_2D, g_ImGuiFontTexture));
		glAssert(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		glAssert(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		glAssert(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, txX, txY, 0, GL_RED, GL_UNSIGNED_BYTE, (const GLvoid*)txbuf));
		io.Fonts->TexID = (void*)g_ImGuiFontTexture;
	
		io.RenderDrawListsFn = &ImGui_Draw;

		ImGui::StyleColorsDark();

		return true;

	}

	static void ImGui_Shutdown()
	{
		glAssert(glDeleteVertexArrays(1, &g_ImGuiVertexArray));
		glAssert(glDeleteBuffers(1, &g_ImGuiVertexBuffer));
		glAssert(glDeleteBuffers(1, &g_ImGuiIndexBuffer));		
		glAssert(glDeleteProgram(g_ImGuiShader));
		glAssert(glDeleteTextures(1, &g_ImGuiFontTexture));
	}

#elif defined(IM3D_DX11)
	static ID3D11InputLayout*        g_ImGuiInputLayout;
	static ID3DBlob*                 g_ImGuiVertexShaderBlob;
	static ID3D11VertexShader*       g_ImGuiVertexShader;
	static ID3DBlob*                 g_ImGuiPixelShaderBlob;
	static ID3D11PixelShader*        g_ImGuiPixelShader;
	static ID3D11RasterizerState*    g_ImGuiRasterizerState;
	static ID3D11BlendState*         g_ImGuiBlendState;
	static ID3D11DepthStencilState*  g_ImGuiDepthStencilState;
	static ID3D11ShaderResourceView* g_ImGuiFontResourceView;
	static ID3D11SamplerState*       g_ImGuiFontSampler;
	static ID3D11Buffer*             g_ImGuiConstantBuffer;
	static ID3D11Buffer*             g_ImGuiVertexBuffer;
	static ID3D11Buffer*             g_ImGuiIndexBuffer;

	static void ImGui_Draw(ImDrawData* _drawData)
	{
		ImGuiIO& io = ImGui::GetIO();
		ID3D11Device* d3d = g_Example->m_d3dDevice;
		ID3D11DeviceContext* ctx = g_Example->m_d3dDeviceCtx;

	 // (re)alloc vertex/index buffers
		static int s_vertexBufferSize = 0;
		if (!g_ImGuiVertexBuffer || s_vertexBufferSize < _drawData->TotalVtxCount)
		{
			if (g_ImGuiVertexBuffer)
			{ 
				g_ImGuiVertexBuffer->Release(); 
				g_ImGuiVertexBuffer = nullptr;	
			}
			s_vertexBufferSize = _drawData->TotalVtxCount;
			g_ImGuiVertexBuffer = CreateVertexBuffer(s_vertexBufferSize * sizeof(ImDrawVert), D3D11_USAGE_DYNAMIC);
		}
		static int s_indexBufferSize = 0;
		if (!g_ImGuiIndexBuffer || s_indexBufferSize < _drawData->TotalIdxCount)
		{
			if (g_ImGuiIndexBuffer)
			{ 
				g_ImGuiIndexBuffer->Release(); 
				g_ImGuiIndexBuffer = nullptr; 
			}
			s_indexBufferSize = _drawData->TotalIdxCount;
			g_ImGuiIndexBuffer = CreateIndexBuffer(s_indexBufferSize * sizeof(ImDrawIdx), D3D11_USAGE_DYNAMIC);
		}

	 // copy and convert all vertices into a single contiguous buffer
		ImDrawVert* vtxDst = (ImDrawVert*)MapBuffer(g_ImGuiVertexBuffer, D3D11_MAP_WRITE_DISCARD);
		ImDrawIdx* idxDst = (ImDrawIdx*)MapBuffer(g_ImGuiIndexBuffer, D3D11_MAP_WRITE_DISCARD);
		for (int i = 0; i < _drawData->CmdListsCount; ++i)
		{
			const ImDrawList* cmdList = _drawData->CmdLists[i];
			memcpy(vtxDst, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idxDst, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtxDst += cmdList->VtxBuffer.Size;
			idxDst += cmdList->IdxBuffer.Size;
		}
		UnmapBuffer(g_ImGuiVertexBuffer);
		UnmapBuffer(g_ImGuiIndexBuffer);
		
	 // update constant buffer
		*(Mat4*)MapBuffer(g_ImGuiConstantBuffer, D3D11_MAP_WRITE_DISCARD)
			= Mat4(
				2.0f/io.DisplaySize.x, 0.0f,                   0.0f, -1.0f,
				0.0f,                  2.0f/-io.DisplaySize.y, 0.0f,  1.0f,
				0.0f,                  0.0f,                   1.0f,  0.0f
				);
		UnmapBuffer(g_ImGuiConstantBuffer);
		
	 // set state
		D3D11_VIEWPORT viewport = {};
		viewport.Width = ImGui::GetIO().DisplaySize.x;
		viewport.Height = ImGui::GetIO().DisplaySize.y;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = viewport.TopLeftY = 0.0f;
		ctx->RSSetViewports(1, &viewport);

		unsigned int stride = sizeof(ImDrawVert);
		unsigned int offset = 0;
		ctx->IASetInputLayout(g_ImGuiInputLayout);
		ctx->IASetVertexBuffers(0, 1, &g_ImGuiVertexBuffer, &stride, &offset);
		ctx->IASetIndexBuffer(g_ImGuiIndexBuffer, sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ctx->VSSetShader(g_ImGuiVertexShader, nullptr, 0);
		ctx->VSSetConstantBuffers(0, 1, &g_ImGuiConstantBuffer);
		ctx->PSSetShader(g_ImGuiPixelShader, nullptr, 0);
		ctx->PSSetSamplers(0, 1, &g_ImGuiFontSampler);

		ctx->OMSetBlendState(g_ImGuiBlendState, nullptr, 0xffffffff);
		ctx->OMSetDepthStencilState(g_ImGuiDepthStencilState, 0);
		ctx->RSSetState(g_ImGuiRasterizerState);
	
		int vtxOffset = 0;
		int idxOffset = 0;
		for (int i = 0; i < _drawData->CmdListsCount; ++i)
		{
			const ImDrawList* cmdList = _drawData->CmdLists[i];
			for (const ImDrawCmd* pcmd = cmdList->CmdBuffer.begin(); pcmd != cmdList->CmdBuffer.end(); ++pcmd)
			{
				if (pcmd->UserCallback)
				{
					pcmd->UserCallback(cmdList, pcmd);
				}
				else
				{
					const D3D11_RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
					ctx->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)&pcmd->TextureId);
					ctx->RSSetScissorRects(1, &r);
					ctx->DrawIndexed(pcmd->ElemCount, idxOffset, vtxOffset);
				}
				idxOffset += pcmd->ElemCount;
			}
			vtxOffset += cmdList->VtxBuffer.Size;
		}
	}
	
	static bool ImGui_Init()
	{
		ImGuiIO& io = ImGui::GetIO();
		ID3D11Device* d3d = g_Example->m_d3dDevice;

		{	g_ImGuiVertexShaderBlob = LoadCompileShader("vs_4_0", "imgui.hlsl", "VERTEX_SHADER\0");
			if (!g_ImGuiVertexShaderBlob)
			{
				return false;
			}
			dxAssert(d3d->CreateVertexShader((DWORD*)g_ImGuiVertexShaderBlob->GetBufferPointer(), g_ImGuiVertexShaderBlob->GetBufferSize(), nullptr, &g_ImGuiVertexShader));
		}
		{	g_ImGuiPixelShaderBlob = LoadCompileShader("ps_4_0", "imgui.hlsl", "PIXEL_SHADER\0");
			if (!g_ImGuiPixelShaderBlob)
			{
				return false;
			}
			dxAssert(d3d->CreatePixelShader((DWORD*)g_ImGuiPixelShaderBlob->GetBufferPointer(), g_ImGuiPixelShaderBlob->GetBufferSize(), nullptr, &g_ImGuiPixelShader));
		}
		{	D3D11_INPUT_ELEMENT_DESC desc[] = 
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)offsetof(ImDrawVert, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)offsetof(ImDrawVert, uv),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (UINT)offsetof(ImDrawVert, col), D3D11_INPUT_PER_VERTEX_DATA, 0 },
				};
			dxAssert(d3d->CreateInputLayout(desc, 3, g_ImGuiVertexShaderBlob->GetBufferPointer(), g_ImGuiVertexShaderBlob->GetBufferSize(), &g_ImGuiInputLayout));
		}
		g_ImGuiConstantBuffer = CreateConstantBuffer(sizeof(Mat4), D3D11_USAGE_DYNAMIC);
		
		{	D3D11_RASTERIZER_DESC desc = {};
			desc.FillMode = D3D11_FILL_SOLID;
			desc.CullMode = D3D11_CULL_NONE;
			desc.ScissorEnable = true;
			desc.DepthClipEnable = true;
			dxAssert(d3d->CreateRasterizerState(&desc, &g_ImGuiRasterizerState));
		}
		{	D3D11_DEPTH_STENCIL_DESC desc = {};
			desc.DepthEnable = false;
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			desc.StencilEnable = false;
			dxAssert(d3d->CreateDepthStencilState(&desc, &g_ImGuiDepthStencilState));
		}
		{	D3D11_BLEND_DESC desc = {};
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			dxAssert(d3d->CreateBlendState(&desc, &g_ImGuiBlendState));
		}

		{	D3D11_SAMPLER_DESC desc = {};
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			dxAssert(d3d->CreateSamplerState(&desc, &g_ImGuiFontSampler));
		}

		unsigned char* txbuf;
		int txX, txY;
		io.Fonts->GetTexDataAsAlpha8(&txbuf, &txX, &txY);
		CreateTexture2D(txX, txY, DXGI_FORMAT_R8_UNORM, &g_ImGuiFontResourceView, txbuf)->Release();
		
		io.Fonts->TexID = (void*)g_ImGuiFontResourceView;
		io.RenderDrawListsFn = &ImGui_Draw;

		return true;
	}

	static void ImGui_Shutdown()
	{
		if (g_ImGuiConstantBuffer)     g_ImGuiConstantBuffer->Release();
		if (g_ImGuiVertexBuffer)       g_ImGuiVertexBuffer->Release();
		if (g_ImGuiIndexBuffer)        g_ImGuiIndexBuffer->Release();
		if (g_ImGuiFontResourceView)   g_ImGuiFontResourceView->Release();
		if (g_ImGuiFontSampler)        g_ImGuiFontSampler->Release();
		if (g_ImGuiBlendState)         g_ImGuiBlendState->Release();
		if (g_ImGuiDepthStencilState)  g_ImGuiDepthStencilState->Release();
		if (g_ImGuiRasterizerState)    g_ImGuiRasterizerState->Release();
		if (g_ImGuiInputLayout)        g_ImGuiInputLayout->Release();
		if (g_ImGuiVertexShader)       g_ImGuiVertexShader->Release();
		if (g_ImGuiVertexShaderBlob)   g_ImGuiVertexShaderBlob->Release();
		if (g_ImGuiPixelShader)        g_ImGuiPixelShader->Release();
		if (g_ImGuiPixelShaderBlob)    g_ImGuiPixelShaderBlob->Release();
	}

#endif

#if defined(IM3D_PLATFORM_WIN)
	static void ImGui_Update()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab]        = VK_TAB;
		io.KeyMap[ImGuiKey_LeftArrow]  = VK_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow]	   = VK_UP;
		io.KeyMap[ImGuiKey_DownArrow]  = VK_DOWN;
		io.KeyMap[ImGuiKey_PageUp]	   = VK_PRIOR;
		io.KeyMap[ImGuiKey_PageDown]   = VK_NEXT;
		io.KeyMap[ImGuiKey_Home]	   = VK_HOME;
		io.KeyMap[ImGuiKey_End]		   = VK_END;
		io.KeyMap[ImGuiKey_Delete]	   = VK_DELETE;
		io.KeyMap[ImGuiKey_Backspace]  = VK_BACK;
		io.KeyMap[ImGuiKey_Enter]	   = VK_RETURN;
		io.KeyMap[ImGuiKey_Escape]	   = VK_ESCAPE;
		io.KeyMap[ImGuiKey_A]		   = 0x41;
		io.KeyMap[ImGuiKey_C]		   = 0x43;
		io.KeyMap[ImGuiKey_V]		   = 0x56;
		io.KeyMap[ImGuiKey_X]		   = 0x58;
		io.KeyMap[ImGuiKey_Y]		   = 0x59;
		io.KeyMap[ImGuiKey_Z]		   = 0x5A;

		io.ImeWindowHandle = g_Example->m_hwnd;
		io.DisplaySize = ImVec2((float)g_Example->m_width, (float)g_Example->m_height);
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
		io.DeltaTime = g_Example->m_deltaTime;

		ImGui::NewFrame();
	}
#endif

/******************************************************************************/
Example* Im3d::g_Example;

bool Example::init(int _width, int _height, const char* _title)
{
	g_Example = this;
	memset(g_Example, 0, sizeof(Example));

	#if defined(IM3D_PLATFORM_WIN)
	 // force the current working directory to the exe location
		TCHAR buf[MAX_PATH] = {};
		DWORD buflen;
		winAssert(buflen = GetModuleFileName(0, buf, MAX_PATH));
		char* pathend = strrchr(buf, (int)'\\');
		*(++pathend) = '\0';
		winAssert(SetCurrentDirectory(buf));
		fprintf(stdout, "Set current directory: '%s'\n", buf);
		
		winAssert(QueryPerformanceFrequency(&g_SysTimerFreq));
		winAssert(QueryPerformanceCounter(&m_currTime));
	#endif

	ImGui::SetCurrentContext(ImGui::CreateContext()); // can't call this in ImGui_Init() because creating the window ends up calling ImGui::GetIO()

	m_width  = _width;
	m_height = _height;
	m_title  = _title;
	if (!InitWindow(m_width, m_height, m_title))
	{
		goto Example_init_fail;
	}
	#if defined(IM3D_OPENGL) 
		if (!InitOpenGL(IM3D_OPENGL_VMAJ, IM3D_OPENGL_VMIN))
		{
			goto Example_init_fail;
		}
	#elif defined(IM3D_DX11)
		if (!InitDx11())
		{
			goto Example_init_fail;
		}
	#endif

	if (!ImGui_Init())
	{
		goto Example_init_fail;
	}	
	if (!Im3d_Init())
	{
		goto Example_init_fail;
	}

	m_camOrtho = false;
	m_camPos = Vec3(0.0f, 2.0f, 3.0f);
	m_camDir = Normalize(Vec3(0.0f, -0.5f, -1.0f));
	m_camFovDeg = 50.0f;
	
	return true;

Example_init_fail:
	shutdown();
	return false;
}

void Example::shutdown()
{
	ImGui_Shutdown();
	Im3d_Shutdown();

	#if defined(IM3D_OPENGL) 
		ShutdownOpenGL();
	#elif defined(IM3D_DX11)
		ShutdownDx11();
	#endif
	
	ShutdownWindow();

	ImGui::EndFrame(); // prevent assert due to locked font atlas in DestroyContext() call below
	ImGui::DestroyContext();
}

bool Example::update()
{
	bool ret = true;
	#if defined(IM3D_PLATFORM_WIN)
		g_Example->m_prevTime = g_Example->m_currTime;
		winAssert(QueryPerformanceCounter(&m_currTime));
		double microseconds = (double)((g_Example->m_currTime.QuadPart - g_Example->m_prevTime.QuadPart) * 1000000ll / g_SysTimerFreq.QuadPart);
		m_deltaTime = (float)(microseconds / 1000000.0);
	
		MSG msg;
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) && msg.message != WM_QUIT)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		ret = msg.message != WM_QUIT;
	#endif
		
	ImGui_Update();

	float kCamSpeed = 2.0f;
	float kCamSpeedMul = 10.0f;
	float kCamRotationMul = 10.0f;
	m_camWorld = LookAt(m_camPos, m_camPos - m_camDir);
	m_camView = Inverse(m_camWorld);
	#if defined(IM3D_PLATFORM_WIN)
		Vec2 cursorPos = getWindowRelativeCursor();
		if (hasFocus())
		{
			if (!ImGui::GetIO().WantCaptureKeyboard)
			{
				if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
				{
					kCamSpeed *= 10.0f;
				}
				if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) == 0 ) // ctrl not pressed
				{
					if (GetAsyncKeyState(0x57) & 0x8000) // W (forward)
					{
						m_camPos = m_camPos - m_camWorld.getCol(2) * (m_deltaTime * kCamSpeed);
					}
					if (GetAsyncKeyState(0x41) & 0x8000) // A (left)
					{
						m_camPos = m_camPos - m_camWorld.getCol(0) * (m_deltaTime * kCamSpeed);
					}
					if (GetAsyncKeyState(0x53) & 0x8000) // S (backward)
					{
						m_camPos = m_camPos + m_camWorld.getCol(2) * (m_deltaTime * kCamSpeed);
					}
					if (GetAsyncKeyState(0x44) & 0x8000) // D (right)
					{
						m_camPos = m_camPos + m_camWorld.getCol(0) * (m_deltaTime * kCamSpeed);
					}
					if (GetAsyncKeyState(0x51) & 0x8000) // Q (down)
					{
						m_camPos = m_camPos - m_camWorld.getCol(1)* (m_deltaTime * kCamSpeed);
					}
					if (GetAsyncKeyState(0x45) & 0x8000) // D (up)
					{
						m_camPos = m_camPos + m_camWorld.getCol(1) * (m_deltaTime * kCamSpeed);
					}
				}
			}
			if (!ImGui::GetIO().WantCaptureMouse)
			{
				if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
				{
					Vec2 cursorDelta = ((cursorPos - m_prevCursorPos) / Vec2((float)m_width, (float)m_height)) * kCamRotationMul;
					m_camDir = Rotation(Vec3(0.0f, 1.0f, 0.0f), -cursorDelta.x) * m_camDir;
					m_camDir = Rotation(m_camWorld.getCol(0), -cursorDelta.y) * m_camDir;
				}
			}
		}
		m_prevCursorPos = cursorPos;
	#endif

	m_camFovRad = Im3d::Radians(m_camFovDeg);
	float n = 0.1f;
	float f = 500.0f;
	float a = (float)m_width / (float)m_height;
	float scale = tanf(m_camFovRad * 0.5f) * n;
	float viewZ = -1.0f;
	
	if (m_camOrtho)
	{
	 // ortho proj
		scale = 5.0f;
		float r = scale * a;
		float l = -scale * a;
		float t = scale;
		float b = -scale;
		m_camProj = Mat4(
		#if defined(IM3D_OPENGL)
			2.0f / (r - l),      0.0f,                 0.0f,               (r + l) / (l - r),
			0.0f,                2.0f / (t - b),       0.0f,               (t + b) / (b - t),
			0.0f,                0.0f,                 2.0f / (n - f),     (n + f) / (n - f),
			0.0f,                0.0f,                 0.0f,               1.0f
		#elif defined(IM3D_DX11)
			2.0f / (r - l),      0.0f,                 0.0f,               (r + l) / (l - r),
			0.0f,                2.0f / (t - b),       0.0f,               (t + b) / (b - t),
			0.0f,                0.0f,                 1.0f / (n - f),     n / (n - f),
			0.0f,                0.0f,                 0.0f,               1.0f
		#endif
			);
	}
	else
	{
	 // infinite perspective proj
		float r = a * scale;
		float l = -r;
		float t = scale;
		float b = -t;

		m_camProj = Mat4(
		#if defined(IM3D_OPENGL)
			2.0f * n / (r - l),  0.0f,                -viewZ * (r + l) / (r - l),   0.0f,
			0.0f,                2.0f * n / (t - b),  -viewZ * (t + b) / (t - b),   0.0f,
			0.0f,                0.0f,                viewZ,                        -2.0f * n,
			0.0f,                0.0f,                viewZ,                        0.0f
		#elif defined(IM3D_DX11)
			2.0f * n / (r - l),  0.0f,                -viewZ * (r + l) / (r - l),   0.0f,
			0.0f,                2.0f * n / (t - b),  -viewZ * (t + b) / (t - b),   0.0f,
			0.0f,                0.0f,                viewZ,                        -n,
			0.0f,                0.0f,                viewZ,                        0.0f
		#endif
			);
	}

	m_camWorld = LookAt(m_camPos, m_camPos + m_camDir * viewZ);
	m_camView  = Inverse(m_camWorld);
	m_camViewProj = m_camProj * m_camView;

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::Begin(
		"Frame Info", 0, 
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_AlwaysAutoResize
		);
		ImGui::Text("%.2f fps", 1.0f / m_deltaTime);
		ImGui::Text("Layers:    %u ", Im3d::GetContext().getLayerCount());
		ImGui::Text("Triangles: %u ", Im3d::GetContext().getPrimitiveCount(Im3d::DrawPrimitive_Triangles));
		ImGui::Text("Lines:     %u ", Im3d::GetContext().getPrimitiveCount(Im3d::DrawPrimitive_Lines));
		ImGui::Text("Points:    %u ", Im3d::GetContext().getPrimitiveCount(Im3d::DrawPrimitive_Points));
		ImGui::Text("Texts:     %u ", Im3d::GetContext().getTextCount());
	ImGui::End();

	Im3d_NewFrame();
	
	return ret;
}

void Example::draw()
{
	static const Vec4 kClearColor(0.5f, 0.5f, 0.5f, 0.0f);

	Im3d_EndFrame();

	ImGui::Render();

	#if defined(IM3D_PLATFORM_WIN)
		winAssert(ValidateRect(m_hwnd, 0)); // suppress WM_PAINT
		
		#if defined(IM3D_OPENGL)
			winAssert(SwapBuffers(m_hdc));
		#elif defined(IM3D_DX11)
			m_dxgiSwapChain->Present(0, 0);
		#endif
	#endif
	
 // reset state & clear backbuffer for next frame
	#if defined(IM3D_OPENGL)
		glAssert(glBindVertexArray(0));
		glAssert(glUseProgram(0));
		glAssert(glViewport(0, 0, m_width, m_height));
		glAssert(glClearColor(kClearColor.x, kClearColor.y, kClearColor.z, kClearColor.w));
		glAssert(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	
	#elif defined (IM3D_DX11)
		m_d3dDeviceCtx->ClearRenderTargetView(m_d3dRenderTarget, kClearColor);
		m_d3dDeviceCtx->ClearDepthStencilView(m_d3dDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0xff); 
	#endif
}

bool Example::hasFocus() const
{
	#if defined(IM3D_PLATFORM_WIN)
		return m_hwnd == GetFocus();
	#endif
}

Vec2 Example::getWindowRelativeCursor() const
{
	#if defined(IM3D_PLATFORM_WIN)
		POINT p = {};
		winAssert(GetCursorPos(&p));
		winAssert(ScreenToClient(m_hwnd, &p));
		return Vec2((float)p.x, (float)p.y);
	#endif
}

void Example::drawTextDrawListsImGui(const Im3d::TextDrawList _textDrawLists[], U32 _count)
{
// Using ImGui here as a simple means of rendering text draw lists, however as with primitives the application is free to draw text in any conceivable  manner.

	// Invisible ImGui window which covers the screen.
	ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32_BLACK_TRANS);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2((float)m_width, (float)m_height));
	ImGui::Begin("Invisible", nullptr, 0
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoInputs
		| ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_NoFocusOnAppearing
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		);

	ImDrawList* imDrawList = ImGui::GetWindowDrawList();
	const Mat4 viewProj = m_camViewProj;
	for (U32 i = 0; i < _count; ++i) 
	{
		const TextDrawList& textDrawList = Im3d::GetTextDrawLists()[i];
		
		if (textDrawList.m_layerId == Im3d::MakeId("NamedLayer")) 
		{
			// The application may group primitives into layers, which can be used to change the draw state (e.g. enable depth testing, use a different shader)
		}

		for (U32 j = 0; j < textDrawList.m_textDataCount; ++j)
		{
			const Im3d::TextData& textData = textDrawList.m_textData[j];
			if (textData.m_positionSize.w == 0.0f || textData.m_color.getA() == 0.0f)
			{
				continue;
			}

			// Project world -> screen space.
			Vec4 clip = viewProj * Vec4(textData.m_positionSize.x, textData.m_positionSize.y, textData.m_positionSize.z, 1.0f);
			Vec2 screen = Vec2(clip.x / clip.w, clip.y / clip.w);
	
			// Cull text which falls offscreen. Note that this doesn't take into account text size but works well enough in practice.
			if (clip.w < 0.0f || screen.x >= 1.0f || screen.y >= 1.0f)
			{
				continue;
			}

			// Pixel coordinates for the ImGuiWindow ImGui.
			screen = screen * Vec2(0.5f) + Vec2(0.5f);
			screen.y = 1.0f - screen.y; // screen space origin is reversed by the projection.
			screen = screen * (Vec2)ImGui::GetWindowSize();

			// All text data is stored in a single buffer; each textData instance has an offset into this buffer.
			const char* text = textDrawList.m_textBuffer + textData.m_textBufferOffset;

			// Calculate the final text size in pixels to apply alignment flags correctly.
			ImGui::SetWindowFontScale(textData.m_positionSize.w); // NB no CalcTextSize API which takes a font/size directly...
			Vec2 textSize = ImGui::CalcTextSize(text, text + textData.m_textLength);
			ImGui::SetWindowFontScale(1.0f);

			// Generate a pixel offset based on text flags.
			Vec2 textOffset = Vec2(-textSize.x * 0.5f, -textSize.y * 0.5f); // default to center
			if ((textData.m_flags & Im3d::TextFlags_AlignLeft) != 0)
			{
				textOffset.x = -textSize.x;
			}
			else if ((textData.m_flags & Im3d::TextFlags_AlignRight) != 0)
			{
				textOffset.x = 0.0f;
			}

			if ((textData.m_flags & Im3d::TextFlags_AlignTop) != 0)
			{
				textOffset.y = -textSize.y;
			}
			else if ((textData.m_flags & Im3d::TextFlags_AlignBottom) != 0)
			{
				textOffset.y = 0.0f;
			}

			// Add text to the window draw list.
			screen = screen + textOffset;
			imDrawList->AddText(nullptr, textData.m_positionSize.w * ImGui::GetFontSize(), screen, textData.m_color.getABGR(), text, text + textData.m_textLength);
		}
	}

	ImGui::End();
	ImGui::PopStyleColor(1);
}