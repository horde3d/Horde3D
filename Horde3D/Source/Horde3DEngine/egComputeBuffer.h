// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2016 Nicolas Schulz and Horde3D team
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#ifndef _egComputeBuffer_H_
#define _egComputeBuffer_H_

#include "egPrerequisites.h"
#include "egResource.h"
#include "egRendererBase.h"

namespace Horde3D {

// =================================================================================================
// Compute Buffer Resource
// =================================================================================================
struct ComputeBufferResData
{
	enum List
	{
		ComputeBufElem = 1000,
		DrawParamsElem,
		CompBufDataSizeI,
		CompBufUseAsVertexBufferI,
		DrawParamsNameStr,
		DrawParamsSizeI,
		DrawParamsOffsetI
	};
};


class ComputeBufferResource : public Resource
{
public:
	static Resource *factoryFunc( const std::string &name, int flags )
	{
		return new ComputeBufferResource( name, flags );
	}

	Resource *clone();

	ComputeBufferResource( const std::string &name, int flags );
	ComputeBufferResource( const std::string &name, uint32 bufferID, uint32 geometryID, int flags );

	~ComputeBufferResource();

	void initDefault();
	void release();
	bool load( const char *data, int size );

	// 	int getElemCount( int elem ) const;
	uint32 getBufferObject() { return _bufferID; }
	uint32 getGeometry() { return _geoID; }

	// internal use only (i.e. extensions)
	bool setGeometry( uint32 geomID );
	bool setBuffer( uint32 bufferID, uint32 bufSize );

	int getElemParamI( int elem, int elemIdx, int param ) const;
	void setElemParamI( int elem, int elemIdx, int param, int value );
	// 	float getElemParamF( int elem, int elemIdx, int param, int compIdx ) const;
	// 	void setElemParamF( int elem, int elemIdx, int param, int compIdx, float value );
	// 	const char *getElemParamStr( int elem, int elemIdx, int param ) const;
	void setElemParamStr( int elem, int elemIdx, int param, const char *value );

	void *mapStream( int elem, int elemIdx, int stream, bool read, bool write );
	void unmapStream();

protected:

	bool createGeometry();

private:

	std::vector< VertexLayoutAttrib >	_vlBindingsData;  /* Vertex binding parameters, if buffer is used for drawing. */
	uint8								*_data;

	uint32								_dataSize;
	uint32								_bufferID;
	uint32								_geoID;

	uint16								_vertexLayout;

	bool								_writeRequested;
	bool								_mapped;
	bool								_geometryParamsSet;
	uint8								_useAsVertexBuf;

	friend class Renderer;
};

typedef SmartResPtr< ComputeBufferResource > PComputeBufferResource;

} // namespace

#endif // _egComputeBuffer_H_

