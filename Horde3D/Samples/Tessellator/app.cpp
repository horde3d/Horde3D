// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
//
// Sample Application
// --------------------------------------
// Copyright (C) 2006-2016 Nicolas Schulz and Horde3D team
//
//
// This sample source file is not covered by the EPL as the rest of the SDK
// and may be used without any restrictions. However, the EPL's disclaimer of
// warranty and liability shall be in effect for this file.
//
// *************************************************************************************************

#include "app.h"
#include "Horde3D.h"
#include "Horde3DUtils.h"
#include <math.h>
#include <iomanip>
#include <random>
#include <memory>
#include <array>

using namespace std;

bool createIcosahedron()
{
	// Create vertex and index data
	std::array< unsigned int, 60 > faces = {
		2, 1, 0,
		3, 2, 0,
		4, 3, 0,
		5, 4, 0,
		1, 5, 0,

		11, 6, 7,
		11, 7, 8,
		11, 8, 9,
		11, 9, 10,
		11, 10, 6,

		1, 2, 6,
		2, 3, 7,
		3, 4, 8,
		4, 5, 9,
		5, 1, 10,

		2, 7, 6,
		3, 8, 7,
		4, 9, 8,
		5, 10, 9,
		1, 6, 10 };

	std::array< float, 12 * 3 > vertData = {
		0.000f, 0.000f, 1.000f,
		0.894f, 0.000f, 0.447f,
		0.276f, 0.851f, 0.447f,
		-0.724f, 0.526f, 0.447f,
		-0.724f, -0.526f, 0.447f,
		0.276f, -0.851f, 0.447f,
		0.724f, 0.526f, -0.447f,
		-0.276f, 0.851f, -0.447f,
		-0.894f, 0.000f, -0.447f,
		-0.276f, -0.851f, -0.447f,
		0.724f, -0.526f, -0.447f,
		0.000f, 0.000f, -1.000f };

	int geo = h3dutCreateGeometryRes( "icosa", 12, faces.size(), vertData.data(), faces.data(), nullptr, nullptr, nullptr, nullptr, nullptr );
	if ( geo == 0 ) return false;

	// Create material 
	std::string matData =	"<Material>\n"
							"<Shader source=\"shaders/tessellation.shader\" />"
							"<Uniform name=\"tessLevelInner\" a=\"1\" />\n"
							"<Uniform name=\"tessLevelOuter\" a=\"1\" />\n"
							"<Uniform name=\"matAmbientCol\" a=\"0.04\" b=\"0.04\" c=\"0.04\" />\n"
							"<Uniform name=\"matDiffuseCol\" a=\"0\" b=\"0.75\" c=\"0.75\" />\n"
							"</Material>";
	
	int mat = h3dAddResource( H3DResTypes::Material, "material", 0 );
	bool res = h3dLoadResource( mat, matData.c_str(), matData.size() );
	if ( !res ) return false;

	return true;
}


TessellatorSample::TessellatorSample( int argc, char** argv ) :
    SampleApplication( argc, argv, "Tessellator - Horde3D Sample", 45.0f, 0.1f, 1000.0f )
{
    _x = 125; _y = 25; _z = 85;
    _rx = -10; _ry = 55;

	_animTime = 0;
	_tessInner = _tessOuter = 1;
}


