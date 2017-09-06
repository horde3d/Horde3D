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
using System.Text;

namespace Horde3DNET.Samples.TesselatorNet
{
    
    internal class Application
    {
        private float _x, _y, _z, _rx, _ry;	// Viewer position and orientation
        private float _velocity;				// Velocity for movement
        private float _curFPS, _timer;
        private string _fpsText;

        private bool _freeze, _debugViewMode;

        // Engine objects
        private int _fontMatRes, _panelMatRes, _logoMatRes;
//        private int _computeMatRes;
        
        private int _statMode = 0;
        //horde3d 1.0
        private int _cam, _hdrPipeRes, _forwardPipeRes;
        private bool _wireframeMode;
        ////

        private bool _initialized = false;
        private int _tessInner = 1;
        private int _tessOuter = 1;
        private int _model;
        private float _rotation;
        // Convert from degrees to radians
        public static float degToRad(float f)
        {
            return f * (3.1415926f / 180.0f);
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct Vec3
        {
	        public float x;
	        public float y;
	        public float z;

	        public Vec3( float xValue, float yValue, float zValue ) 
	        {
                x = xValue; y = yValue; z = zValue;
	        }

	        public static Vec3 operator -( Vec3 v1, Vec3 v2 ) 
	        {
		        return new Vec3( v1.x - v2.x, v1.y - v2.y, v1.z - v2.z );
	        }

	        public static Vec3 operator +( Vec3 v1, Vec3 v2 ) 
	        {
		        return new Vec3( v1.x + v2.x, v1.y + v2.y, v1.z + v2.z );
	        }
	      
        }

        public static Vec3 normalize( Vec3 value )
        {
	        Vec3 retVal = value;
	        float invLen = 1.0f / (float) Math.Sqrt( retVal.x * retVal.x + retVal.y * retVal.y + retVal.z * retVal.z );
	        retVal.x *= invLen;
	        retVal.y *= invLen;
	        retVal.z *= invLen;

	        return retVal;
        }

        public static Vec3 cross( Vec3 left, Vec3 right )
        {
	        return new Vec3( left.y * right.z - left.z * right.y, left.z * right.x - left.x * right.z, left.x * right.y - left.y * right.x );
        }

        public static bool createIcosahedron()
        {
	        // Create vertex and index data
	        float phi = ( 1.0f / ( ( 1.0f + ( float ) Math.Sqrt( 5.0f ) ) / 2.0f ) );

	        Vec3[] vertPos = {  
		        new Vec3( -phi, 1.0f, 0.0f ), new Vec3( phi, 1.0f, 0.0f ), new Vec3( -phi, -1.0f, 0.0f ), 
		        new Vec3( phi, -1.0f, 0.0f ), new Vec3( 0.0f, -phi, 1.0f ), new Vec3( 0.0f, phi, 1.0f ), 
		        new Vec3( 0.0f, -phi, -1.0f ), new Vec3( 0.0f, phi, -1.0f ), new Vec3( 1.0f, 0.0f, -phi ),
		        new Vec3( 1.0f, 0.0f, phi ), new Vec3( -1.0f, 0.0f, -phi ), new Vec3( -1.0f, 0.0f, phi ) };

            float[] vertPosFloat = {
                                       -phi, 1.0f, 0.0f,
                                        phi, 1.0f, 0.0f,
                                       -phi, -1.0f, 0.0f,
                                        phi, -1.0f, 0.0f,
                                        0.0f, -phi, 1.0f,
                                        0.0f, phi, 1.0f,
                                        0.0f, -phi, -1.0f,
                                        0.0f, phi, -1.0f,
                                        1.0f, 0.0f, -phi,
                                        1.0f, 0.0f, phi,
                                        -1.0f, 0.0f, -phi,
                                        -1.0f, 0.0f, phi
                                   };

	        float[] texCoords = {
		        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,

		        1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		        1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		        1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		        1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		        1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,

		        1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		        1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		        1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		        1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		        1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,

		        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };

	        int[] indices = {
		        0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
		        5, 11, 4, 1, 5, 9, 7, 1, 8, 10, 7, 6, 11, 10, 2,
		        3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9,
		        4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7, 9, 8, 1 };

	        Vec3[] normals = new Vec3[ 12 ];
	        short[] normalsShort = new short[ 36 ];

	        // calculate normals
	        for ( int i = 0; i < 12; ++i )
	        {
		        int index0 = indices[ i * 3 + 0 ];
		        int index1 = indices[ i * 3 + 1 ];
		        int index2 = indices[ i * 3 + 2 ];

		        Vec3 v0 = vertPos[ index0 ];
		        Vec3 v1 = vertPos[ index1 ];
		        Vec3 v2 = vertPos[ index2 ];

		        Vec3 e0 = v1 - v0;
		        Vec3 e1 = v2 - v0;

		        Vec3 result = normalize( cross( e0, e1 ) );
		        normals[ index0 ] = result;
		        normals[ index1 ] = result;
		        normals[ index2 ] = result;

        // 		normals[ index0 * 3 + 0 ] += result.x; normals[ index0 * 3 + 1 ] += result.y; normals[ index0 * 3 + 2 ] += result.z;
        // 		normals[ index1 * 3 + 0 ] += result.x; normals[ index1 * 3 + 1 ] += result.y; normals[ index1 * 3 + 2 ] += result.z;
        // 		normals[ index2 * 3 + 0 ] += result.x; normals[ index2 * 3 + 1 ] += result.y; normals[ index2 * 3 + 2 ] += result.z;

	        }

	        for ( int j = 0; j < 12; ++j )
	        {
		        Vec3 v = normalize( normals[ j ] );
		        normalsShort[ j * 3 + 0 ] = ( short ) ( v.x * 32767 );
		        normalsShort[ j * 3 + 1 ] = ( short ) ( v.y * 32767 );
		        normalsShort[ j * 3 + 2 ] = ( short ) ( v.z * 32767 );
	        }

            int geo = Horde3DUtils.createGeometryRes("icosa", 12, indices.Length, vertPosFloat, indices, normalsShort, null, null, texCoords, null);
           
            if (geo == 0) return false;

	        // Create material 
	        String matData = "<Material>\n" +
							 "<Shader source=\"shaders/tessellation.shader\" />" +
							 "<Uniform name=\"tessLevelInner\" a=\"1\" />\n" +
							 "<Uniform name=\"tessLevelOuter\" a=\"1\" />\n" +
							 "<Uniform name=\"matAmbientCol\" a=\"0.04\" b=\"0.04\" c=\"0.04\" />\n" +
							 "<Uniform name=\"matDiffuseCol\" a=\"0\" b=\"0.75\" c=\"0.75\" />\n" +
							 "</Material>";
            
            byte[] matDataBytes = Encoding.ASCII.GetBytes( matData );

	        int mat = h3d.addResource( ( int ) h3d.H3DResTypes.Material, "material", 0 );
            bool res = h3d.loadResource(mat, matDataBytes, matData.Length);
	        if ( !res ) return false;

	        return true;
        }

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

            if (h3d.getDeviceCapabilities(h3d.H3DDeviceCapabilities.GeometryShaders) == 0 ||
                 h3d.getDeviceCapabilities(h3d.H3DDeviceCapabilities.TessellationShaders) == 0)
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

            int lightMatRes = h3d.addResource( ( int ) h3d.H3DResTypes.Material, "materials/light.material.xml", 0 );

	        bool result = createIcosahedron();
	        if ( !result ) return false;

            // Load resources
            Horde3DUtils.loadResourcesFromDisk("../Content");


            // horde3d 1.0
            // Add camera
            _cam = h3d.addCameraNode(h3d.H3DRootNode, "Camera", _forwardPipeRes);
            /////////////

            // Create model and mesh
	        int geo = h3d.findResource( (int) h3d.H3DResTypes.Geometry, "icosa" );
	        int mat = h3d.findResource( ( int ) h3d.H3DResTypes.Material, "material" );

	        int indices = h3d.getResParamI( geo, (int) h3d.H3DGeoRes.GeometryElem, 0, (int) h3d.H3DGeoRes.GeoIndexCountI );
	        int vertices = h3d.getResParamI( geo, (int) h3d.H3DGeoRes.GeometryElem, 0, (int) h3d.H3DGeoRes.GeoVertexCountI );
	
	        _model = h3d.addModelNode( h3d.H3DRootNode, "model", geo );
	        int mesh = h3d.addMeshNode( _model, "icosahedron", mat, 0, indices, 0, vertices - 1 );
	        h3d.setNodeTransform( mesh, 0, 0, 0, 0, 0, 0, 20, 20, 20 );
	        h3d.setNodeParamI( mesh, ( int) h3d.H3DMesh.TessellatableI, 1 ); // Set mesh to use tessellation

            // Add light source
	        int light = h3d.addLightNode( h3d.H3DRootNode, "Light1", lightMatRes, "LIGHTING", "SHADOWMAP" );
  	        h3d.setNodeTransform( light, 125, 25, 85, -10, 55, 0, 1, 1, 1 );
	        h3d.setNodeParamF( light, ( int ) h3d.H3DLight.RadiusF, 0, 1000 );
	        h3d.setNodeParamF( light, ( int ) h3d.H3DLight.FovF, 0, 75 );
	        h3d.setNodeParamI( light, ( int ) h3d.H3DLight.ShadowMapCountI, 3 );
	        h3d.setNodeParamF( light, ( int ) h3d.H3DLight.ShadowSplitLambdaF, 0, 0.9f );
	        h3d.setNodeParamF( light, ( int ) h3d.H3DLight.ShadowMapBiasF, 0, 0.001f );
	        h3d.setNodeParamF( light, ( int ) h3d.H3DLight.ColorF3, 0, 0.9f );
	        h3d.setNodeParamF( light, ( int ) h3d.H3DLight.ColorF3, 1, 0.7f );
	        h3d.setNodeParamF( light, ( int ) h3d.H3DLight.ColorF3, 2, 0.75f );
	        h3d.setNodeParamF( light, ( int ) h3d.H3DLight.ColorMultiplierF, 0, 1.0f );
            
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
                // Calculate animation time in seconds
                _rotation += 0.05f;
                h3d.setNodeTransform(_model, 0, 0, 0, _rotation, _rotation, 0, 1, 1, 1);
            }

