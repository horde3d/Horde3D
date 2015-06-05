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
using System.Runtime.Serialization;
using System.Runtime.InteropServices;
using Horde3DNET.Properties;

namespace Horde3DNET
{
    public static class h3d
    {
        // Predefined constants
        private static int _rootNode = 1;

        public static int H3DRootNode
        {
            get { return _rootNode; }
        }

        /// <summary>
        /// Enum: H3DOptions
        ///       The available engine option parameters.        		
        ///   MaxLogLevel         - Defines the maximum log level; only messages which are smaller or equal to this value
        ///                         (hence more important) are published in the message queue. (Default: 4)
        ///   MaxNumMessages      - Defines the maximum number of messages that can be stored in the message queue (Default: 512)
        ///   TrilinearFiltering  - Enables or disables trilinear filtering for textures. (Values: 0, 1; Default: 1)
        ///   MaxAnisotropy       - Sets the maximum quality for anisotropic filtering. (Values: 1, 2, 4, 8; Default: 1)
        ///   TexCompression      - Enables or disables texture compression; only affects textures that are
        ///                         loaded after setting the option. (Values: 0, 1; Default: 0)
        ///   SRGBLinearization   - Eanbles or disables gamma-to-linear-space conversion of input textures that are tagged as sRGB (Values: 0, 1; Default: 0)
        ///   LoadTextures        - Enables or disables loading of textures referenced by materials; this can be useful to reduce
        ///                         loading times for testing. (Values: 0, 1; Default: 1)
        ///   FastAnimation       - Disables or enables inter-frame interpolation for animations. (Values: 0, 1; Default: 1)
        ///   ShadowMapSize       - Sets the size of the shadow map buffer (Values: 128, 256, 512, 1024, 2048; Default: 1024)
        ///   SampleCount         - Maximum number of samples used for multisampled render targets; only affects pipelines
        ///                         that are loaded after setting the option. (Values: 0, 2, 4, 8, 16; Default: 0)
        ///   WireframeMode       - Enables or disables wireframe rendering
        ///   DebugViewMode       - Enables or disables debug view where geometry is rendered in wireframe without shaders and
        ///                         lights are visualized using their screen space bounding box. (Values: 0, 1; Default: 0)
        ///   DumpFailedShaders   - Enables or disables storing of shader code that failed to compile in a text file; this can be
        ///                         useful in combination with the line numbers given back by the shader compiler. (Values: 0, 1; Default: 0)
        ///   GatherTimeStats     - Enables or disables gathering of time stats that are useful for profiling (Values: 0, 1; Default: 1)
        /// </summary>
        public enum H3DOptions
        {
            MaxLogLevel = 1,
            MaxNumMessages,
            TrilinearFiltering,
            MaxAnisotropy,
            TexCompression,
            SRGBLinearization,
            LoadTextures,
            FastAnimation,
            ShadowMapSize,
            SampleCount,
            WireframeMode,
            DebugViewMode,
            DumpFailedShaders,
            GatherTimeStats
        }

       /// <summary>
       /// Enum: H3DStats
       ///        The available engine statistic parameters.
       /// 		
       ///    TriCount          - Number of triangles that were pushed to the renderer
       ///    BatchCount        - Number of batches (draw calls)
       ///    LightPassCount    - Number of lighting passes
       ///    FrameTime         - Time in ms between two h3dFinalizeFrame calls
       ///    AnimationTime     - CPU time in ms spent for animation
       ///    GeoUpdateTime     - CPU time in ms spent for software skinning and morphing
       ///    ParticleSimTime   - CPU time in ms spent for particle simulation and updates
	   ///	  FwdLightsGPUTime  - GPU time in ms spent for forward lighting passes
       ///    DefLightsGPUTime  - GPU time in ms spent for drawing deferred light volumes
	   ///    ShadowsGPUTime    - GPU time in ms spent for generating shadow maps
       ///    ParticleGPUTime   - GPU time in ms spent for drawing particles
       ///    TextureVMem       - Estimated amount of video memory used by textures (in Mb)
       ///    GeometryVMem      - Estimated amount of video memory used by geometry (in Mb)
       /// </summary>
        public enum H3DStats
        {
            TriCount = 100,
            BatchCount,
            LightPassCount,
            FrameTime,
            AnimationTime,
            GeoUpdateTime,
            ParticleSimTime,
            FwdLightsGPUTime,
            DefLightsGPUTime,
            ShadowsGPUTime,
            ParticleGPUTime,
            TextureVMem,
            GeometryVMem
        }

        /// <summary>
        /// Enum: H3DResTypes
        ///           The available resource types.        		
        ///       Undefined       - An undefined resource, returned by getResourceType in case of error
        ///       SceneGraph      - Scene graph subtree stored in XML format
        ///       Geometry        - Geometrical data containing bones, vertices and triangles
        ///       Animation       - Animation data
        ///       Material        - Material script
        ///       Code            - Text block containing shader source code
        ///       Shader          - Shader program
        ///       Texture         - Texture map
        ///       ParticleEffect  - Particle configuration
        ///       Pipeline        - Rendering pipeline
        /// </summary>
        public enum H3DResTypes
        {
            Undefined = 0,
            SceneGraph,
            Geometry,
            Animation,
            Material,
            Code,
            Shader,
            Texture,
            ParticleEffect,
            Pipeline
        }

        /// <summary>
        /// Enum: H3DResFlags
        ///        The available flags used when adding a resource.
        			
        /// NoQuery           - Excludes resource from being listed by queryUnloadedResource function.
        /// NoTexCompression  - Disables texture compression for Texture resource.
        /// NoTexMipmaps      - Disables generation of mipmaps for Texture resource.
        /// TexCubemap        - Sets Texture resource to be a cubemap.
        /// TexDynamic        - Enables more efficient updates of Texture resource streams.
        /// TexRenderable     - Makes Texture resource usable as render target.
        /// TexSRGB           - Indicates that Texture resource is in sRGB color space and should be converted
        ///                    to linear space when being sampled.
        /// </summary>
        public enum H3DResFlags
        {
            NoQuery = 1,
            NoTexCompression = 2,
            NoTexMipmaps = 4,
            TexCubemap = 8,
            TexDynamic = 16,
            TexRenderable = 32,
            TexSRGB = 64
        }

        /// <summary>
        /// Enum: H3DFormats
		///     The available resource stream formats.
			
		/// Unknown      - Unknown format
        /// TEX_BGRA8    - 8-bit BGRA texture
        /// TEX_DXT1     - DXT1 compressed texture
        /// TEX_DXT3     - DXT3 compressed texture
        /// TEX_DXT5     - DXT5 compressed texture
        /// TEX_RGBA16F  - Half float RGBA texture
        /// TEX_RGBA32F  - Float RGBA texture
        /// </summary>
        public enum H3DFormats
        {
            Unknown = 0,
            TEX_BGRA8,
            TEX_DXT1,
            TEX_DXT3,
            TEX_DXT5,
            TEX_RGBA16F,
            TEX_RGBA32F
        }

        /// <summary>
        /// Enum: H3DGeoRes
        ///           The available Geometry resource accessors.
        		
        ///       GeometryElem         - Base element
        ///       GeoIndexCountI       - Number of indices [read-only]
        ///       GeoVertexCountI      - Number of vertices [read-only]
        ///       GeoIndices16I        - Flag indicating whether index data is 16 or 32 bit [read-only]
        ///       GeoIndexStream       - Triangle index data (uint16 or uint32, depending on flag)
        ///       GeoVertPosStream     - Vertex position data (float x, y, z)
        ///       GeoVertTanStream     - Vertex tangent frame data (float nx, ny, nz, tx, ty, tz, tw)
        ///       GeoVertStaticStream  - Vertex static attribute data (float u0, v0,
        ///                                float4 jointIndices, float4 jointWeights, float u1, v1)
        /// </summary>
        public enum H3DGeoRes
        {
            GeometryElem = 200,
            GeoIndexCountI,
            GeoVertexCountI,
            GeoIndices16I,
            GeoIndexStream,
            GeoVertPosStream,
            GeoVertTanStream,
            GeoVertStaticStream
        }

        /// Enum: H3DAnimRes
        ///     The available Animation resource accessors.	  		
        /// EntityElem      - Stored animation entities (joints and meshes)
        /// EntFrameCountI  - Number of frames stored for a specific entity [read-only]
        ///
        public enum H3DAnimRes
        {
            EntityElem = 300,
            EntFrameCountI
        }

        /// <summary>
        /// Enum: H3DMatRes
        ///       The available Material resource accessors.

        ///   MaterialElem  - Base element
        ///   SamplerElem   - Sampler element
        ///   UniformElem   - Uniform element
        ///   MatClassStr   - Material class
        ///   MatLinkI      - Material resource that is linked to this material
        ///   MatShaderI    - Shader resource
        ///   SampNameStr   - Name of sampler [read-only]
        ///   SampTexResI   - Texture resource bound to sampler
        ///   UnifNameStr   - Name of uniform [read-only]
        ///   UnifValueF4   - Value of uniform (a, b, c, d)
        /// </summary>
        public enum H3DMatRes
        {
            MaterialElem = 400,
            SamplerElem,
            UniformElem,
            MatClassStr,
            MatLinkI,
            MatShaderI,
            SampNameStr,
            SampTexResI,
            UnifNameStr,
            UnifValueF4
        }

        /// <summary>
        /// Enum: H3DShaderRes
        ///       The available Shader resource accessors.

