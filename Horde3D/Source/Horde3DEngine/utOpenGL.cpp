// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2016 Nicolas Schulz and Horde3D team
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#define _CRT_SECURE_NO_WARNINGS
#include "utOpenGL.h"
#include <cstdlib>
#include <cstring>

namespace glExt
{
	bool EXT_framebuffer_object = false;
	bool EXT_framebuffer_multisample = false;
	bool EXT_texture_filter_anisotropic = false;
	bool EXT_texture_compression_s3tc = false;
	bool EXT_texture_sRGB = false;
	bool ARB_texture_float = false;
	bool ARB_texture_non_power_of_two = false;
	bool ARB_timer_query = false;
	bool ARB_texture_buffer_object = false;
	bool OES_EGL_image = false;

	int	majorVersion = 1, minorVersion = 0;
}


namespace h3dGL
{
// GL 1.1
PFNGLGETTEXIMAGEPROC glGetTexImage = 0x0;
PFNGLPOLYGONMODEPROC glPolygonMode = 0x0;
PFNGLCLEARDEPTH glClearDepth = 0x0;
PFNGLDRAWBUFFERPROC glDrawBuffer = 0x0;

// GL 1.2
PFNGLBLENDCOLORPROC glBlendColor = 0x0;
PFNGLBLENDEQUATIONPROC glBlendEquation = 0x0;
PFNGLDRAWRANGEELEMENTSPROC glDrawRangeElements = 0x0;
PFNGLTEXIMAGE3DPROC glTexImage3D = 0x0;
PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D = 0x0;
PFNGLCOPYTEXSUBIMAGE3DPROC glCopyTexSubImage3D = 0x0;

// GL 1.3
PFNGLACTIVETEXTUREPROC glActiveTexture = 0x0;
PFNGLSAMPLECOVERAGEPROC glSampleCoverage = 0x0;
PFNGLCOMPRESSEDTEXIMAGE3DPROC glCompressedTexImage3D = 0x0;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D = 0x0;
PFNGLCOMPRESSEDTEXIMAGE1DPROC glCompressedTexImage1D = 0x0;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC glCompressedTexSubImage3D = 0x0;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glCompressedTexSubImage2D = 0x0;
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC glCompressedTexSubImage1D = 0x0;
PFNGLGETCOMPRESSEDTEXIMAGEPROC glGetCompressedTexImage = 0x0;

// GL 1.4
PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate = 0x0;
PFNGLMULTIDRAWARRAYSPROC glMultiDrawArrays = 0x0;
PFNGLMULTIDRAWELEMENTSPROC glMultiDrawElements = 0x0;
PFNGLPOINTPARAMETERFPROC glPointParameterf = 0x0;
PFNGLPOINTPARAMETERFVPROC glPointParameterfv = 0x0;
PFNGLPOINTPARAMETERIPROC glPointParameteri = 0x0;
PFNGLPOINTPARAMETERIVPROC glPointParameteriv = 0x0;

// GL 1.5
PFNGLGENQUERIESPROC glGenQueries = 0x0;
PFNGLDELETEQUERIESPROC glDeleteQueries = 0x0;
PFNGLISQUERYPROC glIsQuery = 0x0;
PFNGLBEGINQUERYPROC glBeginQuery = 0x0;
PFNGLENDQUERYPROC glEndQuery = 0x0;
PFNGLGETQUERYIVPROC glGetQueryiv = 0x0;
PFNGLGETQUERYOBJECTIVPROC glGetQueryObjectiv = 0x0;
PFNGLGETQUERYOBJECTUIVPROC glGetQueryObjectuiv = 0x0;
PFNGLBINDBUFFERPROC glBindBuffer = 0x0;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = 0x0;
PFNGLGENBUFFERSPROC glGenBuffers = 0x0;
PFNGLISBUFFERPROC glIsBuffer = 0x0;
PFNGLBUFFERDATAPROC glBufferData = 0x0;
PFNGLBUFFERSUBDATAPROC glBufferSubData = 0x0;
PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData = 0x0;
PFNGLMAPBUFFERPROC glMapBuffer = 0x0;
PFNGLUNMAPBUFFERPROC glUnmapBuffer = 0x0;
PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv = 0x0;
PFNGLGETBUFFERPOINTERVPROC glGetBufferPointerv = 0x0;

// GL 2.0
PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate = 0x0;
PFNGLDRAWBUFFERSPROC glDrawBuffers = 0x0;
PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate = 0x0;
PFNGLSTENCILFUNCSEPARATEPROC glStencilFuncSeparate = 0x0;
PFNGLSTENCILMASKSEPARATEPROC glStencilMaskSeparate = 0x0;
PFNGLATTACHSHADERPROC glAttachShader = 0x0;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation = 0x0;
PFNGLCOMPILESHADERPROC glCompileShader = 0x0;
PFNGLCREATEPROGRAMPROC glCreateProgram = 0x0;
PFNGLCREATESHADERPROC glCreateShader = 0x0;
PFNGLDELETEPROGRAMPROC glDeleteProgram = 0x0;
PFNGLDELETESHADERPROC glDeleteShader = 0x0;
PFNGLDETACHSHADERPROC glDetachShader = 0x0;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = 0x0;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = 0x0;
PFNGLGETACTIVEATTRIBPROC glGetActiveAttrib = 0x0;
PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform = 0x0;
PFNGLGETATTACHEDSHADERSPROC glGetAttachedShaders = 0x0;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = 0x0;
PFNGLGETPROGRAMIVPROC glGetProgramiv = 0x0;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = 0x0;
PFNGLGETSHADERIVPROC glGetShaderiv = 0x0;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = 0x0;
PFNGLGETSHADERSOURCEPROC glGetShaderSource = 0x0;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = 0x0;
PFNGLGETUNIFORMFVPROC glGetUniformfv = 0x0;
PFNGLGETUNIFORMIVPROC glGetUniformiv = 0x0;
PFNGLGETVERTEXATTRIBDVPROC glGetVertexAttribdv = 0x0;
PFNGLGETVERTEXATTRIBFVPROC glGetVertexAttribfv = 0x0;
PFNGLGETVERTEXATTRIBIVPROC glGetVertexAttribiv = 0x0;
PFNGLGETVERTEXATTRIBPOINTERVPROC glGetVertexAttribPointerv = 0x0;
PFNGLISPROGRAMPROC glIsProgram = 0x0;
PFNGLISSHADERPROC glIsShader = 0x0;
PFNGLLINKPROGRAMPROC glLinkProgram = 0x0;
PFNGLSHADERSOURCEPROC glShaderSource = 0x0;
PFNGLUSEPROGRAMPROC glUseProgram = 0x0;
PFNGLUNIFORM1FPROC glUniform1f = 0x0;
PFNGLUNIFORM2FPROC glUniform2f = 0x0;
PFNGLUNIFORM3FPROC glUniform3f = 0x0;
PFNGLUNIFORM4FPROC glUniform4f = 0x0;
PFNGLUNIFORM1IPROC glUniform1i = 0x0;
PFNGLUNIFORM2IPROC glUniform2i = 0x0;
PFNGLUNIFORM3IPROC glUniform3i = 0x0;
PFNGLUNIFORM4IPROC glUniform4i = 0x0;
PFNGLUNIFORM1FVPROC glUniform1fv = 0x0;
PFNGLUNIFORM2FVPROC glUniform2fv = 0x0;
PFNGLUNIFORM3FVPROC glUniform3fv = 0x0;
PFNGLUNIFORM4FVPROC glUniform4fv = 0x0;
PFNGLUNIFORM1IVPROC glUniform1iv = 0x0;
PFNGLUNIFORM2IVPROC glUniform2iv = 0x0;
PFNGLUNIFORM3IVPROC glUniform3iv = 0x0;
PFNGLUNIFORM4IVPROC glUniform4iv = 0x0;
PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv = 0x0;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv = 0x0;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = 0x0;
PFNGLVALIDATEPROGRAMPROC glValidateProgram = 0x0;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = 0x0;

// GL 2.1
PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix2x3fv = 0x0;
PFNGLUNIFORMMATRIX3X2FVPROC glUniformMatrix3x2fv = 0x0;
PFNGLUNIFORMMATRIX2X4FVPROC glUniformMatrix2x4fv = 0x0;
PFNGLUNIFORMMATRIX4X2FVPROC glUniformMatrix4x2fv = 0x0;
PFNGLUNIFORMMATRIX3X4FVPROC glUniformMatrix3x4fv = 0x0;
PFNGLUNIFORMMATRIX4X3FVPROC glUniformMatrix4x3fv = 0x0;

// GL 3.0
PFNGLGETSTRINGIPROC glGetStringi = 0x0;
PFNGLCOLORMASKIPROC glColorMaski = 0;
PFNGLGETBOOLEANI_VPROC glGetBooleani_v = 0;
PFNGLGETINTEGERI_VPROC glGetIntegeri_v = 0;
PFNGLENABLEIPROC glEnablei = 0;
PFNGLDISABLEIPROC glDisablei = 0;
PFNGLISENABLEDIPROC glIsEnabledi = 0;
PFNGLBEGINTRANSFORMFEEDBACKPROC glBeginTransformFeedback = 0;
PFNGLENDTRANSFORMFEEDBACKPROC glEndTransformFeedback = 0;
PFNGLBINDBUFFERRANGEPROC glBindBufferRange = 0;
PFNGLBINDBUFFERBASEPROC glBindBufferBase = 0;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC glTransformFeedbackVaryings = 0;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC glGetTransformFeedbackVarying = 0;
PFNGLCLAMPCOLORPROC glClampColor = 0;
PFNGLBEGINCONDITIONALRENDERPROC glBeginConditionalRender = 0;
PFNGLENDCONDITIONALRENDERPROC glEndConditionalRender = 0;
PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer = 0;
PFNGLGETVERTEXATTRIBIIVPROC glGetVertexAttribIiv = 0;
PFNGLGETVERTEXATTRIBIUIVPROC glGetVertexAttribIuiv = 0;
PFNGLVERTEXATTRIBI1IPROC glVertexAttribI1i = 0;
PFNGLVERTEXATTRIBI2IPROC glVertexAttribI2i = 0;
PFNGLVERTEXATTRIBI3IPROC glVertexAttribI3i = 0;
PFNGLVERTEXATTRIBI4IPROC glVertexAttribI4i = 0;
PFNGLVERTEXATTRIBI1UIPROC glVertexAttribI1ui = 0;
PFNGLVERTEXATTRIBI2UIPROC glVertexAttribI2ui = 0;
PFNGLVERTEXATTRIBI3UIPROC glVertexAttribI3ui = 0;
PFNGLVERTEXATTRIBI4UIPROC glVertexAttribI4ui = 0;
PFNGLVERTEXATTRIBI1IVPROC glVertexAttribI1iv = 0;
PFNGLVERTEXATTRIBI2IVPROC glVertexAttribI2iv = 0;
PFNGLVERTEXATTRIBI3IVPROC glVertexAttribI3iv = 0;
PFNGLVERTEXATTRIBI4IVPROC glVertexAttribI4iv = 0;
PFNGLVERTEXATTRIBI1UIVPROC glVertexAttribI1uiv = 0;
PFNGLVERTEXATTRIBI2UIVPROC glVertexAttribI2uiv = 0;
PFNGLVERTEXATTRIBI3UIVPROC glVertexAttribI3uiv = 0;
PFNGLVERTEXATTRIBI4UIVPROC glVertexAttribI4uiv = 0;
PFNGLVERTEXATTRIBI4BVPROC glVertexAttribI4bv = 0;
PFNGLVERTEXATTRIBI4SVPROC glVertexAttribI4sv = 0;
PFNGLVERTEXATTRIBI4UBVPROC glVertexAttribI4ubv = 0;
PFNGLVERTEXATTRIBI4USVPROC glVertexAttribI4usv = 0;
PFNGLGETUNIFORMUIVPROC glGetUniformuiv = 0;
PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation = 0;
PFNGLGETFRAGDATALOCATIONPROC glGetFragDataLocation = 0;
PFNGLUNIFORM1UIPROC glUniform1ui = 0;
PFNGLUNIFORM2UIPROC glUniform2ui = 0;
PFNGLUNIFORM3UIPROC glUniform3ui = 0;
PFNGLUNIFORM4UIPROC glUniform4ui = 0;
PFNGLUNIFORM1UIVPROC glUniform1uiv = 0;
PFNGLUNIFORM2UIVPROC glUniform2uiv = 0;
PFNGLUNIFORM3UIVPROC glUniform3uiv = 0;
PFNGLUNIFORM4UIVPROC glUniform4uiv = 0;
PFNGLTEXPARAMETERIIVPROC glTexParameterIiv = 0;
PFNGLTEXPARAMETERIUIVPROC glTexParameterIuiv = 0;
PFNGLGETTEXPARAMETERIIVPROC glGetTexParameterIiv = 0;
PFNGLGETTEXPARAMETERIUIVPROC glGetTexParameterIuiv = 0;
PFNGLCLEARBUFFERIVPROC glClearBufferiv = 0;
PFNGLCLEARBUFFERUIVPROC glClearBufferuiv = 0;
PFNGLCLEARBUFFERFVPROC glClearBufferfv = 0;
PFNGLCLEARBUFFERFIPROC glClearBufferfi = 0;
PFNGLISRENDERBUFFERPROC glIsRenderbuffer = 0;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = 0;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = 0;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = 0;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = 0;
PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameteriv = 0;
PFNGLISFRAMEBUFFERPROC glIsFramebuffer = 0;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = 0;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = 0;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = 0;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = 0;
PFNGLFRAMEBUFFERTEXTURE1DPROC glFramebufferTexture1D = 0;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = 0;
PFNGLFRAMEBUFFERTEXTURE3DPROC glFramebufferTexture3D = 0;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = 0;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv = 0;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = 0;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer = 0;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample = 0;
PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer = 0;
PFNGLMAPBUFFERRANGEPROC glMapBufferRange = 0;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRange = 0;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = 0;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = 0;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = 0;
PFNGLISVERTEXARRAYPROC glIsVertexArray = 0;

// GL 3.1
PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced = 0;
PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced = 0;
PFNGLTEXBUFFERPROC glTexBuffer = 0;
PFNGLPRIMITIVERESTARTINDEXPROC glPrimitiveRestartIndex = 0;
PFNGLCOPYBUFFERSUBDATAPROC glCopyBufferSubData = 0;
PFNGLGETUNIFORMINDICESPROC glGetUniformIndices = 0;
PFNGLGETACTIVEUNIFORMSIVPROC glGetActiveUniformsiv = 0;
PFNGLGETACTIVEUNIFORMNAMEPROC glGetActiveUniformName = 0;
PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex = 0;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv = 0;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glGetActiveUniformBlockName = 0;
PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding = 0;

// GL 3.2
PFNGLDRAWELEMENTSBASEVERTEXPROC glDrawElementsBaseVertex = 0;
PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC glDrawRangeElementsBaseVertex = 0;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC glDrawElementsInstancedBaseVertex = 0;
PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC glMultiDrawElementsBaseVertex = 0;
PFNGLPROVOKINGVERTEXPROC glProvokingVertex = 0;
PFNGLFENCESYNCPROC glFenceSync = 0;
PFNGLISSYNCPROC glIsSync = 0;
PFNGLDELETESYNCPROC glDeleteSync = 0;
PFNGLCLIENTWAITSYNCPROC glClientWaitSync = 0;
PFNGLWAITSYNCPROC glWaitSync = 0;
PFNGLGETINTEGER64VPROC glGetInteger64v = 0;
PFNGLGETSYNCIVPROC glGetSynciv = 0;
PFNGLGETINTEGER64I_VPROC glGetInteger64i_v = 0;
PFNGLGETBUFFERPARAMETERI64VPROC glGetBufferParameteri64v = 0;
PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture = 0;
PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample = 0;
PFNGLTEXIMAGE3DMULTISAMPLEPROC glTexImage3DMultisample = 0;
PFNGLGETMULTISAMPLEFVPROC glGetMultisamplefv = 0;
PFNGLSAMPLEMASKIPROC glSampleMaski = 0;

// GL 3.3
PFNGLBINDFRAGDATALOCATIONINDEXEDPROC glBindFragDataLocationIndexed = 0;
PFNGLGETFRAGDATAINDEXPROC glGetFragDataIndex = 0;
PFNGLGENSAMPLERSPROC glGenSamplers = 0;
PFNGLDELETESAMPLERSPROC glDeleteSamplers = 0;
PFNGLISSAMPLERPROC glIsSampler = 0;
PFNGLBINDSAMPLERPROC glBindSampler = 0;
PFNGLSAMPLERPARAMETERIPROC glSamplerParameteri = 0;
PFNGLSAMPLERPARAMETERIVPROC glSamplerParameteriv = 0;
PFNGLSAMPLERPARAMETERFPROC glSamplerParameterf = 0;
PFNGLSAMPLERPARAMETERFVPROC glSamplerParameterfv = 0;
PFNGLSAMPLERPARAMETERIIVPROC glSamplerParameterIiv = 0;
PFNGLSAMPLERPARAMETERIUIVPROC glSamplerParameterIuiv = 0;
PFNGLGETSAMPLERPARAMETERIVPROC glGetSamplerParameteriv = 0;
PFNGLGETSAMPLERPARAMETERIIVPROC glGetSamplerParameterIiv = 0;
PFNGLGETSAMPLERPARAMETERFVPROC glGetSamplerParameterfv = 0;
PFNGLGETSAMPLERPARAMETERIUIVPROC glGetSamplerParameterIuiv = 0;
PFNGLQUERYCOUNTERPROC glQueryCounter = 0;
PFNGLGETQUERYOBJECTI64VPROC glGetQueryObjecti64v = 0;
PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64v = 0;
PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor = 0;
PFNGLVERTEXATTRIBP1UIPROC glVertexAttribP1ui = 0;
PFNGLVERTEXATTRIBP1UIVPROC glVertexAttribP1uiv = 0;
PFNGLVERTEXATTRIBP2UIPROC glVertexAttribP2ui = 0;
PFNGLVERTEXATTRIBP2UIVPROC glVertexAttribP2uiv = 0;
PFNGLVERTEXATTRIBP3UIPROC glVertexAttribP3ui = 0;
PFNGLVERTEXATTRIBP3UIVPROC glVertexAttribP3uiv = 0;
PFNGLVERTEXATTRIBP4UIPROC glVertexAttribP4ui = 0;
PFNGLVERTEXATTRIBP4UIVPROC glVertexAttribP4uiv = 0;

// GL 4.0
PFNGLMINSAMPLESHADINGPROC glMinSampleShading = 0;
PFNGLBLENDEQUATIONIPROC glBlendEquationi = 0;
PFNGLBLENDEQUATIONSEPARATEIPROC glBlendEquationSeparatei = 0;
PFNGLBLENDFUNCIPROC glBlendFunci = 0;
PFNGLBLENDFUNCSEPARATEIPROC glBlendFuncSeparatei = 0;
PFNGLDRAWARRAYSINDIRECTPROC glDrawArraysIndirect = 0;
PFNGLDRAWELEMENTSINDIRECTPROC glDrawElementsIndirect = 0;
PFNGLUNIFORM1DPROC glUniform1d = 0;
PFNGLUNIFORM2DPROC glUniform2d = 0;
PFNGLUNIFORM3DPROC glUniform3d = 0;
PFNGLUNIFORM4DPROC glUniform4d = 0;
PFNGLUNIFORM1DVPROC glUniform1dv = 0;
PFNGLUNIFORM2DVPROC glUniform2dv = 0;
PFNGLUNIFORM3DVPROC glUniform3dv = 0;
PFNGLUNIFORM4DVPROC glUniform4dv = 0;
PFNGLUNIFORMMATRIX2DVPROC glUniformMatrix2dv = 0;
PFNGLUNIFORMMATRIX3DVPROC glUniformMatrix3dv = 0;
PFNGLUNIFORMMATRIX4DVPROC glUniformMatrix4dv = 0;
PFNGLUNIFORMMATRIX2X3DVPROC glUniformMatrix2x3dv = 0;
PFNGLUNIFORMMATRIX2X4DVPROC glUniformMatrix2x4dv = 0;
PFNGLUNIFORMMATRIX3X2DVPROC glUniformMatrix3x2dv = 0;
PFNGLUNIFORMMATRIX3X4DVPROC glUniformMatrix3x4dv = 0;
PFNGLUNIFORMMATRIX4X2DVPROC glUniformMatrix4x2dv = 0;
PFNGLUNIFORMMATRIX4X3DVPROC glUniformMatrix4x3dv = 0;
PFNGLGETUNIFORMDVPROC glGetUniformdv = 0;
PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC glGetSubroutineUniformLocation = 0;
PFNGLGETSUBROUTINEINDEXPROC glGetSubroutineIndex = 0;
PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC glGetActiveSubroutineUniformiv = 0;
PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC glGetActiveSubroutineUniformName = 0;
PFNGLGETACTIVESUBROUTINENAMEPROC glGetActiveSubroutineName = 0;
PFNGLUNIFORMSUBROUTINESUIVPROC glUniformSubroutinesuiv = 0;
PFNGLGETUNIFORMSUBROUTINEUIVPROC glGetUniformSubroutineuiv = 0;
PFNGLGETPROGRAMSTAGEIVPROC glGetProgramStageiv = 0;
PFNGLPATCHPARAMETERIPROC glPatchParameteri = 0;
PFNGLPATCHPARAMETERFVPROC glPatchParameterfv = 0;
PFNGLBINDTRANSFORMFEEDBACKPROC glBindTransformFeedback = 0;
PFNGLDELETETRANSFORMFEEDBACKSPROC glDeleteTransformFeedbacks = 0;
PFNGLGENTRANSFORMFEEDBACKSPROC glGenTransformFeedbacks = 0;
PFNGLISTRANSFORMFEEDBACKPROC glIsTransformFeedback = 0;
PFNGLPAUSETRANSFORMFEEDBACKPROC glPauseTransformFeedback = 0;
PFNGLRESUMETRANSFORMFEEDBACKPROC glResumeTransformFeedback = 0;
PFNGLDRAWTRANSFORMFEEDBACKPROC glDrawTransformFeedback = 0;
PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC glDrawTransformFeedbackStream = 0;
PFNGLBEGINQUERYINDEXEDPROC glBeginQueryIndexed = 0;
PFNGLENDQUERYINDEXEDPROC glEndQueryIndexed = 0;
PFNGLGETQUERYINDEXEDIVPROC glGetQueryIndexediv = 0;

// GL 4.1
PFNGLRELEASESHADERCOMPILERPROC glReleaseShaderCompiler = 0;
PFNGLSHADERBINARYPROC glShaderBinary = 0;
PFNGLGETSHADERPRECISIONFORMATPROC glGetShaderPrecisionFormat = 0;
PFNGLDEPTHRANGEFPROC glDepthRangef = 0;
PFNGLCLEARDEPTHFPROC glClearDepthf = 0;
PFNGLGETPROGRAMBINARYPROC glGetProgramBinary = 0;
PFNGLPROGRAMBINARYPROC glProgramBinary = 0;
PFNGLPROGRAMPARAMETERIPROC glProgramParameteri = 0;
PFNGLUSEPROGRAMSTAGESPROC glUseProgramStages = 0;
PFNGLACTIVESHADERPROGRAMPROC glActiveShaderProgram = 0;
PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv = 0;
PFNGLBINDPROGRAMPIPELINEPROC glBindProgramPipeline = 0;
PFNGLDELETEPROGRAMPIPELINESPROC glDeleteProgramPipelines = 0;
PFNGLGENPROGRAMPIPELINESPROC glGenProgramPipelines = 0;
PFNGLISPROGRAMPIPELINEPROC glIsProgramPipeline = 0;
PFNGLGETPROGRAMPIPELINEIVPROC glGetProgramPipelineiv = 0;
PFNGLPROGRAMUNIFORM1IPROC glProgramUniform1i = 0;
PFNGLPROGRAMUNIFORM1IVPROC glProgramUniform1iv = 0;
PFNGLPROGRAMUNIFORM1FPROC glProgramUniform1f = 0;
PFNGLPROGRAMUNIFORM1FVPROC glProgramUniform1fv = 0;
PFNGLPROGRAMUNIFORM1DPROC glProgramUniform1d = 0;
PFNGLPROGRAMUNIFORM1DVPROC glProgramUniform1dv = 0;
PFNGLPROGRAMUNIFORM1UIPROC glProgramUniform1ui = 0;
PFNGLPROGRAMUNIFORM1UIVPROC glProgramUniform1uiv = 0;
PFNGLPROGRAMUNIFORM2IPROC glProgramUniform2i = 0;
PFNGLPROGRAMUNIFORM2IVPROC glProgramUniform2iv = 0;
PFNGLPROGRAMUNIFORM2FPROC glProgramUniform2f = 0;
PFNGLPROGRAMUNIFORM2FVPROC glProgramUniform2fv = 0;
PFNGLPROGRAMUNIFORM2DPROC glProgramUniform2d = 0;
PFNGLPROGRAMUNIFORM2DVPROC glProgramUniform2dv = 0;
PFNGLPROGRAMUNIFORM2UIPROC glProgramUniform2ui = 0;
PFNGLPROGRAMUNIFORM2UIVPROC glProgramUniform2uiv = 0;
PFNGLPROGRAMUNIFORM3IPROC glProgramUniform3i = 0;
PFNGLPROGRAMUNIFORM3IVPROC glProgramUniform3iv = 0;
PFNGLPROGRAMUNIFORM3FPROC glProgramUniform3f = 0;
PFNGLPROGRAMUNIFORM3FVPROC glProgramUniform3fv = 0;
PFNGLPROGRAMUNIFORM3DPROC glProgramUniform3d = 0;
PFNGLPROGRAMUNIFORM3DVPROC glProgramUniform3dv = 0;
PFNGLPROGRAMUNIFORM3UIPROC glProgramUniform3ui = 0;
PFNGLPROGRAMUNIFORM3UIVPROC glProgramUniform3uiv = 0;
PFNGLPROGRAMUNIFORM4IPROC glProgramUniform4i = 0;
PFNGLPROGRAMUNIFORM4IVPROC glProgramUniform4iv = 0;
PFNGLPROGRAMUNIFORM4FPROC glProgramUniform4f = 0;
PFNGLPROGRAMUNIFORM4FVPROC glProgramUniform4fv = 0;
PFNGLPROGRAMUNIFORM4DPROC glProgramUniform4d = 0;
PFNGLPROGRAMUNIFORM4DVPROC glProgramUniform4dv = 0;
PFNGLPROGRAMUNIFORM4UIPROC glProgramUniform4ui = 0;
PFNGLPROGRAMUNIFORM4UIVPROC glProgramUniform4uiv = 0;
PFNGLPROGRAMUNIFORMMATRIX2FVPROC glProgramUniformMatrix2fv = 0;
PFNGLPROGRAMUNIFORMMATRIX3FVPROC glProgramUniformMatrix3fv = 0;
PFNGLPROGRAMUNIFORMMATRIX4FVPROC glProgramUniformMatrix4fv = 0;
PFNGLPROGRAMUNIFORMMATRIX2DVPROC glProgramUniformMatrix2dv = 0;
PFNGLPROGRAMUNIFORMMATRIX3DVPROC glProgramUniformMatrix3dv = 0;
PFNGLPROGRAMUNIFORMMATRIX4DVPROC glProgramUniformMatrix4dv = 0;
PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC glProgramUniformMatrix2x3fv = 0;
PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC glProgramUniformMatrix3x2fv = 0;
PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC glProgramUniformMatrix2x4fv = 0;
PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC glProgramUniformMatrix4x2fv = 0;
PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC glProgramUniformMatrix3x4fv = 0;
PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC glProgramUniformMatrix4x3fv = 0;
PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC glProgramUniformMatrix2x3dv = 0;
PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC glProgramUniformMatrix3x2dv = 0;
PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC glProgramUniformMatrix2x4dv = 0;
PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC glProgramUniformMatrix4x2dv = 0;
PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC glProgramUniformMatrix3x4dv = 0;
PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC glProgramUniformMatrix4x3dv = 0;
PFNGLVALIDATEPROGRAMPIPELINEPROC glValidateProgramPipeline = 0;
PFNGLGETPROGRAMPIPELINEINFOLOGPROC glGetProgramPipelineInfoLog = 0;
PFNGLVERTEXATTRIBL1DPROC glVertexAttribL1d = 0;
PFNGLVERTEXATTRIBL2DPROC glVertexAttribL2d = 0;
PFNGLVERTEXATTRIBL3DPROC glVertexAttribL3d = 0;
PFNGLVERTEXATTRIBL4DPROC glVertexAttribL4d = 0;
PFNGLVERTEXATTRIBL1DVPROC glVertexAttribL1dv = 0;
PFNGLVERTEXATTRIBL2DVPROC glVertexAttribL2dv = 0;
PFNGLVERTEXATTRIBL3DVPROC glVertexAttribL3dv = 0;
PFNGLVERTEXATTRIBL4DVPROC glVertexAttribL4dv = 0;
PFNGLVERTEXATTRIBLPOINTERPROC glVertexAttribLPointer = 0;
PFNGLGETVERTEXATTRIBLDVPROC glGetVertexAttribLdv = 0;
PFNGLVIEWPORTARRAYVPROC glViewportArrayv = 0;
PFNGLVIEWPORTINDEXEDFPROC glViewportIndexedf = 0;
PFNGLVIEWPORTINDEXEDFVPROC glViewportIndexedfv = 0;
PFNGLSCISSORARRAYVPROC glScissorArrayv = 0;
PFNGLSCISSORINDEXEDPROC glScissorIndexed = 0;
PFNGLSCISSORINDEXEDVPROC glScissorIndexedv = 0;
PFNGLDEPTHRANGEARRAYVPROC glDepthRangeArrayv = 0;
PFNGLDEPTHRANGEINDEXEDPROC glDepthRangeIndexed = 0;
PFNGLGETFLOATI_VPROC glGetFloati_v = 0;
PFNGLGETDOUBLEI_VPROC glGetDoublei_v = 0;

// GL 4.2
PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC glDrawArraysInstancedBaseInstance = 0;
PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC glDrawElementsInstancedBaseInstance = 0;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC glDrawElementsInstancedBaseVertexBaseInstance = 0;
PFNGLGETINTERNALFORMATIVPROC glGetInternalformativ = 0;
PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC glGetActiveAtomicCounterBufferiv = 0;
PFNGLBINDIMAGETEXTUREPROC glBindImageTexture = 0;
PFNGLMEMORYBARRIERPROC glMemoryBarrier = 0;
PFNGLTEXSTORAGE1DPROC glTexStorage1D = 0;
PFNGLTEXSTORAGE2DPROC glTexStorage2D = 0;
PFNGLTEXSTORAGE3DPROC glTexStorage3D = 0;
PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC glDrawTransformFeedbackInstanced = 0;
PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC glDrawTransformFeedbackStreamInstanced = 0;

// GL 4.3
PFNGLCLEARBUFFERDATAPROC glClearBufferData = 0;
PFNGLCLEARBUFFERSUBDATAPROC glClearBufferSubData = 0;
PFNGLDISPATCHCOMPUTEPROC glDispatchCompute = 0;
PFNGLDISPATCHCOMPUTEINDIRECTPROC glDispatchComputeIndirect = 0;
PFNGLCOPYIMAGESUBDATAPROC glCopyImageSubData = 0;
PFNGLFRAMEBUFFERPARAMETERIPROC glFramebufferParameteri = 0;
PFNGLGETFRAMEBUFFERPARAMETERIVPROC glGetFramebufferParameteriv = 0;
PFNGLGETINTERNALFORMATI64VPROC glGetInternalformati64v = 0;
PFNGLINVALIDATETEXSUBIMAGEPROC glInvalidateTexSubImage = 0;
PFNGLINVALIDATETEXIMAGEPROC glInvalidateTexImage = 0;
PFNGLINVALIDATEBUFFERSUBDATAPROC glInvalidateBufferSubData = 0;
PFNGLINVALIDATEBUFFERDATAPROC glInvalidateBufferData = 0;
PFNGLINVALIDATEFRAMEBUFFERPROC glInvalidateFramebuffer = 0;
PFNGLINVALIDATESUBFRAMEBUFFERPROC glInvalidateSubFramebuffer = 0;
PFNGLMULTIDRAWARRAYSINDIRECTPROC glMultiDrawArraysIndirect = 0;
PFNGLMULTIDRAWELEMENTSINDIRECTPROC glMultiDrawElementsIndirect = 0;
PFNGLGETPROGRAMINTERFACEIVPROC glGetProgramInterfaceiv = 0;
PFNGLGETPROGRAMRESOURCEINDEXPROC glGetProgramResourceIndex = 0;
PFNGLGETPROGRAMRESOURCENAMEPROC glGetProgramResourceName = 0;
PFNGLGETPROGRAMRESOURCEIVPROC glGetProgramResourceiv = 0;
PFNGLGETPROGRAMRESOURCELOCATIONPROC glGetProgramResourceLocation = 0;
PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC glGetProgramResourceLocationIndex = 0;
PFNGLSHADERSTORAGEBLOCKBINDINGPROC glShaderStorageBlockBinding = 0;
PFNGLTEXBUFFERRANGEPROC glTexBufferRange = 0;
PFNGLTEXSTORAGE2DMULTISAMPLEPROC glTexStorage2DMultisample = 0;
PFNGLTEXSTORAGE3DMULTISAMPLEPROC glTexStorage3DMultisample = 0;
PFNGLTEXTUREVIEWPROC glTextureView = 0;
PFNGLBINDVERTEXBUFFERPROC glBindVertexBuffer = 0;
PFNGLVERTEXATTRIBFORMATPROC glVertexAttribFormat = 0;
PFNGLVERTEXATTRIBIFORMATPROC glVertexAttribIFormat = 0;
PFNGLVERTEXATTRIBLFORMATPROC glVertexAttribLFormat = 0;
PFNGLVERTEXATTRIBBINDINGPROC glVertexAttribBinding = 0;
PFNGLVERTEXBINDINGDIVISORPROC glVertexBindingDivisor = 0;
PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl = 0;
PFNGLDEBUGMESSAGEINSERTPROC glDebugMessageInsert = 0;
//PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback = 0;
PFNGLGETDEBUGMESSAGELOGPROC glGetDebugMessageLog = 0;
PFNGLPUSHDEBUGGROUPPROC glPushDebugGroup = 0;
PFNGLPOPDEBUGGROUPPROC glPopDebugGroup = 0;
PFNGLOBJECTLABELPROC glObjectLabel = 0;
PFNGLGETOBJECTLABELPROC glGetObjectLabel = 0;
PFNGLOBJECTPTRLABELPROC glObjectPtrLabel = 0;
PFNGLGETOBJECTPTRLABELPROC glGetObjectPtrLabel = 0;

// GL 4.4
PFNGLBUFFERSTORAGEPROC glBufferStorage = 0;
PFNGLCLEARTEXIMAGEPROC glClearTexImage = 0;
PFNGLCLEARTEXSUBIMAGEPROC glClearTexSubImage = 0;
PFNGLBINDBUFFERSBASEPROC glBindBuffersBase = 0;
PFNGLBINDBUFFERSRANGEPROC glBindBuffersRange = 0;
PFNGLBINDTEXTURESPROC glBindTextures = 0;
PFNGLBINDSAMPLERSPROC glBindSamplers = 0;
PFNGLBINDIMAGETEXTURESPROC glBindImageTextures = 0;
PFNGLBINDVERTEXBUFFERSPROC glBindVertexBuffers = 0;

// Extensions

// GL_EXT_framebuffer_object
PFNGLISRENDERBUFFEREXTPROC glIsRenderbufferEXT = 0x0;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT = 0x0;
PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT = 0x0;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT = 0x0;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT = 0x0;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT = 0x0;
PFNGLISFRAMEBUFFEREXTPROC glIsFramebufferEXT = 0x0;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = 0x0;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT = 0x0;
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = 0x0;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = 0x0;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT = 0x0;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT = 0x0;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT = 0x0;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT = 0x0;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT = 0x0;
PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT = 0x0;

// GL_EXT_framebuffer_blit
PFNGLBLITFRAMEBUFFEREXTPROC glBlitFramebufferEXT = 0x0;

// GL_EXT_framebuffer_multisample
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT = 0x0;

// GL_ARB_timer_query
PFNGLQUERYCOUNTERPROC glQueryCounterARB = 0x0;
PFNGLGETQUERYOBJECTI64VPROC glGetQueryObjecti64vARB = 0x0;
PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64vARB = 0x0;

// GL_ARB_texture_buffer_object
PFNGLTEXBUFFERPROC glTexBufferARB = 0x0;

// GL_OES_EGL_image
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = 0x0;

}  // namespace h3dGL


