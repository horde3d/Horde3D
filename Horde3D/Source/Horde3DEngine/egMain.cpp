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

#include "egModules.h"
#include "egCom.h"
#include "egExtensions.h"
#include "egRenderer.h"
#include "egModel.h"
#include "egLight.h"
#include "egCamera.h"
#include "egParticle.h"
#include "egTexture.h"
#include <cstdlib>
#include <cstring>
#include <string>

#ifdef PLATFORM_WIN
#   define WIN32_LEAN_AND_MEAN 1
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif

#include "utDebug.h"


// Check some platform-dependent assumptions
// Note: this function is never called but just wraps the compile time asserts
static void __ValidatePlatform__()
{
	ASSERT_STATIC( sizeof( int64 ) == 8 );
	ASSERT_STATIC( sizeof( int ) == 4 );
	ASSERT_STATIC( sizeof( short ) == 2 );
	ASSERT_STATIC( sizeof( char ) == 1 );
}


using namespace Horde3D;
using namespace std;


bool initialized;
const char *emptyCString = "";
std::string emptyString = emptyCString;
std::string strPool[4];  // String pool for avoiding memory allocations of temporary string objects


inline const string &safeStr( const char *str, int index )
{
	if( str != 0x0 ) return strPool[index] = str;
	else return emptyString;
}


// =================================================================================================
// Basic functions
// =================================================================================================

DLLEXP const char *h3dGetVersionString()
{
	return Modules::versionString;
}


DLLEXP bool h3dCheckExtension( const char *extensionName )
{
	return Modules::extMan().checkExtension( safeStr( extensionName, 0 ) );
}


DLLEXP bool h3dGetError()
{
	return Modules::getError();
}


DLLEXP bool h3dInit()
{
	if( initialized )
	{	
		// Init states for additional rendering context
		Modules::renderer().initStates();
		return true;
	}
	initialized = true;

	return Modules::init();
}


DLLEXP void h3dRelease()
{
	Modules::release();
	initialized = false;
}


DLLEXP void h3dRender( NodeHandle cameraNode )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( cameraNode );
	APIFUNC_VALIDATE_NODE_TYPE( sn, SceneNodeTypes::Camera, "h3dRender", APIFUNC_RET_VOID );
	
	Modules::renderer().render( (CameraNode *)sn );
}


DLLEXP void h3dFinalizeFrame()
{
	Modules::renderer().finalizeFrame();
}


DLLEXP void h3dClear()
{
	Modules::sceneMan().removeNode( Modules::sceneMan().getRootNode() );
	Modules::resMan().clear();
}


// =================================================================================================
// General functions
// =================================================================================================

DLLEXP const char *h3dGetMessage( int *level, float *time )
{
	static string msgText;
	static LogMessage msg;
	
	if( Modules::log().getMessage( msg ) )
	{
		if( level != 0x0 ) *level = msg.level;
		if( time != 0x0 ) *time = msg.time;
		return msg.text.c_str();
	}
	else
		return emptyCString;
}


DLLEXP float h3dGetOption( EngineOptions::List param )
{
	return Modules::config().getOption( param );
}


DLLEXP bool h3dSetOption( EngineOptions::List param, float value )
{
	return Modules::config().setOption( param, value );
}


DLLEXP float h3dGetStat( EngineStats::List param, bool reset )
{
	return Modules::stats().getStat( param, reset );
}


DLLEXP void h3dShowOverlays( const float *verts, int vertCount, float colR, float colG,
                             float colB, float colA, uint32 materialRes, int flags )
{
	Resource *resObj = Modules::resMan().resolveResHandle( materialRes ); 
	APIFUNC_VALIDATE_RES_TYPE( resObj, ResourceTypes::Material, "h3dShowOverlays", APIFUNC_RET_VOID );

	float rgba[4] = { colR, colG, colB, colA };
	Modules::renderer().showOverlays( verts, (uint32)vertCount, rgba, (MaterialResource *)resObj, flags );
}


DLLEXP void h3dClearOverlays()
{
	Modules::renderer().clearOverlays();
}


// =================================================================================================
// Resource functions
// =================================================================================================

DLLEXP int h3dGetResType( ResHandle res )
{
	Resource *resObj = Modules::resMan().resolveResHandle( res );
	APIFUNC_VALIDATE_RES( resObj, "h3dGetResType", ResourceTypes::Undefined );
	
	return resObj->getType();
}


