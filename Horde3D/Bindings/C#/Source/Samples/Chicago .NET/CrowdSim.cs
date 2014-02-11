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
using System.Collections.Generic;
using Horde3DNET;
using Horde3DNET.Utils;


namespace Horde3DNET.Samples.ChicagoNET
{
    internal class Particle
    {
        public float px, pz;		// Current postition
        public float dx, dz;		// Destination position
        public float fx, fz;		// Force on particle
        public float ox, oz;		// Orientation vector
        public int node;
        public float animTime;


	    public Particle()
	    {
		    ox = 0; oz = 0;
		    node = 0;
		    animTime = 0;
	    }
    };

    internal class CrowdSim
    {
        private List<Particle> _particles = new List<Particle>();
        private Random _rand;

        private void chooseDestination( ref Particle p )
        {                      
	        // Choose random destination within a circle
            float ang = ((_rand.Next() % 360) / 360.0f) * 6.28f;
            float rad = (_rand.Next() % 30) * 1.0f;

	        p.dx = (float)Math.Sin( ang ) * rad;
            p.dz = (float)Math.Cos(ang) * rad;        
        }

        public void init()
        {
	        // Init random generator
	        _rand = new Random((int)DateTime.Now.Ticks);
        	
	        // Load character with walk animation
            int characterRes = h3d.addResource((int)h3d.H3DResTypes.SceneGraph, "models/man/man.scene.xml", 0);
            int characterWalkRes = h3d.addResource((int)h3d.H3DResTypes.Animation, "animations/man.anim", 0);
	        Horde3DUtils.loadResourcesFromDisk( "../Content" );
        	
	        // Add characters
	        for( int i = 0; i < 100; ++i )
	        {
		        Particle p = new Particle();
        		
		        // Add character to scene and apply animation
                p.node = h3d.addNodes(h3d.H3DRootNode, characterRes);
                h3d.setupModelAnimStage(p.node, 0, characterWalkRes, 0, string.Empty, false);
        		
		        // Characters start in a circle formation
		        p.px = (float)Math.Sin( (i / 100.0f) * 6.28f ) * 10.0f;
		        p.pz = (float)Math.Cos( (i / 100.0f) * 6.28f ) * 10.0f;

		        chooseDestination( ref p );

		        h3d.setNodeTransform( p.node, p.px, 0.02f, p.pz, 0, 0, 0, 1, 1, 1 );

		        _particles.Add( p );
	        }
        }


	    public void update( float fps )
        {
	        // Parameters for three repulsion zones
	        float d1 = 0.25f, d2 = 2.0f, d3 = 4.5f;
	        float f1 = 3.0f, f2 = 1.0f, f3 = 0.1f;
        	
	        for( int i = 0; i < _particles.Count; ++i )
	        {
		        Particle p = _particles[i];
        		
		        // Reset force
		        p.fx = 0; p.fz = 0;
        		
		        // Calculate distance to destination
		        float dist = (float)Math.Sqrt( (p.dx - p.px)*(p.dx - p.px) + (p.dz - p.pz)*(p.dz - p.pz) );
        		
		        // If destination not reached walk to destination
		        if( dist > 3.0f )
		        {
			        // Calculate normalized attraction force to destination
			        float afx = (p.dx - p.px) / dist;
			        float afz = (p.dz - p.pz) / dist;	

			        p.fx += afx * 0.035f; p.fz += afz * 0.035f;

			        // Repulsion forces from other particles
			        for( int j = 0; j < _particles.Count; ++j )
			        {
				        if( j == i ) continue;
        				
				        Particle p2 = _particles[j];
        				
				        float dist2 = (float)Math.Sqrt( (p.px - p2.px)*(p.px - p2.px) + (p.pz - p2.pz)*(p.pz - p2.pz) );
				        float strength = 0;

				        float rfx = (p.px - p2.px) / dist2;
				        float rfz = (p.pz - p2.pz) / dist2;
        				
				        // Use three zones with different repulsion strengths
				        if( dist2 <= d3 && dist2 > d2 )
				        {
					        float m = (f3 - 0) / (d2 - d3);
					        float t = 0 - m * d3;
					        strength = m * dist2 + t;
				        }
				        else if( dist2 <= d2 && dist2 > d1 )
				        {
					        float m = (f2 - f3) / (d1 - d2);
					        float t = f3 - m * d2;
					        strength = m * dist2 + t;
				        }
				        else if( dist2 <= d1 )
				        {
					        float m = (f1 - f2) / (0 - d1);
					        float t = f2 - m * d1;
					        strength = m * dist2 + t;
				        }

				        p.fx += rfx * strength; p.fz += rfz * strength;
			        }
		        }
		        else
		        {
			        // Choose new destination
			        chooseDestination( ref p );
		        }

		        // Make movement frame rate independence
		        p.fx *= (30 / fps);
		        p.fz *= (30 / fps);
		        float vel = (float)Math.Sqrt( p.fx * p.fx + p.fz * p.fz );
        		
		        // Set new position
		        p.px += p.fx; p.pz += p.fz;
        		
		        // Caluclate orientation
		        p.ox = (p.ox + p.fx) / 2;
		        p.oz = (p.oz + p.fz) / 2;

		        // Get rotation from orientation
		        float ry = 0;
		        if( p.oz != 0 ) ry = (float)Math.Atan2( p.ox, p.oz );
		        ry *= 180 / 3.1415f;	// Convert from radians to degrees
        		
		        // Update character scene node position
		        h3d.setNodeTransform( p.node, p.px, 0.02f, p.pz, 0, ry, 0, 1, 1, 1 );
        		
		        // Update animation
		        p.animTime += vel * 35.0f;
                h3d.setModelAnimParams( p.node, 0, p.animTime, 1.0f );
                h3d.updateModel(p.node, (int)(h3d.H3DModelUpdateFlags.Animation | h3d.H3DModelUpdateFlags.Geometry));
            }
        }
    }
}
