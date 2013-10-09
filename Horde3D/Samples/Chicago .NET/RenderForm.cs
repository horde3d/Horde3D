// *************************************************************************************************
//
// Chicago .NET - sample application for h3d .NET wrapper
// ----------------------------------------------------------
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
using System.Drawing;
using System.Windows.Forms;
using System.Diagnostics;

using Horde3DNET;
using Horde3DNET.Utils;
using System.Runtime.InteropServices;


namespace Horde3DNET.Samples.ChicagoNET
{
    internal partial class RenderForm : Form
    {
  
        // standard vars
        private bool appFullscreen = false;
        private float fps = 30.0f;
        IntPtr dc = IntPtr.Zero, hglrc = IntPtr.Zero;

        // main application
        private Application app = new Application();

        // mouse delta calculation
        private Point preMouseMovement;

        // fps calculation        
        private Stopwatch stopWatch = new Stopwatch();
        private Int64 numFrames = 0;

        /// <summary>
        /// Constructor.
        /// </summary>
        public RenderForm()
        {            
            InitializeComponent();

            // Fullscreen
            if (appFullscreen)
            {
                FormBorderStyle = FormBorderStyle.None;
                Left = 0;
                Top = 0;
                Width = Screen.PrimaryScreen.Bounds.Width;
                Height = Screen.PrimaryScreen.Bounds.Height;
            }


            // prepare mouse handling
            Cursor.Hide();            
            MouseResetPosition();
        }

        /// <summary>
        /// All application initialization is performed here, before the main loop thread is executed and the render panel is displayed for the first time.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void RenderForm_Load(object sender, EventArgs e)
        {
            OpenGL.PIXELFORMATDESCRIPTOR pixelformatdescriptor = new OpenGL.PIXELFORMATDESCRIPTOR();
            pixelformatdescriptor.Init();

            dc = OpenGL.GetDC(renderPanel.Handle);
            int pixelFormat = OpenGL.ChoosePixelFormat(dc, ref pixelformatdescriptor);
            if (!OpenGL.SetPixelFormat(dc, pixelFormat, ref pixelformatdescriptor))
                throw new System.ComponentModel.Win32Exception(Marshal.GetLastWin32Error());

            if ((hglrc = OpenGL.wglCreateContext(dc)) == IntPtr.Zero)
                throw new System.ComponentModel.Win32Exception(Marshal.GetLastWin32Error());

            if (!OpenGL.wglMakeCurrent(dc, hglrc))
            {
                OpenGL.wglDeleteContext(hglrc);
                MessageBox.Show("Failed to init OpenGL context\nMake sure you have an OpenGL 2.0 compatible graphics card with the latest drivers installed!\nAlso verify if the pipeline config file exists.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);                
                Environment.Exit(0);
            }

            if (!app.init())
            {
                MessageBox.Show("Failed to init application\nMake sure you have an OpenGL 2.0 compatible graphics card with the latest drivers installed!\nAlso verify if the pipeline config file exists.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                h3d.release();                
                Environment.Exit(0);
            }

            app.resize(renderPanel.Size.Width, renderPanel.Size.Height);
        }

        /// <summary>
        /// Occurs when the form is resized. The rendering output area has to be resized accordingly.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void RenderForm_Resize(object sender, EventArgs e)
        {
            app.resize(renderPanel.Size.Width, renderPanel.Size.Height);
        }

        /// <summary>
        /// The main loop. This method is executed by the application event handler each time the application is idle.
        /// </summary>
        public void MainLoop()
        {
            KeyHandling();
            MouseMovement();
            CalcFPS();

            // render
            app.mainLoop(fps);
            
            // Swap buffers
            OpenGL.SwapBuffers(dc);

            // if mouse tracking is disabled call Invalidate() to redraw the scene
            Invalidate();
        }

        /// <summary>
        /// Occurs when a key is pressed, asynchronously.
        /// </summary>
        private void KeyHandling()
        {
            InputManager.Update();
            app.keyHandler();
        }

        /// <summary>
        /// Occurs when a key is pressed inside the render panel, synchrounously.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void RenderForm_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Escape)
            {
                Close();
            }
            else if( e.KeyCode == Keys.F1)
            {
                    if (!appFullscreen)
                    {
                        FormBorderStyle = FormBorderStyle.None;
                        Left = 0;
                        Top = 0;
                        Width = Screen.PrimaryScreen.Bounds.Width;
                        Height = Screen.PrimaryScreen.Bounds.Height;
                    }
                    else
                    {
                        FormBorderStyle = FormBorderStyle.Sizable;
                        Left = 20;
                        Top = 20;
                        Width = Screen.PrimaryScreen.Bounds.Width / 2;
                        Height = Screen.PrimaryScreen.Bounds.Height / 2;
                    }
                    appFullscreen = !appFullscreen;
            }
            else
                app.keyPressEvent(e.KeyCode);
        }

        /// <summary>
        /// Calculates the mouse movement based on the difference between the old and new position (delta).
        /// </summary>
        private void MouseMovement()
        {
            // calculate delta
            Point delta = new Point(Cursor.Position.X - preMouseMovement.X, Cursor.Position.Y - preMouseMovement.Y);

            // call handler
            app.mouseMoveEvent(delta.X, delta.Y);

            // reset mouse position
            MouseResetPosition();
        }

        /// <summary>
        /// The mouse cursor position is reset to the center of the application window.
        /// </summary>
        private void MouseResetPosition()
        {
            // calculate the center of the form window
            Point pos = Location;
            pos.X += Size.Width /2;
            pos.Y += Size.Height /2;

            // reset cursor position
            Cursor.Position = pos;
            preMouseMovement = pos;
        }

        /// <summary>
        /// Calculates the actual frames per second (fps).
        /// </summary>
        private void CalcFPS()
        {
            // start stopwatch if it isn't running, yet
            if (!stopWatch.IsRunning)
                stopWatch.Start();

            // calculate frames per second (fps)                    
            ++numFrames;
            if ( numFrames >= 3 )
            {
                // calculate fps by deviding the frame count through the elapsed seconds
                fps = numFrames * 1000.0f / stopWatch.Elapsed.Milliseconds;
                numFrames = 0;      // reset number of frames
                stopWatch.Reset();  // prevent a number overflow
                stopWatch.Start();  // restart stopwatch because reset stops the time interval measurement
            }
        }

        /// <summary>
        /// Release the engine and free all objects as well as associated memory, after the form is closed.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void RenderForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            // stop stop watch process
            stopWatch.Stop();

            // release h3d
            app.release();

            OpenGL.wglDeleteContext(hglrc);
            
            Environment.Exit(0);
        }
    }
}