DLLEXP const char *h3dGetResName( ResHandle res )
{
	Resource *resObj = Modules::resMan().resolveResHandle( res );
	APIFUNC_VALIDATE_RES( resObj, "h3dGetResName", emptyCString );
	
	return resObj->getName().c_str();
}


DLLEXP ResHandle h3dGetNextResource( int type, ResHandle start )
{
	Resource *resObj = Modules::resMan().getNextResource( type, start );
	
	return resObj != 0x0 ? resObj->getHandle() : 0;
}


DLLEXP ResHandle h3dFindResource( int type, const char *name )
{
	Resource *resObj = Modules::resMan().findResource( type, safeStr( name, 0 ) );
	
	return resObj != 0x0 ? resObj->getHandle() : 0;
}


DLLEXP ResHandle h3dAddResource( int type, const char *name, int flags )
{
	return Modules::resMan().addResource( type, safeStr( name, 0 ), flags, true );
}


DLLEXP ResHandle h3dCloneResource( ResHandle sourceRes, const char *name )
{
	Resource *resObj = Modules::resMan().resolveResHandle( sourceRes );
	APIFUNC_VALIDATE_RES( resObj, "h3dCloneResource", 0 );
	
	return Modules::resMan().cloneResource( *resObj, safeStr( name, 0 ) );
}


DLLEXP int h3dRemoveResource( ResHandle res )
{
	Resource *resObj = Modules::resMan().resolveResHandle( res );
	APIFUNC_VALIDATE_RES( resObj, "h3dRemoveResource", -1 );
	
	return Modules::resMan().removeResource( *resObj, true );
}


DLLEXP bool h3dIsResLoaded( ResHandle res )
{
	Resource *resObj = Modules::resMan().resolveResHandle( res );
	APIFUNC_VALIDATE_RES( resObj, "h3dIsResLoaded", false );
	
	return resObj->isLoaded();
}


DLLEXP bool h3dLoadResource( ResHandle res, const char *data, int size )
{
	Resource *resObj = Modules::resMan().resolveResHandle( res );
	APIFUNC_VALIDATE_RES( resObj, "h3dLoadResource", false );
	
	Modules::log().writeInfo( "Loading resource '%s'", resObj->getName().c_str() );
	return resObj->load( data, size );
}


DLLEXP void h3dUnloadResource( ResHandle res )
{
	Resource *resObj = Modules::resMan().resolveResHandle( res );
	APIFUNC_VALIDATE_RES( resObj, "h3dUnloadResource", APIFUNC_RET_VOID );

	resObj->unload();
}


DLLEXP int h3dGetResElemCount( ResHandle res, int elem )
{
	Resource *resObj = Modules::resMan().resolveResHandle( res );
	APIFUNC_VALIDATE_RES( resObj, "h3dGetResElemCount", 0 );

	return resObj->getElemCount( elem );
}


DLLEXP int h3dFindResElem( ResHandle res, int elem, int param, const char *value )
{
	Resource *resObj = Modules::resMan().resolveResHandle( res );
	APIFUNC_VALIDATE_RES( resObj, "h3dFindResElem", -1 );

	return resObj->findElem( elem, param, value != 0x0 ? value : emptyCString );
}


DLLEXP int h3dGetResParamI( ResHandle res, int elem, int elemIdx, int param )
{
	Resource *resObj = Modules::resMan().resolveResHandle( res );
	APIFUNC_VALIDATE_RES( resObj, "h3dGetResParamI", Horde3D::Math::MinInt32 );
	
	return resObj->getElemParamI( elem, elemIdx, param );
}


DLLEXP void h3dSetResParamI( ResHandle res, int elem, int elemIdx, int param, int value )
{
	Resource *resObj = Modules::resMan().resolveResHandle( res );
	APIFUNC_VALIDATE_RES( resObj, "h3dSetResParamI", APIFUNC_RET_VOID );

	resObj->setElemParamI( elem, elemIdx, param, value );
}


DLLEXP float h3dGetResParamF( ResHandle res, int elem, int elemIdx, int param, int compIdx )
{
	Resource *resObj = Modules::resMan().resolveResHandle( res );
	APIFUNC_VALIDATE_RES( resObj, "h3dGetResParamF", Horde3D::Math::NaN );

	return resObj->getElemParamF( elem, elemIdx, param, compIdx );
}


