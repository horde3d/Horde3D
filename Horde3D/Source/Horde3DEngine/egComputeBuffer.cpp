#include "egComputeBuffer.h"
#include "egModules.h"
#include "egRenderer.h"
#include "egCom.h"

#include <map>
#include <memory.h>

namespace Horde3D {

using namespace std;

// =================================================================================================
// Compute Buffer Resource
// =================================================================================================

ComputeBufferResource::ComputeBufferResource( const std::string &name, int flags ) :
	Resource( ResourceTypes::ComputeBuffer, name, flags & ResourceFlags::NoQuery ),
	_dataSize( 1024 ), _writeRequested( false ), _bufferID( 0 ), _data( 0 ), _mapped( false ), _useAsVertexBuf( false ),
	_vertexLayout( 0 ), _geoID( 0 ), _geometryParamsSet( false ) 
{
	initDefault();

	_loaded = true;
}


ComputeBufferResource::ComputeBufferResource( const std::string &name, uint32 bufferID, uint32 geometryID, int flags ) : 
	Resource( ResourceTypes::ComputeBuffer, name, flags & ResourceFlags::NoQuery ), _bufferID( bufferID ), _geoID( geometryID ), _geometryParamsSet( true ),
	_useAsVertexBuf( true ), _vertexLayout( 0 ), _mapped( false ), _data( 0 ), _dataSize( 1024 ), _writeRequested( false )
{
	_loaded = true;
}

ComputeBufferResource::~ComputeBufferResource()
{
	release();
}


void ComputeBufferResource::initDefault()
{
	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	if ( rdi->getCaps().computeShaders )
	{
		if ( _bufferID > 0 ) rdi->destroyBuffer( _bufferID );
		_bufferID = rdi->createShaderStorageBuffer( _dataSize, 0 );
		_data = new uint8[ _dataSize ];
	}
	else
		Modules::log().writeError( "Compute shaders are not available. Compute buffer cannot be created." );
}


void ComputeBufferResource::release()
{
	if ( _bufferID )
	{
		Modules::renderer().getRenderDevice()->destroyBuffer( _bufferID );
		delete[] _data; _data = 0;
	}
}


bool ComputeBufferResource::load( const char *data, int size )
{
	//	if ( !Resource::load( data, size ) ) return false;

	// currently not implemented

	return true;
}


Resource *ComputeBufferResource::clone()
{
	ComputeBufferResource *res = new ComputeBufferResource( "", _flags );

	*res = *this;

	if ( !res->_bufferID )
	{
		// no compute shaders
		delete res; res = 0;
		return 0;
	}

	// set resource parameters
	res->_dataSize = _dataSize;
	res->_vertexLayout = _vertexLayout;
	res->_useAsVertexBuf = _useAsVertexBuf;
	res->_vlBindingsData = _vlBindingsData;
	res->_geometryParamsSet = _geometryParamsSet;

	if ( _useAsVertexBuf && _geometryParamsSet )
	{
		// set new geometry, otherwise the original buffer will be used for rendering
		createGeometry();
	}

	// delete small start buffer & copy data from original buffer. 
	// WARNING: currently there is no way to get data that is modified by compute shader (GPU side)
	delete[] res->_data;
	res->_data = new uint8[ res->_dataSize ];

	memcpy( res->_data, _data, _dataSize );

	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();
	rdi->updateBufferData( 0, res->_bufferID, 0, res->_dataSize, res->_data );

	return res;
}


bool ComputeBufferResource::createGeometry()
{
	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	// register new vertex layout, so that shaders could use the buffer data, if needed
	if ( !_vertexLayout )
	{
		_vertexLayout = rdi->registerVertexLayout( ( uint32 ) _vlBindingsData.size(), _vlBindingsData.data() );
		if ( !_vertexLayout ) return false;
	}

	// create new geometry with compute buffer used as vertex buffer
	_geoID = rdi->beginCreatingGeometry( _vertexLayout );

	// calculate stride
	// only float parameter type is currently supported, if buffer is used as vertex buffer
	uint32 stride = 0;
	for ( size_t i = 0; i < _vlBindingsData.size(); ++i )
	{
		stride += _vlBindingsData[ i ].offset;
	}
	stride += sizeof( float ) * _vlBindingsData.at( _vlBindingsData.size() - 1 ).size;

	rdi->setGeomVertexParams( _geoID, _bufferID, 0, 0, stride );
	rdi->finishCreatingGeometry( _geoID );

	// indicate that geometry is already created
	_geometryParamsSet = true;

	return true;
}


bool ComputeBufferResource::setGeometry( uint32 geomID )
{
	if ( geomID == 0 ) // incorrect geometry
	{
		Modules::log().writeError( "Compute buffer resource '%s': %s", _name.c_str(), "incorrect geometry specified." );
		return false;
	}

	_geoID = geomID;
	_useAsVertexBuf = true;
	_geometryParamsSet = true;

	return true;
}


bool ComputeBufferResource::setBuffer( uint32 bufferID, uint32 bufSize )
{
	if ( bufferID == 0 || bufSize == 0 )
	{
		Modules::log().writeError( "Compute buffer resource '%s': %s", _name.c_str(), "incorrect buffer specified." );
		return false;
	}

	_bufferID = bufferID;
	
	if ( _dataSize < bufSize )
	{
		delete[] _data;

		_data = new uint8[ bufSize ];
		_dataSize = bufSize;
	}

	return true;
}


int ComputeBufferResource::getElemParamI( int elem, int elemIdx, int param ) const
{
	switch ( elem )
	{
		case ComputeBufferResData::ComputeBufElem:
		{
			switch ( param )
			{
				case ComputeBufferResData::CompBufDataSizeI:
					return _dataSize;
					break;
				case ComputeBufferResData::CompBufUseAsVertexBufferI:
					return _useAsVertexBuf;
				default:
					break;
			}

			break;
		}
	}

	return Resource::getElemParamI( elem, elemIdx, param );
}


void ComputeBufferResource::setElemParamI( int elem, int elemIdx, int param, int value )
{
	switch ( elem )
	{
		case ComputeBufferResData::ComputeBufElem:
			switch ( param )
			{
				case ComputeBufferResData::CompBufDataSizeI:
					if ( _dataSize < ( uint32 ) value )
					{
						delete[] _data;

						_dataSize = value;
						initDefault();
					}
					else _dataSize = value;

					return;

				case ComputeBufferResData::CompBufUseAsVertexBufferI:
					_useAsVertexBuf = value;
					return;
			}
			break;

		case ComputeBufferResData::DrawParamsElem:
		{
			VertexLayoutAttrib params;
			params.vbSlot = 0; // always zero because only one buffer can be specified at a time
			params.offset = params.size = 0;

			switch ( param )
			{
				case ComputeBufferResData::DrawParamsSizeI:
					if ( _vlBindingsData.empty() || elemIdx == _vlBindingsData.size() )
					{
						params.size = value;
						_vlBindingsData.push_back( params );
					}
					else if ( ( uint32 ) elemIdx > _vlBindingsData.size() )
					{
						// incorrect elemIdx
						Modules::log().writeError( "Compute buffer resource '%s': %s", _name.c_str(), "incorrect elemIdx specified." );
						break;
					}
					else
					{
						_vlBindingsData.at( elemIdx ).size = value;
					}

					return;

				case ComputeBufferResData::DrawParamsOffsetI:
					if ( _vlBindingsData.empty() || elemIdx == _vlBindingsData.size() )
					{
						params.offset = value;
						_vlBindingsData.push_back( params );
					}
					else if ( ( uint32 ) elemIdx > _vlBindingsData.size() )
					{
						// incorrect elemIdx
						Modules::log().writeError( "Compute buffer resource '%s': %s", _name.c_str(), "incorrect elemIdx specified." );
						break;
					}
					else
					{
						_vlBindingsData.at( elemIdx ).offset = value;
					}

					return;

				default:
					break;
			}

			break;
		}
	}

	Resource::setElemParamI( elem, elemIdx, param, value );
}


void ComputeBufferResource::setElemParamStr( int elem, int elemIdx, int param, const char *value )
{
	switch ( elem )
	{
		case ComputeBufferResData::DrawParamsElem:
			switch ( param )
			{
				case ComputeBufferResData::DrawParamsNameStr:
				{
					VertexLayoutAttrib params;
					params.vbSlot = 0; // always zero because only one buffer can be specified at a time
					params.offset = params.size = 0;

					if ( _vlBindingsData.empty() || elemIdx == _vlBindingsData.size() )
					{
						params.semanticName = value;
						_vlBindingsData.push_back( params );
					}
					else if ( ( uint32 ) elemIdx > _vlBindingsData.size() )
					{
						// incorrect elemIdx
						Modules::log().writeError( "Compute buffer resource '%s': %s", _name.c_str(), "incorrect elemIdx specified." );
						break;
					}
					else
					{
						_vlBindingsData.at( elemIdx ).semanticName = value;
					}

					return;
				}
				default:
					break;
			}

		default:
			break;
	}

	Resource::setElemParamStr( elem, elemIdx, param, value );
}


void *ComputeBufferResource::mapStream( int elem, int elemIdx, int stream, bool read, bool write )
{
	if ( ( read || write ) && _bufferID && !_mapped )
	{
		if ( elem == ComputeBufferResData::ComputeBufElem )
		{
//			RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

			// 			_mappedData = Modules::renderer().useScratchBuf( _dataSize );

			if ( read )
			{
				// currently reading back is not supported
				_writeRequested = false;
				return 0;
			}

			if ( write )
				_writeRequested = true;

			_mapped = true;

			return _data;
		}
	}

	return Resource::mapStream( elem, elemIdx, stream, read, write );
}


void ComputeBufferResource::unmapStream()
{
	if ( _bufferID != 0 && _writeRequested )
	{
		RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

		if ( _useAsVertexBuf && !_geometryParamsSet )
		{
			// create new geometry, set vertex bindings, etc.
			createGeometry();
		}

		rdi->updateBufferData( 0, _bufferID, 0, _dataSize, _data );
	}

	_mapped = false;
}

} // namespace