bool isExtensionSupported( const char *extName )
{
	if( glExt::majorVersion < 3 )
	{
		const char *extensions = (char *)glGetString( GL_EXTENSIONS );
		size_t nameLen = strlen( extName );
		const char *pos;
		while( ( pos = strstr( extensions, extName ) ) != 0x0 )
		{
			char c = pos[nameLen];
			if( c == ' ' || c == '\0' ) return true;
			extensions = pos + nameLen;
		}
	}
	else
	{
		GLint numExts;
		glGetIntegerv( GL_NUM_EXTENSIONS, &numExts );
		for( int i = 0; i < numExts; ++i )
		{
			if( strcmp( extName, (char *)glGetStringi( GL_EXTENSIONS, i ) ) == 0 )
				return true;
		}
	}

	return false;
}


void getOpenGLVersion()
{
	char version[8];
	size_t len = strlen( (char *)glGetString( GL_VERSION ) );
	if( len >= 8 ) len = 7;
	
	strncpy( version, (char *)glGetString( GL_VERSION ), len );
	version[len] = '\0';

	char *pos1 = strtok( version, "." );
	if( pos1 )
	{
		glExt::majorVersion = atoi( pos1 );
		char *pos2 = strtok( 0x0, ". " );
		if( pos2 ) glExt::minorVersion = atoi( pos2 );
	}
}


