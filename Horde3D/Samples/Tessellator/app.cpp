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
	float phi = ( 1.0f / ( ( 1.0f + sqrt( 5.0f ) ) / 2.0f ) );

	std::array< float, 12 * 3 > vertPos = {
		-phi, 1.0f, 0.0f, phi, 1.0f, 0.0f, -phi, -1.0f, 0.0f, phi, -1.0f, 0.0f,
		0.0f, -phi, 1.0f, 0.0f, phi, 1.0f, 0.0f, -phi, -1.0f, 0.0f, phi, -1.0f,
		1.0f, 0.0f, -phi, 1.0f, 0.0f, phi, -1.0f, 0.0f, -phi, -1.0f, 0.0f, phi };

	std::array< float, 60 * 2 > texCoords = {
		0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,

		1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,

		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };

	std::array< uint32_t, 60 > indices = {
		0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
		5, 11, 4, 1, 5, 9, 7, 1, 8, 10, 7, 6, 11, 10, 2,
		3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9,
		4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7, 9, 8, 1 };

	std::array< float, 12 * 3 > normals;

	// calculate normals
// 	for ( size_t i = 0; i < 12; ++i )
// 	{
// 		const uint32_t index0 = indices[ i * 3 + 0 ];
// 		const uint32_t index1 = indices[ i * 3 + 1 ];
// 		const uint32_t index2 = indices[ i * 3 + 2 ];
// 
// 		const vec3 &v0 = ( *positions )[ index0 ];
// 		const vec3 &v1 = ( *positions )[ index1 ];
// 		const vec3 &v2 = ( *positions )[ index2 ];
// 
// 		vec3 e0 = v1 - v0;
// 		vec3 e1 = v2 - v0;
// 
// 		( *normals )[ index0 ] = ( *normals )[ index1 ] = ( *normals )[ index2 ] = normalize( cross( e0, e1 ) );
// 	}

	int geo = h3dutCreateGeometryRes( "icosa", 12, indices.size(), vertPos.data(), indices.data(), nullptr, nullptr, nullptr, nullptr, nullptr );
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
    SampleApplication( argc, argv, "Tessellator - Horde3D Sample" )
{
    _x = 125; _y = 25; _z = 85;
    _rx = -10; _ry = 55;

	_animTime = 0;
	_tessInner = _tessOuter = 1;
	_rotation = 0;
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
	H3DNode light = h3dAddLightNode( _cam, "Light1", lightMatRes, "LIGHTING", "" );
//  	h3dSetNodeTransform( light, 125, 25, 85, -10, 55, 0, 1, 1, 1 );
	h3dSetNodeParamF( light, H3DLight::RadiusF, 0, 1000 );
	h3dSetNodeParamF( light, H3DLight::FovF, 0, 75 );
	h3dSetNodeParamI( light, H3DLight::ShadowMapCountI, 3 );
	h3dSetNodeParamF( light, H3DLight::ShadowSplitLambdaF, 0, 0.9f );
	h3dSetNodeParamF( light, H3DLight::ShadowMapBiasF, 0, 0.001f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 0, 0.9f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 1, 0.7f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 2, 0.75f );
	h3dSetNodeParamF( light, H3DLight::ColorMultiplierF, 0, 1.0f );

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

		_rotation += 0.05f;
		h3dSetNodeTransform( _model, 0, 0, 0, _rotation, 0, 0, 1, 1, 1 );

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
