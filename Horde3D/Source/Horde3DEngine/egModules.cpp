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

#include "egResource.h"
#include "egScene.h"
#include "egSceneGraphRes.h"
#include "egModules.h"
#include "egCom.h"
#include "egScene.h"
#include "egLight.h"
#include "egCamera.h"
#include "egResource.h"
#include "egRendererBase.h"
#include "egRenderer.h"
#include "egPipeline.h"
#include "egExtensions.h"

// Extensions
#ifdef CMAKE
	#include "egExtensions_auto_include.h"
#else
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	#include "Terrain/Source/extension.h"
	#pragma comment( lib, "Extension_Terrain.lib" )
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif

#include "utDebug.h"


namespace Horde3D {

const char *Modules::versionString = "Horde3D 1.0.0 Beta5";

bool                   Modules::_errorFlag = false;
EngineConfig           *Modules::_engineConfig = 0x0;
EngineLog              *Modules::_engineLog = 0x0;
StatManager            *Modules::_statManager = 0x0;
SceneManager           *Modules::_sceneManager = 0x0;
ResourceManager        *Modules::_resourceManager = 0x0;
RenderDevice           *Modules::_renderDevice = 0x0;
Renderer               *Modules::_renderer = 0x0;
ExtensionManager       *Modules::_extensionManager = 0x0;

RenderDevice *gRDI = 0x0;

void Modules::installExtensions()
{
#ifdef CMAKE
	#include "egExtensions_auto_install.h"
#else
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	
	extMan().installExtension( new Horde3DTerrain::ExtTerrain() );
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif
}


bool Modules::init()
{
	// Create modules (order is important because of dependencies)
	if( _extensionManager == 0x0 ) _extensionManager = new ExtensionManager();
	if( _engineLog == 0x0 ) _engineLog = new EngineLog();
	if( _engineConfig == 0x0 ) _engineConfig = new EngineConfig();
	if( _sceneManager == 0x0 ) _sceneManager = new SceneManager();
	if( _resourceManager == 0x0 ) _resourceManager = new ResourceManager();
	if( _renderDevice == 0x0 ) _renderDevice = new RenderDevice();
	gRDI = _renderDevice;
	if( _renderer == 0x0 ) _renderer = new Renderer();
	if( _statManager == 0x0 ) _statManager = new StatManager();

	// Init modules
	if( !renderer().init() ) return false;

	// Register resource types
	resMan().registerResType( ResourceTypes::SceneGraph, "SceneGraph", 0x0, 0x0,
		SceneGraphResource::factoryFunc );
	resMan().registerResType( ResourceTypes::Geometry, "Geometry", GeometryResource::initializationFunc,
		GeometryResource::releaseFunc, GeometryResource::factoryFunc );
	resMan().registerResType( ResourceTypes::Animation, "Animation", 0x0, 0x0,
		AnimationResource::factoryFunc );
	resMan().registerResType( ResourceTypes::Material, "Material", 0x0, 0x0,
		MaterialResource::factoryFunc );
	resMan().registerResType( ResourceTypes::Code, "Code", 0x0, 0x0,
		CodeResource::factoryFunc );
	resMan().registerResType( ResourceTypes::Shader, "Shader", 0x0, 0x0,
		ShaderResource::factoryFunc );
	resMan().registerResType( ResourceTypes::Texture, "Texture", TextureResource::initializationFunc,
		TextureResource::releaseFunc, TextureResource::factoryFunc );
	resMan().registerResType( ResourceTypes::ParticleEffect, "ParticleEffect", 0x0, 0x0,
		ParticleEffectResource::factoryFunc );
	resMan().registerResType( ResourceTypes::Pipeline, "Pipeline", 0x0, 0x0,
		PipelineResource::factoryFunc );

	// Register node types
	sceneMan().registerNodeType( SceneNodeTypes::Group, "Group",
		GroupNode::parsingFunc, GroupNode::factoryFunc );
	sceneMan().registerNodeType( SceneNodeTypes::Model, "Model",
		ModelNode::parsingFunc, ModelNode::factoryFunc);
	sceneMan().registerNodeType( SceneNodeTypes::Mesh, "Mesh",
		MeshNode::parsingFunc, MeshNode::factoryFunc );
	sceneMan().registerNodeType( SceneNodeTypes::Joint, "Joint",
		JointNode::parsingFunc, JointNode::factoryFunc );
	sceneMan().registerNodeType( SceneNodeTypes::Light, "Light",
		LightNode::parsingFunc, LightNode::factoryFunc );
	sceneMan().registerNodeType( SceneNodeTypes::Camera, "Camera",
		CameraNode::parsingFunc, CameraNode::factoryFunc );
	sceneMan().registerNodeType( SceneNodeTypes::Emitter, "Emitter",
		EmitterNode::parsingFunc, EmitterNode::factoryFunc );

	// Register render functions
	renderer().registerRenderFunc( SceneNodeTypes::Mesh, Renderer::drawMeshes );
	renderer().registerRenderFunc( SceneNodeTypes::Emitter, Renderer::drawParticles );
	
	// Install extensions
	installExtensions();

	// Create default resources
	TextureResource *tex2DRes = new TextureResource(
		"$Tex2D", 32, 32, 1, TextureFormats::BGRA8, ResourceFlags::NoTexMipmaps );
	void *image = tex2DRes->mapStream( TextureResData::ImageElem, 0, TextureResData::ImgPixelStream, false, true );
	ASSERT( image != 0x0 );
	for( uint32 i = 0; i < 32*32; ++i ) ((uint32 *)image)[i] = 0xffffffff;
	tex2DRes->unmapStream();
	tex2DRes->addRef();
	resMan().addNonExistingResource( *tex2DRes, false );

	TextureResource *texCubeRes = new TextureResource(
		"$TexCube", 32, 32, 1, TextureFormats::BGRA8, ResourceFlags::TexCubemap | ResourceFlags::NoTexMipmaps );
	for( uint32 i = 0; i < 6; ++i )
	{
		image = texCubeRes->mapStream( TextureResData::ImageElem, i, TextureResData::ImgPixelStream, false, true );
		ASSERT( image != 0x0 );
		for( uint32 j = 0; j < 32*32; ++j ) ((uint32 *)image)[j] = 0xff000000;
		texCubeRes->unmapStream();
	}
	texCubeRes->addRef();
	resMan().addNonExistingResource( *texCubeRes, false );

	TextureResource *tex3DRes = new TextureResource(
		"$Tex3D", 16, 16, 4, TextureFormats::BGRA8, ResourceFlags::NoTexMipmaps );
	image = tex3DRes->mapStream( TextureResData::ImageElem, 0, TextureResData::ImgPixelStream, false, true );
	ASSERT( image != 0x0 );
	for( uint32 i = 0; i < 16*16*4; ++i ) ((uint32 *)image)[i] = 0xffffffff;
	tex3DRes->unmapStream();
	tex3DRes->addRef();
	resMan().addNonExistingResource( *tex3DRes, false );
	
	return true;
}


void Modules::release()
{
	// Remove overlays since they reference resources and resource manager is removed before renderer
	if( _renderer ) _renderer->clearOverlays();
	
	// Order of destruction is important
	delete _extensionManager; _extensionManager = 0x0;
	delete _sceneManager; _sceneManager = 0x0;
	delete _resourceManager; _resourceManager = 0x0;
	delete _renderer; _renderer = 0x0;
	delete _renderDevice; _renderDevice = 0x0;
	gRDI = 0x0;
	delete _statManager; _statManager = 0x0;
	delete _engineLog; _engineLog = 0x0;
	delete _engineConfig; _engineConfig = 0x0;
}


void Modules::setError( const char *errorStr1, const char *errorStr2 )
{
	static std::string msg;
	msg.resize( 0 );

	if( errorStr1 ) msg.append( errorStr1 );
	if( errorStr2 ) msg.append( errorStr2 );
	
	_errorFlag = true;
	_engineLog->writeDebugInfo( msg.c_str() );
}


bool Modules::getError()
{
	if( _errorFlag )
	{
		_errorFlag = false;
		return true;
	}
	else
		return false;
}

}  // namespace
