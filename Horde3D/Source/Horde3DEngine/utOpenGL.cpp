*************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2011 Nicolas Schulz
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

	int	majorVersion = 1, minorVersion = 0;
}


namespace h3dGL
{
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
PFNGLCOLORMASKIPROC glColorMaski = nullptr;
PFNGLGETBOOLEANI_VPROC glGetBooleani_v = nullptr;
PFNGLGETINTEGERI_VPROC glGetIntegeri_v = nullptr;
PFNGLENABLEIPROC glEnablei = nullptr;
PFNGLDISABLEIPROC glDisablei = nullptr;
PFNGLISENABLEDIPROC glIsEnabledi = nullptr;
PFNGLBEGINTRANSFORMFEEDBACKPROC glBeginTransformFeedback = nullptr;
PFNGLENDTRANSFORMFEEDBACKPROC glEndTransformFeedback = nullptr;
PFNGLBINDBUFFERRANGEPROC glBindBufferRange = nullptr;
PFNGLBINDBUFFERBASEPROC glBindBufferBase = nullptr;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC glTransformFeedbackVaryings = nullptr;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC glGetTransformFeedbackVarying = nullptr;
PFNGLCLAMPCOLORPROC glClampColor = nullptr;
PFNGLBEGINCONDITIONALRENDERPROC glBeginConditionalRender = nullptr;
PFNGLENDCONDITIONALRENDERPROC glEndConditionalRender = nullptr;
PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer = nullptr;
PFNGLGETVERTEXATTRIBIIVPROC glGetVertexAttribIiv = nullptr;
PFNGLGETVERTEXATTRIBIUIVPROC glGetVertexAttribIuiv = nullptr;
PFNGLVERTEXATTRIBI1IPROC glVertexAttribI1i = nullptr;
PFNGLVERTEXATTRIBI2IPROC glVertexAttribI2i = nullptr;
PFNGLVERTEXATTRIBI3IPROC glVertexAttribI3i = nullptr;
PFNGLVERTEXATTRIBI4IPROC glVertexAttribI4i = nullptr;
PFNGLVERTEXATTRIBI1UIPROC glVertexAttribI1ui = nullptr;
PFNGLVERTEXATTRIBI2UIPROC glVertexAttribI2ui = nullptr;
PFNGLVERTEXATTRIBI3UIPROC glVertexAttribI3ui = nullptr;
PFNGLVERTEXATTRIBI4UIPROC glVertexAttribI4ui = nullptr;
PFNGLVERTEXATTRIBI1IVPROC glVertexAttribI1iv = nullptr;
PFNGLVERTEXATTRIBI2IVPROC glVertexAttribI2iv = nullptr;
PFNGLVERTEXATTRIBI3IVPROC glVertexAttribI3iv = nullptr;
PFNGLVERTEXATTRIBI4IVPROC glVertexAttribI4iv = nullptr;
PFNGLVERTEXATTRIBI1UIVPROC glVertexAttribI1uiv = nullptr;
PFNGLVERTEXATTRIBI2UIVPROC glVertexAttribI2uiv = nullptr;
PFNGLVERTEXATTRIBI3UIVPROC glVertexAttribI3uiv = nullptr;
PFNGLVERTEXATTRIBI4UIVPROC glVertexAttribI4uiv = nullptr;
PFNGLVERTEXATTRIBI4BVPROC glVertexAttribI4bv = nullptr;
PFNGLVERTEXATTRIBI4SVPROC glVertexAttribI4sv = nullptr;
PFNGLVERTEXATTRIBI4UBVPROC glVertexAttribI4ubv = nullptr;
PFNGLVERTEXATTRIBI4USVPROC glVertexAttribI4usv = nullptr;
PFNGLGETUNIFORMUIVPROC glGetUniformuiv = nullptr;
PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation = nullptr;
PFNGLGETFRAGDATALOCATIONPROC glGetFragDataLocation = nullptr;
PFNGLUNIFORM1UIPROC glUniform1ui = nullptr;
PFNGLUNIFORM2UIPROC glUniform2ui = nullptr;
PFNGLUNIFORM3UIPROC glUniform3ui = nullptr;
PFNGLUNIFORM4UIPROC glUniform4ui = nullptr;
PFNGLUNIFORM1UIVPROC glUniform1uiv = nullptr;
PFNGLUNIFORM2UIVPROC glUniform2uiv = nullptr;
PFNGLUNIFORM3UIVPROC glUniform3uiv = nullptr;
PFNGLUNIFORM4UIVPROC glUniform4uiv = nullptr;
PFNGLTEXPARAMETERIIVPROC glTexParameterIiv = nullptr;
PFNGLTEXPARAMETERIUIVPROC glTexParameterIuiv = nullptr;
PFNGLGETTEXPARAMETERIIVPROC glGetTexParameterIiv = nullptr;
PFNGLGETTEXPARAMETERIUIVPROC glGetTexParameterIuiv = nullptr;
PFNGLCLEARBUFFERIVPROC glClearBufferiv = nullptr;
PFNGLCLEARBUFFERUIVPROC glClearBufferuiv = nullptr;
PFNGLCLEARBUFFERFVPROC glClearBufferfv = nullptr;
PFNGLCLEARBUFFERFIPROC glClearBufferfi = nullptr;
PFNGLISRENDERBUFFERPROC glIsRenderbuffer = nullptr;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = nullptr;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = nullptr;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = nullptr;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = nullptr;
PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameteriv = nullptr;
PFNGLISFRAMEBUFFERPROC glIsFramebuffer = nullptr;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = nullptr;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = nullptr;
PFNGLFRAMEBUFFERTEXTURE1DPROC glFramebufferTexture1D = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = nullptr;
PFNGLFRAMEBUFFERTEXTURE3DPROC glFramebufferTexture3D = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = nullptr;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv = nullptr;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = nullptr;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer = nullptr;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample = nullptr;
PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer = nullptr;
PFNGLMAPBUFFERRANGEPROC glMapBufferRange = nullptr;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRange = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
PFNGLISVERTEXARRAYPROC glIsVertexArray = nullptr;

// GL 3.1
PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced = nullptr;
PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced = nullptr;
PFNGLTEXBUFFERPROC glTexBuffer = nullptr;
PFNGLPRIMITIVERESTARTINDEXPROC glPrimitiveRestartIndex = nullptr;
PFNGLCOPYBUFFERSUBDATAPROC glCopyBufferSubData = nullptr;
PFNGLGETUNIFORMINDICESPROC glGetUniformIndices = nullptr;
PFNGLGETACTIVEUNIFORMSIVPROC glGetActiveUniformsiv = nullptr;
PFNGLGETACTIVEUNIFORMNAMEPROC glGetActiveUniformName = nullptr;
PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex = nullptr;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv = nullptr;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glGetActiveUniformBlockName = nullptr;
PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding = nullptr;

// GL 3.2
PFNGLDRAWELEMENTSBASEVERTEXPROC glDrawElementsBaseVertex = nullptr;
PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC glDrawRangeElementsBaseVertex = nullptr;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC glDrawElementsInstancedBaseVertex = nullptr;
PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC glMultiDrawElementsBaseVertex = nullptr;
PFNGLPROVOKINGVERTEXPROC glProvokingVertex = nullptr;
PFNGLFENCESYNCPROC glFenceSync = nullptr;
PFNGLISSYNCPROC glIsSync = nullptr;
PFNGLDELETESYNCPROC glDeleteSync = nullptr;
PFNGLCLIENTWAITSYNCPROC glClientWaitSync = nullptr;
PFNGLWAITSYNCPROC glWaitSync = nullptr;
PFNGLGETINTEGER64VPROC glGetInteger64v = nullptr;
PFNGLGETSYNCIVPROC glGetSynciv = nullptr;
PFNGLGETINTEGER64I_VPROC glGetInteger64i_v = nullptr;
PFNGLGETBUFFERPARAMETERI64VPROC glGetBufferParameteri64v = nullptr;
PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture = nullptr;
PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample = nullptr;
PFNGLTEXIMAGE3DMULTISAMPLEPROC glTexImage3DMultisample = nullptr;
PFNGLGETMULTISAMPLEFVPROC glGetMultisamplefv = nullptr;
PFNGLSAMPLEMASKIPROC glSampleMaski = nullptr;

// GL 3.3
PFNGLBINDFRAGDATALOCATIONINDEXEDPROC glBindFragDataLocationIndexed = nullptr;
PFNGLGETFRAGDATAINDEXPROC glGetFragDataIndex = nullptr;
PFNGLGENSAMPLERSPROC glGenSamplers = nullptr;
PFNGLDELETESAMPLERSPROC glDeleteSamplers = nullptr;
PFNGLISSAMPLERPROC glIsSampler = nullptr;
PFNGLBINDSAMPLERPROC glBindSampler = nullptr;
PFNGLSAMPLERPARAMETERIPROC glSamplerParameteri = nullptr;
PFNGLSAMPLERPARAMETERIVPROC glSamplerParameteriv = nullptr;
PFNGLSAMPLERPARAMETERFPROC glSamplerParameterf = nullptr;
PFNGLSAMPLERPARAMETERFVPROC glSamplerParameterfv = nullptr;
PFNGLSAMPLERPARAMETERIIVPROC glSamplerParameterIiv = nullptr;
PFNGLSAMPLERPARAMETERIUIVPROC glSamplerParameterIuiv = nullptr;
PFNGLGETSAMPLERPARAMETERIVPROC glGetSamplerParameteriv = nullptr;
PFNGLGETSAMPLERPARAMETERIIVPROC glGetSamplerParameterIiv = nullptr;
PFNGLGETSAMPLERPARAMETERFVPROC glGetSamplerParameterfv = nullptr;
PFNGLGETSAMPLERPARAMETERIUIVPROC glGetSamplerParameterIuiv = nullptr;
PFNGLQUERYCOUNTERPROC glQueryCounter = nullptr;
PFNGLGETQUERYOBJECTI64VPROC glGetQueryObjecti64v = nullptr;
PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64v = nullptr;
PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor = nullptr;
PFNGLVERTEXATTRIBP1UIPROC glVertexAttribP1ui = nullptr;
PFNGLVERTEXATTRIBP1UIVPROC glVertexAttribP1uiv = nullptr;
PFNGLVERTEXATTRIBP2UIPROC glVertexAttribP2ui = nullptr;
PFNGLVERTEXATTRIBP2UIVPROC glVertexAttribP2uiv = nullptr;
PFNGLVERTEXATTRIBP3UIPROC glVertexAttribP3ui = nullptr;
PFNGLVERTEXATTRIBP3UIVPROC glVertexAttribP3uiv = nullptr;
PFNGLVERTEXATTRIBP4UIPROC glVertexAttribP4ui = nullptr;
PFNGLVERTEXATTRIBP4UIVPROC glVertexAttribP4uiv = nullptr;

// GL 4.0
PFNGLMINSAMPLESHADINGPROC glMinSampleShading = nullptr;
PFNGLBLENDEQUATIONIPROC glBlendEquationi = nullptr;
PFNGLBLENDEQUATIONSEPARATEIPROC glBlendEquationSeparatei = nullptr;
PFNGLBLENDFUNCIPROC glBlendFunci = nullptr;
PFNGLBLENDFUNCSEPARATEIPROC glBlendFuncSeparatei = nullptr;
PFNGLDRAWARRAYSINDIRECTPROC glDrawArraysIndirect = nullptr;
PFNGLDRAWELEMENTSINDIRECTPROC glDrawElementsIndirect = nullptr;
PFNGLUNIFORM1DPROC glUniform1d = nullptr;
PFNGLUNIFORM2DPROC glUniform2d = nullptr;
PFNGLUNIFORM3DPROC glUniform3d = nullptr;
PFNGLUNIFORM4DPROC glUniform4d = nullptr;
PFNGLUNIFORM1DVPROC glUniform1dv = nullptr;
PFNGLUNIFORM2DVPROC glUniform2dv = nullptr;
PFNGLUNIFORM3DVPROC glUniform3dv = nullptr;
PFNGLUNIFORM4DVPROC glUniform4dv = nullptr;
PFNGLUNIFORMMATRIX2DVPROC glUniformMatrix2dv = nullptr;
PFNGLUNIFORMMATRIX3DVPROC glUniformMatrix3dv = nullptr;
PFNGLUNIFORMMATRIX4DVPROC glUniformMatrix4dv = nullptr;
PFNGLUNIFORMMATRIX2X3DVPROC glUniformMatrix2x3dv = nullptr;
PFNGLUNIFORMMATRIX2X4DVPROC glUniformMatrix2x4dv = nullptr;
PFNGLUNIFORMMATRIX3X2DVPROC glUniformMatrix3x2dv = nullptr;
PFNGLUNIFORMMATRIX3X4DVPROC glUniformMatrix3x4dv = nullptr;
PFNGLUNIFORMMATRIX4X2DVPROC glUniformMatrix4x2dv = nullptr;
PFNGLUNIFORMMATRIX4X3DVPROC glUniformMatrix4x3dv = nullptr;
PFNGLGETUNIFORMDVPROC glGetUniformdv = nullptr;
PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC glGetSubroutineUniformLocation = nullptr;
PFNGLGETSUBROUTINEINDEXPROC glGetSubroutineIndex = nullptr;
PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC glGetActiveSubroutineUniformiv = nullptr;
PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC glGetActiveSubroutineUniformName = nullptr;
PFNGLGETACTIVESUBROUTINENAMEPROC glGetActiveSubroutineName = nullptr;
PFNGLUNIFORMSUBROUTINESUIVPROC glUniformSubroutinesuiv = nullptr;
PFNGLGETUNIFORMSUBROUTINEUIVPROC glGetUniformSubroutineuiv = nullptr;
PFNGLGETPROGRAMSTAGEIVPROC glGetProgramStageiv = nullptr;
PFNGLPATCHPARAMETERIPROC glPatchParameteri = nullptr;
PFNGLPATCHPARAMETERFVPROC glPatchParameterfv = nullptr;
PFNGLBINDTRANSFORMFEEDBACKPROC glBindTransformFeedback = nullptr;
PFNGLDELETETRANSFORMFEEDBACKSPROC glDeleteTransformFeedbacks = nullptr;
PFNGLGENTRANSFORMFEEDBACKSPROC glGenTransformFeedbacks = nullptr;
PFNGLISTRANSFORMFEEDBACKPROC glIsTransformFeedback = nullptr;
PFNGLPAUSETRANSFORMFEEDBACKPROC glPauseTransformFeedback = nullptr;
PFNGLRESUMETRANSFORMFEEDBACKPROC glResumeTransformFeedback = nullptr;
PFNGLDRAWTRANSFORMFEEDBACKPROC glDrawTransformFeedback = nullptr;
PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC glDrawTransformFeedbackStream = nullptr;
PFNGLBEGINQUERYINDEXEDPROC glBeginQueryIndexed = nullptr;
PFNGLENDQUERYINDEXEDPROC glEndQueryIndexed = nullptr;
PFNGLGETQUERYINDEXEDIVPROC glGetQueryIndexediv = nullptr;

// GL 4.1
PFNGLRELEASESHADERCOMPILERPROC glReleaseShaderCompiler = nullptr;
PFNGLSHADERBINARYPROC glShaderBinary = nullptr;
PFNGLGETSHADERPRECISIONFORMATPROC glGetShaderPrecisionFormat = nullptr;
PFNGLDEPTHRANGEFPROC glDepthRangef = nullptr;
PFNGLCLEARDEPTHFPROC glClearDepthf = nullptr;
PFNGLGETPROGRAMBINARYPROC glGetProgramBinary = nullptr;
PFNGLPROGRAMBINARYPROC glProgramBinary = nullptr;
PFNGLPROGRAMPARAMETERIPROC glProgramParameteri = nullptr;
PFNGLUSEPROGRAMSTAGESPROC glUseProgramStages = nullptr;
PFNGLACTIVESHADERPROGRAMPROC glActiveShaderProgram = nullptr;
PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv = nullptr;
PFNGLBINDPROGRAMPIPELINEPROC glBindProgramPipeline = nullptr;
PFNGLDELETEPROGRAMPIPELINESPROC glDeleteProgramPipelines = nullptr;
PFNGLGENPROGRAMPIPELINESPROC glGenProgramPipelines = nullptr;
PFNGLISPROGRAMPIPELINEPROC glIsProgramPipeline = nullptr;
PFNGLGETPROGRAMPIPELINEIVPROC glGetProgramPipelineiv = nullptr;
PFNGLPROGRAMUNIFORM1IPROC glProgramUniform1i = nullptr;
PFNGLPROGRAMUNIFORM1IVPROC glProgramUniform1iv = nullptr;
PFNGLPROGRAMUNIFORM1FPROC glProgramUniform1f = nullptr;
PFNGLPROGRAMUNIFORM1FVPROC glProgramUniform1fv = nullptr;
PFNGLPROGRAMUNIFORM1DPROC glProgramUniform1d = nullptr;
PFNGLPROGRAMUNIFORM1DVPROC glProgramUniform1dv = nullptr;
PFNGLPROGRAMUNIFORM1UIPROC glProgramUniform1ui = nullptr;
PFNGLPROGRAMUNIFORM1UIVPROC glProgramUniform1uiv = nullptr;
PFNGLPROGRAMUNIFORM2IPROC glProgramUniform2i = nullptr;
PFNGLPROGRAMUNIFORM2IVPROC glProgramUniform2iv = nullptr;
PFNGLPROGRAMUNIFORM2FPROC glProgramUniform2f = nullptr;
PFNGLPROGRAMUNIFORM2FVPROC glProgramUniform2fv = nullptr;
PFNGLPROGRAMUNIFORM2DPROC glProgramUniform2d = nullptr;
PFNGLPROGRAMUNIFORM2DVPROC glProgramUniform2dv = nullptr;
PFNGLPROGRAMUNIFORM2UIPROC glProgramUniform2ui = nullptr;
PFNGLPROGRAMUNIFORM2UIVPROC glProgramUniform2uiv = nullptr;
PFNGLPROGRAMUNIFORM3IPROC glProgramUniform3i = nullptr;
PFNGLPROGRAMUNIFORM3IVPROC glProgramUniform3iv = nullptr;
PFNGLPROGRAMUNIFORM3FPROC glProgramUniform3f = nullptr;
PFNGLPROGRAMUNIFORM3FVPROC glProgramUniform3fv = nullptr;
PFNGLPROGRAMUNIFORM3DPROC glProgramUniform3d = nullptr;
PFNGLPROGRAMUNIFORM3DVPROC glProgramUniform3dv = nullptr;
PFNGLPROGRAMUNIFORM3UIPROC glProgramUniform3ui = nullptr;
PFNGLPROGRAMUNIFORM3UIVPROC glProgramUniform3uiv = nullptr;
PFNGLPROGRAMUNIFORM4IPROC glProgramUniform4i = nullptr;
PFNGLPROGRAMUNIFORM4IVPROC glProgramUniform4iv = nullptr;
PFNGLPROGRAMUNIFORM4FPROC glProgramUniform4f = nullptr;
PFNGLPROGRAMUNIFORM4FVPROC glProgramUniform4fv = nullptr;
PFNGLPROGRAMUNIFORM4DPROC glProgramUniform4d = nullptr;
PFNGLPROGRAMUNIFORM4DVPROC glProgramUniform4dv = nullptr;
PFNGLPROGRAMUNIFORM4UIPROC glProgramUniform4ui = nullptr;
PFNGLPROGRAMUNIFORM4UIVPROC glProgramUniform4uiv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2FVPROC glProgramUniformMatrix2fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3FVPROC glProgramUniformMatrix3fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4FVPROC glProgramUniformMatrix4fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2DVPROC glProgramUniformMatrix2dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3DVPROC glProgramUniformMatrix3dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4DVPROC glProgramUniformMatrix4dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC glProgramUniformMatrix2x3fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC glProgramUniformMatrix3x2fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC glProgramUniformMatrix2x4fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC glProgramUniformMatrix4x2fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC glProgramUniformMatrix3x4fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC glProgramUniformMatrix4x3fv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC glProgramUniformMatrix2x3dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC glProgramUniformMatrix3x2dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC glProgramUniformMatrix2x4dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC glProgramUniformMatrix4x2dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC glProgramUniformMatrix3x4dv = nullptr;
PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC glProgramUniformMatrix4x3dv = nullptr;
PFNGLVALIDATEPROGRAMPIPELINEPROC glValidateProgramPipeline = nullptr;
PFNGLGETPROGRAMPIPELINEINFOLOGPROC glGetProgramPipelineInfoLog = nullptr;
PFNGLVERTEXATTRIBL1DPROC glVertexAttribL1d = nullptr;
PFNGLVERTEXATTRIBL2DPROC glVertexAttribL2d = nullptr;
PFNGLVERTEXATTRIBL3DPROC glVertexAttribL3d = nullptr;
PFNGLVERTEXATTRIBL4DPROC glVertexAttribL4d = nullptr;
PFNGLVERTEXATTRIBL1DVPROC glVertexAttribL1dv = nullptr;
PFNGLVERTEXATTRIBL2DVPROC glVertexAttribL2dv = nullptr;
PFNGLVERTEXATTRIBL3DVPROC glVertexAttribL3dv = nullptr;
PFNGLVERTEXATTRIBL4DVPROC glVertexAttribL4dv = nullptr;
PFNGLVERTEXATTRIBLPOINTERPROC glVertexAttribLPointer = nullptr;
PFNGLGETVERTEXATTRIBLDVPROC glGetVertexAttribLdv = nullptr;
PFNGLVIEWPORTARRAYVPROC glViewportArrayv = nullptr;
PFNGLVIEWPORTINDEXEDFPROC glViewportIndexedf = nullptr;
PFNGLVIEWPORTINDEXEDFVPROC glViewportIndexedfv = nullptr;
PFNGLSCISSORARRAYVPROC glScissorArrayv = nullptr;
PFNGLSCISSORINDEXEDPROC glScissorIndexed = nullptr;
PFNGLSCISSORINDEXEDVPROC glScissorIndexedv = nullptr;
PFNGLDEPTHRANGEARRAYVPROC glDepthRangeArrayv = nullptr;
PFNGLDEPTHRANGEINDEXEDPROC glDepthRangeIndexed = nullptr;
PFNGLGETFLOATI_VPROC glGetFloati_v = nullptr;
PFNGLGETDOUBLEI_VPROC glGetDoublei_v = nullptr;

// GL 4.2
PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC glDrawArraysInstancedBaseInstance = nullptr;
PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC glDrawElementsInstancedBaseInstance = nullptr;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC glDrawElementsInstancedBaseVertexBaseInstance = nullptr;
PFNGLGETINTERNALFORMATIVPROC glGetInternalformativ = nullptr;
PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC glGetActiveAtomicCounterBufferiv = nullptr;
PFNGLBINDIMAGETEXTUREPROC glBindImageTexture = nullptr;
PFNGLMEMORYBARRIERPROC glMemoryBarrier = nullptr;
PFNGLTEXSTORAGE1DPROC glTexStorage1D = nullptr;
PFNGLTEXSTORAGE2DPROC glTexStorage2D = nullptr;
PFNGLTEXSTORAGE3DPROC glTexStorage3D = nullptr;
PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC glDrawTransformFeedbackInstanced = nullptr;
PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC glDrawTransformFeedbackStreamInstanced = nullptr;

// GL 4.3
PFNGLCLEARBUFFERDATAPROC glClearBufferData = nullptr;
PFNGLCLEARBUFFERSUBDATAPROC glClearBufferSubData = nullptr;
PFNGLDISPATCHCOMPUTEPROC glDispatchCompute = nullptr;
PFNGLDISPATCHCOMPUTEINDIRECTPROC glDispatchComputeIndirect = nullptr;
PFNGLCOPYIMAGESUBDATAPROC glCopyImageSubData = nullptr;
PFNGLFRAMEBUFFERPARAMETERIPROC glFramebufferParameteri = nullptr;
PFNGLGETFRAMEBUFFERPARAMETERIVPROC glGetFramebufferParameteriv = nullptr;
PFNGLGETINTERNALFORMATI64VPROC glGetInternalformati64v = nullptr;
PFNGLINVALIDATETEXSUBIMAGEPROC glInvalidateTexSubImage = nullptr;
PFNGLINVALIDATETEXIMAGEPROC glInvalidateTexImage = nullptr;
PFNGLINVALIDATEBUFFERSUBDATAPROC glInvalidateBufferSubData = nullptr;
PFNGLINVALIDATEBUFFERDATAPROC glInvalidateBufferData = nullptr;
PFNGLINVALIDATEFRAMEBUFFERPROC glInvalidateFramebuffer = nullptr;
PFNGLINVALIDATESUBFRAMEBUFFERPROC glInvalidateSubFramebuffer = nullptr;
PFNGLMULTIDRAWARRAYSINDIRECTPROC glMultiDrawArraysIndirect = nullptr;
PFNGLMULTIDRAWELEMENTSINDIRECTPROC glMultiDrawElementsIndirect = nullptr;
PFNGLGETPROGRAMINTERFACEIVPROC glGetProgramInterfaceiv = nullptr;
PFNGLGETPROGRAMRESOURCEINDEXPROC glGetProgramResourceIndex = nullptr;
PFNGLGETPROGRAMRESOURCENAMEPROC glGetProgramResourceName = nullptr;
PFNGLGETPROGRAMRESOURCEIVPROC glGetProgramResourceiv = nullptr;
PFNGLGETPROGRAMRESOURCELOCATIONPROC glGetProgramResourceLocation = nullptr;
PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC glGetProgramResourceLocationIndex = nullptr;
PFNGLSHADERSTORAGEBLOCKBINDINGPROC glShaderStorageBlockBinding = nullptr;
PFNGLTEXBUFFERRANGEPROC glTexBufferRange = nullptr;
PFNGLTEXSTORAGE2DMULTISAMPLEPROC glTexStorage2DMultisample = nullptr;
PFNGLTEXSTORAGE3DMULTISAMPLEPROC glTexStorage3DMultisample = nullptr;
PFNGLTEXTUREVIEWPROC glTextureView = nullptr;
PFNGLBINDVERTEXBUFFERPROC glBindVertexBuffer = nullptr;
PFNGLVERTEXATTRIBFORMATPROC glVertexAttribFormat = nullptr;
PFNGLVERTEXATTRIBIFORMATPROC glVertexAttribIFormat = nullptr;
PFNGLVERTEXATTRIBLFORMATPROC glVertexAttribLFormat = nullptr;
PFNGLVERTEXATTRIBBINDINGPROC glVertexAttribBinding = nullptr;
PFNGLVERTEXBINDINGDIVISORPROC glVertexBindingDivisor = nullptr;
PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl = nullptr;
PFNGLDEBUGMESSAGEINSERTPROC glDebugMessageInsert = nullptr;
//PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback = nullptr;
PFNGLGETDEBUGMESSAGELOGPROC glGetDebugMessageLog = nullptr;
PFNGLPUSHDEBUGGROUPPROC glPushDebugGroup = nullptr;
PFNGLPOPDEBUGGROUPPROC glPopDebugGroup = nullptr;
PFNGLOBJECTLABELPROC glObjectLabel = nullptr;
PFNGLGETOBJECTLABELPROC glGetObjectLabel = nullptr;
PFNGLOBJECTPTRLABELPROC glObjectPtrLabel = nullptr;
PFNGLGETOBJECTPTRLABELPROC glGetObjectPtrLabel = nullptr;

// GL 4.4
PFNGLBUFFERSTORAGEPROC glBufferStorage = nullptr;
PFNGLCLEARTEXIMAGEPROC glClearTexImage = nullptr;
PFNGLCLEARTEXSUBIMAGEPROC glClearTexSubImage = nullptr;
PFNGLBINDBUFFERSBASEPROC glBindBuffersBase = nullptr;
PFNGLBINDBUFFERSRANGEPROC glBindBuffersRange = nullptr;
PFNGLBINDTEXTURESPROC glBindTextures = nullptr;
PFNGLBINDSAMPLERSPROC glBindSamplers = nullptr;
PFNGLBINDIMAGETEXTURESPROC glBindImageTextures = nullptr;
PFNGLBINDVERTEXBUFFERSPROC glBindVertexBuffers = nullptr;

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
	return (void *)wglGetProcAddress( funcName );
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
#else
	return (void *)glXGetProcAddressARB( (const GLubyte *)funcName );
#endif
}

