// *************************************************************************************************
//
// Chicago .NET - sample application for h3d .NET wrapper
// ----------------------------------------------------------
//
// Copyright (C) 2006-07 Nicolas Schulz and Martin Burkhard
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
using Horde3DNET.Utils;

namespace Horde3DNET.Samples.ChicagoNET
{
    internal class Application
    {
	    private float			_x, _y, _z, _rx, _ry;	// Viewer position and orientation
	    private float			_velocity;				// Velocity for movement
	    private float			_curFPS, _timer;
        private string          _fpsText;

        private bool            _freeze, _debugViewMode, _wireframeMode;

	    private CrowdSim		_crowdSim;
    	
	    // Engine objects
        private int             _fontMatRes, _panelMatRes, _logoMatRes;
        private int             _statMode = 0;

        private int             _cam, _deferredPipeRes, _forwardPipeRes;
                
        // workaround
        private bool _initialized = false;

    	
        // Convert from degrees to radians
        public static float degToRad( float f ) 
        {
	        return f * (3.1415926f / 180.0f);
        }

	    public Application()
        {
	        _x = 0; _y = 2; _z = 0; _rx = 0; _ry = 0; _velocity = 10.0f;
            _curFPS = 0; _timer = 0.0f;
            _freeze = false; _debugViewMode = false;
            _fpsText = string.Empty;
        }

        public bool init()
        {
	        // Initialize engine
            if (!h3d.init())
            {
                Horde3DUtils.dumpMessages();
                return false;
            }

	        // Set options
	        h3d.setOption( h3d.H3DOptions.LoadTextures, 1 );
	        h3d.setOption( h3d.H3DOptions.TexCompression, 0 );
	        h3d.setOption( h3d.H3DOptions.MaxAnisotropy, 4 );
	        h3d.setOption( h3d.H3DOptions.ShadowMapSize, 2048 );
            h3d.setOption( h3d.H3DOptions.FastAnimation, 1 );

            // Add resources
 	        // Pipelines
            _forwardPipeRes = h3d.addResource((int)h3d.H3DResTypes.Pipeline, "pipelines/forward.pipeline.xml", 0);
            _deferredPipeRes = h3d.addResource((int)h3d.H3DResTypes.Pipeline, "pipelines/deferred.pipeline.xml", 0);
            // Overlays
            _fontMatRes = h3d.addResource((int)h3d.H3DResTypes.Material, "overlays/font.material.xml", 0);            
            _panelMatRes = h3d.addResource( (int) h3d.H3DResTypes.Material, "overlays/panel.material.xml", 0 );
            _logoMatRes = h3d.addResource((int)h3d.H3DResTypes.Material, "overlays/logo.material.xml", 0);
            // Shader for deferred shading
            int lightMatRes = h3d.addResource((int) h3d.H3DResTypes.Material, "materials/light.material.xml", 0);
            // Environment
            int envRes = h3d.addResource((int) h3d.H3DResTypes.SceneGraph, "models/platform/platform.scene.xml", 0);
            // Skybox
            int skyBoxRes = h3d.addResource((int) h3d.H3DResTypes.SceneGraph, "models/skybox/skybox.scene.xml", 0);


            // Load resources
            Horde3DUtils.loadResourcesFromDisk( "../Content" );


            // Add scene nodes
	        // Add camera
	        _cam = h3d.addCameraNode( h3d.H3DRootNode, "Camera", _forwardPipeRes );
            // Add environment
            int env = h3d.addNodes( h3d.H3DRootNode, envRes);
            h3d.setNodeTransform( env, 0, 0, 0, 0, 0, 0, 0.23f, 0.23f, 0.23f );
	        // Add skybox
            int sky = h3d.addNodes(h3d.H3DRootNode, skyBoxRes);
	        h3d.setNodeTransform( sky, 0, 0, 0, 0, 0, 0, 210, 50, 210 );
            // Add light source

            int light = h3d.addLightNode(h3d.H3DRootNode, "Light1", lightMatRes, "LIGHTING", "SHADOWMAP");
            h3d.setNodeTransform( light, 0, 20, 50, -30, 0, 0, 1, 1, 1 );
            h3d.setNodeParamF(light, (int) h3d.H3DLight.RadiusF, 0, 200);
            h3d.setNodeParamF(light, (int) h3d.H3DLight.FovF, 0, 90);
            h3d.setNodeParamI(light, (int) h3d.H3DLight.ShadowMapCountI, 3);
            h3d.setNodeParamF(light, (int) h3d.H3DLight.ShadowSplitLambdaF, 0, 0.9f);
            h3d.setNodeParamF(light, (int) h3d.H3DLight.ShadowMapBiasF, 0,  0.001f);
            h3d.setNodeParamF(light, (int) h3d.H3DLight.ColorF3, 0, 0.9f);
            h3d.setNodeParamF(light, (int) h3d.H3DLight.ColorF3, 1, 0.7f);
            h3d.setNodeParamF(light, (int) h3d.H3DLight.ColorF3, 2, 0.75f);

            _crowdSim = new CrowdSim();        
	        _crowdSim.init();

            _initialized = true;

	        return true;
        }

