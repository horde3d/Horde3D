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

using System.Windows.Forms;

namespace Horde3DNET.Samples.ChicagoNET
{
    internal class InputManager
    {
        [System.Runtime.InteropServices.DllImport("user32.dll")]
        static extern bool GetAsyncKeyState(int vKey);

        private static bool[] _keyStates = new bool[256];

        /// <summary>
        /// This array contains the pressed/unpressed value of each key.
        /// </summary>
        public static bool[] KeyStates
        {
            get { return _keyStates; }
            set { _keyStates = value; }
        }

        /// <summary>
        /// Updates the key states.
        /// </summary>
        public static void Update()
        {
            for (int i = 0; i < 256; ++i)
            {
                if (GetAsyncKeyState(i) ) _keyStates[i] = true;
                else _keyStates[i] = false;
            }
        }

        /// <summary>
        /// Checks if a key is pressed.
        /// </summary>
        /// <param name="keyCode">specify key code</param>
        /// <returns>true if key is pressed, otherwise false</returns>
        public static bool IsKeyDown(Keys keyCode)
        {
            return _keyStates[(int)keyCode];
        }
    }
}