DLLEXP void h3dSetResParamF( ResHandle res, int elem, int elemIdx, int param, int compIdx, float value )
{
	Resource *resObj = Modules::resMan().resolveResHandle( res );
	APIFUNC_VALIDATE_RES( resObj, "h3dSetResParamF", APIFUNC_RET_VOID );

	resObj->setElemParamF( elem, elemIdx, param, compIdx, value );
}


DLLEXP const char *h3dGetResParamStr( ResHandle res, int elem, int elemIdx, int param )
{
	Resource *resObj = Modules::resMan().resolveResHandle( res );
	APIFUNC_VALIDATE_RES( resObj, "h3dGetResParamStr", emptyCString );

	return resObj->getElemParamStr( elem, elemIdx, param );
}


DLLEXP void h3dSetResParamStr( ResHandle res, int elem, int elemIdx, int param, const char *value )
{
	Resource *resObj = Modules::resMan().resolveResHandle( res );
	APIFUNC_VALIDATE_RES( resObj, "h3dSetResParamStr", APIFUNC_RET_VOID );
	
	resObj->setElemParamStr( elem, elemIdx, param, value != 0x0 ? value : emptyCString );
}


DLLEXP void *h3dMapResStream( ResHandle res, int elem, int elemIdx, int stream, bool read, bool write )
{
	Resource *resObj = Modules::resMan().resolveResHandle( res );
	APIFUNC_VALIDATE_RES( resObj, "h3dMapResStream", 0x0 );

	return resObj->mapStream( elem, elemIdx, stream, read, write );
}


DLLEXP void h3dUnmapResStream( ResHandle res )
{
	Resource *resObj = Modules::resMan().resolveResHandle( res );
	APIFUNC_VALIDATE_RES( resObj, "h3dUnmapResStream", APIFUNC_RET_VOID );

	resObj->unmapStream();
}


DLLEXP ResHandle h3dQueryUnloadedResource( int index )
{
	return Modules::resMan().queryUnloadedResource( index );
}


DLLEXP void h3dReleaseUnusedResources()
{
	Modules::resMan().releaseUnusedResources();
}


DLLEXP ResHandle h3dCreateTexture( const char *name, int width, int height, int fmt, int flags )
{
	TextureResource *texRes = new TextureResource( safeStr( name, 0 ), (uint32)width,
		(uint32)height, 1, (TextureFormats::List)fmt, flags );

	ResHandle res = Modules::resMan().addNonExistingResource( *texRes, true );
	if( res == 0 )
	{	
		Modules::log().writeDebugInfo( "Failed to add resource in h3dCreateTexture; maybe the name is already in use?", res );
		delete texRes;
	}

	return res;
}


DLLEXP void h3dSetShaderPreambles( const char *vertPreamble, const char *fragPreamble )
{
	ShaderResource::setPreambles( safeStr( vertPreamble, 0 ), safeStr( fragPreamble, 1 ) );
}


DLLEXP bool h3dSetMaterialUniform( ResHandle materialRes, const char *name, float a, float b, float c, float d )
{
	Resource *resObj = Modules::resMan().resolveResHandle( materialRes );
	APIFUNC_VALIDATE_RES_TYPE( resObj, ResourceTypes::Material, "h3dSetMaterialUniform", false );

	return ((MaterialResource *)resObj)->setUniform( safeStr( name, 0 ), a, b, c, d );
}


DLLEXP void h3dResizePipelineBuffers( ResHandle pipeRes, int width, int height )
{
	Resource *resObj = Modules::resMan().resolveResHandle( pipeRes );
	APIFUNC_VALIDATE_RES_TYPE( resObj, ResourceTypes::Pipeline, "h3dResizePipelineBuffers", APIFUNC_RET_VOID );

	PipelineResource *pipeResObj = (PipelineResource *)resObj;
	pipeResObj->resize( width, height );
}


DLLEXP bool h3dGetRenderTargetData( ResHandle pipelineRes, const char *targetName, int bufIndex,
                                    int *width, int *height, int *compCount, void *dataBuffer, int bufferSize )
{
	if( pipelineRes != 0 )
	{
		Resource *resObj = Modules::resMan().resolveResHandle( pipelineRes );
		APIFUNC_VALIDATE_RES_TYPE( resObj, ResourceTypes::Pipeline, "h3dGetRenderTargetData", false );
		
		return ((PipelineResource *)resObj)->getRenderTargetData( safeStr( targetName, 0 ), bufIndex,
			width, height, compCount, dataBuffer, bufferSize );
	}
	else
	{
		return gRDI->getRenderBufferData( 0, bufIndex, width, height, compCount, dataBuffer, bufferSize );
	}
}


