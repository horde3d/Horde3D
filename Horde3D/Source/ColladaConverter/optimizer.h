// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2020 Nicolas Schulz and Horde3D team
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#ifndef _optimizer_H_
#define _optimizer_H_

#include <vector>
#include <set>
#include <map>

namespace Horde3D {
namespace ColladaConverter {


struct TriGroup;
struct Vertex;
struct OptFace;


struct OptVertex
{
	unsigned int           index;  // Index in vertex array
	float                  score;
	std::set< OptFace * >  faces;  // Faces that are using this vertex

	void updateScore( int cacheIndex );
};

struct OptFace
{
	OptVertex  *verts[3];
	
	float getScore() const { return verts[0]->score + verts[1]->score + verts[2]->score; }
};

class MeshOptimizer
{
public:
	static const int maxCacheSize = 16;
	
	static unsigned int removeDegeneratedTriangles( TriGroup *triGroup, std::vector< Vertex > &vertices,
	                                                std::vector< unsigned int > &indices );
	static float calcCacheEfficiency( TriGroup *triGroup, std::vector< unsigned int > &indices,
	                                  const unsigned int cacheSize = maxCacheSize );
	static void optimizeIndexOrder( TriGroup *triGroup, std::vector< Vertex > &vertices,
	                                std::vector< unsigned int > &indices,
	                                std::map< unsigned int, unsigned int > &vertMap );
};


} // namespace ColladaConverter
} // namespace Horde3D

#endif	// _optimizer_H_