void *platGetProcAddress( const char *funcName )
{
#if defined( PLATFORM_WIN )
	void* retVal = (void *)wglGetProcAddress( funcName );
	// For some reason legacy OpenGL functions are not resolved by wglGetProcAddress
	if( !retVal )
	{
		static HMODULE lib = LoadLibraryW(L"opengl32.dll");
		retVal = (void*) GetProcAddress( lib, funcName);
	}
	return retVal;
#elif defined( PLATFORM_WIN_CE )
	return (void *)eglGetProcAddress( funcName );
#elif defined( PLATFORM_MAC )
	CFStringRef functionName = CFStringCreateWithCString( kCFAllocatorDefault, funcName, kCFStringEncodingASCII );
	CFURLRef bundleURL = CFURLCreateWithFileSystemPath(
		kCFAllocatorDefault, CFSTR( "/System/Library/Frameworks/OpenGL.framework" ), kCFURLPOSIXPathStyle, true );
	CFBundleRef bundle = CFBundleCreate( kCFAllocatorDefault, bundleURL );
   
	void *function = CFBundleGetFunctionPointerForName( bundle, functionName );
   
	CFRelease( bundle );
	CFRelease( bundleURL );
	CFRelease( functionName );
   
	return function; 
#elif defined( HAVE_EGL )
	return (void *)eglGetProcAddress( funcName );
#else
	return (void *)glXGetProcAddressARB( (const GLubyte *)funcName );
#endif
}

void initLegacyExtensions( bool &r )
{
	glExt::EXT_framebuffer_object = isExtensionSupported( "GL_EXT_framebuffer_object" );
	if ( glExt::EXT_framebuffer_object )
	{
		r &= ( glIsRenderbufferEXT = ( PFNGLISRENDERBUFFEREXTPROC ) platGetProcAddress( "glIsRenderbufferEXT" ) ) != 0x0;
		r &= ( glBindRenderbufferEXT = ( PFNGLBINDRENDERBUFFEREXTPROC ) platGetProcAddress( "glBindRenderbufferEXT" ) ) != 0x0;
		r &= ( glDeleteRenderbuffersEXT = ( PFNGLDELETERENDERBUFFERSEXTPROC ) platGetProcAddress( "glDeleteRenderbuffersEXT" ) ) != 0x0;
		r &= ( glGenRenderbuffersEXT = ( PFNGLGENRENDERBUFFERSEXTPROC ) platGetProcAddress( "glGenRenderbuffersEXT" ) ) != 0x0;
		r &= ( glRenderbufferStorageEXT = ( PFNGLRENDERBUFFERSTORAGEEXTPROC ) platGetProcAddress( "glRenderbufferStorageEXT" ) ) != 0x0;
		r &= ( glGetRenderbufferParameterivEXT = ( PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC ) platGetProcAddress( "glGetRenderbufferParameterivEXT" ) ) != 0x0;
		r &= ( glIsFramebufferEXT = ( PFNGLISFRAMEBUFFEREXTPROC ) platGetProcAddress( "glIsFramebufferEXT" ) ) != 0x0;
		r &= ( glBindFramebufferEXT = ( PFNGLBINDFRAMEBUFFEREXTPROC ) platGetProcAddress( "glBindFramebufferEXT" ) ) != 0x0;
		r &= ( glDeleteFramebuffersEXT = ( PFNGLDELETEFRAMEBUFFERSEXTPROC ) platGetProcAddress( "glDeleteFramebuffersEXT" ) ) != 0x0;
		r &= ( glGenFramebuffersEXT = ( PFNGLGENFRAMEBUFFERSEXTPROC ) platGetProcAddress( "glGenFramebuffersEXT" ) ) != 0x0;
		r &= ( glCheckFramebufferStatusEXT = ( PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC ) platGetProcAddress( "glCheckFramebufferStatusEXT" ) ) != 0x0;
		r &= ( glFramebufferTexture1DEXT = ( PFNGLFRAMEBUFFERTEXTURE1DEXTPROC ) platGetProcAddress( "glFramebufferTexture1DEXT" ) ) != 0x0;
		r &= ( glFramebufferTexture2DEXT = ( PFNGLFRAMEBUFFERTEXTURE2DEXTPROC ) platGetProcAddress( "glFramebufferTexture2DEXT" ) ) != 0x0;
		r &= ( glFramebufferTexture3DEXT = ( PFNGLFRAMEBUFFERTEXTURE3DEXTPROC ) platGetProcAddress( "glFramebufferTexture3DEXT" ) ) != 0x0;
		r &= ( glFramebufferRenderbufferEXT = ( PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC ) platGetProcAddress( "glFramebufferRenderbufferEXT" ) ) != 0x0;
		r &= ( glGetFramebufferAttachmentParameterivEXT = ( PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC ) platGetProcAddress( "glGetFramebufferAttachmentParameterivEXT" ) ) != 0x0;
		r &= ( glGenerateMipmapEXT = ( PFNGLGENERATEMIPMAPEXTPROC ) platGetProcAddress( "glGenerateMipmapEXT" ) ) != 0x0;
	}

	glExt::EXT_framebuffer_multisample = isExtensionSupported( "GL_EXT_framebuffer_multisample" ) &&
		isExtensionSupported( "GL_EXT_framebuffer_blit" );

	if ( glExt::EXT_framebuffer_multisample )
	{
		// From GL_EXT_framebuffer_blit
		r &= ( glBlitFramebufferEXT = ( PFNGLBLITFRAMEBUFFEREXTPROC ) platGetProcAddress( "glBlitFramebufferEXT" ) ) != 0x0;
		// From GL_EXT_framebuffer_multisample
		r &= ( glRenderbufferStorageMultisampleEXT = ( PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC ) platGetProcAddress( "glRenderbufferStorageMultisampleEXT" ) ) != 0x0;
	}

	glExt::ARB_texture_float = isExtensionSupported( "GL_ARB_texture_float" ) ||
		isExtensionSupported( "GL_ATI_texture_float" );

	glExt::ARB_texture_non_power_of_two = isExtensionSupported( "GL_ARB_texture_non_power_of_two" );

	glExt::ARB_timer_query = isExtensionSupported( "GL_ARB_timer_query" );
	if ( glExt::ARB_timer_query )
	{
		r &= ( glQueryCounterARB = ( PFNGLQUERYCOUNTERPROC ) platGetProcAddress( "glQueryCounter" ) ) != 0x0;
		r &= ( glGetQueryObjecti64vARB = ( PFNGLGETQUERYOBJECTI64VPROC ) platGetProcAddress( "glGetQueryObjecti64v" ) ) != 0x0;
		r &= ( glGetQueryObjectui64vARB = ( PFNGLGETQUERYOBJECTUI64VPROC ) platGetProcAddress( "glGetQueryObjectui64v" ) ) != 0x0;
	}
	
	glExt::ARB_texture_buffer_object = isExtensionSupported( "GL_ARB_texture_buffer_object" ) && isExtensionSupported( "GL_ARB_texture_rg" );
	if ( glExt::ARB_texture_buffer_object )
	{
		r &= ( glTexBufferARB = ( PFNGLTEXBUFFERPROC ) platGetProcAddress( "glTexBuffer" ) ) != 0x0;
	}
}

void initModernExtensions( bool &r )
{
//	throw std::exception( "The method or operation is not implemented." );
}