// =================================================================================================
// Scene graph functions
// =================================================================================================

DLLEXP int h3dGetNodeType( NodeHandle node )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dGetNodeType", SceneNodeTypes::Undefined );
	
	return sn->getType();
}


DLLEXP NodeHandle h3dGetNodeParent( NodeHandle node )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dGetNodeParent", 0 );
	
	return sn->getParent() != 0x0 ? sn->getParent()->getHandle() : 0;
}


DLLEXP bool h3dSetNodeParent( NodeHandle node, NodeHandle parent )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dSetNodeParent", false );
	SceneNode *snp = Modules::sceneMan().resolveNodeHandle( parent );
	APIFUNC_VALIDATE_NODE( snp, "h3dSetNodeParent", false );
	
	return Modules::sceneMan().relocateNode( *sn, *snp );
}


DLLEXP NodeHandle h3dGetNodeChild( NodeHandle parent, int index )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( parent );
	APIFUNC_VALIDATE_NODE( sn, "h3dGetNodeChild", 0 );

	if( (unsigned)index < sn->getChildren().size() )
		return sn->getChildren()[index]->getHandle();
	else
		return 0;
}


DLLEXP NodeHandle h3dAddNodes( NodeHandle parent, ResHandle sceneGraphRes )
{
	SceneNode *parentNode = Modules::sceneMan().resolveNodeHandle( parent );
	APIFUNC_VALIDATE_NODE( parentNode, "h3dAddNodes", 0 );
	
	Resource *sgRes = Modules::resMan().resolveResHandle( sceneGraphRes );
	APIFUNC_VALIDATE_RES_TYPE( sgRes, ResourceTypes::SceneGraph, "h3dAddNodes", 0 );

	if( !sgRes->isLoaded() )
	{
		Modules::log().writeDebugInfo( "Unloaded SceneGraph resource passed to h3dAddNodes" );
		return 0;
	}
	
	//Modules::log().writeInfo( "Adding nodes from SceneGraph resource '%s'", res->getName().c_str() );
	return Modules::sceneMan().addNodes( *parentNode, *(SceneGraphResource *)sgRes );
}


DLLEXP void h3dRemoveNode( NodeHandle node )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dRemoveNode", APIFUNC_RET_VOID );

	//Modules::log().writeInfo( "Removing node %i", node );
	Modules::sceneMan().removeNode( *sn );
}


DLLEXP bool h3dCheckNodeTransFlag( NodeHandle node, bool reset )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dCheckNodeTransFlag", false );
	
	return sn->checkTransformFlag( reset );
}


DLLEXP void h3dGetNodeTransform( NodeHandle node, float *tx, float *ty, float *tz,
                                 float *rx, float *ry, float *rz, float *sx, float *sy, float *sz )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dGetNodeTransform", APIFUNC_RET_VOID );
	
	Vec3f trans, rot, scale;
	sn->getTransform( trans, rot, scale );

	if( tx != 0x0 ) *tx = trans.x; if( ty != 0x0 ) *ty = trans.y; if( tz != 0x0 ) *tz = trans.z;
	if( rx != 0x0 ) *rx = rot.x; if( ry != 0x0 ) *ry = rot.y; if( rz != 0x0 ) *rz = rot.z;
	if( sx != 0x0 ) *sx = scale.x; if( sy != 0x0 ) *sy = scale.y; if( sz != 0x0 ) *sz = scale.z;
}


DLLEXP void h3dSetNodeTransform( NodeHandle node, float tx, float ty, float tz,
                                 float rx, float ry, float rz, float sx, float sy, float sz )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dSetNodeTransform", APIFUNC_RET_VOID );
	
	sn->setTransform( Vec3f( tx, ty, tz ), Vec3f( rx, ry, rz ), Vec3f( sx, sy, sz ) );
}


DLLEXP void h3dGetNodeTransMats( NodeHandle node, const float **relMat, const float **absMat )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dGetNodeTransMats", APIFUNC_RET_VOID );
	
	sn->getTransMatrices( relMat, absMat );
}