        ///   ContextElem     - Context element 
        ///   SamplerElem     - Sampler element
        ///   UniformElem     - Uniform element
        ///   ContNameStr     - Name of context [read-only]
        ///   SampNameStr     - Name of sampler [read-only]
        ///   UnifNameStr     - Name of uniform [read-only]
        ///   UnifSizeI       - Size (number of components) of uniform [read-only]
        ///   UnifDefValueF4  - Default value of uniform (a, b, c, d)
        /// </summary>
        public enum H3DShaderRes
        {
            ContextElem = 600,
            SamplerElem,
            UniformElem,
            ContNameStr,
            SampNameStr,
            UnifNameStr,
            UnifSizeI,
            UnifDefValueF4
        }

        /// <summary>
        /// Enum: H3DTexRes
        ///           The available Texture resource accessors.

        ///       TextureElem     - Base element
        ///       ImageElem       - Subresources of the texture. A texture consists, depending on the type,
        ///                         of a number of equally sized slices which again can have a fixed number
        ///                         of mipmaps. Each image element represents the base image of a slice or
        ///                         a single mipmap level of the corresponding slice.
        ///       TexFormatI      - Texture format [read-only]
        ///       TexSliceCountI  - Number of slices (1 for 2D texture and 6 for cubemap) [read-only]
        ///       ImgWidthI       - Image width [read-only]
        ///       ImgHeightI      - Image height [read-only]
        ///       ImgPixelStream  - Pixel data of an image. The data layout matches the layout specified
        ///                         by the texture format with the exception that half-float is converted
        ///                         to float. The first element in the data array corresponds to the lower
        ///                         left corner.
       /// </summary>
        public enum H3DTexRes
        {
            TextureElem = 700,
            ImageElem,
            TexFormatI,
            TexSliceCountI,
            ImgWidthI,
            ImgHeightI,
            ImgPixelStream,
            TexNativeRefI
        }


        /// <summary>
        ///  Enum: H3DPartEffRes
        ///            The available ParticleEffect resource accessors.

        ///        ParticleElem     - General particle configuration
        ///        ChanMoveVelElem  - Velocity channel
        ///        ChanRotVelElem   - Angular velocity channel
        ///        ChanSizeElem     - Size channel
        ///        ChanColRElem     - Red color component channel
        ///        ChanColGElem     - Green color component channel
        ///        ChanColBElem     - Blue color component channel
        ///        ChanColAElem     - Alpha channel
        ///        PartLifeMinF     - Minimum value of random life time (in seconds)
        ///        PartLifeMaxF     - Maximum value of random life time (in seconds)
        ///        ChanStartMinF    - Minimum for selecting initial random value of channel
        ///        ChanStartMaxF    - Maximum for selecting initial random value of channel
        ///        ChanEndRateF     - Remaining percentage of initial value when particle is dying
        /// </summary>
        public enum H3DPartEffRes
        {
            ParticleElem = 800,
            ChanMoveVelElem,
            ChanRotVelElem,
            ChanSizeElem,
            ChanColRElem,
            ChanColGElem,
            ChanColBElem,
            ChanColAElem,
            PartLifeMinF,
            PartLifeMaxF,
            ChanStartMinF,
            ChanStartMaxF,
            ChanEndRateF,
            ChanDragElem
        }

        /// <summary>
        /// Enum: H3DPipeRes
        ///       The available Pipeline resource accessors.

        ///   StageElem         - Pipeline stage
        ///   StageNameStr      - Name of stage [read-only]
        ///   StageActivationI  - Flag indicating whether stage is active
        /// </summary>
        public enum H3DPipeRes
        {
            StageElem = 900,
            StageNameStr,
            StageActivationI
        }

        /// <summary>
        /// Enum: H3DNodeTypes
        ///    The available scene node types.

        /// Undefined  - An undefined node type, returned by getNodeType in case of error
        /// Group      - Group of different scene nodes
        /// Model      - 3D model with optional skeleton
        /// Mesh       - Subgroup of a model with triangles of one material
        /// Joint      - Joint for skeletal animation
        /// Light      - Light source
        /// Camera     - Camera giving view on scene
        /// Emitter    - Particle system emitter
        /// </summary>
        public enum H3DNodeTypes
        {
            Undefined = 0,
            Group,
            Model,
            Mesh,
            Joint,
            Light,
            Camera,
            Emitter
        }

        /// <summary>
        /// Enum: H3DNodeFlags
        ///        The available scene node flags.
        ///
        /// NoDraw         - Excludes scene node from all rendering
        /// NoCastShadow   - Excludes scene node from list of shadow casters
        /// NoRayQuery     - Excludes scene node from ray intersection queries
        /// Inactive       - Deactivates scene node so that it is completely ignored
        ///                  (combination of all flags above)            
        /// </summary>
        public enum H3DNodeFlags
        {
            NoDraw = 1,
            NoCastShadow = 2,
            NoRayQuery = 4,
            Inactive = 7  // NoDraw | NoCastShadow | NoRayQuery        
        };

        /// <summary>
        ///	Enum: H3DNodeParams
        ///        The available scene node parameters.

        /// NameStr        - Name of the scene node
        /// AttachmentStr  - Optional application-specific meta data for a node encapsulated
        ///                     in an 'Attachment' XML string
        /// </summary>
        public enum H3DNodeParams
        {
            NameStr = 1,
            AttachmentStr
        }

        /// <summary>
        /// Enum: H3DModel
        ///    The available Model node parameters

        /// GeoResI      - Geometry resource used for the model
        /// SWSkinningI  - Enables or disables software skinning (default: 0)
        /// LodDist1F    - Distance to camera from which on LOD1 is used (default: infinite)
        ///               (must be a positive value larger than 0.0)
        /// LodDist2F    - Distance to camera from which on LOD2 is used
        ///                (may not be smaller than LodDist1) (default: infinite)
        /// LodDist3F    - Distance to camera from which on LOD3 is used
        ///               (may not be smaller than LodDist2) (default: infinite)
        /// LodDist4F    - Distance to camera from which on LOD4 is used
        ///               (may not be smaller than LodDist3) (default: infinite)
        /// AnimCountI  - Number of active animation stages [read-only]
        /// </summary>
        public enum H3DModel
        {
            GeoResI = 200,
            SWSkinningI,
            LodDist1F,
            LodDist2F,
            LodDist3F,
            LodDist4F,
            AnimCountI
        }

        /// <summary>
        /// Enum: H3DMesh
        ///    The available Mesh node parameters.

        /// MatResI      - Material resource used for the mesh
        /// BatchStartI  - First triangle index of mesh in Geometry resource of parent Model node [read-only]
        /// BatchCountI  - Number of triangle indices used for drawing mesh [read-only]
        /// VertRStartI  - First vertex in Geometry resource of parent Model node [read-only]
        /// VertREndI    - Last vertex in Geometry resource of parent Model node [read-only]
        /// LodLevelI    - LOD level of Mesh; the mesh is only rendered if its LOD level corresponds to
        ///                the model's current LOD level which is calculated based on the LOD distances (default: 0)
        /// </summary>
        public enum H3DMesh
        {
            MatResI = 300,
            BatchStartI,
            BatchCountI,
            VertRStartI,
            VertREndI,
            LodLevelI
        }

        /// <summary>
        /// Enum: H3DJoint
        ///    The available Joint node parameters.

        /// JointIndexI  - Index of joint in Geometry resource of parent Model node [read-only]
        /// </summary>
        public enum H3DJoint
        {
            JointIndex = 400
        }

        /// <summary>
        /// Enum: H3DLight
        ///    The available Light node parameters.

        /// MatResI             - Material resource used for the light
        /// RadiusF             - Radius of influence (default: 100.0)
        /// FovF                - Field of view (FOV) angle (default: 90.0)
        /// ColorF3             - Diffuse color RGB (default: 1.0, 1.0, 1.0)
        /// ColorMultiplierF    - Diffuse color multiplier for altering intensity, mainly useful for HDR (default: 1.0)
        /// ShadowMapCountI     - Number of shadow maps used for light source (values: 0, 1, 2, 3, 4; default: 0)]
        /// ShadowSplitLambdaF  - Constant determining segmentation of view frustum for Parallel Split Shadow Maps (default: 0.5)
        /// ShadowMapBiasF      - Bias value for shadow mapping to reduce shadow acne (default: 0.005)
        /// LightingContextStr  - Name of shader context used for computing lighting
        /// ShadowContextStr    - Name of shader context used for generating shadow map
        /// </summary>
        public enum H3DLight
        {
            MatResI = 500,
            RadiusF,
            FovF,
            ColorF3,
            ColorMultiplierF,
            ShadowMapCountI,
            ShadowSplitLambdaF,
            ShadowMapBiasF,
            LightingContextStr,
            ShadowContextStr
        }

        /// <summary>
        /// Enum: H3DCamera
        ///            The available Camera node parameters.
        ///        PipeResI         - Pipeline resource used for rendering
        ///        OutTexResI       - 2D Texture resource used as output buffer (can be 0 to use main framebuffer) (default: 0)
        ///        OutBufIndexI     - Index of the output buffer for stereo rendering (values: 0 for left eye, 1 for right eye) (default: 0)
        ///        LeftPlaneF       - Coordinate of left plane relative to near plane center (default: -0.055228457)
        ///        RightPlaneF      - Coordinate of right plane relative to near plane center (default: 0.055228457)
        ///        BottomPlaneF     - Coordinate of bottom plane relative to near plane center (default: -0.041421354f)
        ///        TopPlaneF        - Coordinate of top plane relative to near plane center (default: 0.041421354f)
        ///        NearPlaneF       - Distance of near clipping plane (default: 0.1)
        ///        FarPlaneF        - Distance of far clipping plane (default: 1000)
        ///        ViewportXI       - Position x-coordinate of the lower left corner of the viewport rectangle (default: 0)
        ///        ViewportYI       - Position y-coordinate of the lower left corner of the viewport rectangle (default: 0)
        ///        ViewportWidthI   - Width of the viewport rectangle (default: 320)
        ///        ViewportHeightI  - Height of the viewport rectangle (default: 240)
        ///        OrthoI           - Flag for setting up an orthographic frustum instead of a perspective one (default: 0)
        ///        OccCullingI      - Flag for enabling occlusion culling (default: 0)
        /// </summary>
        public enum H3DCamera
        {
            PipeResI = 600,
            OutTexResI,
            OutBufIndexI,
            LeftPlaneF,
            RightPlaneF,
            BottomPlaneF,
            TopPlaneF,
            NearPlaneF,
            FarPlaneF,
            ViewportXI,
            ViewportYI,
            ViewportWidthI,
            ViewportHeightI,
            OrthoI,
            OccCullingI
        }