            // Set camera parameters
            h3d.setNodeTransform(_cam, _x, _y, _z, _rx, _ry, 0, 1, 1, 1); //horde3d 1.0

            Horde3DUtils.showFrameStats(_fontMatRes, _panelMatRes, _statMode);

            Horde3DUtils.showText( "Up/Down arrows to modify tessellation level", 1.0f, 0.01f, 0.032f, 1, 1, 1, _fontMatRes);

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
                case Keys.Up:
                    {
                        _tessInner++;
		                _tessOuter++;

		                int mat = h3d.findResource( (int) h3d.H3DResTypes.Material, "material" );
		                h3d.setMaterialUniform( mat, "tessLevelInner", ( float ) _tessInner, 0, 0, 0 );
		                h3d.setMaterialUniform( mat, "tessLevelOuter", ( float ) _tessOuter, 0, 0, 0 );

                        break;
                    }
                case Keys.Down:
                    {
                        _tessInner = (_tessInner == 1) ? 1 : _tessInner - 1;
                        _tessOuter = (_tessOuter == 1) ? 1 : _tessOuter - 1;

                        int mat = h3d.findResource((int)h3d.H3DResTypes.Material, "material");
                        h3d.setMaterialUniform(mat, "tessLevelInner", (float)_tessInner, 0, 0, 0);
                        h3d.setMaterialUniform(mat, "tessLevelOuter", (float)_tessOuter, 0, 0, 0);

                        break;
                    }
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