DLLEXP void h3dSetNodeTransMat( NodeHandle node, const float *mat4x4 )
{
	static Matrix4f mat;
	
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dSetNodeTransMat", APIFUNC_RET_VOID );
	if( mat4x4 == 0x0 )
	{	
		Modules::setError( "Invalid pointer in h3dSetNodeTransMat" );
		return;
	}

	memcpy( mat.c, mat4x4, 16 * sizeof( float ) );
	sn->setTransform( mat );
}


DLLEXP int h3dGetNodeParamI( NodeHandle node, int param )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dGetNodeParamI", Horde3D::Math::MinInt32 );

	return sn->getParamI( param );
}


DLLEXP void h3dSetNodeParamI( NodeHandle node, int param, int value )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dSetNodeParamI", APIFUNC_RET_VOID );

	sn->setParamI( param, value );
}


DLLEXP float h3dGetNodeParamF( NodeHandle node, int param, int compIdx )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dGetNodeParamF", Horde3D::Math::NaN );
	
	return sn->getParamF( param, compIdx );
}


DLLEXP void h3dSetNodeParamF( NodeHandle node, int param, int compIdx, float value )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dSetNodeParamF", APIFUNC_RET_VOID );

	sn->setParamF( param, compIdx, value );
}


DLLEXP const char *h3dGetNodeParamStr( NodeHandle node, int param )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dGetNodeParamStr", emptyCString );
	
	return sn->getParamStr( param );
}


DLLEXP void h3dSetNodeParamStr( NodeHandle node, int param, const char *name )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dSetNodeParamStr", APIFUNC_RET_VOID );
	
	sn->setParamStr( param, name != 0x0 ? name : emptyCString );
}


DLLEXP int h3dGetNodeFlags( NodeHandle node )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dGetNodeFlags", 0 );
	return sn->getFlags();
}


DLLEXP void h3dSetNodeFlags( NodeHandle node, int flags, bool recursive )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dSetNodeFlags", APIFUNC_RET_VOID );
	sn->setFlags( flags, recursive );
}


DLLEXP void h3dGetNodeAABB( NodeHandle node, float *minX, float *minY, float *minZ,
                            float *maxX, float *maxY, float *maxZ )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dGetNodeAABB", APIFUNC_RET_VOID );

	Modules::sceneMan().updateNodes();
	if( minX != 0x0 ) *minX = sn->getBBox().min.x;
	if( minY != 0x0 ) *minY = sn->getBBox().min.y;
	if( minZ != 0x0 ) *minZ = sn->getBBox().min.z;
	if( maxX != 0x0 ) *maxX = sn->getBBox().max.x;
	if( maxY != 0x0 ) *maxY = sn->getBBox().max.y;
	if( maxZ != 0x0 ) *maxZ = sn->getBBox().max.z;
}


DLLEXP int h3dFindNodes( NodeHandle startNode, const char *name, int type )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( startNode );
	APIFUNC_VALIDATE_NODE( sn, "h3dFindNodes", 0 );

	Modules::sceneMan().clearFindResults();
	return Modules::sceneMan().findNodes( *sn, safeStr( name, 0 ), type );
}


DLLEXP NodeHandle h3dGetNodeFindResult( int index )
{
	SceneNode *sn = Modules::sceneMan().getFindResult( index );
	
	return sn != 0x0 ? sn->getHandle() : 0;
}


DLLEXP void h3dSetNodeUniforms( NodeHandle node, float *uniformData, int count )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dSetNodeUniforms", APIFUNC_RET_VOID );
	sn->setCustomInstData( uniformData, (uint32)count );
}


DLLEXP NodeHandle h3dCastRay( NodeHandle node, float ox, float oy, float oz, float dx, float dy, float dz, int numNearest )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dCastRay", 0 );

	Modules::sceneMan().updateNodes();
	return Modules::sceneMan().castRay( *sn, Vec3f( ox, oy, oz ), Vec3f( dx, dy, dz ), numNearest );
}


DLLEXP bool h3dGetCastRayResult( int index, NodeHandle *node, float *distance, float *intersection )
{
	CastRayResult crr;
	if( Modules::sceneMan().getCastRayResult( index, crr ) )
	{
		if( node ) *node = crr.node->getHandle();
		if( distance ) *distance = crr.distance;
		if( intersection )
		{
			intersection[0] = crr.intersection.x;
			intersection[1] = crr.intersection.y;
			intersection[2] = crr.intersection.z;
		}

		return true;
	}

	return false;
}


