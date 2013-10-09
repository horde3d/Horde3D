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

#include "egModel.h"
#include "egMaterial.h"
#include "egModules.h"
#include "egRenderer.h"
#include "egCom.h"
#include <cstring>

#include "utDebug.h"


namespace Horde3D {

using namespace std;


ModelNode::ModelNode( const ModelNodeTpl &modelTpl ) :
	SceneNode( modelTpl ), _geometryRes( modelTpl.geoRes ), _baseGeoRes( 0x0 ),
	_lodDist1( modelTpl.lodDist1 ), _lodDist2( modelTpl.lodDist2 ),
	_lodDist3( modelTpl.lodDist3 ), _lodDist4( modelTpl.lodDist4 ),
	_softwareSkinning( modelTpl.softwareSkinning ), _skinningDirty( false ),
	_nodeListDirty( false ), _morpherUsed( false ), _morpherDirty( false )
{
	if( _geometryRes != 0x0 )
		setParamI( ModelNodeParams::GeoResI, _geometryRes->getHandle() );
}


ModelNode::~ModelNode()
{
	_geometryRes = 0x0;
	_baseGeoRes = 0x0;
}


SceneNodeTpl *ModelNode::parsingFunc( map< string, string > &attribs )
{
	bool result = true;
	
	ModelNodeTpl *modelTpl = new ModelNodeTpl( "", 0x0 );
	
	map< string, string >::iterator itr = attribs.find( "geometry" );
	if( itr != attribs.end() )
	{
		uint32 res = Modules::resMan().addResource( ResourceTypes::Geometry, itr->second, 0, false );
		if( res != 0 )
			modelTpl->geoRes = (GeometryResource *)Modules::resMan().resolveResHandle( res );
	}
	else result = false;
	itr = attribs.find( "softwareSkinning" );
	if( itr != attribs.end() ) 
	{
		if ( _stricmp( itr->second.c_str(), "true" ) == 0 || _stricmp( itr->second.c_str(), "1" ) == 0 )
			modelTpl->softwareSkinning = true;
		else
			modelTpl->softwareSkinning = false;
	}

	itr = attribs.find( "lodDist1" );
	if( itr != attribs.end() ) modelTpl->lodDist1 = (float)atof( itr->second.c_str() );
	itr = attribs.find( "lodDist2" );
	if( itr != attribs.end() ) modelTpl->lodDist2 = (float)atof( itr->second.c_str() );
	itr = attribs.find( "lodDist3" );
	if( itr != attribs.end() ) modelTpl->lodDist3 = (float)atof( itr->second.c_str() );
	itr = attribs.find( "lodDist4" );
	if( itr != attribs.end() ) modelTpl->lodDist4 = (float)atof( itr->second.c_str() );

	if( !result )
	{
		delete modelTpl; modelTpl = 0x0;
	}
	
	return modelTpl;
}


SceneNode *ModelNode::factoryFunc( const SceneNodeTpl &nodeTpl )
{
	if( nodeTpl.type != SceneNodeTypes::Model ) return 0x0;

	return new ModelNode( *(ModelNodeTpl *)&nodeTpl );
}


void ModelNode::recreateNodeListRec( SceneNode *node, bool firstCall )
{
	if( node->getType() == SceneNodeTypes::Mesh )
	{
		_meshList.push_back( (MeshNode *)node );
		_animCtrl.registerNode( (MeshNode *)node );
	}
	else if( node->getType() == SceneNodeTypes::Joint )
	{
		_jointList.push_back( (JointNode *)node );
		_animCtrl.registerNode( (JointNode *)node );
	}
	else if( !firstCall ) return;  // First node is the model
	
	// Children
	for( size_t i = 0, s = node->getChildren().size(); i < s; ++i )
	{
		recreateNodeListRec( node->getChildren()[i], false );
	}
}


void ModelNode::recreateNodeList()
{
	_meshList.resize( 0 );
	_jointList.resize( 0 );
	_animCtrl.clearNodeList();
	
	recreateNodeListRec( this, true );
	updateLocalMeshAABBs();

	_nodeListDirty = false;
}


void ModelNode::setupAnimStage( int stage, AnimationResource *anim, int layer,
                                const string &startNode, bool additive )
{
	if( _nodeListDirty ) recreateNodeList();
	
	if( _animCtrl.setupAnimStage( stage, anim, layer, startNode, additive ) ) markDirty();
}


void ModelNode::getAnimParams( int stage, float *time, float *weight )
{
	_animCtrl.getAnimParams( stage, time, weight );
}


void ModelNode::setAnimParams( int stage, float time, float weight )
{
	if( _animCtrl.setAnimParams( stage, time, weight ) ) markDirty();
}


bool ModelNode::setMorphParam( const string &targetName, float weight )
{
	if( _geometryRes == 0x0 || _morphers.empty() ) return false;

	bool result = false;
	_morpherDirty = true;
	_morpherUsed = false;

	// Set specified morph target or all targets if targetName == ""
	for( uint32 i = 0; i < _morphers.size(); ++i )
	{
		if( targetName == "" || _morphers[i].name == targetName )
		{
			_morphers[i].weight = weight;
			result = true;
		}

		if( _morphers[i].weight != 0 ) _morpherUsed = true;
	}

	markDirty();

	return result;
}


void ModelNode::updateLocalMeshAABBs()
{
	if( _geometryRes == 0x0 ) return;
	
	// Update local mesh AABBs
	for( uint32 i = 0, s = (uint32)_meshList.size(); i < s; ++i )
	{
		MeshNode &mesh = *_meshList[i];
		
		Vec3f &bBMin = mesh._localBBox.min;
		Vec3f &bBMax = mesh._localBBox.max;
				
		if( mesh._vertRStart <= mesh._vertREnd && 
			mesh._vertRStart < _geometryRes->getVertCount() &&
		    mesh._vertREnd < _geometryRes->getVertCount() )
		{
			bBMin = Vec3f( Math::MaxFloat, Math::MaxFloat, Math::MaxFloat );
			bBMax = Vec3f( -Math::MaxFloat, -Math::MaxFloat, -Math::MaxFloat );
			for( uint32 j = mesh._vertRStart; j <= mesh._vertREnd; ++j )
			{
				Vec3f &vertPos = _geometryRes->getVertPosData()[j];

				if( vertPos.x < bBMin.x ) bBMin.x = vertPos.x;
				if( vertPos.y < bBMin.y ) bBMin.y = vertPos.y;
				if( vertPos.z < bBMin.z ) bBMin.z = vertPos.z;
				if( vertPos.x > bBMax.x ) bBMax.x = vertPos.x;
				if( vertPos.y > bBMax.y ) bBMax.y = vertPos.y;
				if( vertPos.z > bBMax.z ) bBMax.z = vertPos.z;
			}

			// Avoid zero box dimensions for planes
			if( bBMax.x - bBMin.x == 0 ) bBMax.x += Math::Epsilon;
			if( bBMax.y - bBMin.y == 0 ) bBMax.y += Math::Epsilon;
			if( bBMax.z - bBMin.z == 0 ) bBMax.z += Math::Epsilon;
		}
		else
		{
			bBMin = Vec3f( 0, 0, 0 );
			bBMax = Vec3f( 0, 0, 0 );
		}
	}
}


void ModelNode::setGeometryRes( GeometryResource &geoRes )
{
	// Init joint data
	_skinMatRows.resize( geoRes._joints.size() * 3 );
	for( uint32 i = 0; i < _skinMatRows.size() / 3; ++i )
	{
		_skinMatRows[i * 3 + 0] = Vec4f( 1, 0, 0, 0 );
		_skinMatRows[i * 3 + 1] = Vec4f( 0, 1, 0, 0 );
		_skinMatRows[i * 3 + 2] = Vec4f( 0, 0, 1, 0 );
	}

	// Copy morph targets
	_morphers.resize( geoRes._morphTargets.size() );
	for( uint32 i = 0; i < _morphers.size(); ++i )
	{	
		Morpher &morpher = _morphers[i]; 
		
		morpher.name = geoRes._morphTargets[i].name;
		morpher.index = i;
		morpher.weight = 0;
	}

	if( !_morphers.empty() || _softwareSkinning )
	{
		Resource *clonedRes = Modules::resMan().resolveResHandle(
			Modules::resMan().cloneResource( geoRes, "" ) );
		_geometryRes = (GeometryResource *)clonedRes;
		_baseGeoRes = &geoRes;
	}
	else
	{
		_geometryRes = &geoRes;
		_baseGeoRes = 0x0;
	}

	_skinningDirty = true;
	updateLocalMeshAABBs();
}


int ModelNode::getParamI( int param )
{
	switch( param )
	{
	case ModelNodeParams::GeoResI:
		return _geometryRes != 0x0 ? _geometryRes->_handle : 0;
	case ModelNodeParams::SWSkinningI:
		return _softwareSkinning ? 1 : 0;
	}

	return SceneNode::getParamI( param );
}


void ModelNode::setParamI( int param, int value )
{
	Resource *res;
	
	switch( param )
	{
	case ModelNodeParams::GeoResI:
		res = Modules::resMan().resolveResHandle( value );
		if( res != 0x0 && res->getType() == ResourceTypes::Geometry )
			setGeometryRes( *(GeometryResource *)res );
		else
			Modules::setError( "Invalid handle in h3dSetNodeParamI for H3DModel::GeoResI" );
		return;
	case ModelNodeParams::SWSkinningI:
		_softwareSkinning = (value != 0);
		if( _softwareSkinning ) _skinningDirty = true;
		if( _softwareSkinning && _baseGeoRes == 0x0 && _geometryRes != 0x0 )
			// Create a local resource copy since it is not yet existing
			setParamI( ModelNodeParams::GeoResI, _geometryRes->getHandle() );
		else if( !_softwareSkinning && _morphers.empty() && _baseGeoRes != 0x0 )
			// Remove the local resource copy by removing reference
			setParamI( ModelNodeParams::GeoResI, _baseGeoRes->getHandle() );
		return;
	}

	SceneNode::setParamI( param, value );
}


float ModelNode::getParamF( int param, int compIdx )
{
	switch( param )
	{
	case ModelNodeParams::LodDist1F:
		return _lodDist1;
	case ModelNodeParams::LodDist2F:
		return _lodDist2;
	case ModelNodeParams::LodDist3F:
		return _lodDist3;
	case ModelNodeParams::LodDist4F:
		return _lodDist4;
	}

	return SceneNode::getParamF( param, compIdx );
}


void ModelNode::setParamF( int param, int compIdx, float value )
{
	switch( param )
	{
	case ModelNodeParams::LodDist1F:
		_lodDist1 = value;
		return;
	case ModelNodeParams::LodDist2F:
		_lodDist2 = value;
		return;
	case ModelNodeParams::LodDist3F:
		_lodDist3 = value;
		return;
	case ModelNodeParams::LodDist4F:
		_lodDist4 = value;
		return;
	}

	SceneNode::setParamF( param, compIdx, value );
}


void ModelNode::update( int flags )
{
	if( flags & ModelUpdateFlags::Animation )
	{
		if( _animCtrl.animate() )
		{	
			_skinningDirty = true;
			markDirty();
			SceneNode::updateTree();
		}
	}
	
	if( flags & ModelUpdateFlags::Geometry )
	{
		// Update geometry for morphers or software skinning
		updateGeometry();
	}
}


bool ModelNode::updateGeometry()
{
	_skinningDirty |= _morpherDirty;
	_skinningDirty &= _softwareSkinning;
	
	if( !_skinningDirty && !_morpherDirty ) return false;

	if( _baseGeoRes == 0x0 || _baseGeoRes->getVertPosData() == 0x0 ||
	    _baseGeoRes->getVertTanData() == 0x0 || _baseGeoRes->getVertStaticData() == 0x0 ) return false;
	if( _geometryRes == 0x0 || _geometryRes->getVertPosData() == 0x0 ||
		_geometryRes->getVertTanData() == 0x0 || _geometryRes->getVertStaticData() == 0x0 ) return false;
	
	Timer *timer = Modules::stats().getTimer( EngineStats::GeoUpdateTime );
	if( Modules::config().gatherTimeStats ) timer->setEnabled( true );
	
	// Reset vertices to base data
	memcpy( _geometryRes->getVertPosData(), _baseGeoRes->getVertPosData(),
	        _geometryRes->_vertCount * sizeof( Vec3f ) );
	memcpy( _geometryRes->getVertTanData(), _baseGeoRes->getVertTanData(),
	        _geometryRes->_vertCount * sizeof( VertexDataTan ) );

	Vec3f *posData = _geometryRes->getVertPosData();
	VertexDataTan *tanData = _geometryRes->getVertTanData();
	VertexDataStatic *staticData = _geometryRes->getVertStaticData();

	if( _morpherUsed )
	{
		// Recalculate vertex positions for morph targets
		for( uint32 i = 0; i < _morphers.size(); ++i )
		{
			if( _morphers[i].weight > Math::Epsilon )
			{
				MorphTarget &mt = _geometryRes->_morphTargets[_morphers[i].index];
				float weight = _morphers[i].weight;
				
				for( uint32 j = 0; j < mt.diffs.size(); ++j )
				{
					MorphDiff &md = mt.diffs[j];
					
					posData[md.vertIndex] += md.posDiff * weight;
					tanData[md.vertIndex].normal += md.normDiff * weight;
					tanData[md.vertIndex].tangent += md.tanDiff * weight;
				}
			}
		}
	}

	if( _skinningDirty )
	{
		Matrix4f skinningMat;
		Vec4f *rows = &_skinMatRows[0];

		for( uint32 i = 0, s = _geometryRes->getVertCount(); i < s; ++i )
		{
			Vec4f *row0 = &rows[ftoi_r( staticData[i].jointVec[0] ) * 3];
			Vec4f *row1 = &rows[ftoi_r( staticData[i].jointVec[1] ) * 3];
			Vec4f *row2 = &rows[ftoi_r( staticData[i].jointVec[2] ) * 3];
			Vec4f *row3 = &rows[ftoi_r( staticData[i].jointVec[3] ) * 3];

			Vec4f weights = *((Vec4f *)&staticData[i].weightVec[0]);

			skinningMat.x[0] = (row0)->x * weights.x + (row1)->x * weights.y + (row2)->x * weights.z + (row3)->x * weights.w;
			skinningMat.x[1] = (row0+1)->x * weights.x + (row1+1)->x * weights.y + (row2+1)->x * weights.z + (row3+1)->x * weights.w;
			skinningMat.x[2] = (row0+2)->x * weights.x + (row1+2)->x * weights.y + (row2+2)->x * weights.z + (row3+2)->x * weights.w;
			skinningMat.x[4] = (row0)->y * weights.x + (row1)->y * weights.y + (row2)->y * weights.z + (row3)->y * weights.w;
			skinningMat.x[5] = (row0+1)->y * weights.x + (row1+1)->y * weights.y + (row2+1)->y * weights.z + (row3+1)->y * weights.w;
			skinningMat.x[6] = (row0+2)->y * weights.x + (row1+2)->y * weights.y + (row2+2)->y * weights.z + (row3+2)->y * weights.w;
			skinningMat.x[8] = (row0)->z * weights.x + (row1)->z * weights.y + (row2)->z * weights.z + (row3)->z * weights.w;
			skinningMat.x[9] = (row0+1)->z * weights.x + (row1+1)->z * weights.y + (row2 + 1)->z * weights.z + (row3+1)->z * weights.w;
			skinningMat.x[10] = (row0+2)->z * weights.x + (row1+2)->z * weights.y + (row2+2)->z * weights.z + (row3+2)->z * weights.w;
			skinningMat.x[12] = (row0)->w * weights.x + (row1)->w * weights.y + (row2)->w * weights.z + (row3)->w * weights.w;
			skinningMat.x[13] = (row0+1)->w * weights.x + (row1+1)->w * weights.y + (row2+1)->w * weights.z + (row3+1)->w * weights.w;
			skinningMat.x[14] = (row0+2)->w * weights.x + (row1+2)->w * weights.y + (row2+2)->w * weights.z + (row3+2)->w * weights.w;

			// Skin position
			posData[i] = skinningMat * posData[i];

			// Skin tangent space basis
			// Note: We skip the normalization of the tangent space basis for performance reasons;
			//       the error is usually not huge and should be hardly noticable
			tanData[i].normal = skinningMat.mult33Vec( tanData[i].normal ); //.normalized();
			tanData[i].tangent = skinningMat.mult33Vec( tanData[i].tangent ); //.normalized();
		}
	}
	else if( _morpherUsed )
	{
		// Renormalize tangent space basis
		for( uint32 i = 0, s = _geometryRes->getVertCount(); i < s; ++i )
		{
			tanData[i].normal.normalize();
			tanData[i].tangent.normalize();
		}
	}

	_morpherDirty = false;
	_skinningDirty = false;
	
	// Upload geometry
	_geometryRes->updateDynamicVertData();

	timer->setEnabled( false );

	return true;
}


uint32 ModelNode::calcLodLevel( const Vec3f &viewPoint )
{
	Vec3f pos( _absTrans.c[3][0], _absTrans.c[3][1], _absTrans.c[3][2] );
	float dist = (pos - viewPoint).length();
	uint32 curLod = 4;
	
	if( dist < _lodDist1 ) curLod = 0;
	else if( dist < _lodDist2 ) curLod = 1;
	else if( dist < _lodDist3 ) curLod = 2;
	else if( dist < _lodDist4 ) curLod = 3;

	return curLod;
}


void ModelNode::setCustomInstData( float *data, uint32 count )
{
	memcpy( _customInstData, data, std::min( count, ModelCustomVecCount * 4 ) * sizeof( float ) );
}


void ModelNode::onPostUpdate()
{
	if( _nodeListDirty ) recreateNodeList();
}


void ModelNode::onFinishedUpdate()
{
	// Update AABBs of skinned meshes
	if( _skinningDirty && !_jointList.empty() && _geometryRes != 0x0 )
	{
		Vec3f bmin( Math::MaxFloat, Math::MaxFloat, Math::MaxFloat );
		Vec3f bmax( -Math::MaxFloat, -Math::MaxFloat, -Math::MaxFloat );
		
		// Calculate AABB of skeleton
		for( uint32 i = 0, s = (uint32)_jointList.size(); i < s; ++i )
		{
			Vec3f pos = _jointList[i]->_relModelMat * Vec3f( 0, 0, 0 );

			if( pos.x < bmin.x ) bmin.x = pos.x;
			if( pos.y < bmin.y ) bmin.y = pos.y;
			if( pos.z < bmin.z ) bmin.z = pos.z;
			if( pos.x > bmax.x ) bmax.x = pos.x;
			if( pos.y > bmax.y ) bmax.y = pos.y;
			if( pos.z > bmax.z ) bmax.z = pos.z;
		}

		// Resize mesh AABBs according to change of skeleton extents
		// Note: This is just a rough approximation but it should be conservative, so AABBs
		//       will become too large but not too small
		for( uint32 i = 0, s = (uint32)_meshList.size(); i < s; ++i )
		{
			Vec3f dmin = bmin - _geometryRes->_skelAABB.min;
			Vec3f dmax = bmax - _geometryRes->_skelAABB.max;
			
			// Clamp so that bounding boxes can only grow and not shrink
			if( dmin.x > 0 ) dmin.x = 0; if( dmin.y > 0 ) dmin.y = 0; if( dmin.z > 0 ) dmin.z = 0;
			if( dmax.x < 0 ) dmax.x = 0; if( dmax.y < 0 ) dmax.y = 0; if( dmax.z < 0 ) dmax.z = 0;
			
			_meshList[i]->_bBox = _meshList[i]->_localBBox;
			_meshList[i]->_bBox.min += dmin;
			_meshList[i]->_bBox.max += dmax;
			_meshList[i]->_bBox.transform( _meshList[i]->_absTrans );
		}
	}

	// Calculate model AABB from mesh AABBs
	_bBox.clear();
	for( uint32 i = 0, s = (uint32)_meshList.size(); i < s; ++i )
	{
		_bBox.makeUnion( _meshList[i]->_bBox ); 
	}
}

}  // namespace