        /// <summary>
        /// Enum: H3DEmitter
        ///    The available Emitter node parameters.

        /// MatResI        - Material resource used for rendering
        /// PartEffResI    - ParticleEffect resource which configures particle properties
        /// MaxCountI      - Maximal number of particles living at the same time
        /// RespawnCountI  - Number of times a single particle is recreated after dying (-1 for infinite)
        /// DelayF         - Time in seconds before emitter begins creating particles (default: 0.0)
        /// EmissionRateF  - Maximal number of particles to be created per second (default: 0.0)
        /// SpreadAngleF   - Angle of cone for random emission direction (default: 0.0)
        /// ForceF3        - Force vector XYZ applied to particles (default: 0.0, 0.0, 0.0)
        /// </summary>

        public enum H3DEmitter
        {
            MatResI = 700,
            PartEffResI,
            MaxCountI,
            RespawnCountI,
            DelayF,
            EmissionRateF,
            SpreadAngleF,
            ForceF3
        }

        /// <summary>
        /// Enum: H3DModelUpdateFlags
        ///       The available flags for h3dUpdateModel.
        ///       
        /// Animation  - Apply animation
        /// Geometry   - Apply morphers and software skinning
        /// </summary>
        public enum H3DModelUpdateFlags
        {
                Animation = 1,
                Geometry = 2
        };


        // --- Basic funtions ---
        /// <summary>
        /// This function returns a string containing the current version of h3d.
        /// </summary>
        /// <returns>The version string</returns>
        public static string getVersionString()
        {
            IntPtr ptr = NativeMethodsEngine.h3dGetVersionString();
            //Console.WriteLine(Marshal.PtrToStringAnsi(ptr));
            return Marshal.PtrToStringAnsi(ptr);
        }

        /// <summary>
        /// This function checks if a specified extension is contained in the DLL/shared object of the engine.
        /// </summary>
        /// <param name="extensionName">name of the extension</param>
        /// <returns>true if extension is implemented, otherwise false</returns>
        public static bool checkExtension(string extensionName)
        {
            return NativeMethodsEngine.h3dCheckExtension(extensionName);
        }

        /// <summary>
        /// Checks if an error occured.
        /// </summary>
        /// <remarks>
        /// This function checks if an error occured in a previous API function call. If an error
        /// flag is set, the function resets the flag and returns true. The function will solely
        /// report errors that originate from a violated precondition, like an invalid parameter
        /// that is passed to an API function. Errors that happen during the execution of a function,
        /// for example failure of initializing the engine due to a missing hardware feature, can
        /// be catched by checking the return value of the corresponding API function.
        /// More information about the error can be retrieved by checking the message queue,
        /// provided that the message level is set accordingly.
        /// </remarks>        
        /// <returns>true in there was an error, otherwise false</returns>
        public static bool getError()
        {
            return NativeMethodsEngine.h3dGetError();
        }

        /// <summary>
        /// This function initializes the graphics engine and makes it ready for use. It has to be the
		/// first call to the engine except for getVersionString. In order to successfully initialize
		/// the engine the calling application must provide a valid OpenGL context. The function can be
		/// called several times on different rendering contexts in order to initialize them.
        /// </summary>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool init()
        {
            if (getVersionString() != Resources.VersionString)
                throw new LibraryIncompatibleException(Resources.LibraryIncompatibleExceptionString);

            return NativeMethodsEngine.h3dInit();
        }

        /// <summary>
        /// This function releases the engine and frees all objects and associated memory. 
        /// It should be called when the application is destroyed.
        /// </summary>
        public static void release()
        {
            NativeMethodsEngine.h3dRelease();
        }

        /// <summary>
        ///    Gets the current viewport parameters.

        /// <summary>
        /// This is the main function of the engine. 
        /// It executes all the rendering, animation and other tasks. 
        /// The function can be called several times per frame, 
        /// for example in order to write to different
        /// output buffers.
        /// <param name="node">camera node used for rendering scene</param>
        /// <returns>true in case of success, otherwise false</returns>
        /// </summary>       
        public static void render(int node)
        {
            NativeMethodsEngine.h3dRender(node);
        }

        /// <summary>
        /// This function tells the engine that the current frame is finished and that all
        /// subsequent rendering operations will be for the next frame.
        /// </summary>
        /// <returns>true in case of success, otherwise false</returns>
        public static void finalizeFrame()
        {
            NativeMethodsEngine.h3dFinalizeFrame();
        }

        /// <summary>
        /// This function removes all nodes from the scene graph except the root node and releases all resources.
        /// Warning: All resource and node IDs are invalid after calling this function.         
        /// </summary>
        public static void clear()
        {
            NativeMethodsEngine.h3dClear();
        }


        // --- General functions ---

        /// <summary>
        /// This function returns the next message string from the message queue and writes additional information to the specified variables. If no message is left over in the queue an empty string is returned.
        /// </summary>
        /// <param name="level">pointer to variable for storing message level indicating importance (can be NULL)</param>
        /// <param name="time">pointer to variable for stroing time when message was added (can be NULL)</param>
        /// <returns>message string or empty string if no message is in queue</returns>
        public static string getMessage(out int level, out float time)
        {
            IntPtr ptr = NativeMethodsEngine.h3dGetMessage(out level, out time);
            return Marshal.PtrToStringAnsi(ptr);
        }

        /// <summary>
        /// This function gets a specified option parameter and returns its value.
        /// </summary>
        /// <param name="param">option parameter</param>
        /// <returns>current value of the specified option parameter</returns>
        public static float getOption(H3DOptions param)
        {
            return NativeMethodsEngine.h3dGetOption(param);
        }

        /// <summary>
        /// This function sets a specified option parameter to a specified value.
        /// </summary>
        /// <param name="param">option parameter</param>
        /// <param name="value">value of the option parameter</param>
        /// <returns>true if the option could be set to the specified value, otherwise false</returns>
        public static bool setOption(H3DOptions param, float value)
        {
            return NativeMethodsEngine.h3dSetOption((int)param, value);
        }

        /// <summary>
        /// Gets a statistic value of the engine.
        /// </summary>
        /// This function returns the value of the specified statistic. The reset flag makes it possible
        /// to reset the statistic value after reading.
        /// <param name="param">statistic parameter</param>
        /// <param name="reset">flag specifying whether statistic value should be reset</param>
        /// <returns>current value of the specified statistic parameter</returns>
        public static float getStat(H3DStats param, bool reset)
        {
            return NativeMethodsEngine.h3dGetStat((int)param, reset);
        }

        /// <summary>
        /// Displays overlays on the screen.
        /// </summary>
        /// <remarks>
        /// This function displays one or more overlays with a specified material and color.
        /// An overlay is a screen-space quad that can be used to render 2D GUI elements. The overlay coordinate
        /// system has its origin (0, 0) at the top-left corner of the screen and its maximum (aspect, 1)
        /// at the bottom-right corner. As the x coordinate of the maximum corresponds to the aspect ratio
        /// of the viewport, the size of overlays can always be the same, even when different screen formats
        /// (standard 4:3, widescreen 16:9, etc.) are used. Texture coordinates are using a system where the
        /// coordinates (0, 0) correspond to the lower left corner of the image.
        /// Overlays are drawn in the order in which they are pushed using this function. Overlays with
        /// the same state will be batched together, so it can make sense to group overlays that have the
        /// same material, color and flags in order to achieve best performance.
        /// Note that the overlays have to be removed manually using the function h3dClearOverlays.
        /// </remarks>
        /// <param name="verts">vertex data (x, y, u, v), interpreted as quads</param>
        /// <param name="vertCount">number of vertices (must be multiple of 4)</param>
        /// <param name="colR">red color value of the overlay that is set for the material's shader</param>
        /// <param name="colG">green color value of the overlay that is set for the material's shader</param>
        /// <param name="colB">blue color value of the overlay that is set for the material's shader</param>
        /// <param name="colA">alpha color value of the overlay that is set for the material's shader</param>
        /// <param name="material">material resource used for rendering</param>
        /// <param name="flags">overlay flags (reserved for future use)</param>
        public static void showOverlays(
                         float[] verts, int vertCount, 
                         float colR, float colG, float colB, float colA,
                         int materialRes, int flags )
        {
            NativeMethodsEngine.h3dShowOverlays(verts, vertCount, colR, colG, colB, colA, materialRes, flags);
        }

        /// <summary>
        /// Removes all overlays.
        /// </summary>
        /// <remarks>This function removes all overlays that were added with h3dShowOverlays.</remarks>
        public static void clearOverlays()
        {
            NativeMethodsEngine.h3dClearOverlays();
        }