DLLEXP int h3dCheckNodeVisibility( NodeHandle node, NodeHandle cameraNode, bool checkOcclusion, bool calcLod )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( node );
	APIFUNC_VALIDATE_NODE( sn, "h3dCheckNodeVisibility", -1 );
	SceneNode *cam = Modules::sceneMan().resolveNodeHandle( cameraNode );
	APIFUNC_VALIDATE_NODE_TYPE( cam, SceneNodeTypes::Camera, "h3dCheckNodeVisibility", -1 );
	
	return Modules::sceneMan().checkNodeVisibility( *sn, *(CameraNode *)cam, checkOcclusion, calcLod );
}


DLLEXP NodeHandle h3dAddGroupNode( NodeHandle parent, const char *name )
{
	SceneNode *parentNode = Modules::sceneMan().resolveNodeHandle( parent );
	APIFUNC_VALIDATE_NODE( parentNode, "h3dAddGroupNode", 0 );

	//Modules::log().writeInfo( "Adding Group node '%s'", safeStr( name ).c_str() );
	GroupNodeTpl tpl( safeStr( name, 0 ) );
	SceneNode *sn = Modules::sceneMan().findType( SceneNodeTypes::Group )->factoryFunc( tpl );
	return Modules::sceneMan().addNode( sn, *parentNode );
}


DLLEXP NodeHandle h3dAddModelNode( NodeHandle parent, const char *name, ResHandle geometryRes )
{
	SceneNode *parentNode = Modules::sceneMan().resolveNodeHandle( parent );
	APIFUNC_VALIDATE_NODE( parentNode, "h3dAddModelNode", 0 );
	Resource *geoRes = Modules::resMan().resolveResHandle( geometryRes );
	APIFUNC_VALIDATE_RES_TYPE( geoRes, ResourceTypes::Geometry, "h3dAddModelNode", 0 );

	//Modules::log().writeInfo( "Adding Model node '%s'", safeStr( name ).c_str() );
	ModelNodeTpl tpl( safeStr( name, 0 ), (GeometryResource *)geoRes );
	SceneNode *sn = Modules::sceneMan().findType( SceneNodeTypes::Model )->factoryFunc( tpl );
	return Modules::sceneMan().addNode( sn, *parentNode );
}


DLLEXP void h3dSetupModelAnimStage( NodeHandle modelNode, int stage, ResHandle animationRes, int layer,
                                    const char *startNode, bool additive )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( modelNode );
	APIFUNC_VALIDATE_NODE_TYPE( sn, SceneNodeTypes::Model, "h3dSetupModelAnimStage", APIFUNC_RET_VOID );
	Resource *animRes = 0x0;
	if( animationRes != 0 )
	{
		animRes = Modules::resMan().resolveResHandle( animationRes );
		APIFUNC_VALIDATE_RES_TYPE( animRes, ResourceTypes::Animation, "h3dSetupModelAnimStage", APIFUNC_RET_VOID );
	}
	
	((ModelNode *)sn)->setupAnimStage( stage, (AnimationResource *)animRes, layer,
	                                   safeStr( startNode, 0 ), additive );
}


DLLEXP void h3dGetModelAnimParams( NodeHandle modelNode, int stage, float *time, float *weight )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( modelNode );
	APIFUNC_VALIDATE_NODE_TYPE( sn, SceneNodeTypes::Model, "h3dGetModelAnimParams", APIFUNC_RET_VOID );
	
	((ModelNode *)sn)->getAnimParams( stage, time, weight );
}


DLLEXP void h3dSetModelAnimParams( NodeHandle modelNode, int stage, float time, float weight )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( modelNode );
	APIFUNC_VALIDATE_NODE_TYPE( sn, SceneNodeTypes::Model, "h3dSetModelAnimParams", APIFUNC_RET_VOID );
	
	((ModelNode *)sn)->setAnimParams( stage, time, weight );
}


DLLEXP bool h3dSetModelMorpher( NodeHandle modelNode, const char *target, float weight )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( modelNode );
	APIFUNC_VALIDATE_NODE_TYPE( sn, SceneNodeTypes::Model, "h3dSetModelMorpher", false );
	
	return ((ModelNode *)sn)->setMorphParam( safeStr( target, 0 ), weight );
}