bool TessellatorSample::initResources()
{
    if ( !SampleApplication::initResources() )
        return false;

    // 1. Add resources

	// Shader for deferred shading
	H3DRes lightMatRes = h3dAddResource( H3DResTypes::Material, "materials/light.material.xml", 0 );

	bool result = createIcosahedron();
	if ( !result ) return false;

    // 2. Load resources

    if ( !h3dutLoadResourcesFromDisk( getResourcePath() ) )
    {
		h3dutDumpMessages();
        return false;
    }

    // 3. Add scene nodes

	// Add camera
	_cam = h3dAddCameraNode( H3DRootNode, "Camera", getPipelineRes() );

	// Create model and mesh
	int geo = h3dFindResource( H3DResTypes::Geometry, "icosa" );
	int mat = h3dFindResource( H3DResTypes::Material, "material" );

	int indices = h3dGetResParamI( geo, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoIndexCountI );
	int vertices = h3dGetResParamI( geo, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoVertexCountI );
	
	_model = h3dAddModelNode( H3DRootNode, "model", geo );
	int mesh = h3dAddMeshNode( _model, "icosahedron", mat, 0, indices, 0, vertices - 1 );
	h3dSetNodeTransform( mesh, 0, 0, 0, 0, 0, 0, 20, 20, 20 );
	h3dSetNodeParamI( mesh, H3DMesh::TessellatableI, 1 ); // Set mesh to use tessellation

    // Add light source
// 	H3DNode light = h3dAddLightNode( H3DRootNode, "Light1", lightMatRes, "LIGHTING", "SHADOWMAP" );
// 	h3dSetNodeTransform( light, 0, 20, 50, -30, 0, 0, 1, 1, 1 );
// 	h3dSetNodeParamF( light, H3DLight::RadiusF, 0, 200 );
// 	h3dSetNodeParamF( light, H3DLight::FovF, 0, 90 );
// 	h3dSetNodeParamI( light, H3DLight::ShadowMapCountI, 3 );
// 	h3dSetNodeParamF( light, H3DLight::ShadowSplitLambdaF, 0, 0.9f );
// 	h3dSetNodeParamF( light, H3DLight::ShadowMapBiasF, 0, 0.001f );
// 	h3dSetNodeParamF( light, H3DLight::ColorF3, 0, 0.9f );
// 	h3dSetNodeParamF( light, H3DLight::ColorF3, 1, 0.7f );
// 	h3dSetNodeParamF( light, H3DLight::ColorF3, 2, 0.75f );

	return true;
}


void TessellatorSample::releaseResources()
{
    SampleApplication::releaseResources();
}


void TessellatorSample::keyEventHandler( int key, int scancode, int action, int mods )
{
	SampleApplication::keyEventHandler( key, scancode, action, mods );

	if ( action != GLFW_PRESS )
		return;

	if ( key == GLFW_KEY_UP )
	{
		_tessInner++;
		_tessOuter++;

		int mat = h3dFindResource( H3DResTypes::Material, "material" );
		h3dSetMaterialUniform( mat, "tessLevelInner", _tessInner, 0, 0, 0 );
		h3dSetMaterialUniform( mat, "tessLevelOuter", _tessOuter, 0, 0, 0 );
	}

	if ( key == GLFW_KEY_DOWN )
	{
		_tessInner == 1 ? 1 : _tessInner--;
		_tessOuter == 1 ? 1 : _tessOuter--;

		int mat = h3dFindResource( H3DResTypes::Material, "material" );
		h3dSetMaterialUniform( mat, "tessLevelInner", _tessInner, 0, 0, 0 );
		h3dSetMaterialUniform( mat, "tessLevelOuter", _tessOuter, 0, 0, 0 );
	}
}

void TessellatorSample::update()
{
    SampleApplication::update();
	float frame_time = 1.0f / getFPS();

    if( !checkFlag( SampleApplication::FreezeMode ) )
	{
		// Calculate animation time in seconds
		_animTime += frame_time;

// 		Set animation time
// 				h3dSetMaterialUniform( _computeMatRes, "deltaTime", 1.0f / H3D_FPS_REFERENCE, 0, 0, 0 );
// 		
// 				// Set attractor point
// 				float angle = ( float ) _animTime * 0.5f;
// 		
// 				float attractorX = cosf( angle );
// 				float attractorY = cosf( angle ) * sinf( angle );
// 				float attractorZ = sinf( angle );
// 		
// 				h3dSetMaterialUniform( _computeMatRes, "attractor", attractorX * 2, attractorY * 2, attractorZ * 2, 0 );
// 		
// 				// Perform computing
// 				h3dCompute( _computeMatRes, "COMPUTE", _computeGroupX /*1024*/, _computeGroupY /*1*/, 1 );
	}
}