bool initOpenGLExtensions( bool forceLegacyFuncs )
{
	bool r = true;
	
	getOpenGLVersion();
	
	if ( forceLegacyFuncs )
	{
		if ( glExt::majorVersion > 2 || (glExt::majorVersion == 2 && glExt::minorVersion >= 1) )
		{
			glExt::majorVersion = 2; glExt::minorVersion = 1;
		}
	}
	// GL 1.1
	r &= (glPolygonMode = (PFNGLPOLYGONMODEPROC) platGetProcAddress( "glPolygonMode" )) != 0x0;
	r &= (glGetTexImage = (PFNGLGETTEXIMAGEPROC) platGetProcAddress( "glGetTexImage" )) != 0x0;
	r &= (glClearDepth = (PFNGLCLEARDEPTH) platGetProcAddress( "glClearDepth" )) != 0x0;
	r &= (glDrawBuffer = (PFNGLDRAWBUFFERPROC) platGetProcAddress( "glDrawBuffer" )) != 0x0;

	// GL 1.2
	r &= (glBlendColor = (PFNGLBLENDCOLORPROC) platGetProcAddress( "glBlendColor" )) != 0x0;
	r &= (glBlendEquation = (PFNGLBLENDEQUATIONPROC) platGetProcAddress( "glBlendEquation" )) != 0x0;
	r &= (glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC) platGetProcAddress( "glDrawRangeElements" )) != 0x0;
	r &= (glTexImage3D = (PFNGLTEXIMAGE3DPROC) platGetProcAddress( "glTexImage3D" )) != 0x0;
	r &= (glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC) platGetProcAddress( "glTexSubImage3D" )) != 0x0;
	r &= (glCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC) platGetProcAddress( "glCopyTexSubImage3D" )) != 0x0;

	// GL 1.3
	r &= (glActiveTexture = (PFNGLACTIVETEXTUREPROC) platGetProcAddress( "glActiveTexture" )) != 0x0;
	r &= (glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC) platGetProcAddress( "glSampleCoverage" )) != 0x0;
	r &= (glCompressedTexImage3D = (PFNGLCOMPRESSEDTEXIMAGE3DPROC) platGetProcAddress( "glCompressedTexImage3D" )) != 0x0;
	r &= (glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) platGetProcAddress( "glCompressedTexImage2D" )) != 0x0;
	r &= (glCompressedTexImage1D = (PFNGLCOMPRESSEDTEXIMAGE1DPROC) platGetProcAddress( "glCompressedTexImage1D" )) != 0x0;
	r &= (glCompressedTexSubImage3D = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) platGetProcAddress( "glCompressedTexSubImage3D" )) != 0x0;
	r &= (glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) platGetProcAddress( "glCompressedTexSubImage2D" )) != 0x0;
	r &= (glCompressedTexSubImage1D = (PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC) platGetProcAddress( "glCompressedTexSubImage1D" )) != 0x0;
	r &= (glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC) platGetProcAddress( "glGetCompressedTexImage" )) != 0x0;
	
	// GL 1.4
	r &= (glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) platGetProcAddress( "glBlendFuncSeparate" )) != 0x0;
	r &= (glMultiDrawArrays = (PFNGLMULTIDRAWARRAYSPROC) platGetProcAddress( "glMultiDrawArrays" )) != 0x0;
	r &= (glMultiDrawElements = (PFNGLMULTIDRAWELEMENTSPROC) platGetProcAddress( "glMultiDrawElements" )) != 0x0;
	r &= (glPointParameterf = (PFNGLPOINTPARAMETERFPROC) platGetProcAddress( "glPointParameterf" )) != 0x0;
	r &= (glPointParameterfv = (PFNGLPOINTPARAMETERFVPROC) platGetProcAddress( "glPointParameterfv" )) != 0x0;
	r &= (glPointParameteri = (PFNGLPOINTPARAMETERIPROC) platGetProcAddress( "glPointParameteri" )) != 0x0;
	r &= (glPointParameteriv = (PFNGLPOINTPARAMETERIVPROC) platGetProcAddress( "glPointParameteriv" )) != 0x0;

	// GL 1.5
	r &= (glGenQueries = (PFNGLGENQUERIESPROC) platGetProcAddress( "glGenQueries" )) != 0x0;
	r &= (glDeleteQueries = (PFNGLDELETEQUERIESPROC) platGetProcAddress( "glDeleteQueries" )) != 0x0;
	r &= (glIsQuery = (PFNGLISQUERYPROC) platGetProcAddress( "glIsQuery" )) != 0x0;
	r &= (glBeginQuery = (PFNGLBEGINQUERYPROC) platGetProcAddress( "glBeginQuery" )) != 0x0;
	r &= (glEndQuery = (PFNGLENDQUERYPROC) platGetProcAddress( "glEndQuery" )) != 0x0;
	r &= (glGetQueryiv = (PFNGLGETQUERYIVPROC) platGetProcAddress( "glGetQueryiv" )) != 0x0;
	r &= (glGetQueryObjectiv = (PFNGLGETQUERYOBJECTIVPROC) platGetProcAddress( "glGetQueryObjectiv" )) != 0x0;
	r &= (glGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVPROC) platGetProcAddress( "glGetQueryObjectuiv" )) != 0x0;
	r &= (glBindBuffer = (PFNGLBINDBUFFERPROC) platGetProcAddress( "glBindBuffer" )) != 0x0;
	r &= (glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) platGetProcAddress( "glDeleteBuffers" )) != 0x0;
	r &= (glGenBuffers = (PFNGLGENBUFFERSPROC) platGetProcAddress( "glGenBuffers" )) != 0x0;
	r &= (glIsBuffer = (PFNGLISBUFFERPROC) platGetProcAddress( "glIsBuffer" )) != 0x0;
	r &= (glBufferData = (PFNGLBUFFERDATAPROC) platGetProcAddress( "glBufferData" )) != 0x0;
	r &= (glBufferSubData = (PFNGLBUFFERSUBDATAPROC) platGetProcAddress( "glBufferSubData" )) != 0x0;
	r &= (glGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC) platGetProcAddress( "glGetBufferSubData" )) != 0x0;
	r &= (glMapBuffer = (PFNGLMAPBUFFERPROC) platGetProcAddress( "glMapBuffer" )) != 0x0;
	r &= (glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) platGetProcAddress( "glUnmapBuffer" )) != 0x0;
	r &= (glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC) platGetProcAddress( "glGetBufferParameteriv" )) != 0x0;
	r &= (glGetBufferPointerv = (PFNGLGETBUFFERPOINTERVPROC) platGetProcAddress( "glGetBufferPointerv" )) != 0x0;
	
	// GL 2.0
	r &= (glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC) platGetProcAddress( "glBlendEquationSeparate" )) != 0x0;
	r &= (glDrawBuffers = (PFNGLDRAWBUFFERSPROC) platGetProcAddress( "glDrawBuffers" )) != 0x0;
	r &= (glStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC) platGetProcAddress( "glStencilOpSeparate" )) != 0x0;
	r &= (glStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC) platGetProcAddress( "glStencilFuncSeparate" )) != 0x0;
	r &= (glStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC) platGetProcAddress( "glStencilMaskSeparate" )) != 0x0;
	r &= (glAttachShader = (PFNGLATTACHSHADERPROC) platGetProcAddress( "glAttachShader" )) != 0x0;
	r &= (glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) platGetProcAddress( "glBindAttribLocation" )) != 0x0;
	r &= (glCompileShader = (PFNGLCOMPILESHADERPROC) platGetProcAddress( "glCompileShader" )) != 0x0;
	r &= (glCreateProgram = (PFNGLCREATEPROGRAMPROC) platGetProcAddress( "glCreateProgram" )) != 0x0;
	r &= (glCreateShader = (PFNGLCREATESHADERPROC) platGetProcAddress( "glCreateShader" )) != 0x0;
	r &= (glDeleteProgram = (PFNGLDELETEPROGRAMPROC) platGetProcAddress( "glDeleteProgram" )) != 0x0;
	r &= (glDeleteShader = (PFNGLDELETESHADERPROC) platGetProcAddress( "glDeleteShader" )) != 0x0;
	r &= (glDetachShader = (PFNGLDETACHSHADERPROC) platGetProcAddress( "glDetachShader" )) != 0x0;
	r &= (glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) platGetProcAddress( "glDisableVertexAttribArray" )) != 0x0;
	r &= (glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) platGetProcAddress( "glEnableVertexAttribArray" )) != 0x0;
	r &= (glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC) platGetProcAddress( "glGetActiveAttrib" )) != 0x0;
	r &= (glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC) platGetProcAddress( "glGetActiveUniform" )) != 0x0;
	r &= (glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC) platGetProcAddress( "glGetAttachedShaders" )) != 0x0;
	r &= (glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) platGetProcAddress( "glGetAttribLocation" )) != 0x0;
	r &= (glGetProgramiv = (PFNGLGETPROGRAMIVPROC) platGetProcAddress( "glGetProgramiv" )) != 0x0;
	r &= (glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) platGetProcAddress( "glGetProgramInfoLog" )) != 0x0;
	r &= (glGetShaderiv = (PFNGLGETSHADERIVPROC) platGetProcAddress( "glGetShaderiv" )) != 0x0;
	r &= (glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) platGetProcAddress( "glGetShaderInfoLog" )) != 0x0;
	r &= (glGetShaderSource = (PFNGLGETSHADERSOURCEPROC) platGetProcAddress( "glGetShaderSource" )) != 0x0;
	r &= (glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) platGetProcAddress( "glGetUniformLocation" )) != 0x0;
	r &= (glGetUniformfv = (PFNGLGETUNIFORMFVPROC) platGetProcAddress( "glGetUniformfv" )) != 0x0;
	r &= (glGetUniformiv = (PFNGLGETUNIFORMIVPROC) platGetProcAddress( "glGetUniformiv" )) != 0x0;
	r &= (glGetVertexAttribdv = (PFNGLGETVERTEXATTRIBDVPROC) platGetProcAddress( "glGetVertexAttribdv" )) != 0x0;
	r &= (glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC) platGetProcAddress( "glGetVertexAttribfv" )) != 0x0;
	r &= (glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC) platGetProcAddress( "glGetVertexAttribiv" )) != 0x0;
	r &= (glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC) platGetProcAddress( "glGetVertexAttribPointerv" )) != 0x0;
	r &= (glIsProgram = (PFNGLISPROGRAMPROC) platGetProcAddress( "glIsProgram" )) != 0x0;
	r &= (glIsShader = (PFNGLISSHADERPROC) platGetProcAddress( "glIsShader" )) != 0x0;
	r &= (glLinkProgram = (PFNGLLINKPROGRAMPROC) platGetProcAddress( "glLinkProgram" )) != 0x0;
	r &= (glShaderSource = (PFNGLSHADERSOURCEPROC) platGetProcAddress( "glShaderSource" )) != 0x0;
	r &= (glUseProgram = (PFNGLUSEPROGRAMPROC) platGetProcAddress( "glUseProgram" )) != 0x0;
	r &= (glUniform1f = (PFNGLUNIFORM1FPROC) platGetProcAddress( "glUniform1f" )) != 0x0;
	r &= (glUniform2f = (PFNGLUNIFORM2FPROC) platGetProcAddress( "glUniform2f" )) != 0x0;
	r &= (glUniform3f = (PFNGLUNIFORM3FPROC) platGetProcAddress( "glUniform3f" )) != 0x0;
	r &= (glUniform4f = (PFNGLUNIFORM4FPROC) platGetProcAddress( "glUniform4f" )) != 0x0;
	r &= (glUniform1i = (PFNGLUNIFORM1IPROC) platGetProcAddress( "glUniform1i" )) != 0x0;
	r &= (glUniform2i = (PFNGLUNIFORM2IPROC) platGetProcAddress( "glUniform2i" )) != 0x0;
	r &= (glUniform3i = (PFNGLUNIFORM3IPROC) platGetProcAddress( "glUniform3i" )) != 0x0;
	r &= (glUniform4i = (PFNGLUNIFORM4IPROC) platGetProcAddress( "glUniform4i" )) != 0x0;
	r &= (glUniform1fv = (PFNGLUNIFORM1FVPROC) platGetProcAddress( "glUniform1fv" )) != 0x0;
	r &= (glUniform2fv = (PFNGLUNIFORM2FVPROC) platGetProcAddress( "glUniform2fv" )) != 0x0;
	r &= (glUniform3fv = (PFNGLUNIFORM3FVPROC) platGetProcAddress( "glUniform3fv" )) != 0x0;
	r &= (glUniform4fv = (PFNGLUNIFORM4FVPROC) platGetProcAddress( "glUniform4fv" )) != 0x0;
	r &= (glUniform1iv = (PFNGLUNIFORM1IVPROC) platGetProcAddress( "glUniform1iv" )) != 0x0;
	r &= (glUniform2iv = (PFNGLUNIFORM2IVPROC) platGetProcAddress( "glUniform2iv" )) != 0x0;
	r &= (glUniform3iv = (PFNGLUNIFORM3IVPROC) platGetProcAddress( "glUniform3iv" )) != 0x0;
	r &= (glUniform4iv = (PFNGLUNIFORM4IVPROC) platGetProcAddress( "glUniform4iv" )) != 0x0;
	r &= (glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC) platGetProcAddress( "glUniformMatrix2fv" )) != 0x0;
	r &= (glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC) platGetProcAddress( "glUniformMatrix3fv" )) != 0x0;
	r &= (glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) platGetProcAddress( "glUniformMatrix4fv" )) != 0x0;
	r &= (glValidateProgram = (PFNGLVALIDATEPROGRAMPROC) platGetProcAddress( "glValidateProgram" )) != 0x0;
	r &= (glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) platGetProcAddress( "glVertexAttribPointer" )) != 0x0;

	// GL 2.1
	r &= (glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC) platGetProcAddress( "glUniformMatrix2x3fv" )) != 0x0;
	r &= (glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC) platGetProcAddress( "glUniformMatrix3x2fv" )) != 0x0;
	r &= (glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC) platGetProcAddress( "glUniformMatrix2x4fv" )) != 0x0;
	r &= (glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC) platGetProcAddress( "glUniformMatrix4x2fv" )) != 0x0;
	r &= (glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC) platGetProcAddress( "glUniformMatrix3x4fv" )) != 0x0;
	r &= (glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC) platGetProcAddress( "glUniformMatrix4x3fv" )) != 0x0;

	// GL 3.0 - GL 3.3 
	if( glExt::majorVersion >= 3 )
	{
		// GL 3.0
		r &= ( glGetStringi = (PFNGLGETSTRINGIPROC) platGetProcAddress( "glGetStringi" )) != 0;
		r &= ( glColorMaski = ( PFNGLCOLORMASKIPROC ) platGetProcAddress( "glColorMaski" ) ) != 0;
		r &= ( glGetBooleani_v = ( PFNGLGETBOOLEANI_VPROC ) platGetProcAddress( "glGetBooleani_v" ) ) != 0;
		r &= ( glGetIntegeri_v = ( PFNGLGETINTEGERI_VPROC ) platGetProcAddress( "glGetIntegeri_v" ) ) != 0;
		r &= ( glEnablei = ( PFNGLENABLEIPROC ) platGetProcAddress( "glEnablei" ) ) != 0;
		r &= ( glDisablei = ( PFNGLDISABLEIPROC ) platGetProcAddress( "glDisablei" ) ) != 0;
		r &= ( glIsEnabledi = ( PFNGLISENABLEDIPROC ) platGetProcAddress( "glIsEnabledi" ) ) != 0;
		r &= ( glBeginTransformFeedback = ( PFNGLBEGINTRANSFORMFEEDBACKPROC ) platGetProcAddress( "glBeginTransformFeedback" ) ) != 0;
		r &= ( glEndTransformFeedback = ( PFNGLENDTRANSFORMFEEDBACKPROC ) platGetProcAddress( "glEndTransformFeedback" ) ) != 0;
		r &= ( glBindBufferRange = ( PFNGLBINDBUFFERRANGEPROC ) platGetProcAddress( "glBindBufferRange" ) ) != 0;
		r &= ( glBindBufferBase = ( PFNGLBINDBUFFERBASEPROC ) platGetProcAddress( "glBindBufferBase" ) ) != 0;
		r &= ( glTransformFeedbackVaryings = ( PFNGLTRANSFORMFEEDBACKVARYINGSPROC ) platGetProcAddress( "glTransformFeedbackVaryings" ) ) != 0;
		r &= ( glGetTransformFeedbackVarying = ( PFNGLGETTRANSFORMFEEDBACKVARYINGPROC ) platGetProcAddress( "glGetTransformFeedbackVarying" ) ) != 0;
		r &= ( glClampColor = ( PFNGLCLAMPCOLORPROC ) platGetProcAddress( "glClampColor" ) ) != 0;
		r &= ( glBeginConditionalRender = ( PFNGLBEGINCONDITIONALRENDERPROC ) platGetProcAddress( "glBeginConditionalRender" ) ) != 0;
		r &= ( glEndConditionalRender = ( PFNGLENDCONDITIONALRENDERPROC ) platGetProcAddress( "glEndConditionalRender" ) ) != 0;
		r &= ( glVertexAttribIPointer = ( PFNGLVERTEXATTRIBIPOINTERPROC ) platGetProcAddress( "glVertexAttribIPointer" ) ) != 0;
		r &= ( glGetVertexAttribIiv = ( PFNGLGETVERTEXATTRIBIIVPROC ) platGetProcAddress( "glGetVertexAttribIiv" ) ) != 0;
		r &= ( glGetVertexAttribIuiv = ( PFNGLGETVERTEXATTRIBIUIVPROC ) platGetProcAddress( "glGetVertexAttribIuiv" ) ) != 0;
		r &= ( glVertexAttribI1i = ( PFNGLVERTEXATTRIBI1IPROC ) platGetProcAddress( "glVertexAttribI1i" ) ) != 0;
		r &= ( glVertexAttribI2i = ( PFNGLVERTEXATTRIBI2IPROC ) platGetProcAddress( "glVertexAttribI2i" ) ) != 0;
		r &= ( glVertexAttribI3i = ( PFNGLVERTEXATTRIBI3IPROC ) platGetProcAddress( "glVertexAttribI3i" ) ) != 0;
		r &= ( glVertexAttribI4i = ( PFNGLVERTEXATTRIBI4IPROC ) platGetProcAddress( "glVertexAttribI4i" ) ) != 0;
		r &= ( glVertexAttribI1ui = ( PFNGLVERTEXATTRIBI1UIPROC ) platGetProcAddress( "glVertexAttribI1ui" ) ) != 0;
		r &= ( glVertexAttribI2ui = ( PFNGLVERTEXATTRIBI2UIPROC ) platGetProcAddress( "glVertexAttribI2ui" ) ) != 0;
		r &= ( glVertexAttribI3ui = ( PFNGLVERTEXATTRIBI3UIPROC ) platGetProcAddress( "glVertexAttribI3ui" ) ) != 0;
		r &= ( glVertexAttribI4ui = ( PFNGLVERTEXATTRIBI4UIPROC ) platGetProcAddress( "glVertexAttribI4ui" ) ) != 0;
		r &= ( glVertexAttribI1iv = ( PFNGLVERTEXATTRIBI1IVPROC ) platGetProcAddress( "glVertexAttribI1iv" ) ) != 0;
		r &= ( glVertexAttribI2iv = ( PFNGLVERTEXATTRIBI2IVPROC ) platGetProcAddress( "glVertexAttribI2iv" ) ) != 0;
		r &= ( glVertexAttribI3iv = ( PFNGLVERTEXATTRIBI3IVPROC ) platGetProcAddress( "glVertexAttribI3iv" ) ) != 0;
		r &= ( glVertexAttribI4iv = ( PFNGLVERTEXATTRIBI4IVPROC ) platGetProcAddress( "glVertexAttribI4iv" ) ) != 0;
		r &= ( glVertexAttribI1uiv = ( PFNGLVERTEXATTRIBI1UIVPROC ) platGetProcAddress( "glVertexAttribI1uiv" ) ) != 0;
		r &= ( glVertexAttribI2uiv = ( PFNGLVERTEXATTRIBI2UIVPROC ) platGetProcAddress( "glVertexAttribI2uiv" ) ) != 0;
		r &= ( glVertexAttribI3uiv = ( PFNGLVERTEXATTRIBI3UIVPROC ) platGetProcAddress( "glVertexAttribI3uiv" ) ) != 0;
		r &= ( glVertexAttribI4uiv = ( PFNGLVERTEXATTRIBI4UIVPROC ) platGetProcAddress( "glVertexAttribI4uiv" ) ) != 0;
		r &= ( glVertexAttribI4bv = ( PFNGLVERTEXATTRIBI4BVPROC ) platGetProcAddress( "glVertexAttribI4bv" ) ) != 0;
		r &= ( glVertexAttribI4sv = ( PFNGLVERTEXATTRIBI4SVPROC ) platGetProcAddress( "glVertexAttribI4sv" ) ) != 0;
		r &= ( glVertexAttribI4ubv = ( PFNGLVERTEXATTRIBI4UBVPROC ) platGetProcAddress( "glVertexAttribI4ubv" ) ) != 0;
		r &= ( glVertexAttribI4usv = ( PFNGLVERTEXATTRIBI4USVPROC ) platGetProcAddress( "glVertexAttribI4usv" ) ) != 0;
		r &= ( glGetUniformuiv = ( PFNGLGETUNIFORMUIVPROC ) platGetProcAddress( "glGetUniformuiv" ) ) != 0;
		r &= ( glBindFragDataLocation = ( PFNGLBINDFRAGDATALOCATIONPROC ) platGetProcAddress( "glBindFragDataLocation" ) ) != 0;
		r &= ( glGetFragDataLocation = ( PFNGLGETFRAGDATALOCATIONPROC ) platGetProcAddress( "glGetFragDataLocation" ) ) != 0;
		r &= ( glUniform1ui = ( PFNGLUNIFORM1UIPROC ) platGetProcAddress( "glUniform1ui" ) ) != 0;
		r &= ( glUniform2ui = ( PFNGLUNIFORM2UIPROC ) platGetProcAddress( "glUniform2ui" ) ) != 0;
		r &= ( glUniform3ui = ( PFNGLUNIFORM3UIPROC ) platGetProcAddress( "glUniform3ui" ) ) != 0;
		r &= ( glUniform4ui = ( PFNGLUNIFORM4UIPROC ) platGetProcAddress( "glUniform4ui" ) ) != 0;
		r &= ( glUniform1uiv = ( PFNGLUNIFORM1UIVPROC ) platGetProcAddress( "glUniform1uiv" ) ) != 0;
		r &= ( glUniform2uiv = ( PFNGLUNIFORM2UIVPROC ) platGetProcAddress( "glUniform2uiv" ) ) != 0;
		r &= ( glUniform3uiv = ( PFNGLUNIFORM3UIVPROC ) platGetProcAddress( "glUniform3uiv" ) ) != 0;
		r &= ( glUniform4uiv = ( PFNGLUNIFORM4UIVPROC ) platGetProcAddress( "glUniform4uiv" ) ) != 0;
		r &= ( glTexParameterIiv = ( PFNGLTEXPARAMETERIIVPROC ) platGetProcAddress( "glTexParameterIiv" ) ) != 0;
		r &= ( glTexParameterIuiv = ( PFNGLTEXPARAMETERIUIVPROC ) platGetProcAddress( "glTexParameterIuiv" ) ) != 0;
		r &= ( glGetTexParameterIiv = ( PFNGLGETTEXPARAMETERIIVPROC ) platGetProcAddress( "glGetTexParameterIiv" ) ) != 0;
		r &= ( glGetTexParameterIuiv = ( PFNGLGETTEXPARAMETERIUIVPROC ) platGetProcAddress( "glGetTexParameterIuiv" ) ) != 0;
		r &= ( glClearBufferiv = ( PFNGLCLEARBUFFERIVPROC ) platGetProcAddress( "glClearBufferiv" ) ) != 0;
		r &= ( glClearBufferuiv = ( PFNGLCLEARBUFFERUIVPROC ) platGetProcAddress( "glClearBufferuiv" ) ) != 0;
		r &= ( glClearBufferfv = ( PFNGLCLEARBUFFERFVPROC ) platGetProcAddress( "glClearBufferfv" ) ) != 0;
		r &= ( glClearBufferfi = ( PFNGLCLEARBUFFERFIPROC ) platGetProcAddress( "glClearBufferfi" ) ) != 0;
		r &= ( glIsRenderbuffer = ( PFNGLISRENDERBUFFERPROC ) platGetProcAddress( "glIsRenderbuffer" ) ) != 0;
		r &= ( glBindRenderbuffer = ( PFNGLBINDRENDERBUFFERPROC ) platGetProcAddress( "glBindRenderbuffer" ) ) != 0;
		r &= ( glDeleteRenderbuffers = ( PFNGLDELETERENDERBUFFERSPROC ) platGetProcAddress( "glDeleteRenderbuffers" ) ) != 0;
		r &= ( glGenRenderbuffers = ( PFNGLGENRENDERBUFFERSPROC ) platGetProcAddress( "glGenRenderbuffers" ) ) != 0;
		r &= ( glRenderbufferStorage = ( PFNGLRENDERBUFFERSTORAGEPROC ) platGetProcAddress( "glRenderbufferStorage" ) ) != 0;
		r &= ( glGetRenderbufferParameteriv = ( PFNGLGETRENDERBUFFERPARAMETERIVPROC ) platGetProcAddress( "glGetRenderbufferParameteriv" ) ) != 0;
		r &= ( glIsFramebuffer = ( PFNGLISFRAMEBUFFERPROC ) platGetProcAddress( "glIsFramebuffer" ) ) != 0;
		r &= ( glBindFramebuffer = ( PFNGLBINDFRAMEBUFFERPROC ) platGetProcAddress( "glBindFramebuffer" ) ) != 0;
		r &= ( glDeleteFramebuffers = ( PFNGLDELETEFRAMEBUFFERSPROC ) platGetProcAddress( "glDeleteFramebuffers" ) ) != 0;
		r &= ( glGenFramebuffers = ( PFNGLGENFRAMEBUFFERSPROC ) platGetProcAddress( "glGenFramebuffers" ) ) != 0;
		r &= ( glCheckFramebufferStatus = ( PFNGLCHECKFRAMEBUFFERSTATUSPROC ) platGetProcAddress( "glCheckFramebufferStatus" ) ) != 0;
		r &= ( glFramebufferTexture1D = ( PFNGLFRAMEBUFFERTEXTURE1DPROC ) platGetProcAddress( "glFramebufferTexture1D" ) ) != 0;
		r &= ( glFramebufferTexture2D = ( PFNGLFRAMEBUFFERTEXTURE2DPROC ) platGetProcAddress( "glFramebufferTexture2D" ) ) != 0;
		r &= ( glFramebufferTexture3D = ( PFNGLFRAMEBUFFERTEXTURE3DPROC ) platGetProcAddress( "glFramebufferTexture3D" ) ) != 0;
		r &= ( glFramebufferRenderbuffer = ( PFNGLFRAMEBUFFERRENDERBUFFERPROC ) platGetProcAddress( "glFramebufferRenderbuffer" ) ) != 0;
		r &= ( glGetFramebufferAttachmentParameteriv = ( PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC ) platGetProcAddress( "glGetFramebufferAttachmentParameteriv" ) ) != 0;
		r &= ( glGenerateMipmap = ( PFNGLGENERATEMIPMAPPROC ) platGetProcAddress( "glGenerateMipmap" ) ) != 0;
		r &= ( glBlitFramebuffer = ( PFNGLBLITFRAMEBUFFERPROC ) platGetProcAddress( "glBlitFramebuffer" ) ) != 0;
		r &= ( glRenderbufferStorageMultisample = ( PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC ) platGetProcAddress( "glRenderbufferStorageMultisample" ) ) != 0;
		r &= ( glFramebufferTextureLayer = ( PFNGLFRAMEBUFFERTEXTURELAYERPROC ) platGetProcAddress( "glFramebufferTextureLayer" ) ) != 0;
		r &= ( glMapBufferRange = ( PFNGLMAPBUFFERRANGEPROC ) platGetProcAddress( "glMapBufferRange" ) ) != 0;
		r &= ( glFlushMappedBufferRange = ( PFNGLFLUSHMAPPEDBUFFERRANGEPROC ) platGetProcAddress( "glFlushMappedBufferRange" ) ) != 0;
		r &= ( glBindVertexArray = ( PFNGLBINDVERTEXARRAYPROC ) platGetProcAddress( "glBindVertexArray" ) ) != 0;
		r &= ( glDeleteVertexArrays = ( PFNGLDELETEVERTEXARRAYSPROC ) platGetProcAddress( "glDeleteVertexArrays" ) ) != 0;
		r &= ( glGenVertexArrays = ( PFNGLGENVERTEXARRAYSPROC ) platGetProcAddress( "glGenVertexArrays" ) ) != 0;
		r &= ( glIsVertexArray = ( PFNGLISVERTEXARRAYPROC ) platGetProcAddress( "glIsVertexArray" ) ) != 0;

		// GL 3.1
		if ( glExt::majorVersion * 10 + glExt::minorVersion >= 31 )
		{
			r &= ( glDrawArraysInstanced = ( PFNGLDRAWARRAYSINSTANCEDPROC ) platGetProcAddress( "glDrawArraysInstanced" ) ) != 0;
			r &= ( glDrawElementsInstanced = ( PFNGLDRAWELEMENTSINSTANCEDPROC ) platGetProcAddress( "glDrawElementsInstanced" ) ) != 0;
			r &= ( glTexBuffer = ( PFNGLTEXBUFFERPROC ) platGetProcAddress( "glTexBuffer" ) ) != 0;
			r &= ( glPrimitiveRestartIndex = ( PFNGLPRIMITIVERESTARTINDEXPROC ) platGetProcAddress( "glPrimitiveRestartIndex" ) ) != 0;
			r &= ( glCopyBufferSubData = ( PFNGLCOPYBUFFERSUBDATAPROC ) platGetProcAddress( "glCopyBufferSubData" ) ) != 0;
			r &= ( glGetUniformIndices = ( PFNGLGETUNIFORMINDICESPROC ) platGetProcAddress( "glGetUniformIndices" ) ) != 0;
			r &= ( glGetActiveUniformsiv = ( PFNGLGETACTIVEUNIFORMSIVPROC ) platGetProcAddress( "glGetActiveUniformsiv" ) ) != 0;
			r &= ( glGetActiveUniformName = ( PFNGLGETACTIVEUNIFORMNAMEPROC ) platGetProcAddress( "glGetActiveUniformName" ) ) != 0;
			r &= ( glGetUniformBlockIndex = ( PFNGLGETUNIFORMBLOCKINDEXPROC ) platGetProcAddress( "glGetUniformBlockIndex" ) ) != 0;
			r &= ( glGetActiveUniformBlockiv = ( PFNGLGETACTIVEUNIFORMBLOCKIVPROC ) platGetProcAddress( "glGetActiveUniformBlockiv" ) ) != 0;
			r &= ( glGetActiveUniformBlockName = ( PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC ) platGetProcAddress( "glGetActiveUniformBlockName" ) ) != 0;
			r &= ( glUniformBlockBinding = ( PFNGLUNIFORMBLOCKBINDINGPROC ) platGetProcAddress( "glUniformBlockBinding" ) ) != 0;
		}

		// GL 3.2
		if ( glExt::majorVersion * 10 + glExt::minorVersion >= 32 )
		{
			r &= ( glDrawElementsBaseVertex = ( PFNGLDRAWELEMENTSBASEVERTEXPROC ) platGetProcAddress( "glDrawElementsBaseVertex" ) ) != 0;
			r &= ( glDrawRangeElementsBaseVertex = ( PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC ) platGetProcAddress( "glDrawRangeElementsBaseVertex" ) ) != 0;
			r &= ( glDrawElementsInstancedBaseVertex = ( PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC ) platGetProcAddress( "glDrawElementsInstancedBaseVertex" ) ) != 0;
			r &= ( glMultiDrawElementsBaseVertex = ( PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC ) platGetProcAddress( "glMultiDrawElementsBaseVertex" ) ) != 0;
			r &= ( glProvokingVertex = ( PFNGLPROVOKINGVERTEXPROC ) platGetProcAddress( "glProvokingVertex" ) ) != 0;
			r &= ( glFenceSync = ( PFNGLFENCESYNCPROC ) platGetProcAddress( "glFenceSync" ) ) != 0;
			r &= ( glIsSync = ( PFNGLISSYNCPROC ) platGetProcAddress( "glIsSync" ) ) != 0;
			r &= ( glDeleteSync = ( PFNGLDELETESYNCPROC ) platGetProcAddress( "glDeleteSync" ) ) != 0;
			r &= ( glClientWaitSync = ( PFNGLCLIENTWAITSYNCPROC ) platGetProcAddress( "glClientWaitSync" ) ) != 0;
			r &= ( glWaitSync = ( PFNGLWAITSYNCPROC ) platGetProcAddress( "glWaitSync" ) ) != 0;
			r &= ( glGetInteger64v = ( PFNGLGETINTEGER64VPROC ) platGetProcAddress( "glGetInteger64v" ) ) != 0;
			r &= ( glGetSynciv = ( PFNGLGETSYNCIVPROC ) platGetProcAddress( "glGetSynciv" ) ) != 0;
			r &= ( glGetInteger64i_v = ( PFNGLGETINTEGER64I_VPROC ) platGetProcAddress( "glGetInteger64i_v" ) ) != 0;
			r &= ( glGetBufferParameteri64v = ( PFNGLGETBUFFERPARAMETERI64VPROC ) platGetProcAddress( "glGetBufferParameteri64v" ) ) != 0;
			r &= ( glFramebufferTexture = ( PFNGLFRAMEBUFFERTEXTUREPROC ) platGetProcAddress( "glFramebufferTexture" ) ) != 0;
			r &= ( glTexImage2DMultisample = ( PFNGLTEXIMAGE2DMULTISAMPLEPROC ) platGetProcAddress( "glTexImage2DMultisample" ) ) != 0;
			r &= ( glTexImage3DMultisample = ( PFNGLTEXIMAGE3DMULTISAMPLEPROC ) platGetProcAddress( "glTexImage3DMultisample" ) ) != 0;
			r &= ( glGetMultisamplefv = ( PFNGLGETMULTISAMPLEFVPROC ) platGetProcAddress( "glGetMultisamplefv" ) ) != 0;
			r &= ( glSampleMaski = ( PFNGLSAMPLEMASKIPROC ) platGetProcAddress( "glSampleMaski" ) ) != 0;
		}

		// GL 3.3
		if ( glExt::majorVersion * 10 + glExt::minorVersion >= 33 )
		{
			r &= ( glBindFragDataLocationIndexed = ( PFNGLBINDFRAGDATALOCATIONINDEXEDPROC ) platGetProcAddress( "glBindFragDataLocationIndexed" ) ) != 0;
			r &= ( glGetFragDataIndex = ( PFNGLGETFRAGDATAINDEXPROC ) platGetProcAddress( "glGetFragDataIndex" ) ) != 0;
			r &= ( glGenSamplers = ( PFNGLGENSAMPLERSPROC ) platGetProcAddress( "glGenSamplers" ) ) != 0;
			r &= ( glDeleteSamplers = ( PFNGLDELETESAMPLERSPROC ) platGetProcAddress( "glDeleteSamplers" ) ) != 0;
			r &= ( glIsSampler = ( PFNGLISSAMPLERPROC ) platGetProcAddress( "glIsSampler" ) ) != 0;
			r &= ( glBindSampler = ( PFNGLBINDSAMPLERPROC ) platGetProcAddress( "glBindSampler" ) ) != 0;
			r &= ( glSamplerParameteri = ( PFNGLSAMPLERPARAMETERIPROC ) platGetProcAddress( "glSamplerParameteri" ) ) != 0;
			r &= ( glSamplerParameteriv = ( PFNGLSAMPLERPARAMETERIVPROC ) platGetProcAddress( "glSamplerParameteriv" ) ) != 0;
			r &= ( glSamplerParameterf = ( PFNGLSAMPLERPARAMETERFPROC ) platGetProcAddress( "glSamplerParameterf" ) ) != 0;
			r &= ( glSamplerParameterfv = ( PFNGLSAMPLERPARAMETERFVPROC ) platGetProcAddress( "glSamplerParameterfv" ) ) != 0;
			r &= ( glSamplerParameterIiv = ( PFNGLSAMPLERPARAMETERIIVPROC ) platGetProcAddress( "glSamplerParameterIiv" ) ) != 0;
			r &= ( glSamplerParameterIuiv = ( PFNGLSAMPLERPARAMETERIUIVPROC ) platGetProcAddress( "glSamplerParameterIuiv" ) ) != 0;
			r &= ( glGetSamplerParameteriv = ( PFNGLGETSAMPLERPARAMETERIVPROC ) platGetProcAddress( "glGetSamplerParameteriv" ) ) != 0;
			r &= ( glGetSamplerParameterIiv = ( PFNGLGETSAMPLERPARAMETERIIVPROC ) platGetProcAddress( "glGetSamplerParameterIiv" ) ) != 0;
			r &= ( glGetSamplerParameterfv = ( PFNGLGETSAMPLERPARAMETERFVPROC ) platGetProcAddress( "glGetSamplerParameterfv" ) ) != 0;
			r &= ( glGetSamplerParameterIuiv = ( PFNGLGETSAMPLERPARAMETERIUIVPROC ) platGetProcAddress( "glGetSamplerParameterIuiv" ) ) != 0;
			r &= ( glQueryCounter = ( PFNGLQUERYCOUNTERPROC ) platGetProcAddress( "glQueryCounter" ) ) != 0;
			r &= ( glGetQueryObjecti64v = ( PFNGLGETQUERYOBJECTI64VPROC ) platGetProcAddress( "glGetQueryObjecti64v" ) ) != 0;
			r &= ( glGetQueryObjectui64v = ( PFNGLGETQUERYOBJECTUI64VPROC ) platGetProcAddress( "glGetQueryObjectui64v" ) ) != 0;
			r &= ( glVertexAttribDivisor = ( PFNGLVERTEXATTRIBDIVISORPROC ) platGetProcAddress( "glVertexAttribDivisor" ) ) != 0;
			r &= ( glVertexAttribP1ui = ( PFNGLVERTEXATTRIBP1UIPROC ) platGetProcAddress( "glVertexAttribP1ui" ) ) != 0;
			r &= ( glVertexAttribP1uiv = ( PFNGLVERTEXATTRIBP1UIVPROC ) platGetProcAddress( "glVertexAttribP1uiv" ) ) != 0;
			r &= ( glVertexAttribP2ui = ( PFNGLVERTEXATTRIBP2UIPROC ) platGetProcAddress( "glVertexAttribP2ui" ) ) != 0;
			r &= ( glVertexAttribP2uiv = ( PFNGLVERTEXATTRIBP2UIVPROC ) platGetProcAddress( "glVertexAttribP2uiv" ) ) != 0;
			r &= ( glVertexAttribP3ui = ( PFNGLVERTEXATTRIBP3UIPROC ) platGetProcAddress( "glVertexAttribP3ui" ) ) != 0;
			r &= ( glVertexAttribP3uiv = ( PFNGLVERTEXATTRIBP3UIVPROC ) platGetProcAddress( "glVertexAttribP3uiv" ) ) != 0;
			r &= ( glVertexAttribP4ui = ( PFNGLVERTEXATTRIBP4UIPROC ) platGetProcAddress( "glVertexAttribP4ui" ) ) != 0;
			r &= ( glVertexAttribP4uiv = ( PFNGLVERTEXATTRIBP4UIVPROC ) platGetProcAddress( "glVertexAttribP4uiv" ) ) != 0;
		}
	}
	
	// GL 4.0 - GL 4.4
	if ( glExt::majorVersion >= 4 )
	{
		// GL 4.0
		r &= ( glMinSampleShading = ( PFNGLMINSAMPLESHADINGPROC ) platGetProcAddress( "glMinSampleShading" ) ) != 0;
		r &= ( glBlendEquationi = ( PFNGLBLENDEQUATIONIPROC ) platGetProcAddress( "glBlendEquationi" ) ) != 0;
		r &= ( glBlendEquationSeparatei = ( PFNGLBLENDEQUATIONSEPARATEIPROC ) platGetProcAddress( "glBlendEquationSeparatei" ) ) != 0;
		r &= ( glBlendFunci = ( PFNGLBLENDFUNCIPROC ) platGetProcAddress( "glBlendFunci" ) ) != 0;
		r &= ( glBlendFuncSeparatei = ( PFNGLBLENDFUNCSEPARATEIPROC ) platGetProcAddress( "glBlendFuncSeparatei" ) ) != 0;
		r &= ( glDrawArraysIndirect = ( PFNGLDRAWARRAYSINDIRECTPROC ) platGetProcAddress( "glDrawArraysIndirect" ) ) != 0;
		r &= ( glDrawElementsIndirect = ( PFNGLDRAWELEMENTSINDIRECTPROC ) platGetProcAddress( "glDrawElementsIndirect" ) ) != 0;
		r &= ( glUniform1d = ( PFNGLUNIFORM1DPROC ) platGetProcAddress( "glUniform1d" ) ) != 0;
		r &= ( glUniform2d = ( PFNGLUNIFORM2DPROC ) platGetProcAddress( "glUniform2d" ) ) != 0;
		r &= ( glUniform3d = ( PFNGLUNIFORM3DPROC ) platGetProcAddress( "glUniform3d" ) ) != 0;
		r &= ( glUniform4d = ( PFNGLUNIFORM4DPROC ) platGetProcAddress( "glUniform4d" ) ) != 0;
		r &= ( glUniform1dv = ( PFNGLUNIFORM1DVPROC ) platGetProcAddress( "glUniform1dv" ) ) != 0;
		r &= ( glUniform2dv = ( PFNGLUNIFORM2DVPROC ) platGetProcAddress( "glUniform2dv" ) ) != 0;
		r &= ( glUniform3dv = ( PFNGLUNIFORM3DVPROC ) platGetProcAddress( "glUniform3dv" ) ) != 0;
		r &= ( glUniform4dv = ( PFNGLUNIFORM4DVPROC ) platGetProcAddress( "glUniform4dv" ) ) != 0;
		r &= ( glUniformMatrix2dv = ( PFNGLUNIFORMMATRIX2DVPROC ) platGetProcAddress( "glUniformMatrix2dv" ) ) != 0;
		r &= ( glUniformMatrix3dv = ( PFNGLUNIFORMMATRIX3DVPROC ) platGetProcAddress( "glUniformMatrix3dv" ) ) != 0;
		r &= ( glUniformMatrix4dv = ( PFNGLUNIFORMMATRIX4DVPROC ) platGetProcAddress( "glUniformMatrix4dv" ) ) != 0;
		r &= ( glUniformMatrix2x3dv = ( PFNGLUNIFORMMATRIX2X3DVPROC ) platGetProcAddress( "glUniformMatrix2x3dv" ) ) != 0;
		r &= ( glUniformMatrix2x4dv = ( PFNGLUNIFORMMATRIX2X4DVPROC ) platGetProcAddress( "glUniformMatrix2x4dv" ) ) != 0;
		r &= ( glUniformMatrix3x2dv = ( PFNGLUNIFORMMATRIX3X2DVPROC ) platGetProcAddress( "glUniformMatrix3x2dv" ) ) != 0;
		r &= ( glUniformMatrix3x4dv = ( PFNGLUNIFORMMATRIX3X4DVPROC ) platGetProcAddress( "glUniformMatrix3x4dv" ) ) != 0;
		r &= ( glUniformMatrix4x2dv = ( PFNGLUNIFORMMATRIX4X2DVPROC ) platGetProcAddress( "glUniformMatrix4x2dv" ) ) != 0;
		r &= ( glUniformMatrix4x3dv = ( PFNGLUNIFORMMATRIX4X3DVPROC ) platGetProcAddress( "glUniformMatrix4x3dv" ) ) != 0;
		r &= ( glGetUniformdv = ( PFNGLGETUNIFORMDVPROC ) platGetProcAddress( "glGetUniformdv" ) ) != 0;
		r &= ( glGetSubroutineUniformLocation = ( PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC ) platGetProcAddress( "glGetSubroutineUniformLocation" ) ) != 0;
		r &= ( glGetSubroutineIndex = ( PFNGLGETSUBROUTINEINDEXPROC ) platGetProcAddress( "glGetSubroutineIndex" ) ) != 0;
		r &= ( glGetActiveSubroutineUniformiv = ( PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC ) platGetProcAddress( "glGetActiveSubroutineUniformiv" ) ) != 0;
		r &= ( glGetActiveSubroutineUniformName = ( PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC ) platGetProcAddress( "glGetActiveSubroutineUniformName" ) ) != 0;
		r &= ( glGetActiveSubroutineName = ( PFNGLGETACTIVESUBROUTINENAMEPROC ) platGetProcAddress( "glGetActiveSubroutineName" ) ) != 0;
		r &= ( glUniformSubroutinesuiv = ( PFNGLUNIFORMSUBROUTINESUIVPROC ) platGetProcAddress( "glUniformSubroutinesuiv" ) ) != 0;
		r &= ( glGetUniformSubroutineuiv = ( PFNGLGETUNIFORMSUBROUTINEUIVPROC ) platGetProcAddress( "glGetUniformSubroutineuiv" ) ) != 0;
		r &= ( glGetProgramStageiv = ( PFNGLGETPROGRAMSTAGEIVPROC ) platGetProcAddress( "glGetProgramStageiv" ) ) != 0;
		r &= ( glPatchParameteri = ( PFNGLPATCHPARAMETERIPROC ) platGetProcAddress( "glPatchParameteri" ) ) != 0;
		r &= ( glPatchParameterfv = ( PFNGLPATCHPARAMETERFVPROC ) platGetProcAddress( "glPatchParameterfv" ) ) != 0;
		r &= ( glBindTransformFeedback = ( PFNGLBINDTRANSFORMFEEDBACKPROC ) platGetProcAddress( "glBindTransformFeedback" ) ) != 0;
		r &= ( glDeleteTransformFeedbacks = ( PFNGLDELETETRANSFORMFEEDBACKSPROC ) platGetProcAddress( "glDeleteTransformFeedbacks" ) ) != 0;
		r &= ( glGenTransformFeedbacks = ( PFNGLGENTRANSFORMFEEDBACKSPROC ) platGetProcAddress( "glGenTransformFeedbacks" ) ) != 0;
		r &= ( glIsTransformFeedback = ( PFNGLISTRANSFORMFEEDBACKPROC ) platGetProcAddress( "glIsTransformFeedback" ) ) != 0;
		r &= ( glPauseTransformFeedback = ( PFNGLPAUSETRANSFORMFEEDBACKPROC ) platGetProcAddress( "glPauseTransformFeedback" ) ) != 0;
		r &= ( glResumeTransformFeedback = ( PFNGLRESUMETRANSFORMFEEDBACKPROC ) platGetProcAddress( "glResumeTransformFeedback" ) ) != 0;
		r &= ( glDrawTransformFeedback = ( PFNGLDRAWTRANSFORMFEEDBACKPROC ) platGetProcAddress( "glDrawTransformFeedback" ) ) != 0;
		r &= ( glDrawTransformFeedbackStream = ( PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC ) platGetProcAddress( "glDrawTransformFeedbackStream" ) ) != 0;
		r &= ( glBeginQueryIndexed = ( PFNGLBEGINQUERYINDEXEDPROC ) platGetProcAddress( "glBeginQueryIndexed" ) ) != 0;
		r &= ( glEndQueryIndexed = ( PFNGLENDQUERYINDEXEDPROC ) platGetProcAddress( "glEndQueryIndexed" ) ) != 0;
		r &= ( glGetQueryIndexediv = ( PFNGLGETQUERYINDEXEDIVPROC ) platGetProcAddress( "glGetQueryIndexediv" ) ) != 0;

		if ( glExt::minorVersion >= 1 )
		{
			// GL 4.1
			r &= ( glReleaseShaderCompiler = ( PFNGLRELEASESHADERCOMPILERPROC ) platGetProcAddress( "glReleaseShaderCompiler" ) ) != 0;
			r &= ( glShaderBinary = ( PFNGLSHADERBINARYPROC ) platGetProcAddress( "glShaderBinary" ) ) != 0;
			r &= ( glGetShaderPrecisionFormat = ( PFNGLGETSHADERPRECISIONFORMATPROC ) platGetProcAddress( "glGetShaderPrecisionFormat" ) ) != 0;
			r &= ( glDepthRangef = ( PFNGLDEPTHRANGEFPROC ) platGetProcAddress( "glDepthRangef" ) ) != 0;
			r &= ( glClearDepthf = ( PFNGLCLEARDEPTHFPROC ) platGetProcAddress( "glClearDepthf" ) ) != 0;
			r &= ( glGetProgramBinary = ( PFNGLGETPROGRAMBINARYPROC ) platGetProcAddress( "glGetProgramBinary" ) ) != 0;
			r &= ( glProgramBinary = ( PFNGLPROGRAMBINARYPROC ) platGetProcAddress( "glProgramBinary" ) ) != 0;
			r &= ( glProgramParameteri = ( PFNGLPROGRAMPARAMETERIPROC ) platGetProcAddress( "glProgramParameteri" ) ) != 0;
			r &= ( glUseProgramStages = ( PFNGLUSEPROGRAMSTAGESPROC ) platGetProcAddress( "glUseProgramStages" ) ) != 0;
			r &= ( glActiveShaderProgram = ( PFNGLACTIVESHADERPROGRAMPROC ) platGetProcAddress( "glActiveShaderProgram" ) ) != 0;
			r &= ( glCreateShaderProgramv = ( PFNGLCREATESHADERPROGRAMVPROC ) platGetProcAddress( "glCreateShaderProgramv" ) ) != 0;
			r &= ( glBindProgramPipeline = ( PFNGLBINDPROGRAMPIPELINEPROC ) platGetProcAddress( "glBindProgramPipeline" ) ) != 0;
			r &= ( glDeleteProgramPipelines = ( PFNGLDELETEPROGRAMPIPELINESPROC ) platGetProcAddress( "glDeleteProgramPipelines" ) ) != 0;
			r &= ( glGenProgramPipelines = ( PFNGLGENPROGRAMPIPELINESPROC ) platGetProcAddress( "glGenProgramPipelines" ) ) != 0;
			r &= ( glIsProgramPipeline = ( PFNGLISPROGRAMPIPELINEPROC ) platGetProcAddress( "glIsProgramPipeline" ) ) != 0;
			r &= ( glGetProgramPipelineiv = ( PFNGLGETPROGRAMPIPELINEIVPROC ) platGetProcAddress( "glGetProgramPipelineiv" ) ) != 0;
			r &= ( glProgramUniform1i = ( PFNGLPROGRAMUNIFORM1IPROC ) platGetProcAddress( "glProgramUniform1i" ) ) != 0;
			r &= ( glProgramUniform1iv = ( PFNGLPROGRAMUNIFORM1IVPROC ) platGetProcAddress( "glProgramUniform1iv" ) ) != 0;
			r &= ( glProgramUniform1f = ( PFNGLPROGRAMUNIFORM1FPROC ) platGetProcAddress( "glProgramUniform1f" ) ) != 0;
			r &= ( glProgramUniform1fv = ( PFNGLPROGRAMUNIFORM1FVPROC ) platGetProcAddress( "glProgramUniform1fv" ) ) != 0;
			r &= ( glProgramUniform1d = ( PFNGLPROGRAMUNIFORM1DPROC ) platGetProcAddress( "glProgramUniform1d" ) ) != 0;
			r &= ( glProgramUniform1dv = ( PFNGLPROGRAMUNIFORM1DVPROC ) platGetProcAddress( "glProgramUniform1dv" ) ) != 0;
			r &= ( glProgramUniform1ui = ( PFNGLPROGRAMUNIFORM1UIPROC ) platGetProcAddress( "glProgramUniform1ui" ) ) != 0;
			r &= ( glProgramUniform1uiv = ( PFNGLPROGRAMUNIFORM1UIVPROC ) platGetProcAddress( "glProgramUniform1uiv" ) ) != 0;
			r &= ( glProgramUniform2i = ( PFNGLPROGRAMUNIFORM2IPROC ) platGetProcAddress( "glProgramUniform2i" ) ) != 0;
			r &= ( glProgramUniform2iv = ( PFNGLPROGRAMUNIFORM2IVPROC ) platGetProcAddress( "glProgramUniform2iv" ) ) != 0;
			r &= ( glProgramUniform2f = ( PFNGLPROGRAMUNIFORM2FPROC ) platGetProcAddress( "glProgramUniform2f" ) ) != 0;
			r &= ( glProgramUniform2fv = ( PFNGLPROGRAMUNIFORM2FVPROC ) platGetProcAddress( "glProgramUniform2fv" ) ) != 0;
			r &= ( glProgramUniform2d = ( PFNGLPROGRAMUNIFORM2DPROC ) platGetProcAddress( "glProgramUniform2d" ) ) != 0;
			r &= ( glProgramUniform2dv = ( PFNGLPROGRAMUNIFORM2DVPROC ) platGetProcAddress( "glProgramUniform2dv" ) ) != 0;
			r &= ( glProgramUniform2ui = ( PFNGLPROGRAMUNIFORM2UIPROC ) platGetProcAddress( "glProgramUniform2ui" ) ) != 0;
			r &= ( glProgramUniform2uiv = ( PFNGLPROGRAMUNIFORM2UIVPROC ) platGetProcAddress( "glProgramUniform2uiv" ) ) != 0;
			r &= ( glProgramUniform3i = ( PFNGLPROGRAMUNIFORM3IPROC ) platGetProcAddress( "glProgramUniform3i" ) ) != 0;
			r &= ( glProgramUniform3iv = ( PFNGLPROGRAMUNIFORM3IVPROC ) platGetProcAddress( "glProgramUniform3iv" ) ) != 0;
			r &= ( glProgramUniform3f = ( PFNGLPROGRAMUNIFORM3FPROC ) platGetProcAddress( "glProgramUniform3f" ) ) != 0;
			r &= ( glProgramUniform3fv = ( PFNGLPROGRAMUNIFORM3FVPROC ) platGetProcAddress( "glProgramUniform3fv" ) ) != 0;
			r &= ( glProgramUniform3d = ( PFNGLPROGRAMUNIFORM3DPROC ) platGetProcAddress( "glProgramUniform3d" ) ) != 0;
			r &= ( glProgramUniform3dv = ( PFNGLPROGRAMUNIFORM3DVPROC ) platGetProcAddress( "glProgramUniform3dv" ) ) != 0;
			r &= ( glProgramUniform3ui = ( PFNGLPROGRAMUNIFORM3UIPROC ) platGetProcAddress( "glProgramUniform3ui" ) ) != 0;
			r &= ( glProgramUniform3uiv = ( PFNGLPROGRAMUNIFORM3UIVPROC ) platGetProcAddress( "glProgramUniform3uiv" ) ) != 0;
			r &= ( glProgramUniform4i = ( PFNGLPROGRAMUNIFORM4IPROC ) platGetProcAddress( "glProgramUniform4i" ) ) != 0;
			r &= ( glProgramUniform4iv = ( PFNGLPROGRAMUNIFORM4IVPROC ) platGetProcAddress( "glProgramUniform4iv" ) ) != 0;
			r &= ( glProgramUniform4f = ( PFNGLPROGRAMUNIFORM4FPROC ) platGetProcAddress( "glProgramUniform4f" ) ) != 0;
			r &= ( glProgramUniform4fv = ( PFNGLPROGRAMUNIFORM4FVPROC ) platGetProcAddress( "glProgramUniform4fv" ) ) != 0;
			r &= ( glProgramUniform4d = ( PFNGLPROGRAMUNIFORM4DPROC ) platGetProcAddress( "glProgramUniform4d" ) ) != 0;
			r &= ( glProgramUniform4dv = ( PFNGLPROGRAMUNIFORM4DVPROC ) platGetProcAddress( "glProgramUniform4dv" ) ) != 0;
			r &= ( glProgramUniform4ui = ( PFNGLPROGRAMUNIFORM4UIPROC ) platGetProcAddress( "glProgramUniform4ui" ) ) != 0;
			r &= ( glProgramUniform4uiv = ( PFNGLPROGRAMUNIFORM4UIVPROC ) platGetProcAddress( "glProgramUniform4uiv" ) ) != 0;
			r &= ( glProgramUniformMatrix2fv = ( PFNGLPROGRAMUNIFORMMATRIX2FVPROC ) platGetProcAddress( "glProgramUniformMatrix2fv" ) ) != 0;
			r &= ( glProgramUniformMatrix3fv = ( PFNGLPROGRAMUNIFORMMATRIX3FVPROC ) platGetProcAddress( "glProgramUniformMatrix3fv" ) ) != 0;
			r &= ( glProgramUniformMatrix4fv = ( PFNGLPROGRAMUNIFORMMATRIX4FVPROC ) platGetProcAddress( "glProgramUniformMatrix4fv" ) ) != 0;
			r &= ( glProgramUniformMatrix2dv = ( PFNGLPROGRAMUNIFORMMATRIX2DVPROC ) platGetProcAddress( "glProgramUniformMatrix2dv" ) ) != 0;
			r &= ( glProgramUniformMatrix3dv = ( PFNGLPROGRAMUNIFORMMATRIX3DVPROC ) platGetProcAddress( "glProgramUniformMatrix3dv" ) ) != 0;
			r &= ( glProgramUniformMatrix4dv = ( PFNGLPROGRAMUNIFORMMATRIX4DVPROC ) platGetProcAddress( "glProgramUniformMatrix4dv" ) ) != 0;
			r &= ( glProgramUniformMatrix2x3fv = ( PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC ) platGetProcAddress( "glProgramUniformMatrix2x3fv" ) ) != 0;
			r &= ( glProgramUniformMatrix3x2fv = ( PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC ) platGetProcAddress( "glProgramUniformMatrix3x2fv" ) ) != 0;
			r &= ( glProgramUniformMatrix2x4fv = ( PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC ) platGetProcAddress( "glProgramUniformMatrix2x4fv" ) ) != 0;
			r &= ( glProgramUniformMatrix4x2fv = ( PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC ) platGetProcAddress( "glProgramUniformMatrix4x2fv" ) ) != 0;
			r &= ( glProgramUniformMatrix3x4fv = ( PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC ) platGetProcAddress( "glProgramUniformMatrix3x4fv" ) ) != 0;
			r &= ( glProgramUniformMatrix4x3fv = ( PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC ) platGetProcAddress( "glProgramUniformMatrix4x3fv" ) ) != 0;
			r &= ( glProgramUniformMatrix2x3dv = ( PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC ) platGetProcAddress( "glProgramUniformMatrix2x3dv" ) ) != 0;
			r &= ( glProgramUniformMatrix3x2dv = ( PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC ) platGetProcAddress( "glProgramUniformMatrix3x2dv" ) ) != 0;
			r &= ( glProgramUniformMatrix2x4dv = ( PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC ) platGetProcAddress( "glProgramUniformMatrix2x4dv" ) ) != 0;
			r &= ( glProgramUniformMatrix4x2dv = ( PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC ) platGetProcAddress( "glProgramUniformMatrix4x2dv" ) ) != 0;
			r &= ( glProgramUniformMatrix3x4dv = ( PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC ) platGetProcAddress( "glProgramUniformMatrix3x4dv" ) ) != 0;
			r &= ( glProgramUniformMatrix4x3dv = ( PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC ) platGetProcAddress( "glProgramUniformMatrix4x3dv" ) ) != 0;
			r &= ( glValidateProgramPipeline = ( PFNGLVALIDATEPROGRAMPIPELINEPROC ) platGetProcAddress( "glValidateProgramPipeline" ) ) != 0;
			r &= ( glGetProgramPipelineInfoLog = ( PFNGLGETPROGRAMPIPELINEINFOLOGPROC ) platGetProcAddress( "glGetProgramPipelineInfoLog" ) ) != 0;
			r &= ( glVertexAttribL1d = ( PFNGLVERTEXATTRIBL1DPROC ) platGetProcAddress( "glVertexAttribL1d" ) ) != 0;
			r &= ( glVertexAttribL2d = ( PFNGLVERTEXATTRIBL2DPROC ) platGetProcAddress( "glVertexAttribL2d" ) ) != 0;
			r &= ( glVertexAttribL3d = ( PFNGLVERTEXATTRIBL3DPROC ) platGetProcAddress( "glVertexAttribL3d" ) ) != 0;
			r &= ( glVertexAttribL4d = ( PFNGLVERTEXATTRIBL4DPROC ) platGetProcAddress( "glVertexAttribL4d" ) ) != 0;
			r &= ( glVertexAttribL1dv = ( PFNGLVERTEXATTRIBL1DVPROC ) platGetProcAddress( "glVertexAttribL1dv" ) ) != 0;
			r &= ( glVertexAttribL2dv = ( PFNGLVERTEXATTRIBL2DVPROC ) platGetProcAddress( "glVertexAttribL2dv" ) ) != 0;
			r &= ( glVertexAttribL3dv = ( PFNGLVERTEXATTRIBL3DVPROC ) platGetProcAddress( "glVertexAttribL3dv" ) ) != 0;
			r &= ( glVertexAttribL4dv = ( PFNGLVERTEXATTRIBL4DVPROC ) platGetProcAddress( "glVertexAttribL4dv" ) ) != 0;
			r &= ( glVertexAttribLPointer = ( PFNGLVERTEXATTRIBLPOINTERPROC ) platGetProcAddress( "glVertexAttribLPointer" ) ) != 0;
			r &= ( glGetVertexAttribLdv = ( PFNGLGETVERTEXATTRIBLDVPROC ) platGetProcAddress( "glGetVertexAttribLdv" ) ) != 0;
			r &= ( glViewportArrayv = ( PFNGLVIEWPORTARRAYVPROC ) platGetProcAddress( "glViewportArrayv" ) ) != 0;
			r &= ( glViewportIndexedf = ( PFNGLVIEWPORTINDEXEDFPROC ) platGetProcAddress( "glViewportIndexedf" ) ) != 0;
			r &= ( glViewportIndexedfv = ( PFNGLVIEWPORTINDEXEDFVPROC ) platGetProcAddress( "glViewportIndexedfv" ) ) != 0;
			r &= ( glScissorArrayv = ( PFNGLSCISSORARRAYVPROC ) platGetProcAddress( "glScissorArrayv" ) ) != 0;
			r &= ( glScissorIndexed = ( PFNGLSCISSORINDEXEDPROC ) platGetProcAddress( "glScissorIndexed" ) ) != 0;
			r &= ( glScissorIndexedv = ( PFNGLSCISSORINDEXEDVPROC ) platGetProcAddress( "glScissorIndexedv" ) ) != 0;
			r &= ( glDepthRangeArrayv = ( PFNGLDEPTHRANGEARRAYVPROC ) platGetProcAddress( "glDepthRangeArrayv" ) ) != 0;
			r &= ( glDepthRangeIndexed = ( PFNGLDEPTHRANGEINDEXEDPROC ) platGetProcAddress( "glDepthRangeIndexed" ) ) != 0;
			r &= ( glGetFloati_v = ( PFNGLGETFLOATI_VPROC ) platGetProcAddress( "glGetFloati_v" ) ) != 0;
			r &= ( glGetDoublei_v = ( PFNGLGETDOUBLEI_VPROC ) platGetProcAddress( "glGetDoublei_v" ) ) != 0;
		}

		if ( glExt::minorVersion >= 2 )
		{
			// GL 4.2
			r &= ( glDrawArraysInstancedBaseInstance = ( PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC ) platGetProcAddress( "glDrawArraysInstancedBaseInstance" ) ) != 0;
			r &= ( glDrawElementsInstancedBaseInstance = ( PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC ) platGetProcAddress( "glDrawElementsInstancedBaseInstance" ) ) != 0;
			r &= ( glDrawElementsInstancedBaseVertexBaseInstance = ( PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC ) platGetProcAddress( "glDrawElementsInstancedBaseVertexBaseInstance" ) ) != 0;
			r &= ( glGetInternalformativ = ( PFNGLGETINTERNALFORMATIVPROC ) platGetProcAddress( "glGetInternalformativ" ) ) != 0;
			r &= ( glGetActiveAtomicCounterBufferiv = ( PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC ) platGetProcAddress( "glGetActiveAtomicCounterBufferiv" ) ) != 0;
			r &= ( glBindImageTexture = ( PFNGLBINDIMAGETEXTUREPROC ) platGetProcAddress( "glBindImageTexture" ) ) != 0;
			r &= ( glMemoryBarrier = ( PFNGLMEMORYBARRIERPROC ) platGetProcAddress( "glMemoryBarrier" ) ) != 0;
			r &= ( glTexStorage1D = ( PFNGLTEXSTORAGE1DPROC ) platGetProcAddress( "glTexStorage1D" ) ) != 0;
			r &= ( glTexStorage2D = ( PFNGLTEXSTORAGE2DPROC ) platGetProcAddress( "glTexStorage2D" ) ) != 0;
			r &= ( glTexStorage3D = ( PFNGLTEXSTORAGE3DPROC ) platGetProcAddress( "glTexStorage3D" ) ) != 0;
			r &= ( glDrawTransformFeedbackInstanced = ( PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC ) platGetProcAddress( "glDrawTransformFeedbackInstanced" ) ) != 0;
			r &= ( glDrawTransformFeedbackStreamInstanced = ( PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC ) platGetProcAddress( "glDrawTransformFeedbackStreamInstanced" ) ) != 0;
		}

		if ( glExt::minorVersion >= 3 )
		{
			// GL 4.3
			r &= ( glClearBufferData = ( PFNGLCLEARBUFFERDATAPROC ) platGetProcAddress( "glClearBufferData" ) ) != 0;
			r &= ( glClearBufferSubData = ( PFNGLCLEARBUFFERSUBDATAPROC ) platGetProcAddress( "glClearBufferSubData" ) ) != 0;
			r &= ( glDispatchCompute = ( PFNGLDISPATCHCOMPUTEPROC ) platGetProcAddress( "glDispatchCompute" ) ) != 0;
			r &= ( glDispatchComputeIndirect = ( PFNGLDISPATCHCOMPUTEINDIRECTPROC ) platGetProcAddress( "glDispatchComputeIndirect" ) ) != 0;
			r &= ( glCopyImageSubData = ( PFNGLCOPYIMAGESUBDATAPROC ) platGetProcAddress( "glCopyImageSubData" ) ) != 0;
			r &= ( glFramebufferParameteri = ( PFNGLFRAMEBUFFERPARAMETERIPROC ) platGetProcAddress( "glFramebufferParameteri" ) ) != 0;
			r &= ( glGetFramebufferParameteriv = ( PFNGLGETFRAMEBUFFERPARAMETERIVPROC ) platGetProcAddress( "glGetFramebufferParameteriv" ) ) != 0;
			r &= ( glGetInternalformati64v = ( PFNGLGETINTERNALFORMATI64VPROC ) platGetProcAddress( "glGetInternalformati64v" ) ) != 0;
			r &= ( glInvalidateTexSubImage = ( PFNGLINVALIDATETEXSUBIMAGEPROC ) platGetProcAddress( "glInvalidateTexSubImage" ) ) != 0;
			r &= ( glInvalidateTexImage = ( PFNGLINVALIDATETEXIMAGEPROC ) platGetProcAddress( "glInvalidateTexImage" ) ) != 0;
			r &= ( glInvalidateBufferSubData = ( PFNGLINVALIDATEBUFFERSUBDATAPROC ) platGetProcAddress( "glInvalidateBufferSubData" ) ) != 0;
			r &= ( glInvalidateBufferData = ( PFNGLINVALIDATEBUFFERDATAPROC ) platGetProcAddress( "glInvalidateBufferData" ) ) != 0;
			r &= ( glInvalidateFramebuffer = ( PFNGLINVALIDATEFRAMEBUFFERPROC ) platGetProcAddress( "glInvalidateFramebuffer" ) ) != 0;
			r &= ( glInvalidateSubFramebuffer = ( PFNGLINVALIDATESUBFRAMEBUFFERPROC ) platGetProcAddress( "glInvalidateSubFramebuffer" ) ) != 0;
			r &= ( glMultiDrawArraysIndirect = ( PFNGLMULTIDRAWARRAYSINDIRECTPROC ) platGetProcAddress( "glMultiDrawArraysIndirect" ) ) != 0;
			r &= ( glMultiDrawElementsIndirect = ( PFNGLMULTIDRAWELEMENTSINDIRECTPROC ) platGetProcAddress( "glMultiDrawElementsIndirect" ) ) != 0;
			r &= ( glGetProgramInterfaceiv = ( PFNGLGETPROGRAMINTERFACEIVPROC ) platGetProcAddress( "glGetProgramInterfaceiv" ) ) != 0;
			r &= ( glGetProgramResourceIndex = ( PFNGLGETPROGRAMRESOURCEINDEXPROC ) platGetProcAddress( "glGetProgramResourceIndex" ) ) != 0;
			r &= ( glGetProgramResourceName = ( PFNGLGETPROGRAMRESOURCENAMEPROC ) platGetProcAddress( "glGetProgramResourceName" ) ) != 0;
			r &= ( glGetProgramResourceiv = ( PFNGLGETPROGRAMRESOURCEIVPROC ) platGetProcAddress( "glGetProgramResourceiv" ) ) != 0;
			r &= ( glGetProgramResourceLocation = ( PFNGLGETPROGRAMRESOURCELOCATIONPROC ) platGetProcAddress( "glGetProgramResourceLocation" ) ) != 0;
			r &= ( glGetProgramResourceLocationIndex = ( PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC ) platGetProcAddress( "glGetProgramResourceLocationIndex" ) ) != 0;
			r &= ( glShaderStorageBlockBinding = ( PFNGLSHADERSTORAGEBLOCKBINDINGPROC ) platGetProcAddress( "glShaderStorageBlockBinding" ) ) != 0;
			r &= ( glTexBufferRange = ( PFNGLTEXBUFFERRANGEPROC ) platGetProcAddress( "glTexBufferRange" ) ) != 0;
			r &= ( glTexStorage2DMultisample = ( PFNGLTEXSTORAGE2DMULTISAMPLEPROC ) platGetProcAddress( "glTexStorage2DMultisample" ) ) != 0;
			r &= ( glTexStorage3DMultisample = ( PFNGLTEXSTORAGE3DMULTISAMPLEPROC ) platGetProcAddress( "glTexStorage3DMultisample" ) ) != 0;
			r &= ( glTextureView = ( PFNGLTEXTUREVIEWPROC ) platGetProcAddress( "glTextureView" ) ) != 0;
			r &= ( glBindVertexBuffer = ( PFNGLBINDVERTEXBUFFERPROC ) platGetProcAddress( "glBindVertexBuffer" ) ) != 0;
			r &= ( glVertexAttribFormat = ( PFNGLVERTEXATTRIBFORMATPROC ) platGetProcAddress( "glVertexAttribFormat" ) ) != 0;
			r &= ( glVertexAttribIFormat = ( PFNGLVERTEXATTRIBIFORMATPROC ) platGetProcAddress( "glVertexAttribIFormat" ) ) != 0;
			r &= ( glVertexAttribLFormat = ( PFNGLVERTEXATTRIBLFORMATPROC ) platGetProcAddress( "glVertexAttribLFormat" ) ) != 0;
			r &= ( glVertexAttribBinding = ( PFNGLVERTEXATTRIBBINDINGPROC ) platGetProcAddress( "glVertexAttribBinding" ) ) != 0;
			r &= ( glVertexBindingDivisor = ( PFNGLVERTEXBINDINGDIVISORPROC ) platGetProcAddress( "glVertexBindingDivisor" ) ) != 0;
			r &= ( glDebugMessageControl = ( PFNGLDEBUGMESSAGECONTROLPROC ) platGetProcAddress( "glDebugMessageControl" ) ) != 0;
			r &= ( glDebugMessageInsert = ( PFNGLDEBUGMESSAGEINSERTPROC ) platGetProcAddress( "glDebugMessageInsert" ) ) != 0;
//			r &= ( glDebugMessageCallback = ( PFNGLDEBUGMESSAGECALLBACKPROC ) platGetProcAddress( "glDebugMessageCallback" ) ) != 0;
			r &= ( glGetDebugMessageLog = ( PFNGLGETDEBUGMESSAGELOGPROC ) platGetProcAddress( "glGetDebugMessageLog" ) ) != 0;
			r &= ( glPushDebugGroup = ( PFNGLPUSHDEBUGGROUPPROC ) platGetProcAddress( "glPushDebugGroup" ) ) != 0;
			r &= ( glPopDebugGroup = ( PFNGLPOPDEBUGGROUPPROC ) platGetProcAddress( "glPopDebugGroup" ) ) != 0;
			r &= ( glObjectLabel = ( PFNGLOBJECTLABELPROC ) platGetProcAddress( "glObjectLabel" ) ) != 0;
			r &= ( glGetObjectLabel = ( PFNGLGETOBJECTLABELPROC ) platGetProcAddress( "glGetObjectLabel" ) ) != 0;
			r &= ( glObjectPtrLabel = ( PFNGLOBJECTPTRLABELPROC ) platGetProcAddress( "glObjectPtrLabel" ) ) != 0;
			r &= ( glGetObjectPtrLabel = ( PFNGLGETOBJECTPTRLABELPROC ) platGetProcAddress( "glGetObjectPtrLabel" ) ) != 0;
		}

		if ( glExt::minorVersion >= 4 )
		{
			// GL 4.4
			r &= ( glBufferStorage = ( PFNGLBUFFERSTORAGEPROC ) platGetProcAddress( "glBufferStorage" ) ) != 0;
			r &= ( glClearTexImage = ( PFNGLCLEARTEXIMAGEPROC ) platGetProcAddress( "glClearTexImage" ) ) != 0;
			r &= ( glClearTexSubImage = ( PFNGLCLEARTEXSUBIMAGEPROC ) platGetProcAddress( "glClearTexSubImage" ) ) != 0;
			r &= ( glBindBuffersBase = ( PFNGLBINDBUFFERSBASEPROC ) platGetProcAddress( "glBindBuffersBase" ) ) != 0;
			r &= ( glBindBuffersRange = ( PFNGLBINDBUFFERSRANGEPROC ) platGetProcAddress( "glBindBuffersRange" ) ) != 0;
			r &= ( glBindTextures = ( PFNGLBINDTEXTURESPROC ) platGetProcAddress( "glBindTextures" ) ) != 0;
			r &= ( glBindSamplers = ( PFNGLBINDSAMPLERSPROC ) platGetProcAddress( "glBindSamplers" ) ) != 0;
			r &= ( glBindImageTextures = ( PFNGLBINDIMAGETEXTURESPROC ) platGetProcAddress( "glBindImageTextures" ) ) != 0;
			r &= ( glBindVertexBuffers = ( PFNGLBINDVERTEXBUFFERSPROC ) platGetProcAddress( "glBindVertexBuffers" ) ) != 0;
		}
	}

	// Extensions
 	if ( glExt::majorVersion < 3 )
		initLegacyExtensions( r );
	else
		initModernExtensions( r );

	glExt::OES_EGL_image = isExtensionSupported( "GL_OES_EGL_image" );
	if( glExt::OES_EGL_image )
	{
		r &= ( glEGLImageTargetTexture2DOES = ( PFNGLEGLIMAGETARGETTEXTURE2DOESPROC ) platGetProcAddress( "glEGLImageTargetTexture2DOES" ) ) != 0x0;
	}

	// Default extensions, suitable for any OpenGL version
	glExt::EXT_texture_filter_anisotropic = isExtensionSupported( "GL_EXT_texture_filter_anisotropic" );

	glExt::EXT_texture_compression_s3tc = isExtensionSupported( "GL_EXT_texture_compression_s3tc" ) || isExtensionSupported( "GL_S3_s3tc" );

	glExt::EXT_texture_sRGB = isExtensionSupported( "GL_EXT_texture_sRGB" );

	return r;
}