DLLEXP void h3dUpdateModel( NodeHandle modelNode, int flags )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( modelNode );
	APIFUNC_VALIDATE_NODE_TYPE( sn, SceneNodeTypes::Model, "h3dUpdateModel", APIFUNC_RET_VOID );

	((ModelNode *)sn)->update( flags );
}


DLLEXP NodeHandle h3dAddMeshNode( NodeHandle parent, const char *name, ResHandle materialRes,
                                  int batchStart, int batchCount, int vertRStart, int vertREnd )
{
	SceneNode *parentNode = Modules::sceneMan().resolveNodeHandle( parent );
	APIFUNC_VALIDATE_NODE( parentNode, "h3dAddMeshNode", 0 );
	Resource *matRes = Modules::resMan().resolveResHandle( materialRes );
	APIFUNC_VALIDATE_RES_TYPE( matRes, ResourceTypes::Material, "h3dAddMeshNode", 0 );

	//Modules::log().writeInfo( "Adding Mesh node '%s'", safeStr( name ).c_str() );
	MeshNodeTpl tpl( safeStr( name, 0 ), (MaterialResource *)matRes, (unsigned)batchStart,
	                 (unsigned)batchCount, (unsigned)vertRStart, (unsigned)vertREnd );
	SceneNode *sn = Modules::sceneMan().findType( SceneNodeTypes::Mesh )->factoryFunc( tpl );
	return Modules::sceneMan().addNode( sn, *parentNode );
}


DLLEXP NodeHandle h3dAddJointNode( NodeHandle parent, const char *name, int jointIndex )
{
	SceneNode *parentNode = Modules::sceneMan().resolveNodeHandle( parent );
	APIFUNC_VALIDATE_NODE( parentNode, "h3dAddJointNode", 0 );

	//Modules::log().writeInfo( "Adding Joint node '%s'", safeStr( name ).c_str() );
	JointNodeTpl tpl( safeStr( name, 0 ), (unsigned)jointIndex );
	SceneNode *sn = Modules::sceneMan().findType( SceneNodeTypes::Joint )->factoryFunc( tpl );
	return Modules::sceneMan().addNode( sn, *parentNode );
}


DLLEXP NodeHandle h3dAddLightNode( NodeHandle parent, const char *name, ResHandle materialRes,
                                   const char *lightingContext, const char *shadowContext )
{
	SceneNode *parentNode = Modules::sceneMan().resolveNodeHandle( parent );
	APIFUNC_VALIDATE_NODE( parentNode, "h3dAddLightNode", 0 );
	Resource *matRes = Modules::resMan().resolveResHandle( materialRes );
	if(matRes != 0x0 )
	{
		APIFUNC_VALIDATE_RES_TYPE( matRes, ResourceTypes::Material, "h3dAddLightNode", 0 );
	}
	
	//Modules::log().writeInfo( "Adding Light node '%s'", safeStr( name ).c_str() );
	LightNodeTpl tpl( safeStr( name, 0 ), (MaterialResource *)matRes,
	                  safeStr( lightingContext, 1 ), safeStr( shadowContext, 2 ) );
	SceneNode *sn = Modules::sceneMan().findType( SceneNodeTypes::Light )->factoryFunc( tpl );
	return Modules::sceneMan().addNode( sn, *parentNode );
}


DLLEXP NodeHandle h3dAddCameraNode( NodeHandle parent, const char *name, ResHandle pipelineRes )
{
	SceneNode *parentNode = Modules::sceneMan().resolveNodeHandle( parent );
	APIFUNC_VALIDATE_NODE( parentNode, "h3dAddCameraNode", 0 );
	Resource *pipeRes = Modules::resMan().resolveResHandle( pipelineRes );
	APIFUNC_VALIDATE_RES_TYPE( pipeRes, ResourceTypes::Pipeline, "h3dAddCameraNode", 0 );
	
	//Modules::log().writeInfo( "Adding Camera node '%s'", safeStr( name ).c_str() );
	CameraNodeTpl tpl( safeStr( name, 0 ), (PipelineResource *)pipeRes );
	SceneNode *sn = Modules::sceneMan().findType( SceneNodeTypes::Camera )->factoryFunc( tpl );
	return Modules::sceneMan().addNode( sn, *parentNode );
}


