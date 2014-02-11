// *************************************************************************************************
//
// Knight .NET - sample application for h3d .NET wrapper
// ----------------------------------------------------------
//
// Copyright (C) 2007 Nicolas Schulz and Martin Burkhard
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
using Horde3DNET;
using Horde3DNET.Utils;

namespace Horde3DNET.Samples.KnightNET
{
    internal class Application
    {
	    private float			_x, _y, _z, _rx, _ry;	// Viewer position and orientation
	    private float			_velocity;				// Velocity for movement
        private float           _curFPS, _timer;
        private string          _fpsText;

        private bool            _freeze, _debugViewMode;
        private float           _animTime, _weight;
    	
	    // Engine objects
	    private int	            _fontMatRes, _panelMatRes, _logoMatRes;
        private int             _knight, _particleSys;

        private int             _statMode = 0;
        //horde3d 1.0
        private int             _cam, _hdrPipeRes, _forwardPipeRes;
        private bool            _wireframeMode;
        ////

        // workaround
        private bool            _initialized = false;

    	
        // Convert from degrees to radians
        public static float degToRad( float f ) 
        {
	        return f * (3.1415926f / 180.0f);
        }

	    public Application()
        {
            _x = 5; _y = 3; _z = 19; _rx = 7; _ry = 15; _velocity = 10.0f;
            _curFPS = 30; _timer = 0;

            _freeze = false; _debugViewMode = false;
            _animTime = 0; _weight = 1.0f;
            _fpsText = string.Empty;
            _wireframeMode = false;
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
            h3d.setOption( h3d.H3DOptions.FastAnimation, 0 );
	        h3d.setOption( h3d.H3DOptions.MaxAnisotropy, 4 );
	        h3d.setOption( h3d.H3DOptions.ShadowMapSize, 2048 );

            // Add resources
            // added horde3d 1.0            
	        // Pipelines
            _hdrPipeRes = h3d.addResource( (int) h3d.H3DResTypes.Pipeline, "pipelines/hdr.pipeline.xml", 0);
            _forwardPipeRes = h3d.addResource((int) h3d.H3DResTypes.Pipeline, "pipelines/forward.pipeline.xml", 0);
            // Overlays
            _fontMatRes = h3d.addResource((int)h3d.H3DResTypes.Material, "overlays/font.material.xml", 0);
            _panelMatRes = h3d.addResource((int)h3d.H3DResTypes.Material, "overlays/panel.material.xml", 0);
            _logoMatRes = h3d.addResource((int)h3d.H3DResTypes.Material, "overlays/logo.material.xml", 0);
            // Environment
            int envRes = h3d.addResource((int)h3d.H3DResTypes.SceneGraph, "models/sphere/sphere.scene.xml", 0);
	        // Knight
            int knightRes = h3d.addResource((int)h3d.H3DResTypes.SceneGraph, "models/knight/knight.scene.xml", 0);
            int knightAnim1Res = h3d.addResource((int)h3d.H3DResTypes.Animation, "animations/knight_order.anim", 0);
            int knightAnim2Res = h3d.addResource((int)h3d.H3DResTypes.Animation, "animations/knight_attack.anim", 0);
	        // Particle system
            int particleSysRes = h3d.addResource((int)h3d.H3DResTypes.SceneGraph, "particles/particleSys1/particleSys1.scene.xml", 0);


            // Load resources
            Horde3DUtils.loadResourcesFromDisk( "../Content" );


            // horde3d 1.0
	        // Add camera
            _cam = h3d.addCameraNode(h3d.H3DRootNode, "Camera", _hdrPipeRes);
            /////////////
            // Add scene nodes
            // Add environment
            int env = h3d.addNodes( h3d.H3DRootNode, envRes );
	        h3d.setNodeTransform( env, 0, -20, 0, 0, 0, 0, 20, 20, 20 ); //horde3d 1.0
	        
            // Add knight
            _knight = h3d.addNodes(h3d.H3DRootNode, knightRes);
	        h3d.setNodeTransform( _knight, 0, 0, 0, 0, 180, 0, 0.1f, 0.1f, 0.1f );
	        h3d.setupModelAnimStage( _knight, 0, knightAnim1Res, 0, string.Empty, false );
            h3d.setupModelAnimStage(_knight, 1, knightAnim2Res, 0, string.Empty, false);

            //horde3d 1.0
            // Attach particle system to hand joint
	        h3d.findNodes( _knight, "Bip01_R_Hand", (int) h3d.H3DNodeTypes.Joint );
	        int hand = h3d.getNodeFindResult( 0 );
	        _particleSys = h3d.addNodes( hand, particleSysRes );
	        h3d.setNodeTransform( _particleSys, 0, 40, 0, 90, 0, 0, 1, 1, 1 );
            /////////


            // Add light source
            int light = h3d.addLightNode(h3d.H3DRootNode, "Light1", 0, "LIGHTING", "SHADOWMAP");
            h3d.setNodeTransform(light, 0, 1, 15, 30, 0, 0, 1, 1, 1);

	        h3d.setNodeTransform( light, 0, 15, 10, -60, 0, 0, 1, 1, 1 );
            h3d.setNodeParamF(light,(int) h3d.H3DLight.RadiusF, 0, 30);
            h3d.setNodeParamF(light, (int)h3d.H3DLight.FovF, 0, 90);
            h3d.setNodeParamI(light, (int)h3d.H3DLight.ShadowMapCountI, 1);
            h3d.setNodeParamF(light, (int)h3d.H3DLight.ShadowMapBiasF, 0, 0.01f);
            h3d.setNodeParamF( light, (int)h3d.H3DLight.ColorF3, 0, 1.0f );
            h3d.setNodeParamF(light, (int)h3d.H3DLight.ColorF3, 1, 0.8f);
            h3d.setNodeParamF(light, (int)h3d.H3DLight.ColorF3, 2, 0.7f);            
            /////////////

	        // Customize post processing effects
            int matRes = h3d.findResource((int)h3d.H3DResTypes.Material, "pipelines/postHDR.material.xml");
            // hdrParams: exposure, brightpass threshold, brightpass offset
            h3d.setMaterialUniform(matRes, "hdrParams", 2.5f, 0.5f, 0.08f, 0);

            _initialized = true;

	        return true;
        }

