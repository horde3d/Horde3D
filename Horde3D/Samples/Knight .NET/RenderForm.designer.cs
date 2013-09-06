// *************************************************************************************************
//
// Knight .NET - sample application for h3d .NET wrapper
// ----------------------------------------------------------
//
// Copyright (C) 2007 Nicolas Schulz and Martin Burkhard
//
// This file is intended for use as a code example, and may be used, modified, 
// or distributed in source or object code form, without restriction. 
// This sample is not covered by the LGPL.
//
// The code and information is provided "as-is" without warranty of any kind, 
// either expressed or implied.
//
// *************************************************************************************************

namespace Horde3DNET.Samples.KnightNET
{
    internal partial class RenderForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.renderPanel = new System.Windows.Forms.Panel();
            this.SuspendLayout();
            // 
            // renderPanel
            // 
            this.renderPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.renderPanel.Location = new System.Drawing.Point(0, 0);
            this.renderPanel.Name = "renderPanel";
            this.renderPanel.Size = new System.Drawing.Size(800, 600);
            this.renderPanel.TabIndex = 0;
            // 
            // RenderForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 600);
            this.Controls.Add(this.renderPanel);
            this.MaximizeBox = false;
            this.Name = "RenderForm";
            this.Text = "Knight - Horde3D .NET Sample (beta)";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.RenderForm_FormClosed);
            this.Resize += new System.EventHandler(this.RenderForm_Resize);
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.RenderForm_KeyDown);
            this.Load += new System.EventHandler(this.RenderForm_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel renderPanel;
    }
}