        public void mainLoop(float fps)
        {
	        _curFPS = fps;
            _timer += 1 / fps;
            
	        h3d.setOption( h3d.H3DOptions.WireframeMode, _wireframeMode ? 1.0f : 0.0f );
            h3d.setOption( h3d.H3DOptions.DebugViewMode, _debugViewMode ? 1.0f : 0.0f );
        	
	        if( !_freeze )
	        {
		        _crowdSim.update( _curFPS );
	        }

            // Set camera parameters
            h3d.setNodeTransform(_cam, _x, _y, _z, _rx, _ry, 0, 1, 1, 1);

            if (_statMode > 0 )
            {
                Horde3DUtils.showFrameStats(_fontMatRes, _panelMatRes, _statMode);
            }
            
            // Show logo                        
            float ww = h3d.getNodeParamI(_cam, (int)h3d.H3DCamera.ViewportWidthI) / (float)h3d.getNodeParamI(_cam, (int)h3d.H3DCamera.ViewportHeightI);
            float[] ovLogo = new float[] { ww - 0.4f, 0.8f, 0, 1, ww - 0.4f, 1, 0, 0, ww, 1, 1, 0, ww, 0.8f, 1, 1 };
            h3d.showOverlays(ovLogo, 4, 1, 1, 1, 1, _logoMatRes, 0);

            // Render scene
            h3d.render(_cam);

            // Finish rendering of frame
            h3d.finalizeFrame();

            // Clear Overlays
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
            h3d.resizePipelineBuffers(_deferredPipeRes, width, height);
            // Set virtual camera parameters
            h3d.setupCameraView(_cam, 45.0f, (float) width/height, 0.1f, 1000.0f);
        }

        public void keyPressEvent(Keys key)
        {
            switch (key)
            {
                case Keys.Space:
                    _freeze = !_freeze;
                    break;

                case Keys.F3:
                    if(  h3d.getNodeParamI( _cam, (int) h3d.H3DCamera.PipeResI ) == _forwardPipeRes )
                        h3d.setNodeParamI(_cam, (int) h3d.H3DCamera.PipeResI, _deferredPipeRes);
                    else
                        h3d.setNodeParamI(_cam, (int) h3d.H3DCamera.PipeResI, _forwardPipeRes);
                    break;

                case Keys.F7:
                    _debugViewMode = !_debugViewMode;
                    break;

                case Keys.F8:
                    _wireframeMode = !_wireframeMode;
                    break;

                case Keys.F9:
                	_statMode += 1;
		            if( _statMode > Horde3DUtils.MaxStatMode ) _statMode = 0;
                    break;
            }
        }

        public void keyHandler()
        {
            float curVel = _velocity / _curFPS;

            if(InputManager.IsKeyDown(Keys.W))
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

            if(InputManager.IsKeyDown(Keys.A))
            {                
                // Strafe left
                _x += (float)Math.Sin(degToRad(_ry - 90)) * curVel;
                _z += (float)Math.Cos(degToRad(_ry - 90)) * curVel;
            }

            if(InputManager.IsKeyDown(Keys.D))
            { 
                // Strafe right
                _x += (float)Math.Sin(degToRad(_ry + 90)) * curVel;
                _z += (float)Math.Cos(degToRad(_ry + 90)) * curVel;
            }
        }

        public void mouseMoveEvent(float dX, float dY)
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
