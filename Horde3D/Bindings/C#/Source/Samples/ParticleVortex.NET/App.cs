// *************************************************************************************************
//
// ParticleVortex .NET - sample application for h3d .NET wrapper
// ----------------------------------------------------------
//
// Copyright (C) 2017 Horde3D team
//
// This file is intended for use as a code example, and may be used, modified, 
// or distributed in source or object code form, without restriction. 
// This sample is not covered by the EPL.
//
// The code and information is provided "as-is" without warranty of any kind, 
// either expressed or implied.
//
// *************************************************************************************************

using System;
using System.Windows.Forms;
using System.Collections.Generic;
using Horde3DNET;
using Horde3DNET.Utils;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Formatters.Binary;
using System.IO;

namespace Horde3DNET.Samples.ParticleVortexNet
{
    
    [StructLayout(LayoutKind.Sequential,Pack=1)]
    public struct vec4
    {
        // Thanks to OpenGL implementations bad support for vec3 in buffers, passed to glsl, padding is required
        public float x;
        public float y;
        public float z;
        public float w;
    }

    [Serializable]
    [StructLayout(LayoutKind.Sequential, Pack=1)]  
    public struct ParticleData
    {
	    public vec4 position; 
	    public vec4 velocity;
    }

    internal class Application
    {
        private float _x, _y, _z, _rx, _ry;	// Viewer position and orientation
        private float _velocity;				// Velocity for movement
        private float _curFPS, _timer;
        private string _fpsText;

        private bool _freeze, _debugViewMode;

        // Engine objects
        private int _fontMatRes, _panelMatRes, _logoMatRes;
        private int _computeMatRes;
        
        private int _statMode = 0;
        //horde3d 1.0
        private int _cam, _hdrPipeRes, _forwardPipeRes;
        private bool _wireframeMode;
        ////

        // workaround
        private bool _initialized = false;
        private int _compNode;
        private int _computeGroupX;
        private int _computeGroupY;

        // Convert from degrees to radians
        public static float degToRad(float f)
        {
            return f * (3.1415926f / 180.0f);
        }

        public static void normalize( ref float x, ref float y, ref float z )
        {
            // Calculate length
            float length = (float) Math.Sqrt(x * x + y * y + z * z);
            float invLength = 1.0f / length;

            x *= invLength;
            y *= invLength;
            z *= invLength;
        }

