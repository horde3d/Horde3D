#include "egCompute.h"
#include "egModules.h"
#include "egRenderer.h"
#include "egCom.h"

#include <map>

namespace Horde3D {

using namespace std;

// =================================================================================================
// Compute Buffer Resource
// =================================================================================================

ComputeBufferResource::ComputeBufferResource( const std::string &name, int flags ) : 
	Resource( ResourceTypes::ComputeBuffer, name, flags & ResourceFlags::NoQuery ),
	_dataSize( 1024 ), _writeRequested( false ), _bufferID( 0 ), _data( nullptr ), _mapped( false ), _numElements( 0 ), _useAsVertexBuf( false ),
	_vertexLayout( 0 ), _drawType( -1 ), _geoID( 0 ), _geometryParamsSet( false )
{
	initDefault();

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
		_bufferID = rdi->createShaderStorageBuffer( _dataSize, nullptr );
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
		delete[] _data; _data = nullptr;
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
		delete res; res = nullptr;
		return nullptr;
	}

	// set resource parameters
	res->_dataSize = _dataSize;
	res->_drawType = _drawType;
	res->_vertexLayout = _vertexLayout;
	res->_useAsVertexBuf = _useAsVertexBuf;
	res->_dataParams = _dataParams;
	res->_numElements = _numElements;
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
		_vertexLayout = rdi->registerVertexLayout( _dataParams.size(), _dataParams.data() );
		if ( !_vertexLayout ) return false;
	}

	// create new geometry with compute buffer as vertex buffer
	_geoID = rdi->beginCreatingGeometry( _vertexLayout );

	// calculate stride
	// only float parameter type is currently supported, if buffer is used as vertex buffer
	uint32 stride = 0;
	for ( size_t i = 0; i < _dataParams.size(); ++i )
	{
		stride += _dataParams[ i ].offset;
	}
	stride += sizeof( float ) * _dataParams.at( _dataParams.size() - 1 ).size;

	rdi->setGeomVertexParams( _geoID, _bufferID, 0, 0, stride );
	rdi->finishCreatingGeometry( _geoID );

	// indicate that geometry is already created
	_geometryParamsSet = true;

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

		case ComputeBufferResData::DrawTypeElem:
		{
			switch ( param )
			{
				case ComputeBufferResData::DataDrawTypeI:
					return _drawType;
				default:
					break;
			}

			break;
		}
		
		case ComputeBufferResData::DrawParamsElem:
		{
			switch ( param )
			{
				case ComputeBufferResData::DrawParamsElementsCountI:
					return _numElements;
				default:
					break;
			}
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

		case ComputeBufferResData::DrawTypeElem:
			switch ( param )
			{
				case ComputeBufferResData::DataDrawTypeI:
					if ( value < 0 || value > 2 ) // Triangles - 0, Lines - 1, Points - 2
					{
						Modules::log().writeError( "Compute buffer resource '%s': %s", _name.c_str(), "wrong draw type specified." );
						break;
					}

					_drawType = value;
					return;
				default:
					break;
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
					if ( _dataParams.empty() || elemIdx == _dataParams.size() )
					{
						params.size = value;
						_dataParams.push_back( params );
					}
					else if ( ( uint32 ) elemIdx > _dataParams.size() )
					{
						// incorrect elemIdx
						Modules::log().writeError( "Compute buffer resource '%s': %s", _name.c_str(), "incorrect elemIdx specified." );
						break;
					}
					else
					{
						_dataParams.at( elemIdx ).size = value;
					}

					return;

				case ComputeBufferResData::DrawParamsOffsetI:
					if ( _dataParams.empty() || elemIdx == _dataParams.size() )
					{
						params.offset = value;
						_dataParams.push_back( params );
					}
					else if ( ( uint32 ) elemIdx > _dataParams.size() )
					{
						// incorrect elemIdx
						Modules::log().writeError( "Compute buffer resource '%s': %s", _name.c_str(), "incorrect elemIdx specified." );
						break;
					}
					else
					{
						_dataParams.at( elemIdx ).offset = value;
					}

					return;

				case ComputeBufferResData::DrawParamsElementsCountI:
					_numElements = value;
					return;

				default:
					break;
			}

			break;
		}
	}

	return Resource::setElemParamI( elem, elemIdx, param, value );
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

					if ( _dataParams.empty() || elemIdx == _dataParams.size() )
					{
						params.semanticName = value;
						_dataParams.push_back( params );
					}
					else if ( ( uint32 ) elemIdx > _dataParams.size() )
					{
						// incorrect elemIdx
						Modules::log().writeError( "Compute buffer resource '%s': %s", _name.c_str(), "incorrect elemIdx specified." );
						break;
					}
					else
					{
						_dataParams.at( elemIdx ).semanticName = value;
					}

					return;
				}
				default:
					break;
			}
		default:
			break;
	}

	return Resource::setElemParamStr( elem, elemIdx, param, value );
}


void *ComputeBufferResource::mapStream( int elem, int elemIdx, int stream, bool read, bool write )
{
	if ( ( read || write ) && _bufferID && !_mapped )
	{
		if ( elem == ComputeBufferResData::ComputeBufElem )
		{
			RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

			// 			_mappedData = Modules::renderer().useScratchBuf( _dataSize );

			if ( read )
			{
				// currently reading back is not supported
				_writeRequested = false;
				return nullptr;
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


// =================================================================================================
// Compute Node
// =================================================================================================

ComputeNode::ComputeNode( const ComputeNodeTpl &computeTpl ) : SceneNode( computeTpl )
{
	_compBufferRes = computeTpl.compBufRes;
	_materialRes = computeTpl.matRes;

	_renderable = true;

	_localBBox.min = Vec3f( 0, 0, 0 );
	_localBBox.max = Vec3f( 1, 1, 1 );
}


ComputeNode::~ComputeNode()
{

}


SceneNodeTpl *ComputeNode::parsingFunc( map< string, string > &attribs )
{
// 	map< string, string >::iterator itr;
// 	ComputeNodeTpl *computeTpl = new ComputeNodeTpl( "", nullptr, nullptr );

	return nullptr;
}


SceneNode *ComputeNode::factoryFunc( const SceneNodeTpl &nodeTpl )
{
	if ( nodeTpl.type != SceneNodeTypes::Compute ) return nullptr;

	return new ComputeNode( *( ComputeNodeTpl * ) &nodeTpl );
}

void ComputeNode::onPostUpdate()
{
	_bBox = _localBBox;
	_bBox.transform( _absTrans );
}

} // namespace