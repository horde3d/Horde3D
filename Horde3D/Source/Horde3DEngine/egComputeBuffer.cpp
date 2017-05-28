#include "egComputeBuffer.h"
#include "egModules.h"
#include "egRenderer.h"
#include "egCom.h"
#include "utXML.h"

#include <map>
#include <memory.h>

namespace Horde3D {

using namespace std;

// =================================================================================================
// Compute Buffer Resource
// =================================================================================================

ComputeBufferResource::ComputeBufferResource( const std::string &name, int flags ) :
	Resource( ResourceTypes::ComputeBuffer, name, flags & ResourceFlags::NoQuery ),
	_dataSize( 1024 ), _writeRequested( false ), _bufferID( 0 ), _mapped( false ), _useAsVertexBuf( false ),
	_vertexLayout( 0 ), _geoID( 0 ), _geometryParamsSet( false ), _bufferRecreated( false )
{
	initDefault();

	_loaded = true;
}


ComputeBufferResource::ComputeBufferResource( const std::string &name, uint32 bufferID, uint32 geometryID, int flags ) : 
	Resource( ResourceTypes::ComputeBuffer, name, flags & ResourceFlags::NoQuery ), _bufferID( bufferID ), _geoID( geometryID ), _geometryParamsSet( true ),
	_useAsVertexBuf( true ), _vertexLayout( 0 ), _mapped( false ), _dataSize( 1024 ), 
	_writeRequested( false ), _bufferRecreated( false )
{
	_loaded = true;
}


ComputeBufferResource::~ComputeBufferResource()
{
	release();
}


void ComputeBufferResource::initDefault()
{
	createBuffer( 1024, 0x0 );
}


void ComputeBufferResource::createBuffer( uint32 size, uint8 *data )
{
	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	if ( rdi->getCaps().computeShaders )
	{
		if ( _bufferID > 0 ) rdi->destroyBuffer( _bufferID );
		_bufferID = rdi->createShaderStorageBuffer( size, data );
	}
	else
		Modules::log().writeError( "Compute shaders are not available. Compute buffer cannot be created." );
}


void ComputeBufferResource::release()
{
	if ( _bufferID )
	{
		Modules::renderer().getRenderDevice()->destroyBuffer( _bufferID );
	}
}


bool ComputeBufferResource::raiseError( const std::string &msg, int line )
{
	// Unmap buffer if it is mapped during loading
	if ( _mapped )
	{
		unmapStream();
	}

	// Reset
	release();
	initDefault();

	if ( line < 0 )
		Modules::log().writeError( "Compute buffer resource '%s': %s", _name.c_str(), msg.c_str() );
	else
		Modules::log().writeError( "Compute buffer resource '%s' in line %i: %s", _name.c_str(), line, msg.c_str() );

	return false;
}


bool ComputeBufferResource::load( const char *data, int size )
{
	if ( !Resource::load( data, size ) ) return false;

	XMLDoc doc;
	doc.parseBuffer( data, size );
	if ( doc.hasError() )
		return raiseError( "XML parsing error" );

	XMLNode rootNode = doc.getRootNode();
	if ( strcmp( rootNode.getName(), "ComputeBuffer" ) != 0 )
		return raiseError( "Not a compute buffer resource file" );

	if ( rootNode.getAttribute( "dataSize" ) == 0x0 ) return raiseError( "Missing ComputeBuffer attribute 'dataSize'" );
	if ( rootNode.getAttribute( "drawable" ) == 0x0 ) return raiseError( "Missing ComputeBuffer attribute 'drawable'" );

	// Buffer size
	int size = atoi( rootNode.getAttribute( "dataSize", "" ) );
	if ( size < 0 )
	{
		return raiseError( "Incorrect 'dataSize' value specified" );
	}

	setElemParamI( ComputeBufferResData::ComputeBufElem, 0, ComputeBufferResData::CompBufDataSizeI, size );

	// Buffer is drawable
	if ( _stricmp( rootNode.getAttribute( "drawable", "false" ), "true" ) == 0 ||
		 _stricmp( rootNode.getAttribute( "drawable", "0" ), "1" ) == 0 )
	{
		_useAsVertexBuf = true;
	}

	// Vertex bindings
	int totalBindingsCount = rootNode.countChildNodes( "Bindings" );
	if ( totalBindingsCount > 0 ) _vlBindingsData.resize( totalBindingsCount );

	XMLNode node1 = rootNode.getFirstChild( "Bindings" );
	while ( !node1.isEmpty() )
	{
		if ( node1.getAttribute( "name" ) == 0x0 ) return raiseError( "Missing Bindings attribute 'name'" );
		if ( node1.getAttribute( "offset" ) == 0x0 ) return raiseError( "Missing Bindings attribute 'offset'" );
		if ( node1.getAttribute( "size" ) == 0x0 ) return raiseError( "Missing Bindings attribute 'size'" );
		if ( node1.getAttribute( "attribNumber" ) == 0x0 ) return raiseError( "Missing Bindings attribute 'attribNumber'" );

		VertexLayoutAttrib layout;
		layout.semanticName = node1.getAttribute( "name", "0" );
		layout.offset = atoi( node1.getAttribute( "offset", "0" ) );
		layout.size = atoi( node1.getAttribute( "size", "0" ) );
		layout.vbSlot = 0;

		int curAttribSlot = atoi( node1.getAttribute( "attribNumber" ) );
		if ( curAttribSlot >= 0 && curAttribSlot < totalBindingsCount )
		{
			_vlBindingsData[ curAttribSlot ] = layout;
		} 
		else
		{
			return raiseError( "Incorrect attribNumber value" );
		}

		node1 = node1.getNextSibling( "Bindings" );
	}

	if ( _useAsVertexBuf && !_vlBindingsData.empty() )
	{
		if ( !createGeometry() ) return false;
	}

	// Data
	// Currently only float data is supported
	XMLNode node1 = rootNode.getFirstChild( "Data" );
	while ( !node1.isEmpty() )
	{
		// parser assumes that values are separated by ';' character
		uint8 *bufData = ( uint8 *) mapStream( ComputeBufferResData::ComputeBufElem, 0, 0, false, true );
		const char *strData = node1.getText();

		uint8 *pBufData = bufData;
		const char *pStrData = strData;
		
		int strDataSize = strlen( strData );
		const char *end = strData + strDataSize;

		const char *valueStartPos = pStrData;
		const char *valueEndPos = 0x0;
		int charCounter = 0;
		int bytesCopied = 0;

		while ( pStrData < end )
		{
			if ( *pStrData == ';' )
			{
				// convert and append data to buffer
				if ( bytesCopied + 4 >= _dataSize ) return raiseError( "Data size in 'Data' section exceeds the specified buffer size" );

				std::string val( valueStartPos, valueEndPos );
				float fval = ( float ) atof( val.c_str() );

				memcpy( pBufData, &fval, 4 );

				if ( pStrData + 1 < end )
				{
					valueStartPos = pStrData + 1;
					valueEndPos = valueStartPos;
				}

				pBufData += 4;
				bytesCopied += 4;
				charCounter = 0;
			}
			else
			{
				charCounter++;
				
				if ( charCounter < 32 ) valueEndPos++;
				else 
				{
					return raiseError( "Incorrect value in 'Data' element" );
				}
			}
	
			pStrData++;
		}

	}

	return true;
}


Resource *ComputeBufferResource::clone()
{
	ComputeBufferResource *res = new ComputeBufferResource( "", _flags );

	*res = *this;

	if ( !res->_bufferID )
	{
		// no compute shaders
		delete res; res = 0x0;
		return 0;
	}

	// set resource parameters
	res->_dataSize = _dataSize;
	res->_vertexLayout = _vertexLayout;
	res->_useAsVertexBuf = _useAsVertexBuf;
	res->_vlBindingsData = _vlBindingsData;
	res->_geometryParamsSet = _geometryParamsSet;
	res->_bufferRecreated = true; 

	if ( _useAsVertexBuf && _geometryParamsSet )
	{
		// set new geometry, otherwise the original buffer will be used for rendering
		createGeometry();
	}

	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	// map original buffer and copy its contents to the new buffer
	uint8 *data = ( uint8 * ) mapStream( ComputeBufferResData::ComputeBufElem, 0, 0, true, false );
	rdi->updateBufferData( 0, res->_bufferID, 0, res->_dataSize, data );
	unmapStream();

	return res;
}


bool ComputeBufferResource::createGeometry()
{
	RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

	// register new vertex layout, so that shaders could use the buffer data, if needed
	if ( !_vertexLayout )
	{
		_vertexLayout = rdi->registerVertexLayout( ( uint32 ) _vlBindingsData.size(), _vlBindingsData.data() );
		if ( !_vertexLayout )
		{
			Modules::log().writeDebugInfo( "Too many vertex layouts. Increase the max count of vertex layouts in renderer backend." );
			return false;
		}
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
	_dataSize = bufSize;

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

		case ComputeBufferResData::DrawParamsElem:
		{
			switch ( param )
			{
				case ComputeBufferResData::DrawParamsOffsetI:
					if ( _vlBindingsData.empty() || elemIdx < 0 || elemIdx > _vlBindingsData.size() - 1 ) break;

					return _vlBindingsData.at( elemIdx ).offset;
				case ComputeBufferResData::DrawParamsSizeI:
					if ( _vlBindingsData.empty() || elemIdx < 0 || elemIdx > _vlBindingsData.size() - 1 )

					return _vlBindingsData.at( elemIdx ).size;
				case  ComputeBufferResData::DrawParamsCountI:
					return (int) _vlBindingsData.size();
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
						_dataSize = value;
						createBuffer( _dataSize, 0x0 );
						_bufferRecreated = true;
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


const char *ComputeBufferResource::getElemParamStr( int elem, int elemIdx, int param ) const
{
	switch ( elem )
	{
		case ComputeBufferResData::DrawParamsElem:
			switch ( param )
			{
				case ComputeBufferResData::DrawParamsNameStr:
					if ( _vlBindingsData.empty() || elemIdx < 0 || (unsigned ) elemIdx > _vlBindingsData.size() - 1 )
					{
						// incorrect elemIdx
						Modules::log().writeError( "Compute buffer resource '%s': %s", _name.c_str(), "incorrect elemIdx specified." );
						return "";
					}

					return _vlBindingsData.at( elemIdx ).semanticName.c_str();
				default:
					break;
			}
		default:
			break;
	}

	return Resource::getElemParamStr( elem, elemIdx, param );
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
			RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

			_mapped = true;

			// Ensure that we are getting data that is not updated right now by the GPU
			rdi->setMemoryBarrier( RDIDrawBarriers::VertexBufferBarrier );

			if ( read )
			{
				_writeRequested = false;
				return rdi->mapBuffer( 0, _bufferID, 0, _dataSize, Read );
			}

			if ( write )
			{
				_writeRequested = true;

				if ( _bufferRecreated )
				{
					return Modules::renderer().useScratchBuf( _dataSize, 1 );
				} 
				else
				{
					return rdi->mapBuffer( 0, _bufferID, 0, _dataSize, Write );
				}
			}

		}
	}

	return Resource::mapStream( elem, elemIdx, stream, read, write );
}


void ComputeBufferResource::unmapStream()
{
	if ( _mapped && _bufferID != 0 )
	{
		RenderDeviceInterface *rdi = Modules::renderer().getRenderDevice();

		if ( _useAsVertexBuf && !_geometryParamsSet )
		{
			// create new geometry, set vertex bindings, etc.
			createGeometry();
		}

		if ( _bufferRecreated )
		{
			// GL4: for some reason NVIDIA hardware has a significant performance drop if you upload 
			// a large chunk of data to a not initialized buffer (with 0x0 as data) with buffer map
			// but has no performance drop if you upload it with BufferData. Therefore this workaround with scratch buffer is used.
			rdi->updateBufferData( _geoID, _bufferID, 0, _dataSize, Modules::renderer().useScratchBuf( _dataSize, 1 ) );

			_bufferRecreated = false;
		} 
		else
		{
			rdi->unmapBuffer( 0, _bufferID );
		}
	}

	_mapped = false;
}

} // namespace
