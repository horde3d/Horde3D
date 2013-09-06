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

namespace Horde3DNET.Samples.ChicagoNET
{
    static class Program
    {
        private static RenderForm form;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            System.Windows.Forms.Application.EnableVisualStyles();
            System.Windows.Forms.Application.SetCompatibleTextRenderingDefault(false);
            System.Windows.Forms.Application.Idle += new EventHandler(Application_Idle);

            System.IO.Directory.SetCurrentDirectory(System.IO.Path.GetDirectoryName(System.Windows.Forms.Application.ExecutablePath));
            form = new RenderForm();
            System.Windows.Forms.Application.Run(form);

        }

        /// <summary>
        /// The main loop is called each time the application is idle.
        /// </summary>
        /// <remarks>
        /// That way the application is single threaded and therefore safe against deadlock and race conditions.
        /// </remarks>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        static void Application_Idle(object sender, EventArgs e)
        {
            form.MainLoop();
        }
    }
}