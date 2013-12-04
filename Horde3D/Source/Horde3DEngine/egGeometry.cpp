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

#include "egGeometry.h"
#include "egResource.h"
#include "egAnimation.h"
#include "egModules.h"
#include "egCom.h"
#include "egRenderer.h"
#include <cstring>

#include "utDebug.h"


namespace Horde3D {

using namespace std;


uint32 GeometryResource::defVertBuffer = 0;
uint32 GeometryResource::defIndexBuffer = 0;
int GeometryResource::mappedWriteStream = -1;


void GeometryResource::initializationFunc()
{
	defVertBuffer = gRDI->createVertexBuffer( 0, 0x0 );
	defIndexBuffer = gRDI->createIndexBuffer( 0, 0x0 );
}


void GeometryResource::releaseFunc()
{
	gRDI->destroyBuffer( defVertBuffer );
	gRDI->destroyBuffer( defIndexBuffer );
}


GeometryResource::GeometryResource( const string &name, int flags ) :
	Resource( ResourceTypes::Geometry, name, flags )
{
	initDefault();
}


GeometryResource::~GeometryResource()
{
	release();
}


Resource *GeometryResource::clone()
{
	GeometryResource *res = new GeometryResource( "", _flags );

	*res = *this;

	// Make a deep copy of the data
	res->_indexData = new char[_indexCount * (_16BitIndices ? 2 : 4)];
	res->_vertPosData = new Vec3f[_vertCount];
	res->_vertTanData = new VertexDataTan[_vertCount];
	res->_vertStaticData = new VertexDataStatic[_vertCount];
	memcpy( res->_indexData, _indexData, _indexCount * (_16BitIndices ? 2 : 4) );
	memcpy( res->_vertPosData, _vertPosData, _vertCount * sizeof( Vec3f ) );
	memcpy( res->_vertTanData, _vertTanData, _vertCount * sizeof( VertexDataTan ) );
	memcpy( res->_vertStaticData, _vertStaticData, _vertCount * sizeof( VertexDataStatic ) );
	res->_indexBuf = gRDI->createIndexBuffer( _indexCount * (_16BitIndices ? 2 : 4), _indexData );
	res->_posVBuf = gRDI->createVertexBuffer( _vertCount * sizeof( Vec3f ), _vertPosData );
	res->_tanVBuf = gRDI->createVertexBuffer( _vertCount * sizeof( VertexDataTan ), _vertTanData );
	res->_staticVBuf = gRDI->createVertexBuffer( _vertCount * sizeof( VertexDataStatic ), _vertStaticData );
	
	return res;
}


void GeometryResource::initDefault()
{
	_indexCount = 0;
	_vertCount = 0;
	_indexData = 0x0;
	_vertPosData = 0x0;
	_vertTanData = 0x0;
	_vertStaticData = 0x0;
	_16BitIndices = false;
	_indexBuf = defIndexBuffer;
	_posVBuf = defVertBuffer;
	_tanVBuf = defVertBuffer;
	_staticVBuf = defVertBuffer;
	_minMorphIndex = 0; _maxMorphIndex = 0;
	_skelAABB.min = Vec3f( 0, 0, 0 );
	_skelAABB.max = Vec3f( 0, 0, 0 );
}


void GeometryResource::release()
{
	if( _posVBuf != 0 && _posVBuf != defVertBuffer )
	{
		gRDI->destroyBuffer( _posVBuf );
		_posVBuf = 0;
	}
	if( _tanVBuf != 0 && _tanVBuf != defVertBuffer )
	{
		gRDI->destroyBuffer( _tanVBuf );
		_tanVBuf = 0;
	}
	if( _staticVBuf != 0 && _staticVBuf != defVertBuffer )
	{
		gRDI->destroyBuffer( _staticVBuf );
		_staticVBuf = 0;
	}
	
	if( _indexBuf != 0 && _indexBuf != defIndexBuffer )
	{
		gRDI->destroyBuffer( _indexBuf );
		_indexBuf = 0;
	}

	delete[] _indexData; _indexData = 0x0;
	delete[] _vertPosData; _vertPosData = 0x0;
	delete[] _vertTanData; _vertTanData = 0x0;
	delete[] _vertStaticData; _vertStaticData = 0x0;
	_joints.clear();
	_morphTargets.clear();
}


bool GeometryResource::raiseError( const string &msg )
{
	// Reset
	release();
	initDefault();

	Modules::log().writeError( "Geometry resource '%s': %s", _name.c_str(), msg.c_str() );
	
	return false;
}


bool GeometryResource::load( const char *data, int size )
{
	if( !Resource::load( data, size ) ) return false;

	// Make sure header is available
	if( size < 8 )
		return raiseError( "Invalid geometry resource" );
	
	char *pData = (char *)data;
	
	// Check header and version
	char id[4];
	memcpy( &id, pData, 4 ); pData += 4;
	if( id[0] != 'H' || id[1] != '3' || id[2] != 'D' || id[3] != 'G' )
		return raiseError( "Invalid geometry resource" );

	uint32 version;
	memcpy( &version, pData, sizeof( uint32 ) ); pData += sizeof( uint32 );
	if( version != 5 ) return raiseError( "Unsupported version of geometry file" );

	// Load joints
	uint32 count;
	memcpy( &count, pData, sizeof( uint32 ) ); pData += sizeof( uint32 );

	if( count > 75 )
		Modules::log().writeWarning( "Geometry resource '%s': Model has more than 75 joints; this may cause defective behavior", _name.c_str() );

	_joints.resize( count );
	for( uint32 i = 0; i < count; ++i )
	{
		Joint &joint = _joints[i];
		
		// Inverse bind matrix
		for( uint32 j = 0; j < 16; ++j )
		{
			memcpy( &joint.invBindMat.x[j], pData, sizeof( float ) ); pData += sizeof( float );
		}
	}
	
	// Load vertex stream data
	uint32 streamSize;
	memcpy( &count, pData, sizeof( uint32 ) ); pData += sizeof( uint32 );			// Number of streams
	memcpy( &streamSize, pData, sizeof( uint32 ) ); pData += sizeof( uint32 );	// Number of vertices

	_vertCount = streamSize;
	_vertPosData = new Vec3f[_vertCount];
	_vertTanData = new VertexDataTan[_vertCount];
	_vertStaticData = new VertexDataStatic[_vertCount];
	Vec3f *bitangents = new Vec3f[_vertCount];

	// Init with default data
	memset( _vertPosData, 0, _vertCount * sizeof( Vec3f ) );
	memset( _vertTanData, 0, _vertCount * sizeof( VertexDataTan ) );
	memset( _vertStaticData, 0, _vertCount * sizeof( VertexDataStatic ) );
	for( uint32 i = 0; i < _vertCount; ++i ) _vertStaticData[i].weightVec[0] = 1;

	for( uint32 i = 0; i < count; ++i )
	{
		unsigned char uc;
		short sh;
		uint32 streamID, streamElemSize;
		memcpy( &streamID, pData, sizeof( uint32 ) ); pData += sizeof( uint32 );
		memcpy( &streamElemSize, pData, sizeof( uint32 ) ); pData += sizeof( uint32 );
		std::string errormsg;

		switch( streamID )
		{
		case 0:		// Position
			if( streamElemSize != 12 )
			{
				errormsg = "Invalid position base stream";
				break;
			}
			for( uint32 j = 0; j < streamSize; ++j )
			{
				memcpy( &_vertPosData[j].x, pData, sizeof( float ) ); pData += sizeof( float );
				memcpy( &_vertPosData[j].y, pData, sizeof( float ) ); pData += sizeof( float );
				memcpy( &_vertPosData[j].z, pData, sizeof( float ) ); pData += sizeof( float );
			}
			break;
		case 1:		// Normal
			if( streamElemSize != 6 )
			{
				errormsg = "Invalid normal base stream";
				break;
			}
			for( uint32 j = 0; j < streamSize; ++j )
			{
				memcpy( &sh, pData, sizeof( short ) ); pData += sizeof( short ); _vertTanData[j].normal.x = sh / 32767.0f;
				memcpy( &sh, pData, sizeof( short ) ); pData += sizeof( short ); _vertTanData[j].normal.y = sh / 32767.0f;
				memcpy( &sh, pData, sizeof( short ) ); pData += sizeof( short ); _vertTanData[j].normal.z = sh / 32767.0f;
			}
			break;
		case 2:		// Tangent
			if( streamElemSize != 6 )
			{
				errormsg = "Invalid tangent base stream";
				break;
			}
			for( uint32 j = 0; j < streamSize; ++j )
			{
				memcpy( &sh, pData, sizeof( short ) ); pData += sizeof( short ); _vertTanData[j].tangent.x = sh / 32767.0f;
				memcpy( &sh, pData, sizeof( short ) ); pData += sizeof( short ); _vertTanData[j].tangent.y = sh / 32767.0f;
				memcpy( &sh, pData, sizeof( short ) ); pData += sizeof( short ); _vertTanData[j].tangent.z = sh / 32767.0f;
			}
			break;
		case 3:		// Bitangent
			if( streamElemSize != 6 )
			{
				errormsg = "Invalid bitangent base stream";
				break;
			}
			for( uint32 j = 0; j < streamSize; ++j )
			{
				memcpy( &sh, pData, sizeof( short ) ); pData += sizeof( short ); bitangents[j].x = sh / 32767.0f;
				memcpy( &sh, pData, sizeof( short ) ); pData += sizeof( short ); bitangents[j].y = sh / 32767.0f;
				memcpy( &sh, pData, sizeof( short ) ); pData += sizeof( short ); bitangents[j].z = sh / 32767.0f;
			}
			break;
		case 4:		// Joint indices
			if( streamElemSize != 4 )
			{
				errormsg = "Invalid joint stream";
				break;
			}
			for( uint32 j = 0; j < streamSize; ++j )
			{
				memcpy( &uc, pData, sizeof( char ) ); pData += sizeof( char ); _vertStaticData[j].jointVec[0] = (float)uc;
				memcpy( &uc, pData, sizeof( char ) ); pData += sizeof( char ); _vertStaticData[j].jointVec[1] = (float)uc;
				memcpy( &uc, pData, sizeof( char ) ); pData += sizeof( char ); _vertStaticData[j].jointVec[2] = (float)uc;
				memcpy( &uc, pData, sizeof( char ) ); pData += sizeof( char ); _vertStaticData[j].jointVec[3] = (float)uc;
			}
			break;
		case 5:		// Weights
			if( streamElemSize != 4 )
			{
				errormsg = "Invalid weight stream";
				break;
			}
			for( uint32 j = 0; j < streamSize; ++j )
			{
				memcpy( &uc, pData, sizeof( char ) ); pData += sizeof( char ); _vertStaticData[j].weightVec[0] = uc / 255.0f;
				memcpy( &uc, pData, sizeof( char ) ); pData += sizeof( char ); _vertStaticData[j].weightVec[1] = uc / 255.0f;
				memcpy( &uc, pData, sizeof( char ) ); pData += sizeof( char ); _vertStaticData[j].weightVec[2] = uc / 255.0f;
				memcpy( &uc, pData, sizeof( char ) ); pData += sizeof( char ); _vertStaticData[j].weightVec[3] = uc / 255.0f;
			}
			break;
		case 6:		// Texture Coord Set 1
			if( streamElemSize != 8 )
			{
				errormsg = "Invalid texCoord1 stream";
				break;
			}
			for( uint32 j = 0; j < streamSize; ++j )
			{
				memcpy( &_vertStaticData[j].u0, pData, sizeof( float ) ); pData += sizeof( float );
				memcpy( &_vertStaticData[j].v0, pData, sizeof( float ) ); pData += sizeof( float );
			}
			break;
		case 7:		// Texture Coord Set 2
			if( streamElemSize != 8 )
			{
				errormsg = "Invalid texCoord2 stream";
				break;
			}
			for( uint32 j = 0; j < streamSize; ++j )
			{
				memcpy( &_vertStaticData[j].u1, pData, sizeof( float ) ); pData += sizeof( float );
				memcpy( &_vertStaticData[j].v1, pData, sizeof( float ) ); pData += sizeof( float );
			}
			break;
		default:
			pData += streamElemSize * streamSize;
			Modules::log().writeWarning( "Geometry resource '%s': Ignoring unsupported vertex base stream", _name.c_str() );
			continue;
		}
		if (!errormsg.empty())
		{
			delete[] bitangents;
			return raiseError(errormsg);
		}
	}

	// Prepare bitangent data (TODO: Should be done in ColladaConv)
	for( uint32 i = 0; i < _vertCount; ++i )
	{
		_vertTanData[i].handedness = _vertTanData[i].normal.cross( _vertTanData[i].tangent ).dot( bitangents[i] ) < 0 ? -1.0f : 1.0f;
	}
	delete[] bitangents;
		
	// Load triangle indices
	memcpy( &count, pData, sizeof( uint32 ) ); pData += sizeof( uint32 );

	_indexCount = count;
    _16BitIndices = _vertCount<= 65536;
	_indexData = new char[count * (_16BitIndices ? 2 : 4)];
	if( _16BitIndices )
	{
		uint32 index;
		uint16 *pIndexData = (uint16 *)_indexData;
		for( uint32 i = 0; i < count; ++i )
		{
			memcpy( &index, pData, sizeof( uint32 ) ); pData += sizeof( uint32 );
			pIndexData[i] = (uint16)index;
		}
	}
	else
	{
		uint32 *pIndexData = (uint32 *)_indexData;
		for( uint32 i = 0; i < count; ++i )
		{
			memcpy( &pIndexData[i], pData, sizeof( uint32 ) ); pData += sizeof( uint32 );
		}
	}

	// Load morph targets
	uint32 numTargets;
	memcpy( &numTargets, pData, sizeof( uint32 ) ); pData += sizeof( uint32 );

	_morphTargets.resize( numTargets );
	for( uint32 i = 0; i < numTargets; ++i )
	{
		MorphTarget &mt = _morphTargets[i];
		char name[256];
		
		memcpy( name, pData, 256 ); pData += 256;
		mt.name = name;
		
		// Read vertex indices
		uint32 morphStreamSize;
		memcpy( &morphStreamSize, pData, sizeof( uint32 ) ); pData += sizeof( uint32 );
		mt.diffs.resize( morphStreamSize );
		for( uint32 j = 0; j < morphStreamSize; ++j )
		{
			memcpy( &mt.diffs[j].vertIndex, pData, sizeof( uint32 ) ); pData += sizeof( uint32 );
		}
		
		// Loop over streams
		memcpy( &count, pData, sizeof( uint32 ) ); pData += sizeof( uint32 );
		for( uint32 j = 0; j < count; ++j )
		{
			uint32 streamID, streamElemSize;
			memcpy( &streamID, pData, sizeof( uint32 ) ); pData += sizeof( uint32 );
			memcpy( &streamElemSize, pData, sizeof( uint32 ) ); pData += sizeof( uint32 );

			switch( streamID )
			{
			case 0:		// Position
				if( streamElemSize != 12 ) return raiseError( "Invalid position morph stream" );
				for( uint32 k = 0; k < morphStreamSize; ++k )
				{
					memcpy( &mt.diffs[k].posDiff.x, pData, sizeof( float ) ); pData += sizeof( float );
					memcpy( &mt.diffs[k].posDiff.y, pData, sizeof( float ) ); pData += sizeof( float );
					memcpy( &mt.diffs[k].posDiff.z, pData, sizeof( float ) ); pData += sizeof( float );
				}
				break;
			case 1:		// Normal
				if( streamElemSize != 12 ) return raiseError( "Invalid normal morph stream" );
				for( uint32 k = 0; k < morphStreamSize; ++k )
				{
					memcpy( &mt.diffs[k].normDiff.x, pData, sizeof( float ) ); pData += sizeof( float );
					memcpy( &mt.diffs[k].normDiff.y, pData, sizeof( float ) ); pData += sizeof( float );
					memcpy( &mt.diffs[k].normDiff.z, pData, sizeof( float ) ); pData += sizeof( float );
				}
				break;
			case 2:		// Tangent
				if( streamElemSize != 12 ) return raiseError( "Invalid tangent morph stream" );
				for( uint32 k = 0; k < morphStreamSize; ++k )
				{
					memcpy( &mt.diffs[k].tanDiff.x, pData, sizeof( float ) ); pData += sizeof( float );
					memcpy( &mt.diffs[k].tanDiff.y, pData, sizeof( float ) ); pData += sizeof( float );
					memcpy( &mt.diffs[k].tanDiff.z, pData, sizeof( float ) ); pData += sizeof( float );
				}
				break;
			case 3:		// Bitangent
				if( streamElemSize != 12 ) return raiseError( "Invalid bitangent morph stream" );
				
				// Skip data (TODO: remove from format)
				pData += morphStreamSize * sizeof( float ) * 3;
				break;
			default:
				pData += streamElemSize * morphStreamSize;
				Modules::log().writeWarning( "Geometry resource '%s': Ignoring unsupported vertex morph stream", _name.c_str() );
				continue;
			}
		}
	}

	// Find min/max morph target vertex indices
	_minMorphIndex = (unsigned)_vertCount;
	_maxMorphIndex = 0;
	for( uint32 i = 0; i < _morphTargets.size(); ++i )
	{
		for( uint32 j = 0; j < _morphTargets[i].diffs.size(); ++j )
		{
			_minMorphIndex = std::min( _minMorphIndex, _morphTargets[i].diffs[j].vertIndex );
			_maxMorphIndex = std::max( _maxMorphIndex, _morphTargets[i].diffs[j].vertIndex );
		}
	}
	if( _minMorphIndex > _maxMorphIndex )
	{
		_minMorphIndex = 0; _maxMorphIndex = 0;
	}

	// Find AABB of skeleton in bind pose
	for( uint32 i = 0; i < (uint32)_joints.size(); ++i )
	{
		Vec3f pos = _joints[i].invBindMat.inverted() * Vec3f( 0, 0, 0 );
		if( pos.x < _skelAABB.min.x ) _skelAABB.min.x = pos.x;
		if( pos.y < _skelAABB.min.y ) _skelAABB.min.y = pos.y;
		if( pos.z < _skelAABB.min.z ) _skelAABB.min.z = pos.z;
		if( pos.x > _skelAABB.max.x ) _skelAABB.max.x = pos.x;
		if( pos.y > _skelAABB.max.y ) _skelAABB.max.y = pos.y;
		if( pos.z > _skelAABB.max.z ) _skelAABB.max.z = pos.z;
	}

	// Add default joint if necessary
	if( _joints.empty() )
	{
		_joints.push_back( Joint() );
	}

	// Upload data
	if( _vertCount > 0 && _indexCount > 0 )
	{
		// Upload indices
		_indexBuf = gRDI->createIndexBuffer( _indexCount * (_16BitIndices ? 2 : 4), _indexData );
		
		// Upload vertices
		_posVBuf = gRDI->createVertexBuffer(_vertCount * sizeof( Vec3f ), _vertPosData );
		_tanVBuf = gRDI->createVertexBuffer( _vertCount * sizeof( VertexDataTan ), _vertTanData );
		_staticVBuf = gRDI->createVertexBuffer( _vertCount * sizeof( VertexDataStatic ), _vertStaticData );
	}
	
	return true;
}


int GeometryResource::getElemCount( int elem )
{
	switch( elem )
	{
	case GeometryResData::GeometryElem:
		return 1;
	default:
		return Resource::getElemCount( elem );
	}
}


int GeometryResource::getElemParamI( int elem, int elemIdx, int param )
{
	switch( elem )
	{
	case GeometryResData::GeometryElem:
		switch( param )
		{
		case GeometryResData::GeoIndexCountI:
			return (int)_indexCount;
		case GeometryResData::GeoIndices16I:
			return _16BitIndices ? 1 : 0;
		case GeometryResData::GeoVertexCountI:
			return (int)_vertCount;
		}
		break;
	}
	
	return Resource::getElemParamI( elem, elemIdx, param );
}


void *GeometryResource::mapStream( int elem, int elemIdx, int stream, bool read, bool write )
{
	if( read || write )
	{
		mappedWriteStream = -1;
		
		switch( elem )
		{
		case GeometryResData::GeometryElem:
			switch( stream )
			{
			case GeometryResData::GeoIndexStream:
				if( write ) mappedWriteStream = GeometryResData::GeoIndexStream;
				return _indexData;
			case GeometryResData::GeoVertPosStream:
				if( write ) mappedWriteStream = GeometryResData::GeoVertPosStream;
				return _vertPosData != 0x0 ? _vertPosData : 0x0;
			case GeometryResData::GeoVertTanStream:
				if( write ) mappedWriteStream = GeometryResData::GeoVertTanStream;
				return _vertTanData != 0x0 ? _vertTanData : 0x0;
			case GeometryResData::GeoVertStaticStream:
				if( write ) mappedWriteStream = GeometryResData::GeoVertStaticStream;
				return _vertStaticData != 0x0 ? _vertStaticData : 0x0;
			}
		}
	}

	return Resource::mapStream( elem, elemIdx, stream, read, write );
}


void GeometryResource::unmapStream()
{
	if( mappedWriteStream )
	{
		switch( mappedWriteStream )
		{
		case GeometryResData::GeoIndexStream:
			if( _indexData != 0x0 )
				gRDI->updateBufferData( _indexBuf, 0, _indexCount * (_16BitIndices ? 2 : 4), _indexData );
			break;
		case GeometryResData::GeoVertPosStream:
			if( _vertPosData != 0x0 )
				gRDI->updateBufferData( _posVBuf, 0, _vertCount * sizeof( Vec3f ), _vertPosData );
			break;
		case GeometryResData::GeoVertTanStream:
			if( _vertTanData != 0x0 )
				gRDI->updateBufferData( _tanVBuf, 0, _vertCount * sizeof( VertexDataTan ), _vertTanData );
			break;
		case GeometryResData::GeoVertStaticStream:
			if( _vertStaticData != 0x0 )
				gRDI->updateBufferData( _staticVBuf, 0, _vertCount * sizeof( VertexDataStatic ), _vertStaticData );
			break;
		}

		mappedWriteStream = -1;
	}
}


void GeometryResource::updateDynamicVertData()
{
	// Upload dynamic stream data
	if( _vertPosData != 0x0 )
	{
		gRDI->updateBufferData( _posVBuf, 0, _vertCount * sizeof( Vec3f ), _vertPosData );
	}
	if( _vertTanData != 0x0 )
	{
		gRDI->updateBufferData( _tanVBuf, 0, _vertCount * sizeof( VertexDataTan ), _vertTanData );
	}
}

}  // namespace