        // --- Resource functions ---
        /// <summary>
        /// This function returns the type of a specified resource. 
        /// If the resource handle is invalid, the function returns the resource type 'Unknown'.
        /// </summary>
        /// <param name="res">handle to the resource whose type will be returned</param>
        /// <returns>type of the scene node</returns>
        public static int getResType(int res)
        {
            return NativeMethodsEngine.h3dGetResType(res);
        }

        /// <summary>
        /// Returns the name of a resource.
        /// </summary>
        /// This function returns a pointer to the name of a specified resource. If the resource handle
        /// is invalid, the function returns an empty string.        
        /// <param name="res">handle to the resource</param>
        /// <returns>name of the resource or empty string in case of failure</returns>
        public static string getResName(int res)
        {
            return Marshal.PtrToStringAnsi(NativeMethodsEngine.h3dGetResName(res));
        }

        /// <summary>
        /// This function searches the resource of the specified type and name and returns its handle. If
        /// the resource is not available in the resource manager a zero handle is returned.
        /// </summary>
        /// <param name="type">type of the resource</param>
        /// <param name="start">name of the resource</param>
        /// <returns></returns>
        public static int getNextResource(int type, int start)
        {
            return (int)NativeMethodsEngine.h3dGetNextResource(type, start);
        }

        /// <summary>
        /// This function searches the resource of the specified type and name and returns its handle. If the resource is not available in the resource manager a zero handle is returned.
        /// </summary>
        /// <remarks>
        /// The content path of the specified ResourceType is added automatically.
        /// </remarks>
        /// <param name="type">type of the resource</param>
        /// <param name="name">name of the resource</param>
        /// <returns>handle to the resource or 0 if not found</returns>
        public static int findResource(int type, string name)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);
            return (int)NativeMethodsEngine.h3dFindResource(type, name);
        }

        /// <summary>
        /// Finds a resource element with the specified property value.        
        /// </summary>
        /// <remarks>This function searches in a specified resource for the first element of the specified type that has the property with the specified name set to the specified search value.  If such element is found, its index is returned, otherwise the function returns -1.  All string comparisons done for the search are case-sensitive.</remarks>
        /// <param name="type">the resource to be accessed</param>
        /// <param name="elem">	element type</param>
        /// <param name="param">parameter name</param>
        /// <param name="value">parameter value to be searched for</param>
        /// <returns></returns>
        public static int findResourceElem(int type, int elem, int param, string value)
        {
            if (value == null)
            {
                throw new ArgumentNullException("name", Resources.StringNullExceptionString);
            }

            return (int)NativeMethodsEngine.h3dFindResElem(type, elem, param, value);
        }

        /// <summary>
        /// This function tries to add a resource of a specified type and name to the resource manager. 
        /// If a resource of the same type and name is already found, the handle to the existing resource is returned instead of creating a new one.
        /// </summary>
        /// <param name="type">type of the resource</param>
        /// <param name="name">name of the resource</param>
        /// <param name="flags">flags used for creating the resource</param>
        /// <returns>handle to the resource to be added or 0 in case of failure</returns>
        public static int addResource(int type, string name, int flags)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);

            return (int)NativeMethodsEngine.h3dAddResource(type, name, flags);
        }

        /// <summary>
        /// Duplicates a resource.
        /// </summary>
        /// This function duplicates a specified resource. In the cloning process a new resource with the
        /// specified name is added to the resource manager and filled with the data of the specified source
        /// resource. If the specified name for the new resource is already in use, the function fails and
        /// returns 0. If the name string is empty, a unique name for the resource is generated automatically.
        /// <remarks>*Note: The name string may not contain a colon character (:)*</remarks>
        /// <param name="sourceRes">handle to resource to be cloned</param>
        /// <param name="name">name of new resource (can be empty for auto-naming)</param>
        /// <returns>handle to the cloned resource or 0 in case of failure</returns>
        public static int cloneResource(int sourceRes, string name)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);
            return (int)NativeMethodsEngine.h3dCloneResource(sourceRes, name);
        }

        /// <summary>
        /// Removes a resource.
        /// </summary>
        /// This function decreases the user reference count of a specified resource. When the user reference
        /// count is zero and there are no internal references, the resource can be released and removed using
        /// the API fuction releaseUnusedResources.
        /// <param name="res">handle to the resource to be removed</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static int removeResource(int res)
        {
            return NativeMethodsEngine.h3dRemoveResource(res);
        }

        /// <summary>
        /// Checks if a resource is loaded.
        /// </summary>
        /// This function checks if the specified resource has been successfully loaded.
        /// <param name="res">handle to the resource to be checked</param>
        /// <returns>true if resource is loaded, otherwise or in case of failure false</returns>
        public static bool isResLoaded(int res)
        {
            return NativeMethodsEngine.h3dIsResLoaded(res);
        }

        /// <summary>
        /// Loads a resource.
        /// </summary>
        /// <remarks>
        /// This function loads data for a resource that was previously added to the resource manager.
		/// If data is a NULL-pointer the resource manager is told that the resource doesn't have any data
		/// (e.g. the corresponding file was not found). In this case, the resource remains in the unloaded state
		/// but is no more returned when querying unloaded resources. When the specified resource is already loaded,
        /// the function returns false.
        /// 
        /// *Important Note: XML-data must be NULL-terminated*
        /// </remarks>
        /// <param name="name">res handle to the resource for which data will be loaded</param>
        /// <param name="data">the data to be loaded</param>
        /// <param name="size">size of the data block</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool loadResource(int res, byte[] data, int size)
        {            
            if (data == null) throw new ArgumentNullException("data");

            if (data.Length < size)
                throw new ArgumentException(Resources.LoadResourceArgumentExceptionString, "data");

            // allocate memory for resource data
            IntPtr ptr = Marshal.AllocHGlobal(size + 1);

            // copy byte data into allocated memory
            Marshal.Copy(data, 0, ptr, size);

            // terminate data block
            Marshal.WriteByte(ptr, size, 0x00);

            // load resource
            bool result = NativeMethodsEngine.h3dLoadResource(res, ptr, size);

            // free previously allocated memory
            Marshal.FreeHGlobal(ptr);

            return result;
        }

        /// <summary>
        /// This function unloads a previously loaded resource and restores the default values it had before loading. The state is set back to unloaded which makes it possible to load the resource again.
        /// </summary>
        /// <param name="res">handle to resource to be unloaded</param>        
        public static void unloadResource(int res)
        {
            NativeMethodsEngine.h3dUnloadResource((int)res);
        }

        /// <summary>
        /// Gets the number of elements in a resource.
        /// </summary>
        /// This function returns how many elements of the specified element type a specified
		/// resource has.
        /// <param name="res">handle to the resource to be accessed</param>
        /// <param name="elem">element type</param>
        /// <returns>number of elements</returns>
        public static int getResElemCount(int res, int elem)
        {
            return NativeMethodsEngine.h3dGetResElemCount(res, elem);
        }

        /// <summary>
        /// Finds a resource element with the specified property value.        
        /// </summary>
        /// <remarks>
        /// This function searches in a specified resource for the first element of the specified
        /// type that has the property with the specified name set to the specified search value.
        /// If such element is found, its index is returned, otherwise the function returns -1. All
        /// string comparisons done for the search are case-sensitive.
        /// </remarks>        
        /// <param name="res">handle to the resource to be accessed</param>
        /// <param name="elem">element type</param>
        /// <param name="param">parameter name</param>
        /// <param name="value">parameter value to be searched for</param>
        /// <returns>index of element or -1 if element not found</returns>
        public static int findResElem(int res, int elem, int param, string value)
        {
            return NativeMethodsEngine.h3dFindResElem(res, elem, param, value);
        }


        /// <summary>
        /// Gets a property of a resource.
        /// </summary>
        /// This function returns a specified property of the specified resource.
		/// The property must be of the type int.
        /// <param name="res">handle to the resource to be accessed</param>
        /// <param name="elem">element type</param>
        /// <param name="elemIdx">index of element</param>
        /// <param name="param">parameter to be accessed</param>
        /// <returns>value of the parameter</returns>
        public static int getResParamI(int res, int elem, int elemIdx, int param)
        {
            return NativeMethodsEngine.h3dGetResParamI(res, elem, elemIdx, param);
        }

        /// <summary>
        /// Sets a property of a resource.
        /// </summary>
        /// This function sets a specified property of the specified resource to a specified value.
        /// The property must be of the type int.
        /// <param name="res">handle to the node to be modified</param>
        /// <param name="elem">element type</param>
        /// <param name="elemIdx">index of element</param>
        /// <param name="param">parameter to be modified</param>
        /// <param name="value">new value for the specified parameter</param>        
        public static void setResParamI(int res, int elem, int elemIdx, int param, int value)
        {
            NativeMethodsEngine.h3dSetResParamI(res, elem, elemIdx, param, value);
        }

        /// <summary>
        /// Gets a property of a resource.
        /// </summary>
        /// This function returns a specified property of the specified resource.
        /// The property must be of the type float.
        /// <param name="res">handle to the resource to be accessed</param>        
        /// <param name="elem">element type</param>
        /// <param name="elemIdx">index of element</param>
        /// <param name="param">parameter to be accessed</param>
        /// <param name="compIdx">component of the parameter to be accessed</param>
        /// <returns>value of the parameter</returns>
        public static float getResParamF(int res, int elem, int elemIdx, int param, int compIdx)
        {
            return NativeMethodsEngine.h3dGetResParamF(res, elem, elemIdx, param, compIdx);
        }

        /// <summary>
        /// Sets a property of a resource.
        /// </summary>
        /// This function sets a specified property of the specified resource to a specified value.
        /// The property must be of the type float.
        /// <param name="res">handle to the node to be modified</param>
        /// <param name="elem">element type</param>
        /// <param name="elemIdx">index of element</param>
        /// <param name="compIdx">component of the parameter to be accessed</param>
        /// <param name="param">parameter to be modified</param>
        /// <param name="value">new value for the specified parameter</param>        
        public static void setResParamF(int res, int elem, int elemIdx, int param, int compIdx, float value)
        {
            NativeMethodsEngine.h3dSetResParamF(res, elem, elemIdx, param, compIdx, value);
        }

        /// <summary>
        /// Gets a property of a resource.
        /// </summary>
        /// This function returns a specified property of the specified resource.
        /// The property must be of the type string (const char *).
        ///         
        /// <param name="res">handle to the resource to be accessed</param>
        /// <param name="elem">element type</param>
        /// <param name="elemIdx">index of element</param>       
        /// <param name="param">parameter to be accessed</param>
        /// <returns>value of the property or empty string if no such property exists</returns>
        public static string getResParamStr(int res, int elem, int elemIdx, int param)
        {
            return Marshal.PtrToStringAnsi(NativeMethodsEngine.h3dGetResParamStr(res, elem, elemIdx, param));
        }

        /// <summary>
        /// Sets a property of a resource.
        /// </summary>
        /// This function sets a specified property of the specified resource to a specified value.
        /// The property must be of the type string (const char *).
        /// <param name="res">handle to the node to be modified</param>
        /// <param name="elem">element type</param>
        /// <param name="elemIdx">index of element</param>
        /// <param name="param">parameter to be modified</param>
        /// <param name="value">new value for the specified parameter</param>
        public static void setResParamStr(int res, int elem, int elemIdx, int param, string value)
        {
            NativeMethodsEngine.h3dSetResParamStr(res, elem, elemIdx, param, value);
        }

        /// <summary>
        /// Maps the stream of a resource element.
        /// </summary>
        /// This function maps the specified stream of a specified resource element and returns a
        /// pointer to the stream data. The required access to the data can be specified with the read
        /// write parameters. If read is false, the pointer will usually not contain meaningful data.
        /// Not all resource streams can be mapped with both read and write access. If it is not
        /// possible to map the stream, the function will return a NULL pointer. A mapped stream should
        /// be unmapped again as soon as possible but always before subsequent API calls are made. It
        /// is only possible to map one stream per resource at a time.
        /// <param name="res">handle to the resource to be mapped</param>
        /// <param name="elem">element type</param>
        /// <param name="elemIdx">index of element</param>
        /// <param name="stream">stream to be mapped</param>
        /// <param name="read">flag indicating whether read access is required</param>
        /// <param name="write">flag indicating whether write access is required</param>
        /// <returns>pointer to stream data or NULL if stream cannot be mapped</returns>
        public static IntPtr mapResStream(int res, int elem, int elemIdx, int stream, bool read, bool write)
        {
            return NativeMethodsEngine.h3dMapResStream(res, elem, elemIdx, stream, read, write);
        }

        /// <summary>
        /// Unmaps a previously mapped resource stream.
        /// </summary>
        /// This function unmaps a resource stream that has been mapped before.
        /// <param name="res">handle to the resource to be unmapped</param>        
        public static void unmapResStream(int res)
        {
            NativeMethodsEngine.h3dUnmapResStream(res);
        }

        /// <summary>
        /// Returns handle to an unloaded resource.
        /// </summary>
        /// This function looks for a resource that is not yet loaded and returns its handle.
        /// If there are no unloaded resources or the zero based index specified is greater than the number
        /// of the currently unloaded resources, 0 is returned.
        /// <param name="index">index of unloaded resource within the internal list of unloaded resources (starting with 0) </param>
        /// <returns>handle to an unloaded resource or 0</returns>
        public static int queryUnloadedResource(int index)
        {
            return NativeMethodsEngine.h3dQueryUnloadedResource(index);            
        }

        /// <summary>
        /// This function releases resources that are no longer used. 
        /// Unused resources were either told to be released by the user calling removeResource() or are no more referenced by any other engine objects.
        /// </summary>
        public static void releaseUnusedResources()
        {
            NativeMethodsEngine.h3dReleaseUnusedResources();
        }

        /// <summary>
        /// Adds a Texture2D resource.
        /// </summary>
        /// This function tries to create and add a Texture resource with the specified name to the resource
        /// manager. If a Texture resource with the same name is already existing, the function fails. The
        /// texture is initialized with the specified dimensions and the resource is declared as loaded. This
        /// function is especially useful to create dynamic textures (e.g. for displaying videos) or output buffers
        /// for render-to-texture.
        /// <remarks>*Note: The name string may not contain a colon character (:)*</remarks>
        /// <param name="name">name of the resource</param>
        /// <param name="width">width of the texture image</param>
        /// <param name="height">height of the texture image</param>
        /// <param name="fmt">texture format (see stream formats)</param>
        /// <param name="flags">flags used for creating the resource</param>
        /// <returns>handle to the created resource or 0 in case of failure</returns>
        public static int createTexture(string name, int width, int height, int fmt, int flags)
        {
            return NativeMethodsEngine.h3dCreateTexture(name, width, height, fmt, flags);
        }

        /// <summary>
        /// Sets preambles of all Shader resources.
        /// </summary>
        /// This function defines a header that is inserted at the beginning of all shaders. The preamble
        /// is used when a shader is compiled, so changing it will not affect any shaders that are already
        /// compiled. The preamble is useful for setting platform-specific defines that can be employed for
        /// creating several shader code paths, e.g. for supporting different hardware capabilities.
        /// <param name="vertPreamble">preamble text of vertex shaders (default: empty string)</param>
        /// <param name="fragPreamble">preamble text of fragment shaders (default: empty string)</param>
        public static void setShaderPreambles(string vertPreamble, string fragPreamble)
        {
            NativeMethodsEngine.h3dSetShaderPreambles(vertPreamble, fragPreamble);
        }

        // Material specific
        /// <summary>
        /// This function sets the specified shader uniform of the specified material to the specified values.
        /// </summary>
        /// <param name="matRes">handle to the Material resource to be accessed</param>
        /// <param name="name">name of the uniform as defined in Material resource</param>
        /// <param name="a">value of first component</param>
        /// <param name="b">value of second component</param>
        /// <param name="c">value of third component</param>
        /// <param name="d">value of fourth component</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool setMaterialUniform(int materialRes, string name, float a, float b, float c, float d)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);

            return NativeMethodsEngine.h3dSetMaterialUniform(materialRes, name, a, b, c, d);
        }

        /// <summary>
        /// This function sets the base width and height which affects render targets with relative (in percent) size 
        /// specification. Changing the base size is usually desired after engine initialization and when the window
        /// is being resized. Note that in case several cameras use the same pipeline resource instance, the change
        /// will affect all cameras.
        /// </summary>
        /// <param name="pipeRes">the pipeline resource instance to be changed</param>        
        /// <param name="width">base width in pixels used for render targets with relative size</param>
        /// <param name="height">base height in pixels used for render targets with relative size</param>
        public static void resizePipelineBuffers(int pipeRes, int width, int height)
        {
            NativeMethodsEngine.h3dResizePipelineBuffers(pipeRes, width, height);
        }

        /// <summary>
        /// Reads the pixel data of a pipeline render target buffer.
        /// </summary>
        /// This function reads the pixels of the specified buffer of the specified render target from the specified
        /// pipeline resource and stores it in the specified float array. To calculate the size required for the array this
        /// function can be called with a NULL pointer for dataBuffer and pointers to variables where the width,
        /// height and number of (color) components (e.g. 4 for RGBA or 1 for depth) will be stored.
        /// The function is not intended to be used for real-time scene rendering but rather as a tool for debugging.
        /// For more information about the render buffers please refer to the Pipeline documentation.
        /// <param name="pipelineRes">handle to pipeline resource</param>
        /// <param name="targetName">unique name of render target to access</param>
        /// <param name="bufIndex">index of buffer to be accessed</param>
        /// <param name="width">pointer to variable where the width of the buffer will be stored (can be NULL)</param>
        /// <param name="height">pointer to variable where the height of the buffer will be stored (can be NULL</param>
        /// <param name="compCount">pointer to variable where the number of components will be stored (can be NULL)</param>
        /// <param name="dataBuffer">pointer to float array where the pixel data will be stored (can be NULL)</param>
        /// <param name="bufferSize">size of dataBuffer array in bytes</param>
        /// <returns></returns>
        //public static bool getPipelineRenderTargetData(int pipelineRes, string targetName, int bufIndex,
        //    out int width, out int height, out int compCount, float[] dataBuffer, int bufferSize)
        //{
            
        //}

        // SceneGraph functions
        /// <summary>
        /// This function returns the type of a specified scene node. 
        /// If the node handle is invalid, the function returns the node type 'Unknown'.
        /// </summary>
        /// <param name="node">handle to the scene node whose type will be returned</param>
        /// <returns>type of the scene node</returns>
        public static H3DNodeTypes getNodeType(int node)
        {
            return NativeMethodsEngine.h3dGetNodeType(node);
        }

        /// <summary>
        /// Returns the parent of a scene node.
        /// </summary>
        /// This function returns the handle to the parent node of a specified scene node. If the specified
        /// node handle is invalid or the root node, 0 is returned.
        /// <param name="node">handle to the scene node</param>
        /// <returns>handle to parent node or 0 in case of failure</returns>
        public static int getNodeParent(int node)
        {
            return NativeMethodsEngine.h3dGetNodeParent(node);
        }

        /// <summary>
        /// Relocates a node in the scene graph.
        /// </summary>
        /// This function relocates a scene node. It detaches the node from its current parent and attaches
        /// it to the specified new parent node. If the attachment to the new parent is not possible, the
        /// function returns false. Relocation is not possible for the H3DRootNode.
        /// <param name="node">handle to the scene node to be relocated</param>
        /// <param name="parent">handle to the new parent node</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool setNodeParent(int node, int parent)
        {
            return NativeMethodsEngine.h3dSetNodeParent(node, parent);
        }

        /// <summary>
        /// Returns the handle to a child node.
        /// </summary>
        /// 
        /// This function looks for the n-th (index) child node of a specified node and returns its handle. If the child
        /// doesn't exist, the function returns 0.
        /// <param name="node">handle to the parent node</param>
        /// <param name="index">index of the child node</param>
        /// <returns>handle to the child node or 0 if child doesn't exist</returns>
        public static int getNodeChild(int node, int index)
        {
            return NativeMethodsEngine.h3dGetNodeChild(node, index);
        }

        /// <summary>
        /// This function creates several new nodes as described in a SceneGraph resource and attaches them to a specified parent node.
        /// </summary>
        /// <param name="parent">handle to parent node to which the root of the new nodes will be attached</param>
        /// <param name="res">handle to the SceneGraph resource</param>
        /// <returns>handle to the root of the created nodes or 0 in case of failure</returns>
        public static int addNodes(int parent, int res)
        {
            return (int)NativeMethodsEngine.h3dAddNodes(parent, res);
        }

        /// <summary>
        /// This function removes the specified node and all of it's children from the scene.
        /// </summary>
        /// <param name="node">handle to the node to be removed</param>        
        public static void removeNode(int node)
        {
            NativeMethodsEngine.h3dRemoveNode(node);
        }

        /// <summary>
        /// Checks if a scene node has been transformed by the engine.
        /// </summary>
        /// This function checks if a scene node has been transformed by the engine since the last
        /// time the transformation flag was reset. Therefore, it stores a flag that is set to true when a
        /// setTransformation function is called explicitely by the application or when the node transformation
        /// has been updated by the animation system. The function also makes it possible to reset the
        /// transformation flag.
        /// <param name="node">handle to the node to be accessed</param>
        /// <param name="reset">flag indicating whether transformation flag shall be reset</param>
        /// <returns>true if node has been transformed, otherwise false</returns>
        public static bool checkNodeTransFlag(int node, bool reset)
        {
            return NativeMethodsEngine.h3dCheckNodeTransFlag(node, reset);
        }

        /// <summary>
        /// This function gets the translation, rotation and scale of a specified scene node object.
        /// The coordinates are in local space and contain the transformation of the node relative to its parent.
        /// </summary>
        /// <param name="node">handle to the node which will be accessed</param>
        /// <param name="px">x variable where position of the node will be stored</param>
        /// <param name="py">y variable where position of the node will be stored</param>
        /// <param name="pz">z variable where position of the node will be stored</param>
        /// <param name="rx">x variable where rotation of the node in Euler angles (degrees) will be stored</param>
        /// <param name="ry">y variable where rotation of the node in Euler angles (degrees) will be stored</param>
        /// <param name="rz">z variable where rotation of the node in Euler angles (degrees) will be stored</param>
        /// <param name="sx">x variable where scale of the node will be stored</param>
        /// <param name="sy">y variable where scale of the node will be stored</param>
        /// <param name="sz">z variable where scale of the node will be stored</param>        
        public static void getNodeTransform(int node, out float px, out float py, out float pz,
                                out float rx, out float ry, out float rz, out float sx, out float sy, out float sz)
        {
            NativeMethodsEngine.h3dGetNodeTransform(node, out px, out py, out pz, out rx, out ry, out rz, out sx, out sy, out sz);
        }

        /// <summary>
        /// This function sets the relative translation, rotation and scale of a specified scene node object.
        /// The coordinates are in local space and contain the transformation of the node relative to its parent.
        /// </summary>
        /// <param name="node">handle to the node which will be modified</param>
        /// <param name="px">x position of the node</param>
        /// <param name="py">y position of the node</param>
        /// <param name="pz">z position of the node</param>
        /// <param name="rx">x rotation of the node in Euler angles (degrees)</param>
        /// <param name="ry">y rotation of the node in Euler angles (degrees)</param>
        /// <param name="rz">z rotation of the node in Euler angles (degrees)</param>
        /// <param name="sx">x scale of the node</param>
        /// <param name="sy">y scale of the node</param>
        /// <param name="sz">z scale of the node</param>        
        public static void setNodeTransform(int node, float px, float py, float pz,
                                float rx, float ry, float rz, float sx, float sy, float sz)
        {
            NativeMethodsEngine.h3dSetNodeTransform(node, px, py, pz, rx, ry, rz, sx, sy, sz);
        }

        /// <summary>
        /// This function returns a pointer to the relative and absolute transformation matrices of the specified node in the specified pointer variables.
        /// </summary>
        /// <param name="node">handle to the scene node whose matrices will be accessed</param>
        /// <param name="relMat">pointer to a variable where the address of the relative transformation matrix will be stored</param>
        /// <param name="absMat">pointer to a variable where the address of the absolute transformation matrix will be stored</param>        
        public static void getNodeTransMats(int node, out IntPtr relMat, out IntPtr absMat)
        {
            NativeMethodsEngine.h3dGetNodeTransMats(node, out relMat, out absMat);
        }

        /// <summary>
        /// This function sets the relative transformation matrix of the specified scene node. 
        /// It is basically the same as setNodeTransform but takes directly a matrix instead of individual transformation parameters.
        /// </summary>
        /// <param name="node">handle to the scene node whose matrix will be updated</param>
        /// <param name="mat4x4">array of a 4x4 matrix in column major order</param>        
        public static void setNodeTransMat(int node, float[] mat4x4)
        {
            if (mat4x4.Length != 16) throw new ArgumentOutOfRangeException("mat4x4", Resources.MatrixOutOfRangeExceptionString);

            NativeMethodsEngine.h3dSetNodeTransMat(node, mat4x4);
        }
   
        /// <summary>
        /// Gets a property of a scene node.
        /// </summary>
        /// <remarks>This function returns a specified property of the specified node.
		/// The property must be of the type int or H3DRes.</remarks>
        /// <param name="node">handle to the node to be accessed</param>
        /// <param name="param">parameter to be accessed</param>
        /// <returns></returns>
        public static int getNodeParamI(int node, int param)
        {
            return NativeMethodsEngine.h3dGetNodeParamI(node, param);
        }

        /// <summary>
        /// Sets a property of a scene node.
        /// </summary>
        /// <remarks>This function sets a specified property of the specified node to a specified value.
        /// The property must be of the type int or ResHandle.</remarks>
        /// <param name="node">handle to the node to be modified</param>
        /// <param name="param">parameter to be modified</param>
        /// <param name="value">new value for the specified parameter</param>        
        public static void setNodeParamI(int node, int param, int value)
        {
            NativeMethodsEngine.h3dSetNodeParamI(node, param, value);
        }
        /// <summary>
        /// Gets a property of a scene node.
        /// </summary>
        /// <remarks>This function returns a specified property of the specified node. Float properties can have
		/// several indexed components, for example the x, y and z components of a vector.
        /// The property must be of the type float.</remarks>
        /// <param name="node">handle to the node to be accessed</param>
        /// <param name="param">parameter to be accessed</param>
        /// <param name="compIdx">component of the parameter to be accessed</param>
        /// <returns>value of the parameter</returns>
        public static float getNodeParamF(int node, int param, int compIdx)
        {
            return NativeMethodsEngine.h3dGetNodeParamF(node, param, compIdx);
        }

        /// <summary>
        /// Sets a property of a scene node.
        /// </summary>
        /// <remarks>This function sets a specified property of the specified node to a specified value.
		/// The property must be of the type float.</remarks>
        /// <param name="node">handle to the node to be modified</param>
        /// <param name="param">parameter to be modified</param>
        /// <param name="compIdx>component of the parameter to be modified</param>
        /// <param name="value">new value for the specified parameter</param>        
        public static void setNodeParamF(int node, int param, int compIdx, float value)
        {
            NativeMethodsEngine.h3dSetNodeParamF(node, param, compIdx, value);
        }

        /// <summary>
        /// Gets a property of a scene node.
        /// </summary>
        /// This function returns a specified property of the specified node.
        /// The property must be of the type string (const char *).
        /// <param name="node">handle to the node to be accessed</param>
        /// <param name="param">parameter to be accessed</param>
        /// <returns>value of the property or empty string if no such property exists</returns>
        public static string getNodeParamStr(int node, int param)
        {
            return Marshal.PtrToStringAnsi(NativeMethodsEngine.h3dGetNodeParamStr(node, param));
        }

        /// <summary>
        /// Sets a property of a scene node.
        /// </summary>
        /// This function sets a specified property of the specified node to a specified value.
        /// The property must be of the type string.
        /// 
        /// <param name="node">handle to the node to be modified</param>
        /// <param name="param">parameter to be modified</param>
        /// <param name="value">new value for the specified parameter</param>        
        public static void setNodeParamStr(int node, int param, string value)
        {
            if( value == null) throw new ArgumentNullException("value", Resources.StringNullExceptionString);
            NativeMethodsEngine.h3dSetNodeParamStr(node, param, value);
        }

        /// <summary>
        /// Gets the scene node flags.
        /// </summary>
        /// This function returns a bit mask containing the set scene node flags.
        /// <param name="node">handle to the node to be accessed</param>
        /// <returns>flag bitmask</returns>
        public static int getNodeFlags( int node )
        {
            return NativeMethodsEngine.h3dGetNodeFlags( node );
        }

        /// <summary>
		///  Sets the scene node flags.
        /// </summary>
        /// This function sets the flags of the specified scene node.
        /// <param name="node">handle to the node to be modified</param>
        /// <param name="flags">new flag bitmask</param>
        /// <param name="recursive">specifies whether flags should be applied recursively to all child nodes</param>
        public static void setNodeFlags(int node, int flags, bool recursive)
        {
            NativeMethodsEngine.h3dSetNodeFlags(node, flags, recursive);
        }


        /// <summary>
        /// This function stores the world coordinates of the axis aligned bounding box of a specified node in the specified variables. 
        /// The bounding box is represented using the minimum and maximum coordinates on all three axes. 
        /// </summary>
        /// <param name="node">handle to the node which will be accessed</param>
        /// <param name="minX">variable where minimum x-coordinates will be stored</param>
        /// <param name="minY">variable where minimum y-coordinates will be stored</param>
        /// <param name="minZ">variable where minimum z-coordinates will be stored</param>
        /// <param name="maxX">variable where maximum x-coordinates will be stored</param>
        /// <param name="maxY">variable where maximum y-coordinates will be stored</param>
        /// <param name="maxZ">variable where maximum z-coordinates will be stored</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool getNodeAABB(int node, out float minX, out float minY, out float minZ, out float maxX, out float maxY, out float maxZ)
        {
            return NativeMethodsEngine.h3dGetNodeAABB(node, out minX, out minY, out minZ, out maxX, out maxY, out maxZ);
        }

        // added (h3d 1.0)
        /// <summary>
        /// Finds scene nodes with the specified properties.
        /// </summary>
        /// <remarks> This function loops recursively over all children of startNode and 
        /// adds them to an internal list of results if they match the specified name and type. 
        /// The result list is cleared each time this function is called. The function returns the 
        /// number of nodes which were found and added to the list.</remarks>
        /// <param name="node"></param>
        /// <param name="name"></param>
        /// <param name="type"></param>
        /// <returns></returns>
        public static int findNodes(int node, string name, int type)
        {
            return NativeMethodsEngine.h3dFindNodes(node, name, type);
        }

        /// <summary>
        /// Gets a result from the findNodes query.
        /// </summary>
        /// <remarks>This function returns the n-th (index) result of a previous findNodes query. The result is the handle
        /// to a scene node with the poperties specified at the findNodes query. If the index doesn't exist in the
        /// result list the function returns 0.</remarks>
        /// <param name="index">index of search result</param>
        /// <returns>handle to scene node from findNodes query or 0 if result doesn't exist</returns>
        public static int getNodeFindResult(int index)
        {
            return NativeMethodsEngine.h3dGetNodeFindResult(index);
        }

        /// <summary>
        /// Sets per-instance uniform data for a node.
        /// </summary>
        /// <remarks>
		/// This function sets the custom per-instance uniform data for a node that can be accessed
		/// from within a shader. The specified number of floats is copied from the specified memory location.
		/// Currently only Model nodes will store this data.</remarks>
        /// <param name="node">node for which data will be set</param>
        /// <param name="uniformData">pointer to float array</param>
        /// <param name="count">number of floats to be copied</param>
        /// <returns></returns>
        public static void setNodeUniforms(int node, float[] uniformData, int count)
        {
            NativeMethodsEngine.h3dSetNodeUniforms(node, uniformData, count);
        }
        
        /// <summary>
        /// This function checks recursively if the specified ray intersects the specified node or one of its children.
        /// The function finds intersections relative to the ray origin and returns the number of intersecting scene
        /// nodes. The ray is a line segment and is specified by a starting point (the origin) and a finite direction
        /// vector which also defines its length. Currently this function is limited to returning intersections with Meshes.        
        /// </summary>
        /// <param name="node">node at which intersection check is beginning</param>
        /// <param name="ox">ray origin</param>
        /// <param name="oy">ray origin</param>
        /// <param name="oz">ray origin</param>
        /// <param name="dx">ray direction vector also specifying ray length</param>
        /// <param name="dy">ray direction vector also specifying ray length</param>
        /// <param name="dz">ray direction vector also specifying ray length</param>
        /// <param name="numNearest">maximum number of intersection points to be stored (0 for all)</param>
        /// <returns>handle to nearest intersected node or 0 if no node was hit</returns>
        public static int castRay(int node, float ox, float oy, float oz, float dx, float dy, float dz, int numNearest)
        {
            return NativeMethodsEngine.h3dCastRay(node, ox, oy, oz, dx, dy, dz, numNearest);
        }


        /// <summary>
        /// Returns a result of a previous castRay query.
        /// </summary>
        /// This functions is used to access the results of a previous castRay query. The index is used to access
        /// a specific result. The intersection data is copied to the specified variables.
        /// <param name="index">index of result to be accessed (range: 0 to number of results returned by castRay)</param>
        /// <param name="node">handle of intersected node</param>
        /// <param name="distance">distance from ray origin to intersection point</param>
        /// <param name="intersection">coordinates of intersection point (float[3] array)</param>
        /// <returns></returns>
        public static bool getCastRayResult(int index, out int node, out float distance, float[] intersection)
        {
            return NativeMethodsEngine.h3dGetCastRayResult(index, out node, out distance, intersection);
        }

        /// <summary>
        /// Checks if a node is visible.
        /// </summary>
        /// <remarks>This function checks if a specified node is visible from the perspective of a specified
        /// camera. The function always checks if the node is in the camera's frustum. If checkOcclusion
        /// is true, the function will take into account the occlusion culling information from the previous
        /// frame (if occlusion culling is disabled the flag is ignored). The flag calcLod determines whether the
        /// detail level for the node should be returned in case it is visible. The function returns -1 if the node
        /// is not visible, otherwise 0 (base LOD level) or the computed LOD level.</remarks>
        /// <param name="node">node to be checked for visibility</param>
        /// <param name="cameraNode">camera node from which the visibility test is done</param>
        /// <param name="checkOcclusion">specifies if occlusion info from previous frame should be taken into account</param>
        /// <param name="calcLod">specifies if LOD level should be computed</param>
        /// <returns>computed LOD level or -1 if node is not visible</returns>
        public static int checkNodeVisibility(int node, int cameraNode, bool checkOcclusion, bool calcLod)
        {
            return NativeMethodsEngine.h3dCheckNodeVisibility(node, cameraNode, checkOcclusion, calcLod);
        }

        // Group specific
        /// <summary>
        /// This function creates a new Group node and attaches it to the specified parent node.
        /// </summary>
        /// <param name="parent">handle to parent node to which the new node will be attached</param>
        /// <param name="name">name of the node</param>
        /// <returns>handle to the created node or 0 in case of failure</returns>
        public static int addGroupNode(int parent, string name)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);

            return (int)NativeMethodsEngine.h3dAddGroupNode(parent, name);
        }


        // Model specific
        /// <summary>
        /// This function creates a new Model node and attaches it to the specified parent node.
        /// </summary>
        /// <param name="parent">handle to parent node to which the new node will be attached</param>
        /// <param name="name">name of the node</param>
        /// <param name="geoRes">Geometry resource used by Model node</param>
        /// <returns>handle to the created node or 0 in case of failure</returns>
        public static int addModelNode(int parent, string name, int geoRes)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);

            return (int)NativeMethodsEngine.h3dAddModelNode(parent, name, geoRes);
        }

        /// <summary>
        /// This function is used to setup the specified animation stage/slot of the specified Model node.
        /// </summary>
        /// <remarks>
        /// The function can be used for animation blending and mixing. There is a fixed number of stages
        /// (by default 16) on which different animations can be played. The start node determines the first node
        /// (Joint or Mesh) to which the animation is recursively applied. If the start node is an empty string, the
        /// animation affects all animatable nodes (Joints and Meshes) of the model. If a NULL-handle is used for
        /// animationRes, the stage is cleared and the currently set animation is removed.
        ///
        /// The layer determines the priority of the animation and how the weights are distributed. See
        /// h3dSetModelAnimParams for more information.
        ///		
        /// A simple way to do animation mixing is using additive animations. If a stage is configured to be
        /// additive, the engine calculates the difference between the current frame and the first frame in the
        /// animation and adds this delta to the current transformation of the joints or meshes. Additive animations
        /// ignore the weight and layer settings. They are only applied by the engine if a non-additive animation
        /// is assigned to the model as well.        
        /// </remarks>
        /// <param name="node">handle to the Model node to be modified</param>
        /// <param name="stage">index of the animation stage to be configured</param>
        /// <param name="animationRes">handle to Animation resource (can be 0)</param>
        /// <param name="layer">layer id</param>
        /// <param name="startNode">name of first node to which animation shall be applied (or empty string)</param>
        /// <param name="additive">flag indicating whether stage is additive</param>        
        public static void setupModelAnimStage(int node, int stage, int animationRes, int layer, string startNode, bool additive)
        {
            if (startNode == null) throw new ArgumentNullException("startNode", Resources.StringNullExceptionString);

            NativeMethodsEngine.h3dSetupModelAnimStage(node, stage, animationRes, layer, startNode, additive);
        }

        /// <summary>
        /// This function gets the current animation time and weight for a specified stage of the specified model.</summary>
        /// <remarks>
        /// The time corresponds to the frames of the animation and the animation is looped if the time is higher than the maximum number of frames in the Animation resource. 
        /// The weight is used for animation blending and determines how much influence the stage has compared to the other active stages.
        /// </remarks>
        /// <param name="node">handle to the node to be accessed</param>
        /// <param name="stage">index of the animation stage to be accessed</param>
        /// <param name="time">variable where the time of the animation stage will be stored</param>
        /// <param name="weight">variable where the blend weight of the animation stage will be stored</param>        
        public static void getModelAnimParams(int node, int stage, out float time, out float weight)
        {
            NativeMethodsEngine.h3dGetModelAnimParams(node, stage, out time, out weight);
        }

        /// <summary>
        /// This function sets the current animation time and weight for a specified stage of the specified model.</summary>
        /// <remarks>
        /// The time corresponds to the frames of the animation and the animation is looped if the time is higher than the maximum number of frames in the Animation resource. 
        /// The weight is used for animation blending and determines how much influence the stage has compared to the other active stages. 
        /// When the sum of the weights of all stages is more than one, the animations on the lower stages get priority. 
        /// The function operates on Model nodes but accepts also Group nodes in which case the call is passed recursively to the Model child nodes.
        /// </remarks>
        /// <param name="node">handle to the node to be modified</param>
        /// <param name="stage">index of the animation stage to be modified</param>
        /// <param name="time">new animation time</param>
        /// <param name="weight">new animation weight</param>        
        public static void setModelAnimParams(int node, int stage, float time, float weight)
        {
            NativeMethodsEngine.h3dSetModelAnimParams(node, stage, time, weight);
        }

        /// <summary>
        /// This function sets the weight of a specified morph target. If the target parameter is an empty string the weight of all morph targets in the specified Model node is modified. The function operates on Model nodes but accepts also Group nodes in which case the call is passed recursively to the Model child nodes. If the specified morph target is not found the function returns false.
        /// </summary>
        /// <param name="node">handle to the node to be modified</param>
        /// <param name="target">name of morph target</param>
        /// <param name="weight">new weight for morph target</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool setModelMorpher(int node, string target, float weight)
        {
            if (target == null) throw new ArgumentNullException("target", Resources.StringNullExceptionString);

            return NativeMethodsEngine.h3dSetModelMorpher(node, target, weight);
        }
        

        /// <summary>
        /// Applies animation and/or geometry updates.
        /// <remarks>
		/// This function applies skeletal animation and geometry updates to the specified model, depending on
		/// the specified update flags. Geometry updates include morph targets and software skinning if enabled.
		/// If the animation or morpher parameters did not change, the function returns immediately. This function
		/// has to be called so that changed animation or morpher parameters will take effect.
        /// </remarks>
        /// <param name="modelNode">handle to the Model node to be updated</param>
        /// <param name="flags">combination of H3DModelUpdateFlags flags</param>
        public static void updateModel(int modelNode, int flags)
        {
            NativeMethodsEngine.h3dUpdateModel(modelNode, flags);
        }



        // Mesh specific
        /// <summary>
        /// This function creates a new Mesh node and attaches it to the specified parent node.
        /// </summary>
        /// <param name="parent">handle to parent node to which the new node will be attached</param>
        /// <param name="name">name of the node</param>
        /// <param name="matRes">Material resource used by Mesh node</param>
        /// <param name="batchStart">first vertex index in Geometry resource of parent Model node</param>
        /// <param name="batchCount">number of vertex indices in Geometry resource of parent Model node</param>
        /// <param name="vertRStart">minimum vertex array index contained in Geometry resource indices of parent Model node</param>
        /// <param name="vertREnd">maximum vertex array index contained in Geometry resource indices of parent Model node</param>
        /// <returns>handle to the created node or 0 in case of failure</returns>
        public static int addMeshNode(int parent, string name, int matRes, int batchStart, int batchCount, int vertRStart, int vertREnd)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);
            return (int)NativeMethodsEngine.h3dAddMeshNode(parent, name, matRes, batchStart, batchCount, vertRStart, vertREnd);
        }

   
        // Joint specific
        /// <summary>
        /// This function creates a new Joint node and attaches it to the specified parent node.
        /// </summary>
        /// <param name="parent">handle to parent node to which the new node will be attached</param>
        /// <param name="name">name of the node</param>
        /// <param name="jointIndex">index of joint in Geometry resource of parent Model node</param>
        /// <returns>handle to the created node or 0 in case of failure</returns>
        public static int addJointNode(int parent, string name, int jointIndex)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);

            return (int)NativeMethodsEngine.h3dAddJointNode(parent, name, jointIndex);
        }

        // Light specific
        /// <summary>
        /// This function creates a new Light node and attaches it to the specified parent node. The direction vector of the untransformed light node is pointing along the the negative z-axis. The specified material resource can define uniforms and projective textures. Furthermore it can contain a shader for doing lighting calculations if deferred shading is used. If no material is required the parameter can be zero. The context names define which shader contexts are used when rendering shadow maps or doing light calculations for forward rendering configurations.
        /// </summary>
        /// <param name="parent">handle to parent node to which the new node will be attached</param>
        /// <param name="name">name of the node</param>
        /// <param name="materialRes">material resource for light configuration or 0 if not used</param>
        /// <param name="lightingContext">name of the shader context used for doing light calculations</param>
        /// <param name="shadowContext">name of the shader context used for doing shadow map rendering</param>
        /// <returns>handle to the created node or 0 in case of failure</returns>
        public static int addLightNode(int parent, string name, int materialRes, string lightingContext, string shadowContext)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);

            return (int)NativeMethodsEngine.h3dAddLightNode(parent, name, materialRes, lightingContext, shadowContext);
        }

        // Camera specific
        /// <summary>
        /// This function creates a new Camera node and attaches it to the specified parent node.
        /// </summary>
        /// <param name="parent">handle to parent node to which the new node will be attached</param>
        /// <param name="name">name of the node</param>
        /// <returns>handle to the created node or 0 in case of failure</returns>
        public static int addCameraNode(int parent, string name, int pipelineRes)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);

            return (int)NativeMethodsEngine.h3dAddCameraNode(parent, name, pipelineRes);
        }
        


        /// <summary>
        /// This function calculates the view frustum planes of the specified camera node using the specified view parameters.
        /// </summary>
        /// <param name="node">handle to the Camera node which will be modified</param>
        /// <param name="fov">field of view (FOV) in degrees</param>
        /// <param name="aspect">aspect ratio</param>
        /// <param name="nearDist">distance of near clipping plane</param>
        /// <param name="farDist">distance of far clipping plane</param>        
        public static void setupCameraView(int node, float fov, float aspect, float nearDist, float farDist)
        {
            NativeMethodsEngine.h3dSetupCameraView(node, fov, aspect, nearDist, farDist);
        }

        /// <summary>
        /// This function gets the camera projection matrix used for bringing the geometry to
        /// screen space and copies it to the specified array.
        /// </summary>
        /// <param name="node">handle to Camera node</param>
        /// <param name="projMat">pointer to float array with 16 elements</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static void getCameraProjMat(int node, float[] projMat)
        {
            if (projMat.Length != 16) throw new ArgumentOutOfRangeException("projMat", Resources.MatrixOutOfRangeExceptionString);

            NativeMethodsEngine.h3dGetCameraProjMat(node, projMat);
        }



        // Emitter specific
        /// <summary>
        /// This function creates a new Emitter node and attaches it to the specified parent node.
        /// </summary>
        /// <param name="parent">handle to parent node to which the new node will be attached</param>
        /// <param name="name">name of the node</param>
        /// <param name="matRes">handle to material resource used for rendering</param>
        /// <param name="effectRes">handle to effect resource used for configuring particle properties</param>
        /// <param name="maxParticleCount">maximal number of particles living at the same time</param>
        /// <param name="respawnCount">number of times a single particle is recreated after dying (-1 for infinite)</param>
        /// <returns>handle to the created node or 0 in case of failure</returns>
        public static int addEmitterNode(int parent, string name, int matRes, int effectRes, int maxParticleCount, int respawnCount)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);

            return (int)NativeMethodsEngine.h3dAddEmitterNode(parent, name, matRes, effectRes, maxParticleCount, respawnCount);
        }


        /// <summary>
        /// This function advances the simulation time of a particle system and continues the particle simulation with timeDelta being the time elapsed since the last call of this function.
        /// </summary>
        /// <param name="node">handle to the Emitter node which will be modified</param>
        /// <param name="timeDelta">time delta in seconds</param>        
        public static void updateEmitter(int node, float timeDelta)
        {
            NativeMethodsEngine.h3dUpdateEmitter(node, timeDelta);
        }

        /// <summary>
        /// Checks if an Emitter node is still alive.
        /// </summary>
        /// This function checks if a particle system is still active and has living particles or
        /// will spawn new particles. The specified node must be an Emitter node. The function can be
        /// used to check when a not infinitely running emitter for an effect like an explosion can be
        /// removed from the scene.
        /// <param name="emitterNode">emitterNode	- handle to the Emitter node which is checked</param>
        /// <returns>true if Emitter will no more emit any particles, otherwise or in case of failure false</returns>
        public static bool hasEmitterFinished(int emitterNode)
        {
            return NativeMethodsEngine.h3dHasEmitterFinished(emitterNode);
        }
    }

    /// <summary>
    /// This Exception is thrown in case you don't use the correct h3d engine library version.
    /// </summary>
    [Serializable]
    public class LibraryIncompatibleException : Exception
    {
        public LibraryIncompatibleException()
        {
        }

        public LibraryIncompatibleException(string message)
            : base(message)
        {
        }

        public LibraryIncompatibleException(string message, Exception e)
            : base(message,e)
        {
        }

        protected LibraryIncompatibleException(SerializationInfo info, StreamingContext context) : base(info,context)
        {
        }
    }
}