        public void mainLoop(float fps)
        {
	        _curFPS = fps;
            _timer += 1f / fps;

            h3d.setOption( h3d.H3DOptions.DebugViewMode, _debugViewMode ? 1.0f : 0.0f );
	        h3d.setOption( h3d.H3DOptions.WireframeMode, _wireframeMode ? 1.0f : 0.0f );
        	
	        if( !_freeze )
	        {
		        _animTime += 1.0f / _curFPS;

		        // Do animation blending
		        h3d.setModelAnimParams( _knight, 0, _animTime * 24.0f, _weight );
		        h3d.setModelAnimParams( _knight, 1, _animTime * 24.0f, 1.0f - _weight );
                h3d.updateModel( _knight, (int)(h3d.H3DModelUpdateFlags.Animation | h3d.H3DModelUpdateFlags.Geometry ));

		        // Animate particle system                                
                int cnt = cnt = h3d.findNodes(_particleSys, "", (int)h3d.H3DNodeTypes.Emitter);
		        for( int i = 0; i < cnt; ++i )
                    h3d.updateEmitter(h3d.getNodeFindResult(i), 1.0f / _curFPS);
	        }

            // Set camera parameters
	        h3d.setNodeTransform( _cam, _x, _y, _z, _rx ,_ry, 0, 1, 1, 1 ); //horde3d 1.0

            if (_statMode > 0)
            {
                Horde3DUtils.showFrameStats( _fontMatRes, _panelMatRes, _statMode );		        

                string text = string.Format("Weight: {0:F2}", _weight);
                Horde3DUtils.showText(text, 0.03f, 0.24f, 0.026f, 1, 1, 1, _fontMatRes );
            }

            // Show logo                        
            float ww = h3d.getNodeParamI(_cam, (int)h3d.H3DCamera.ViewportWidthI) / (float)h3d.getNodeParamI(_cam, (int)h3d.H3DCamera.ViewportHeightI);
            float[] ovLogo = new float[] { ww - 0.4f, 0.8f, 0, 1, ww - 0.4f, 1, 0, 0, ww, 1, 1, 0, ww, 0.8f, 1, 1 };
            h3d.showOverlays(ovLogo, 4, 1, 1, 1, 1, _logoMatRes, 0);

            // Render scene
            h3d.render( _cam );//horde3D 1.0

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
            h3d.setupCameraView(_cam, 45.0f, (float)width / height, 0.1f, 1000.0f);
        }

        public void keyPressEvent(Keys key)
        {
            switch (key)
            {
                case Keys.Space:
                    _freeze = !_freeze;
                    break;
	            case Keys.F3:
	                if( h3d.getNodeParamI( _cam, (int) h3d.H3DCamera.PipeResI ) == _hdrPipeRes )
                        h3d.setNodeParamI(_cam, (int) h3d.H3DCamera.PipeResI, _forwardPipeRes);
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

            if (InputManager.IsKeyDown(Keys.D1))	// 1
            {
                _weight += 2 / _curFPS;
                if (_weight > 1) _weight = 1;
            }
            if (InputManager.IsKeyDown(Keys.D2))	// 2
            {
                _weight -= 2 / _curFPS;
                if (_weight < 0) _weight = 0;
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