DLLEXP void h3dSetupCameraView( NodeHandle cameraNode, float fov, float aspect, float nearDist, float farDist )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( cameraNode );
	APIFUNC_VALIDATE_NODE_TYPE( sn, SceneNodeTypes::Camera, "h3dSetupCameraView", APIFUNC_RET_VOID );
	
	((CameraNode *)sn)->setupViewParams( fov, aspect, nearDist, farDist );
}


DLLEXP void h3dGetCameraProjMat( NodeHandle cameraNode, float *projMat )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( cameraNode );
	APIFUNC_VALIDATE_NODE_TYPE( sn, SceneNodeTypes::Camera, "h3dGetCameraProjMat", APIFUNC_RET_VOID );
	if( projMat == 0x0 )
	{
		Modules::setError( "Invalid pointer in h3dGetCameraProjMat" );
		return;
	}
	
	Modules::sceneMan().updateNodes();
	memcpy( projMat, ((CameraNode *)sn)->getProjMat().x, 16 * sizeof( float ) );
}

DLLEXP void h3dSetCameraProjMat( NodeHandle cameraNode, float *projMat )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( cameraNode );
	APIFUNC_VALIDATE_NODE_TYPE( sn, SceneNodeTypes::Camera, "h3dSetCameraProjMat", APIFUNC_RET_VOID );
	if( projMat == 0x0 )
	{
		Modules::setError( "Invalid pointer in h3dSetCameraProjMat" );
		return;
	}
	
	Modules::sceneMan().updateNodes();
	((CameraNode *)sn)->setProjectionMatrix( projMat );
}


DLLEXP NodeHandle h3dAddEmitterNode( NodeHandle parent, const char *name, ResHandle materialRes,
                                     ResHandle particleEffectRes, int maxParticleCount, int respawnCount )
{
	SceneNode *parentNode = Modules::sceneMan().resolveNodeHandle( parent );
	APIFUNC_VALIDATE_NODE( parentNode, "h3dAddEmitterNode", 0 );
	Resource *matRes = Modules::resMan().resolveResHandle( materialRes );
	APIFUNC_VALIDATE_RES_TYPE( matRes, ResourceTypes::Material, "h3dAddEmitterNode", 0 );
	Resource *effRes = Modules::resMan().resolveResHandle( particleEffectRes );
	APIFUNC_VALIDATE_RES_TYPE( effRes, ResourceTypes::ParticleEffect, "h3dAddEmitterNode", 0 );
	
	//Modules::log().writeInfo( "Adding Emitter node '%s'", safeStr( name ).c_str() );
	EmitterNodeTpl tpl( safeStr( name, 0 ), (MaterialResource *)matRes, (ParticleEffectResource *)effRes,
	                    (unsigned)maxParticleCount, respawnCount );
	SceneNode *sn = Modules::sceneMan().findType( SceneNodeTypes::Emitter )->factoryFunc( tpl );
	return Modules::sceneMan().addNode( sn, *parentNode );
}


DLLEXP void h3dUpdateEmitter( NodeHandle emitterNode, float timeDelta )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( emitterNode );
	APIFUNC_VALIDATE_NODE_TYPE( sn, SceneNodeTypes::Emitter, "h3dUpdateEmitter", APIFUNC_RET_VOID );
	
	((EmitterNode *)sn)->update( timeDelta );
}


DLLEXP bool h3dHasEmitterFinished( NodeHandle emitterNode )
{
	SceneNode *sn = Modules::sceneMan().resolveNodeHandle( emitterNode );
	APIFUNC_VALIDATE_NODE_TYPE( sn, SceneNodeTypes::Emitter, "h3dHasEmitterFinished", false );
	
	return ((EmitterNode *)sn)->hasFinished();
}


// =================================================================================================
// DLL entry point
// =================================================================================================


#ifdef PLATFORM_WIN
BOOL APIENTRY DllMain( HANDLE /*hModule*/, DWORD /*ul_reason_for_call*/, LPVOID /*lpReserved*/ )
{
	#if defined( _MSC_VER ) && defined( _DEBUG )
	//_crtBreakAlloc = 1397;
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif
	
	/*switch( ul_reason_for_call )
	{
	case DLL_PROCESS_DETACH:
		_CrtDumpMemoryLeaks();
		break;
	}*/
	
	return TRUE;
}
#endif
