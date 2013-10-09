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
using Horde3DNET.Properties;

namespace Horde3DNET.Utils
{
    public static class Horde3DUtils
    {
        /// <summary>        
        /// MaxStatMode  - Maximum stat mode number supported in showFrameStats
        /// </summary>
        public const int MaxStatMode = 2;

        // Utilities functions
        /// <summary>
        /// FreeMem is not supported. The purpose is to free memory allocated by the h3d library.
        /// </summary>
        public static void freeMem(IntPtr ptr)
        {
            NativeMethodsUtils.h3dutFreeMem(ptr);
        }
        
        /// <summary>
        /// This utility function pops all messages from the message queue and writes them to a HTML formated log file 'EngineLog.html'.
        /// </summary>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool dumpMessages()
        {
            return NativeMethodsUtils.h3dutDumpMessages();
        }

        /// <summary>
        /// This function returns the search path of a specified resource type.
        /// </summary>
        /// <param name="type">type of resource</param>
        /// <returns>the search path string</returns>
        public static string getResourcePath(h3d.H3DResTypes type)
        {
            IntPtr ptr = NativeMethodsUtils.h3dutGetResourcePath(type);
            return Marshal.PtrToStringAnsi(ptr);
        }

        /// <summary>
        /// This function sets the search path for a specified resource type. 
        /// Whenever a new resource is added, the specified path is concatenated to the name of the created resource.
        /// </summary>
        /// <param name="type">type of resource</param>
        /// <param name="path">path where the resources can be found (without slash or backslash at the end)</param>
        public static void setResourcePath(h3d.H3DResTypes type, string path)
        {
            if (path == null) throw new ArgumentNullException("path", Resources.StringNullExceptionString);

            NativeMethodsUtils.h3dutSetResourcePath(type, path);
        }

        /// <summary>
        /// This utility function loads previously added and still unloaded resources from a specified directory on a data drive. 
        /// All resource names are directly converted to filenames when being loaded.
        /// </summary>
        /// <param name="contenDir">directory where data is located on the drive</param>
        /// <returns>false if at least one resource could not be loaded, otherwise true</returns>
        public static bool loadResourcesFromDisk(string contenDir)
        {
            if (contenDir == null) throw new ArgumentNullException("contenDir", Resources.StringNullExceptionString);

            return NativeMethodsUtils.h3dutLoadResourcesFromDisk(contenDir);
        }

        /// <summary>
        /// Creates a Geometry resource from specified vertex data.
        /// </summary>
        /// This utility function allocates and initializes a Geometry resource
        /// with the specified vertex attributes and indices. The optional tangent space
        /// data (normal, tangent, bitangent) is encoded as int16, where -1.0 maps to
        /// -32'767 and 1.0f to +32'767.
        /// <param name="name">unique name of the new Geometry resource </param>
        /// <param name="numVertices">number of vertices</param>
        /// <param name="numTriangleIndices">number of vertex indices</param>
        /// <param name="posData">vertex positions (xyz)</param>
        /// <param name="indexData">indices defining triangles</param>
        /// <param name="normalData">normals xyz (optional, can be NULL)</param>
        /// <param name="tangentData">tangents xyz (optional, can be NULL)</param>
        /// <param name="bitangentData">bitangents xyz (required if tangents specified, otherwise NULL)</param>
        /// <param name="texData1">first texture coordinate uv set (optional, can be NULL)</param>
        /// <param name="texData2">second texture coordinate uv set (optional, can be NULL)</param>
        /// <returns>handle to new Geometry resource or 0 in case of failure</returns>
        public static int createGeometryRes(string name, int numVertices, int numTriangleIndices,
                                           float[] posData, int[] indexData, short[] normalData,
                                           short[] tangentData, short[] bitangentData,
                                           float[] texData1, float[] texData2)
        {
            return NativeMethodsUtils.h3dutCreateGeometryRes(name, numVertices, numTriangleIndices, posData, indexData, normalData, tangentData, bitangentData, texData1, texData2);
        }


        /// <summary>
        /// This utility function allocates memory for the pointer outData and creates a TGA image from the specified pixel data. 
        /// The dimensions of the image have to be specified as well as the bit depth of the pixel data. 
        /// The created TGA-image-data can be used as Texture2D or TexureCube resource in the engine.
        /// </summary>
        /// <remarks>
        /// The memory allocated by this routine will be freed automatically.
        /// </remarks>
        /// <param name="pixels">pointer to pixel source data from which TGA-image-data is constructed; memory layout: pixel with position (x, y) in image (origin of image is upper left corner) has memory location (y * width + x) * (bpp / 8) in pixels array</param>
        /// <param name="width">image width of source data</param>
        /// <param name="height">image height of source data</param>
        /// <param name="bpp">color bit depth of source data (Valid: 24, 32)</param>
        /// <param name="outData">the created TGA data</param>
        /// <param name="outSize">size of TGA data</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool createTGAImage(byte[] pixels, int width, int height, int bpp, out byte[] outData, out int outSize)
        {
            IntPtr pixelPtr, dataPtr;
            uint size;

            // Allocate data to store pixel content
            pixelPtr = Marshal.AllocHGlobal(pixels.Length);

            // Copy pixel content to memory location
            Marshal.Copy(pixels, 0, pixelPtr, pixels.Length);

            // create TGA Image 
            bool result = NativeMethodsUtils.h3dutCreateTGAImage(pixelPtr, (uint)width, (uint)height, (uint)bpp, out dataPtr, out size);

            // free allocated memory
            Marshal.FreeHGlobal(pixelPtr);

            // set size of TGA data
            outSize = (int)size;

            // create new array for TGA data
            outData = new byte[outSize];

            // copy TGA data from memory into data array
            Marshal.Copy(dataPtr, outData, 0, outSize);

            // free allocated memory for TGA data
            //NativeMethodsUtils.freeMem(dataPtr);

            return result;
        }

