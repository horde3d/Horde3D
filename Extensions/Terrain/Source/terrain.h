// *************************************************************************************************
//
// Horde3D Terrain Extension
// --------------------------------------------------------
// Copyright (C) 2006-2011 Nicolas Schulz and Volker Wiendl
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/legal/epl-v10.html
//
// *************************************************************************************************

#ifndef _Horde3DTerrain_terrain_H_
#define _Horde3DTerrain_terrain_H_

#include "egPrerequisites.h"
#include "utMath.h"
#include "egMaterial.h"
#include "egTexture.h"
#include "egScene.h"


namespace Horde3DTerrain {

using namespace Horde3D;


const int SNT_TerrainNode = 100;

extern const char *vsTerrainDebugView;
extern const char *fsTerrainDebugView;	

struct TerrainNodeTpl : public SceneNodeTpl
{
	PTextureResource   hmapRes;
	PMaterialResource  matRes;
	float              meshQuality;
	float              skirtHeight;
	int                blockSize;

	TerrainNodeTpl( const std::string &name, TextureResource *hmapRes, MaterialResource *matRes ) :
		SceneNodeTpl( SNT_TerrainNode, name ), hmapRes( hmapRes ), matRes( matRes ),
		meshQuality( 50.0f ), skirtHeight( 0.1f ), blockSize( 17 )
	{
	}
};


struct TerrainNodeParams
{
	enum List
	{
		HeightTexResI = 10000,
		MatResI,
		MeshQualityF,
		SkirtHeightF,
		BlockSizeI
	};
};

struct BlockInfo
{	
	float minHeight;
	float maxHeight;
	float geoError;		// Maximum geometric error

	BlockInfo() : minHeight( 1.0f ), maxHeight( 0.0f ), geoError( 0.0f ) {}
};

class TerrainNode : public SceneNode
{
public:
	~TerrainNode();

	static SceneNodeTpl *parsingFunc( std::map< std::string, std::string > &attribs );
	static SceneNode *factoryFunc( const SceneNodeTpl &nodeTpl );
	static void renderFunc(uint32 firstItem, uint32 lastItem, const std::string &shaderContext, const std::string &theClass,
		bool debugView, const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order, int occSet );

	bool canAttach( SceneNode &parent );
	int getParamI( int param );
	void setParamI( int param, int value );
	float getParamF( int param, int compIdx );
	void setParamF( int param, int compIdx, float value );

	bool checkIntersection( const Vec3f &rayOrig, const Vec3f &rayDir, Vec3f &intsPos ) const;

	ResHandle createGeometryResource( const std::string &name, float lodThreshold );
	
	float getHeight( float x, float y )
		{ return _heightData[ftoi_r( y * _hmapSize ) * (_hmapSize + 1) + ftoi_r( x * _hmapSize ) ] / 65535.0f; }

public:
	static uint32 vlTerrain;
	static ShaderCombination debugViewShader;

protected:
	TerrainNode( const TerrainNodeTpl &terrainTpl );
	
	void onPostUpdate();
	
	bool updateHeightData( TextureResource &hmap );
	void calcMaxLevel();
	
	uint32 getVertexCount();
	float *createVertices();
	uint32 getIndexCount();
	uint16 *createIndices();
	void recreateVertexBuffer();
	
	void buildBlockInfo( BlockInfo &block, float minU, float minV, float maxU, float maxV );
	void createBlockTree();

	static void drawTerrainBlock( TerrainNode *terrain, float minU, float minV, float maxU, float maxV,
	                              int level, float scale, const Vec3f &localCamPos, const Frustum *frust1,
	                              const Frustum *frust2, int uni_terBlockParams );

	uint32 calculateGeometryBlockCount( float lodThreshold, float minU, float minV,
	                                    float maxU, float maxV, int level, float scale);
	void createGeometryVertices( float lodThreshold, float minU, float minV,
	                             float maxU, float maxV, int level, float scale, 
	                             float *&vertData, unsigned int *&indexData, uint32 &indexOffset );

protected:
	PMaterialResource  _materialRes;
	uint32             _blockSize;
	float              _skirtHeight;
	float              _lodThreshold;
	
	uint32             _hmapSize;
	uint16             *_heightData;
	uint32             _maxLevel;
	float              *_heightArray;
	uint32             _vertexBuffer, _indexBuffer;
	BoundingBox        _localBBox;

	std::vector< BlockInfo >  _blockTree;
};

}  // namespace

#endif // _Horde3DTerrain_terrain_H_