void InitLegacyExtensions( bool &r )
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
	
}

void InitModernExtensions( bool &r )
{
//	throw std::exception( "The method or operation is not implemented." );
}

bool initOpenGLExtensions()
{
	bool r = true;
	
	getOpenGLVersion();
	
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

	/*// GL 2.1
	r &= (glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC) platGetProcAddress( "glUniformMatrix2x3fv" )) != 0x0;
	r &= (glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC) platGetProcAddress( "glUniformMatrix3x2fv" )) != 0x0;
	r &= (glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC) platGetProcAddress( "glUniformMatrix2x4fv" )) != 0x0;
	r &= (glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC) platGetProcAddress( "glUniformMatrix4x2fv" )) != 0x0;
	r &= (glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC) platGetProcAddress( "glUniformMatrix3x4fv" )) != 0x0;
	r &= (glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC) platGetProcAddress( "glUniformMatrix4x3fv" )) != 0x0;*/

	// GL 3.0 
	if( glExt::majorVersion >= 3 )
	{
		r &= ( glGetStringi = (PFNGLGETSTRINGIPROC) platGetProcAddress( "glGetStringi" )) != 0x0;
		r &= ( glColorMaski = ( PFNGLCOLORMASKIPROC ) platGetProcAddress( "glColorMaski" ) ) != nullptr;
		r &= ( glGetBooleani_v = ( PFNGLGETBOOLEANI_VPROC ) platGetProcAddress( "glGetBooleani_v" ) ) != nullptr;
		r &= ( glGetIntegeri_v = ( PFNGLGETINTEGERI_VPROC ) platGetProcAddress( "glGetIntegeri_v" ) ) != nullptr;
		r &= ( glEnablei = ( PFNGLENABLEIPROC ) platGetProcAddress( "glEnablei" ) ) != nullptr;
		r &= ( glDisablei = ( PFNGLDISABLEIPROC ) platGetProcAddress( "glDisablei" ) ) != nullptr;
		r &= ( glIsEnabledi = ( PFNGLISENABLEDIPROC ) platGetProcAddress( "glIsEnabledi" ) ) != nullptr;
		r &= ( glBeginTransformFeedback = ( PFNGLBEGINTRANSFORMFEEDBACKPROC ) platGetProcAddress( "glBeginTransformFeedback" ) ) != nullptr;
		r &= ( glEndTransformFeedback = ( PFNGLENDTRANSFORMFEEDBACKPROC ) platGetProcAddress( "glEndTransformFeedback" ) ) != nullptr;
		r &= ( glBindBufferRange = ( PFNGLBINDBUFFERRANGEPROC ) platGetProcAddress( "glBindBufferRange" ) ) != nullptr;
		r &= ( glBindBufferBase = ( PFNGLBINDBUFFERBASEPROC ) platGetProcAddress( "glBindBufferBase" ) ) != nullptr;
		r &= ( glTransformFeedbackVaryings = ( PFNGLTRANSFORMFEEDBACKVARYINGSPROC ) platGetProcAddress( "glTransformFeedbackVaryings" ) ) != nullptr;
		r &= ( glGetTransformFeedbackVarying = ( PFNGLGETTRANSFORMFEEDBACKVARYINGPROC ) platGetProcAddress( "glGetTransformFeedbackVarying" ) ) != nullptr;
		r &= ( glClampColor = ( PFNGLCLAMPCOLORPROC ) platGetProcAddress( "glClampColor" ) ) != nullptr;
		r &= ( glBeginConditionalRender = ( PFNGLBEGINCONDITIONALRENDERPROC ) platGetProcAddress( "glBeginConditionalRender" ) ) != nullptr;
		r &= ( glEndConditionalRender = ( PFNGLENDCONDITIONALRENDERPROC ) platGetProcAddress( "glEndConditionalRender" ) ) != nullptr;
		r &= ( glVertexAttribIPointer = ( PFNGLVERTEXATTRIBIPOINTERPROC ) platGetProcAddress( "glVertexAttribIPointer" ) ) != nullptr;
		r &= ( glGetVertexAttribIiv = ( PFNGLGETVERTEXATTRIBIIVPROC ) platGetProcAddress( "glGetVertexAttribIiv" ) ) != nullptr;
		r &= ( glGetVertexAttribIuiv = ( PFNGLGETVERTEXATTRIBIUIVPROC ) platGetProcAddress( "glGetVertexAttribIuiv" ) ) != nullptr;
		r &= ( glVertexAttribI1i = ( PFNGLVERTEXATTRIBI1IPROC ) platGetProcAddress( "glVertexAttribI1i" ) ) != nullptr;
		r &= ( glVertexAttribI2i = ( PFNGLVERTEXATTRIBI2IPROC ) platGetProcAddress( "glVertexAttribI2i" ) ) != nullptr;
		r &= ( glVertexAttribI3i = ( PFNGLVERTEXATTRIBI3IPROC ) platGetProcAddress( "glVertexAttribI3i" ) ) != nullptr;
		r &= ( glVertexAttribI4i = ( PFNGLVERTEXATTRIBI4IPROC ) platGetProcAddress( "glVertexAttribI4i" ) ) != nullptr;
		r &= ( glVertexAttribI1ui = ( PFNGLVERTEXATTRIBI1UIPROC ) platGetProcAddress( "glVertexAttribI1ui" ) ) != nullptr;
		r &= ( glVertexAttribI2ui = ( PFNGLVERTEXATTRIBI2UIPROC ) platGetProcAddress( "glVertexAttribI2ui" ) ) != nullptr;
		r &= ( glVertexAttribI3ui = ( PFNGLVERTEXATTRIBI3UIPROC ) platGetProcAddress( "glVertexAttribI3ui" ) ) != nullptr;
		r &= ( glVertexAttribI4ui = ( PFNGLVERTEXATTRIBI4UIPROC ) platGetProcAddress( "glVertexAttribI4ui" ) ) != nullptr;
		r &= ( glVertexAttribI1iv = ( PFNGLVERTEXATTRIBI1IVPROC ) platGetProcAddress( "glVertexAttribI1iv" ) ) != nullptr;
		r &= ( glVertexAttribI2iv = ( PFNGLVERTEXATTRIBI2IVPROC ) platGetProcAddress( "glVertexAttribI2iv" ) ) != nullptr;
		r &= ( glVertexAttribI3iv = ( PFNGLVERTEXATTRIBI3IVPROC ) platGetProcAddress( "glVertexAttribI3iv" ) ) != nullptr;
		r &= ( glVertexAttribI4iv = ( PFNGLVERTEXATTRIBI4IVPROC ) platGetProcAddress( "glVertexAttribI4iv" ) ) != nullptr;
		r &= ( glVertexAttribI1uiv = ( PFNGLVERTEXATTRIBI1UIVPROC ) platGetProcAddress( "glVertexAttribI1uiv" ) ) != nullptr;
		r &= ( glVertexAttribI2uiv = ( PFNGLVERTEXATTRIBI2UIVPROC ) platGetProcAddress( "glVertexAttribI2uiv" ) ) != nullptr;
		r &= ( glVertexAttribI3uiv = ( PFNGLVERTEXATTRIBI3UIVPROC ) platGetProcAddress( "glVertexAttribI3uiv" ) ) != nullptr;
		r &= ( glVertexAttribI4uiv = ( PFNGLVERTEXATTRIBI4UIVPROC ) platGetProcAddress( "glVertexAttribI4uiv" ) ) != nullptr;
		r &= ( glVertexAttribI4bv = ( PFNGLVERTEXATTRIBI4BVPROC ) platGetProcAddress( "glVertexAttribI4bv" ) ) != nullptr;
		r &= ( glVertexAttribI4sv = ( PFNGLVERTEXATTRIBI4SVPROC ) platGetProcAddress( "glVertexAttribI4sv" ) ) != nullptr;
		r &= ( glVertexAttribI4ubv = ( PFNGLVERTEXATTRIBI4UBVPROC ) platGetProcAddress( "glVertexAttribI4ubv" ) ) != nullptr;
		r &= ( glVertexAttribI4usv = ( PFNGLVERTEXATTRIBI4USVPROC ) platGetProcAddress( "glVertexAttribI4usv" ) ) != nullptr;
		r &= ( glGetUniformuiv = ( PFNGLGETUNIFORMUIVPROC ) platGetProcAddress( "glGetUniformuiv" ) ) != nullptr;
		r &= ( glBindFragDataLocation = ( PFNGLBINDFRAGDATALOCATIONPROC ) platGetProcAddress( "glBindFragDataLocation" ) ) != nullptr;
		r &= ( glGetFragDataLocation = ( PFNGLGETFRAGDATALOCATIONPROC ) platGetProcAddress( "glGetFragDataLocation" ) ) != nullptr;
		r &= ( glUniform1ui = ( PFNGLUNIFORM1UIPROC ) platGetProcAddress( "glUniform1ui" ) ) != nullptr;
		r &= ( glUniform2ui = ( PFNGLUNIFORM2UIPROC ) platGetProcAddress( "glUniform2ui" ) ) != nullptr;
		r &= ( glUniform3ui = ( PFNGLUNIFORM3UIPROC ) platGetProcAddress( "glUniform3ui" ) ) != nullptr;
		r &= ( glUniform4ui = ( PFNGLUNIFORM4UIPROC ) platGetProcAddress( "glUniform4ui" ) ) != nullptr;
		r &= ( glUniform1uiv = ( PFNGLUNIFORM1UIVPROC ) platGetProcAddress( "glUniform1uiv" ) ) != nullptr;
		r &= ( glUniform2uiv = ( PFNGLUNIFORM2UIVPROC ) platGetProcAddress( "glUniform2uiv" ) ) != nullptr;
		r &= ( glUniform3uiv = ( PFNGLUNIFORM3UIVPROC ) platGetProcAddress( "glUniform3uiv" ) ) != nullptr;
		r &= ( glUniform4uiv = ( PFNGLUNIFORM4UIVPROC ) platGetProcAddress( "glUniform4uiv" ) ) != nullptr;
		r &= ( glTexParameterIiv = ( PFNGLTEXPARAMETERIIVPROC ) platGetProcAddress( "glTexParameterIiv" ) ) != nullptr;
		r &= ( glTexParameterIuiv = ( PFNGLTEXPARAMETERIUIVPROC ) platGetProcAddress( "glTexParameterIuiv" ) ) != nullptr;
		r &= ( glGetTexParameterIiv = ( PFNGLGETTEXPARAMETERIIVPROC ) platGetProcAddress( "glGetTexParameterIiv" ) ) != nullptr;
		r &= ( glGetTexParameterIuiv = ( PFNGLGETTEXPARAMETERIUIVPROC ) platGetProcAddress( "glGetTexParameterIuiv" ) ) != nullptr;
		r &= ( glClearBufferiv = ( PFNGLCLEARBUFFERIVPROC ) platGetProcAddress( "glClearBufferiv" ) ) != nullptr;
		r &= ( glClearBufferuiv = ( PFNGLCLEARBUFFERUIVPROC ) platGetProcAddress( "glClearBufferuiv" ) ) != nullptr;
		r &= ( glClearBufferfv = ( PFNGLCLEARBUFFERFVPROC ) platGetProcAddress( "glClearBufferfv" ) ) != nullptr;
		r &= ( glClearBufferfi = ( PFNGLCLEARBUFFERFIPROC ) platGetProcAddress( "glClearBufferfi" ) ) != nullptr;
		r &= ( glIsRenderbuffer = ( PFNGLISRENDERBUFFERPROC ) platGetProcAddress( "glIsRenderbuffer" ) ) != nullptr;
		r &= ( glBindRenderbuffer = ( PFNGLBINDRENDERBUFFERPROC ) platGetProcAddress( "glBindRenderbuffer" ) ) != nullptr;
		r &= ( glDeleteRenderbuffers = ( PFNGLDELETERENDERBUFFERSPROC ) platGetProcAddress( "glDeleteRenderbuffers" ) ) != nullptr;
		r &= ( glGenRenderbuffers = ( PFNGLGENRENDERBUFFERSPROC ) platGetProcAddress( "glGenRenderbuffers" ) ) != nullptr;
		r &= ( glRenderbufferStorage = ( PFNGLRENDERBUFFERSTORAGEPROC ) platGetProcAddress( "glRenderbufferStorage" ) ) != nullptr;
		r &= ( glGetRenderbufferParameteriv = ( PFNGLGETRENDERBUFFERPARAMETERIVPROC ) platGetProcAddress( "glGetRenderbufferParameteriv" ) ) != nullptr;
		r &= ( glIsFramebuffer = ( PFNGLISFRAMEBUFFERPROC ) platGetProcAddress( "glIsFramebuffer" ) ) != nullptr;
		r &= ( glBindFramebuffer = ( PFNGLBINDFRAMEBUFFERPROC ) platGetProcAddress( "glBindFramebuffer" ) ) != nullptr;
		r &= ( glDeleteFramebuffers = ( PFNGLDELETEFRAMEBUFFERSPROC ) platGetProcAddress( "glDeleteFramebuffers" ) ) != nullptr;
		r &= ( glGenFramebuffers = ( PFNGLGENFRAMEBUFFERSPROC ) platGetProcAddress( "glGenFramebuffers" ) ) != nullptr;
		r &= ( glCheckFramebufferStatus = ( PFNGLCHECKFRAMEBUFFERSTATUSPROC ) platGetProcAddress( "glCheckFramebufferStatus" ) ) != nullptr;
		r &= ( glFramebufferTexture1D = ( PFNGLFRAMEBUFFERTEXTURE1DPROC ) platGetProcAddress( "glFramebufferTexture1D" ) ) != nullptr;
		r &= ( glFramebufferTexture2D = ( PFNGLFRAMEBUFFERTEXTURE2DPROC ) platGetProcAddress( "glFramebufferTexture2D" ) ) != nullptr;
		r &= ( glFramebufferTexture3D = ( PFNGLFRAMEBUFFERTEXTURE3DPROC ) platGetProcAddress( "glFramebufferTexture3D" ) ) != nullptr;
		r &= ( glFramebufferRenderbuffer = ( PFNGLFRAMEBUFFERRENDERBUFFERPROC ) platGetProcAddress( "glFramebufferRenderbuffer" ) ) != nullptr;
		r &= ( glGetFramebufferAttachmentParameteriv = ( PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC ) platGetProcAddress( "glGetFramebufferAttachmentParameteriv" ) ) != nullptr;
		r &= ( glGenerateMipmap = ( PFNGLGENERATEMIPMAPPROC ) platGetProcAddress( "glGenerateMipmap" ) ) != nullptr;
		r &= ( glBlitFramebuffer = ( PFNGLBLITFRAMEBUFFERPROC ) platGetProcAddress( "glBlitFramebuffer" ) ) != nullptr;
		r &= ( glRenderbufferStorageMultisample = ( PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC ) platGetProcAddress( "glRenderbufferStorageMultisample" ) ) != nullptr;
		r &= ( glFramebufferTextureLayer = ( PFNGLFRAMEBUFFERTEXTURELAYERPROC ) platGetProcAddress( "glFramebufferTextureLayer" ) ) != nullptr;
		r &= ( glMapBufferRange = ( PFNGLMAPBUFFERRANGEPROC ) platGetProcAddress( "glMapBufferRange" ) ) != nullptr;
		r &= ( glFlushMappedBufferRange = ( PFNGLFLUSHMAPPEDBUFFERRANGEPROC ) platGetProcAddress( "glFlushMappedBufferRange" ) ) != nullptr;
		r &= ( glBindVertexArray = ( PFNGLBINDVERTEXARRAYPROC ) platGetProcAddress( "glBindVertexArray" ) ) != nullptr;
		r &= ( glDeleteVertexArrays = ( PFNGLDELETEVERTEXARRAYSPROC ) platGetProcAddress( "glDeleteVertexArrays" ) ) != nullptr;
		r &= ( glGenVertexArrays = ( PFNGLGENVERTEXARRAYSPROC ) platGetProcAddress( "glGenVertexArrays" ) ) != nullptr;
		r &= ( glIsVertexArray = ( PFNGLISVERTEXARRAYPROC ) platGetProcAddress( "glIsVertexArray" ) ) != nullptr;

	}
	
	// Extensions
	if ( glExt::majorVersion < 3 )
		InitLegacyExtensions( r );
	else
		InitModernExtensions( r );

	// Default extensions, suitable for any OpenGL version
	glExt::EXT_texture_filter_anisotropic = isExtensionSupported( "GL_EXT_texture_filter_anisotropic" );

	glExt::EXT_texture_compression_s3tc = isExtensionSupported( "GL_EXT_texture_compression_s3tc" );

	glExt::EXT_texture_sRGB = isExtensionSupported( "GL_EXT_texture_sRGB" );

	return r;
}

