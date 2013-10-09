// *************************************************************************************************
//
// h3d .NET wrapper
// ----------------------------------
// Copyright (C) 2007 Martin Burkhard
// Copyright (C) 2009 Volker Wiendl
//
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

using System;
using System.Runtime.InteropServices;
using System.Security;

namespace Horde3DNET
{
    /// <summary>
    /// Separates native methods from managed code.
    /// </summary>
    internal static class NativeMethodsEngine
    {
        private const string ENGINE_DLL = "Horde3D.dll";

        // added (h3d 1.0)        

        // --- Basic funtions ---
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern IntPtr h3dGetVersionString();        

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool h3dCheckExtension(string extensionName);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool h3dGetError();
        
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool h3dInit();

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void h3dRelease();

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void h3dResizePipelineBuffers( int pipeRes, int width, int height );

        //horde3d 1.0
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dRender(int node);
        /////

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dFinalizeFrame();

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void h3dClear();


        // --- General functions ---
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern IntPtr h3dGetMessage(out int level, out float time);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern float h3dGetOption(h3d.H3DOptions param);


        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool h3dSetOption(int param, float value);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern float h3dGetStat(int param, [MarshalAs(UnmanagedType.U1)]bool reset);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void h3dShowOverlays(float[] verts, int vertCount, float colR, float colG, float colB, float colA, int material, int flags );

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void h3dClearOverlays();

        // --- Resource functions ---
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dGetResType(int res);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern IntPtr h3dGetResName(int res);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dGetNextResource(int type, int start);
        
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dFindResource(int type, string name);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dAddResource(int type, string name, int flags);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dCloneResource(int sourceRes, string name);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dRemoveResource(int res);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static extern bool h3dIsResLoaded(int res);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool h3dLoadResource(int name, IntPtr data, int size);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dUnloadResource(int res);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dGetResElemCount(int res, int elem);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dFindResElem(int res, int elem, int param, string value );

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dGetResParamI(int res, int elem, int elemIdx, int param);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dSetResParamI(int res, int elem, int elemIdx, int param, int value);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern float h3dGetResParamF(int res, int elem, int elemIdx, int param, int compIdx);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dSetResParamF(int res, int elem, int elemIdx, int param, int compIdx, float value);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern IntPtr h3dGetResParamStr(int res, int elem, int elemIdx, int param);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void h3dSetResParamStr(int res, int elem, int elemIdx, int param, string value);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern IntPtr h3dMapResStream(int res, int elem, int elemIdx, int stream, [MarshalAs(UnmanagedType.U1)]bool read, [MarshalAs(UnmanagedType.U1)]bool write);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dUnmapResStream(int res);
        
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dQueryUnloadedResource(int index);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void h3dReleaseUnusedResources();

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dCreateTexture(string name, int width, int height, int fmt, int flags);

        // --- Shader specific ---
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dSetShaderPreambles( string vertPreamble, string fragPreamble );

        // --- Material specific ---
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool h3dSetMaterialUniform(int materialRes, string name, float a, float b, float c, float d);


        //DLL bool h3dGetPipelineRenderTargetData( ResHandle pipelineRes, const char *targetName,
        //                              int bufIndex, int *width, int *height, int *compCount,
        //                              float *dataBuffer, int bufferSize );
        
        // --- Scene graph functions ---
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern h3d.H3DNodeTypes h3dGetNodeType(int node);        

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dGetNodeParent(int node);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool h3dSetNodeParent(int node, int parent);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dGetNodeChild(int parent, int index);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dAddNodes(int parent, int res);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dRemoveNode(int node);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dGetNodeFlags(int node);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dSetNodeFlags(int node, int flags, [MarshalAs(UnmanagedType.U1)]bool recursive);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type
        internal static extern bool h3dCheckNodeTransFlag(int node, [MarshalAs(UnmanagedType.U1)]bool reset);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dGetNodeTransform(int node, out float px, out float py, out float pz,
                                out float rx, out float ry, out float rz, out float sx, out float sy, out float sz);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dSetNodeTransform(int node, float px, float py, float pz,
                                float rx, float ry, float rz, float sx, float sy, float sz);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]                
        internal static extern void h3dGetNodeTransMats(int node, out IntPtr relMat, out IntPtr absMat);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dSetNodeTransMat(int node, float[] mat4x4);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dGetNodeParamI(int node, int param);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dSetNodeParamI(int node, int param, int value);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern float h3dGetNodeParamF(int node, int param, int compIdx);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dSetNodeParamF(int node, int param, int compIdx, float value);
        
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern IntPtr h3dGetNodeParamStr(int node, int param);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dSetNodeParamStr(int node, int param, string value);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool h3dGetNodeAABB(int node, out float minX, out float minY, out float minZ, out float maxX, out float maxY, out float maxZ);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dFindNodes(int node, string name, int type);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dGetNodeFindResult(int index);
        
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void h3dSetNodeUniforms(int node, float[] uniformData, int count);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dCastRay(int node, float ox, float oy, float oz, float dx, float dy, float dz, int numNearest);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool h3dGetCastRayResult(int index, out int node, out float distance, float[] intersection);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dCheckNodeVisibility(int node, int cameraNode, [MarshalAs(UnmanagedType.U1)]bool checkOcclusion, [MarshalAs(UnmanagedType.U1)]bool calcLod);

        // Group specific
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dAddGroupNode(int parent, string name);

        // Model specific
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dAddModelNode(int parent, string name, int geoRes);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dSetupModelAnimStage(int node, int stage, int animationRes,
                                      int layer, string startNode, [MarshalAs(UnmanagedType.U1)]bool additive);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void h3dGetModelAnimParams(int node, int stage, out float time, out float weight);
        
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dSetModelAnimParams(int node, int stage, float time, float weight);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool h3dSetModelMorpher(int node, string target, float weight);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void h3dUpdateModel(int modelNode, int flags);

        // Mesh specific
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dAddMeshNode(int parent, string name, int matRes, 
								    int batchStart, int batchCount,
							    int vertRStart, int vertREnd );
      
        // Joint specific
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dAddJointNode(int parent, string name, int jointIndex);

        // Light specific
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dAddLightNode(int parent, string name, int materialRes,
                                     string lightingContext, string shadowContext);

          // Camera specific
        //horde3d 1.0
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dAddCameraNode(int parent, string name, int pipelineRes);
        /////

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dSetupCameraView(int node, float fov, float aspect, float nearDist, float farDist);
            
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern void h3dGetCameraProjMat(int node, float[] projMat);


	    // Emitter specific
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dAddEmitterNode(int parent, string name,
								       int matRes, int effectRes,
								       int maxParticleCount, int respawnCount );

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void h3dUpdateEmitter(int node, float timeDelta);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool h3dHasEmitterFinished(int emitterNode);

    }
}