        /// <summary>
        /// Writes the content of the backbuffer to a tga file.
        /// </summary>
        /// This function reads back the content of the backbuffer and writes it to a tga file with the
		/// specified filename and path.
        /// <param name="filename">filename and path of the output tga file</param>
        /// <returns>true if the file could be written, otherwise false</returns>
        public static bool createSnapshot(string filename)
        {
            return NativeMethodsUtils.h3dutScreenshot(filename); 
        }

        /// <summary>
        /// Calculates the ray originating at the specified camera and window coordinates
        /// </summary>
        /// This utility function takes normalized window coordinates (ranging from 0 to 1 with the
        /// origin being the bottom left corner of the window) and returns ray origin and direction
        /// given camera. The function is especially useful for selecting objects by clicking
        /// on them.
        /// <param name="cameraNode">camera used for picking</param>
        /// <param name="nwx">normalized window coordinates</param>
        /// <param name="nwy">normalized window coordinates</param>
        /// <param name="ox">calculated ray origin</param>
        /// <param name="oy">calculated ray origin</param>
        /// <param name="oz">calculated ray origin</param>
        /// <param name="dx">calculated ray direction</param>
        /// <param name="dy">calculated ray direction</param>
        /// <param name="dz">calculated ray direction</param>
        public static void pickRay( 
            int cameraNode, float nwx, float nwy, 
            out float ox, out float oy, out float oz,
            out float dx, out float dy, out float dz)
        {
            NativeMethodsUtils.h3dutPickRay(cameraNode, nwx, nwy, out ox, out oy, out oz, out dx, out dy, out dz);
        }


        /// <summary>
        /// This utility function takes normalized window coordinates (ranging from 0 to 1 with the origin being the bottom left corner of the window) and returns the scene node which is visible at that location. The function is especially useful for selecting objects by clicking on them. Currently picking is only working for Meshes.
        /// </summary>
        /// <param name="node">Camera node used for picking</param>
        /// <param name="nwx">normalized window x coordinate</param>
        /// <param name="nwy">normalized window y coordinate</param>
        /// <returns>handle of picked node or 0 if no node was hit</returns>
        public static int pickNode( int node, float nwx, float nwy)
        {
            return NativeMethodsUtils.h3dutPickNode(node, nwx, nwy);
        }

        /// <summary>
        /// This utility function uses overlays to display a text string at a specified position on the screen. 
        /// </summary>
        /// <remarks>
        /// The font texture of the specified font material has to be a regular 16x16 grid containing all ASCII characters in row-major order. 
        /// The layer corresponds to the layer parameter of overlays.
        /// </remarks>
        /// <param name="text">text string to be displayed</param>
        /// <param name="x">x position of the lower left corner of the first character; for more details on coordinate system see overlay documentation</param>
        /// <param name="y">y position of the lower left corner of the first character; for more details on coordinate system see overlay documentation</param>
        /// <param name="size">size factor of the font</param>
        /// <param name="colR">red part of font color</param>
        /// <param name="colG">green part of font color</param>
        /// <param name="colB">blue part of font color</param>
        /// <param name="fontMatRes">font material resource used for rendering</param>
        /// <param name="layer">layer index of the font overlays (values: 0-7)</param>        
        public static void showText(string text, float x, float y, float size,
                                    float colR, float colG, float colB,
                                    int fontMatRes)
        {
            if (text == null) throw new ArgumentNullException("text", Resources.StringNullExceptionString);            
            if (fontMatRes < 0) throw new ArgumentOutOfRangeException("fontMatRes", Resources.UIntOutOfRangeExceptionString);

            NativeMethodsUtils.h3dutShowText(text, x, y, size, colR, colG, colB, fontMatRes );
        }

        /// <summary>
        /// This utility function displays an info box with statistics for the current frame on the screen.
		/// Since the statistic counters are reset after the call, the function should be called exactly once
		/// per frame to obtain correct values.
        /// </summary>
        /// <param name="fontMaterialRes">font material resource used for drawing text</param>
        /// <param name="panelMaterialRes">material resource used for drawing info box</param>
        /// <param name="mode">display mode, specifying which data is shown (<= MaxStatMode)</param>
        public static void showFrameStats(int fontMaterialRes, int panelMaterialRes, int mode)
        {
            NativeMethodsUtils.h3dutShowFrameStats(fontMaterialRes, panelMaterialRes, mode);
        }
    }
}
