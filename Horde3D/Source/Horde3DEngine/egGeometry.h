// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2011 Nicolas Schulz
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#ifndef _egGeometry_H_
#define _egGeometry_H_

#include "egPrerequisites.h"
#include "egResource.h"
#include "egPrimitives.h"
#include "utMath.h"


namespace Horde3D {

// =================================================================================================
// Geometry Resource
// =================================================================================================

struct GeometryResData
{
	enum List
	{
		GeometryElem = 200,
		GeoIndexCountI,
		GeoVertexCountI,
		GeoIndices16I,
		GeoIndexStream,
		GeoVertPosStream,
		GeoVertTanStream,
		GeoVertStaticStream
	};
};

// =================================================================================================

struct VertexDataTan
{
	Vec3f  normal;
	Vec3f  tangent;
	float  handedness;
};

struct VertexDataStatic		// Vertex data that is usally static
{
	float  u0, v0;
	float  jointVec[4];		// Float faster than int for some GPUs
	float  weightVec[4];
	float  u1, v1;
};


struct Joint
{
	Matrix4f  invBindMat;
};


struct MorphDiff
{
	uint32  vertIndex;
	Vec3f   posDiff;
	Vec3f   normDiff, tanDiff;
};


struct MorphTarget
{
	std::string               name;
	std::vector< MorphDiff >  diffs;
};

// =================================================================================================

class GeometryResource : public Resource
{
public:
	static void initializationFunc();
	static void releaseFunc();
	static Resource *factoryFunc( const std::string &name, int flags )
		{ return new GeometryResource( name, flags ); }
	
	GeometryResource( const std::string &name, int flags );
	~GeometryResource();
	Resource *clone();
	
	void initDefault();
	void release();
	bool load( const char *data, int size );

	int getElemCount( int elem );
	int getElemParamI( int elem, int elemIdx, int param );
	void *mapStream( int elem, int elemIdx, int stream, bool read, bool write );
	void unmapStream();

	void updateDynamicVertData();

	uint32 getVertCount() { return _vertCount; }
	char *getIndexData() { return _indexData; }
	Vec3f *getVertPosData() { return _vertPosData; }
	VertexDataTan *getVertTanData() { return _vertTanData; }
	VertexDataStatic *getVertStaticData() { return _vertStaticData; }
	uint32 getPosVBuf() { return _posVBuf; }
	uint32 getTanVBuf() { return _tanVBuf; }
	uint32 getStaticVBuf() { return _staticVBuf; }
	uint32 getIndexBuf() { return _indexBuf; }
	Matrix4f &getInvBindMat( uint32 jointIndex ) { return _joints[jointIndex].invBindMat; }

public:
	static uint32 defVertBuffer, defIndexBuffer;

private:
	bool raiseError( const std::string &msg );

private:
	static int                  mappedWriteStream;
	
	uint32                      _indexBuf, _posVBuf, _tanVBuf, _staticVBuf;

	uint32                      _indexCount, _vertCount;
	bool                        _16BitIndices;
	char                        *_indexData;
	Vec3f                       *_vertPosData;
	VertexDataTan               *_vertTanData;
	VertexDataStatic            *_vertStaticData;
	
	std::vector< Joint >        _joints;
	BoundingBox                 _skelAABB;
	std::vector< MorphTarget >  _morphTargets;
	uint32                      _minMorphIndex, _maxMorphIndex;

	friend class Renderer;
	friend class ModelNode;
	friend class MeshNode;
};

typedef SmartResPtr< GeometryResource > PGeometryResource;

}
#endif //_egGeometry_H_