        [DllImport("msvcrt.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl, SetLastError = false)]
        static extern IntPtr memcpy(IntPtr dest, IntPtr src, int count);

        public Application()
        {
            _x = 125; _y = 25; _z = 85; _rx = -10; _ry = 55; _velocity = 10.0f;
            _curFPS = 30; _timer = 0;

            _freeze = false; _debugViewMode = false;
            _fpsText = string.Empty;
            _wireframeMode = false;
        }

        public bool init()
        {
            // Initialize engine
            if (!h3d.init(h3d.H3DRenderDevice.OpenGL4))
            {
                Horde3DUtils.dumpMessages();
                return false;
            }

            if ( h3d.getDeviceCapabilities(h3d.H3DDeviceCapabilities.GeometryShaders ) == 0 ||
                 h3d.getDeviceCapabilities(h3d.H3DDeviceCapabilities.ComputeShaders) == 0 )
            {
                return false;
            }

            // Set options
            h3d.setOption(h3d.H3DOptions.LoadTextures, 1);
            h3d.setOption(h3d.H3DOptions.TexCompression, 0);
            h3d.setOption(h3d.H3DOptions.FastAnimation, 0);
            h3d.setOption(h3d.H3DOptions.MaxAnisotropy, 4);
            h3d.setOption(h3d.H3DOptions.ShadowMapSize, 2048);

            // Add resources
            // added horde3d 1.0            
            // Pipelines
            _hdrPipeRes = h3d.addResource((int)h3d.H3DResTypes.Pipeline, "pipelines/hdr.pipeline.xml", 0);
            _forwardPipeRes = h3d.addResource((int)h3d.H3DResTypes.Pipeline, "pipelines/forward.pipeline.xml", 0);
            // Overlays
            _fontMatRes = h3d.addResource((int)h3d.H3DResTypes.Material, "overlays/font.material.xml", 0);
            _panelMatRes = h3d.addResource((int)h3d.H3DResTypes.Material, "overlays/panel.material.xml", 0);
            _logoMatRes = h3d.addResource((int)h3d.H3DResTypes.Material, "overlays/logo.material.xml", 0);

            // Shader that contains geometry and compute shaders for particles
	        _computeMatRes = h3d.addResource( (int) h3d.H3DResTypes.Material, "materials/compute.material.xml", 0 );
	        int computeDrawMatRes = h3d.addResource( (int) h3d.H3DResTypes.Material, "materials/computeDraw.material.xml", 0 );

            // 2. Specify compute buffer parameters

        	// Create compute buffer 
	        int compBuf = h3d.addResource( (int) h3d.H3DResTypes.ComputeBuffer, "CompBuf", 0 );

        	// Generate random position data for particles
	        int particlesCount = 1000000;
            ParticleData[] particles = new ParticleData[ particlesCount ];

            float maxDiapason10 = 10.0f;
            float minDiapason10 = -10.0f;
            float maxDiapason3 = 3.0f;
            float minDiapason3 = -3.0f;
            float maxDiapason30 = 30.0f;
            float minDiapason30 = -30.0f;
            float maxDiapason100 = 100.0f;
            float minDiapason100 = -100.0f;

            Random rand = new Random( (int) DateTime.Now.Ticks );
            float x, y, z = 0;
            float tmpVal = 0;
            float angle = 0;

            for ( int i = 0; i < particlesCount; i++ )
            {
                ParticleData data = new ParticleData();

                data.position = new vec4();
                
                // Set random position
                x = (maxDiapason10 + 1 - minDiapason10 ) * (float) rand.NextDouble() + minDiapason10;
                y = (maxDiapason3 + 1 - minDiapason3) * (float)rand.NextDouble() + minDiapason3;
                z = (maxDiapason30 + 1 - minDiapason30) * (float)rand.NextDouble() + minDiapason30;

                normalize( ref x, ref y, ref z );

                tmpVal = (maxDiapason100 + 1 - minDiapason100) * (float)rand.NextDouble() + minDiapason100;
                
                data.position.x = x * tmpVal;
                data.position.y = y * tmpVal;
                data.position.z = z * tmpVal;

                angle = -(float)Math.Atan2( x, z );

                data.velocity = new vec4(); 
                data.velocity.x = (float)Math.Cos(angle);
                data.velocity.y = 0;
                data.velocity.z = ( float ) Math.Sin( angle ) * 5.0f;

                particles[ i ] = data;
            }
       
            // Set size of the compute buffer
	        h3d.setResParamI( compBuf, (int) h3d.H3DComputeBufRes.ComputeBufElem, 0, (int) h3d.H3DComputeBufRes.CompBufDataSizeI, particlesCount * 32 );

	        // Mark that compute buffer will be used for rendering as a vertex buffer
	        h3d.setResParamI( compBuf, (int) h3d.H3DComputeBufRes.ComputeBufElem, 0, (int) h3d.H3DComputeBufRes.CompBufDrawableI, 1 );

	        // Set vertex binding parameters.
	        // Name - name of the parameter. Used for binding parameter to shader variable.
	        // Size - number of components (4 float for particle position, so 4), 
	        // Offset - number of bytes. For second parameter it is 16, because the first parameter had 4 floats (16 bytes)
	        h3d.setResParamStr( compBuf, (int) h3d.H3DComputeBufRes.DrawParamsElem, 0, (int) h3d.H3DComputeBufRes.DrawParamsNameStr, "partPosition" );
	        h3d.setResParamI( compBuf, (int) h3d.H3DComputeBufRes.DrawParamsElem, 0, (int) h3d.H3DComputeBufRes.DrawParamsSizeI, /*3*/ 4 );
	        h3d.setResParamI( compBuf, (int) h3d.H3DComputeBufRes.DrawParamsElem, 0, (int) h3d.H3DComputeBufRes.DrawParamsOffsetI, 0 );
	        h3d.setResParamStr( compBuf, (int) h3d.H3DComputeBufRes.DrawParamsElem, 1, (int) h3d.H3DComputeBufRes.DrawParamsNameStr, "partVelocity" );
	        h3d.setResParamI( compBuf, (int) h3d.H3DComputeBufRes.DrawParamsElem, 1, (int) h3d.H3DComputeBufRes.DrawParamsSizeI, /*3*/ 4 );
	        h3d.setResParamI( compBuf, (int) h3d.H3DComputeBufRes.DrawParamsElem, 1, (int) h3d.H3DComputeBufRes.DrawParamsOffsetI, /*12*/ 16 );

            // Fill compute buffer with generated data
	        IntPtr dataPtr = h3d.mapResStream( compBuf, (int) h3d.H3DComputeBufRes.ComputeBufElem, 0, 0, false, true );

            // Some unsafe magic to copy array to c pointer
            unsafe
            {
                GCHandle handle = GCHandle.Alloc(particles, GCHandleType.Pinned);
                try
                {
                    IntPtr pointer = handle.AddrOfPinnedObject();
                    memcpy(dataPtr, pointer, particlesCount * 32);
                }
                finally
                {
                    if (handle.IsAllocated)
                    {
                        handle.Free();
                    }
                }
            }

            h3d.unmapResStream( compBuf );

            // Load resources
            Horde3DUtils.loadResourcesFromDisk("../Content");


            // horde3d 1.0
            // Add camera
            _cam = h3d.addCameraNode(h3d.H3DRootNode, "Camera", _forwardPipeRes);
            /////////////
            // Add scene nodes
            // In order to draw the results of compute buffer we need a compute node
	        _compNode = h3d.addComputeNode( h3d.H3DRootNode, "Vortex", computeDrawMatRes, compBuf, 2, particlesCount );

	        // Set node AABB size because currently there is no way to do it otherwise
            h3d.setNodeParamF(_compNode, (int)h3d.H3DComputeNode.AABBMinF, 0, -30.0f); // x
	        h3d.setNodeParamF( _compNode, (int) h3d.H3DComputeNode.AABBMinF, 1, -30.0f ); // y
	        h3d.setNodeParamF( _compNode, (int) h3d.H3DComputeNode.AABBMinF, 2, -30.0f ); // z
	        h3d.setNodeParamF( _compNode, (int) h3d.H3DComputeNode.AABBMaxF, 0, 30.0f ); // x
	        h3d.setNodeParamF( _compNode, (int) h3d.H3DComputeNode.AABBMaxF, 1, 30.0f ); // y
	        h3d.setNodeParamF( _compNode, (int) h3d.H3DComputeNode.AABBMaxF, 2, 30.0f ); // z

	        // Set material uniforms that will not be changed during runtime
	        h3d.setMaterialUniform( _computeMatRes, "totalParticles", ( float ) particlesCount, 0, 0, 0 );
             
            // Calculate number of groups for compute shader
	        int numGroups = ( particlesCount % 1024 != 0 ) ? ( ( particlesCount / 1024 ) + 1 ) : ( particlesCount / 1024 );
	        double root = Math.Pow( ( double ) numGroups, ( double ) ( 1.0 / 2.0 ) );
	        root = Math.Ceiling( root );
	        _computeGroupX = _computeGroupY = ( int ) root;


            _initialized = true;

            return true;
        }

        public void mainLoop(float fps)
        {
            _curFPS = fps;
            _timer += 1f / fps;

            h3d.setOption(h3d.H3DOptions.DebugViewMode, _debugViewMode ? 1.0f : 0.0f);
            h3d.setOption(h3d.H3DOptions.WireframeMode, _wireframeMode ? 1.0f : 0.0f);

            if (!_freeze)
            {
                // Set animation time
                h3d.setMaterialUniform( _computeMatRes, "deltaTime", 1.0f / 30.0F, 0, 0, 0 );

                // Set attractor point
                float angle = (float) _timer * 0.5f;

                float attractorX = (float)Math.Cos( angle );
                float attractorY = (float)Math.Cos( angle ) * (float)Math.Sin( angle );
                float attractorZ = (float)Math.Sin( angle );

                h3d.setMaterialUniform( _computeMatRes, "attractor", attractorX * 2, attractorY * 2, attractorZ * 2, 0 );

                // Perform computing
                h3d.compute( _computeMatRes, "COMPUTE", _computeGroupX /*1024*/, _computeGroupY /*1*/, 1 );
            }

            // Set camera parameters
            h3d.setNodeTransform(_cam, _x, _y, _z, _rx, _ry, 0, 1, 1, 1); //horde3d 1.0

            Horde3DUtils.showFrameStats(_fontMatRes, _panelMatRes, _statMode);		        

            // Show logo                        
            float ww = h3d.getNodeParamI(_cam, (int)h3d.H3DCamera.ViewportWidthI) / (float)h3d.getNodeParamI(_cam, (int)h3d.H3DCamera.ViewportHeightI);
            float[] ovLogo = new float[] { ww - 0.4f, 0.8f, 0, 1, ww - 0.4f, 1, 0, 0, ww, 1, 1, 0, ww, 0.8f, 1, 1 };
            h3d.showOverlays(ovLogo, 4, 1, 1, 1, 1, _logoMatRes, 0);

            // Render scene
            h3d.render(_cam);//horde3D 1.0

            h3d.finalizeFrame();

            //horde3D 1.0
            h3d.clearOverlays();

            // Write all messages to log file
            Horde3DUtils.dumpMessages();
        }

        public void release()
        {
            // Release engine
            h3d.release();
        }

        public void resize(int width, int height)
        {
            if (!_initialized) return;

            // Resize viewport
            h3d.setNodeParamI(_cam, (int)h3d.H3DCamera.ViewportXI, 0);
            h3d.setNodeParamI(_cam, (int)h3d.H3DCamera.ViewportYI, 0);
            h3d.setNodeParamI(_cam, (int)h3d.H3DCamera.ViewportWidthI, width);
            h3d.setNodeParamI(_cam, (int)h3d.H3DCamera.ViewportHeightI, height);
            h3d.resizePipelineBuffers(_forwardPipeRes, width, height);
            h3d.resizePipelineBuffers(_hdrPipeRes, width, height);
            // Set virtual camera parameters
            h3d.setupCameraView(_cam, 45.0f, (float)width / height, 0.1f, 5000.0f);
        }

        public void keyPressEvent(Keys key)
        {
            switch (key)
            {
                case Keys.Space:
                    _freeze = !_freeze;
                    break;
                case Keys.F3:
                    if (h3d.getNodeParamI(_cam, (int)h3d.H3DCamera.PipeResI) == _hdrPipeRes)
                        h3d.setNodeParamI(_cam, (int)h3d.H3DCamera.PipeResI, _forwardPipeRes);
                    else
                        h3d.setNodeParamI(_cam, (int)h3d.H3DCamera.PipeResI, _hdrPipeRes);
                    break;
                case Keys.F7:
                    _debugViewMode = !_debugViewMode;
                    break;
                case Keys.F8:
                    _wireframeMode = !_wireframeMode;
                    break;
                case Keys.F9:
                    _statMode += 1;
                    if (_statMode > Horde3DUtils.MaxStatMode) _statMode = 0;
                    break;
            }
        }

        public void keyHandler()
        {
            float curVel = _velocity / _curFPS;

            if (InputManager.IsKeyDown(Keys.W))
            {
                // Move forward
                _x -= (float)Math.Sin(degToRad(_ry)) * (float)Math.Cos(-degToRad(_rx)) * curVel;
                _y -= (float)Math.Sin(-degToRad(_rx)) * curVel;
                _z -= (float)Math.Cos(degToRad(_ry)) * (float)Math.Cos(-degToRad(_rx)) * curVel;
            }

            if (InputManager.IsKeyDown(Keys.S))
            {
                // Move backward
                _x += (float)Math.Sin(degToRad(_ry)) * (float)Math.Cos(-degToRad(_rx)) * curVel;
                _y += (float)Math.Sin(-degToRad(_rx)) * curVel;
                _z += (float)Math.Cos(degToRad(_ry)) * (float)Math.Cos(-degToRad(_rx)) * curVel;
            }

            if (InputManager.IsKeyDown(Keys.A))
            {
                // Strafe left
                _x += (float)Math.Sin(degToRad(_ry - 90)) * curVel;
                _z += (float)Math.Cos(degToRad(_ry - 90)) * curVel;
            }

            if (InputManager.IsKeyDown(Keys.D))
            {
                // Strafe right
                _x += (float)Math.Sin(degToRad(_ry + 90)) * curVel;
                _z += (float)Math.Cos(degToRad(_ry + 90)) * curVel;
            }

         
        }

        public void mouseHandler(float dX, float dY)
        {
            // Look left/right
            _ry -= dX / 100 * 30;

            // Loop up/down but only in a limited range
            _rx -= dY / 100 * 30;
            if (_rx > 90) _rx = 90;
            if (_rx < -90) _rx = -90;
        }
    }
}
