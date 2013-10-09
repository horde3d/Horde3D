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

namespace Horde3DNET.Utils
{
    /// <summary>
    /// Separates native methods from managed code.
    /// </summary>
    internal static class NativeMethodsUtils
    {
        public const string UTILS_DLL = "Horde3DUtils.dll";

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void h3dutFreeMem(IntPtr ptr);

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool h3dutDumpMessages();

        // Utilities
        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern IntPtr h3dutGetResourcePath(h3d.H3DResTypes type);

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void h3dutSetResourcePath(h3d.H3DResTypes type, string path);
             

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool h3dutLoadResourcesFromDisk(string contentDir);

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern int h3dutCreateGeometryRes(string name, int numVertices, int numTriangleIndices,
                                           float[] posData, int[] indexData, short[] normalData,
                                           short[] tangentData, short[] bitangentData,
                                           float[] texData1, float[] texData2);

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool h3dutCreateTGAImage(IntPtr pixels, uint width, uint height, uint bpp, out IntPtr outData, out uint outSize);

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void h3dutPickRay(int cameraNode, float nwx, float nwy,
            out float ox, out float oy, out float oz,
            out float dx, out float dy, out float dz);

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int h3dutPickNode(int node, float nwx, float nwy);

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void h3dutShowText(string text, float x, float y, float size,
                                             float colR, float colG, float colB,
                                             int fontMatRes );

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void h3dutShowFrameStats(int fontMaterialRes, int panelMaterialRes, int mode);

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool h3dutScreenshot(string filename);

    }
}
