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

#if defined( _MSC_VER )
#	if _MSC_VER >= 1400
#		define _CRT_SECURE_NO_DEPRECATE
#	endif
#endif

#include "converter.h"
#include "optimizer.h"
#include "utPlatform.h"
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;


Converter::Converter( ColladaDocument &doc, const string &outPath, float *lodDists ) :
	_daeDoc( doc )
{
	_outPath = outPath;
	
	_lodDist1 = lodDists[0];
	_lodDist2 = lodDists[1];
	_lodDist3 = lodDists[2];
	_lodDist4 = lodDists[3];
	
	_frameCount = 0;
	_maxLodLevel = 0;
	_animNotSampled = false;
}


Converter::~Converter()
{
	for( unsigned int i = 0; i < _nodes.size(); ++i ) delete _nodes[i];	
}


Matrix4f Converter::getNodeTransform( DaeNode &node, unsigned int frame )
{
	// Note: Function assumes sampled animation data
	
	for( unsigned int i = 0; i < node.transStack.size(); ++i )
	{
		int compIndex;
		DaeSampler *sampler = _daeDoc.libAnimations.findAnimForTarget( node.id, node.transStack[i].sid, &compIndex );

		if( sampler != 0x0 )
		{
			if( compIndex >= 0 )	// Handle animation of single components like X or ANGLE
			{
				if( sampler->output->floatArray.size() != _frameCount )
					_animNotSampled = true;
				else
					node.transStack[i].animValues[compIndex] = sampler->output->floatArray[frame];	
			}
			else
			{
				unsigned int size = 0;
				switch( node.transStack[i].type )
				{
				case DaeTransformation::MATRIX:
					size = 16;
					break;
				// Note: Routine assumes that order is X, Y, Z, ANGLE
				case DaeTransformation::SCALE:
				case DaeTransformation::TRANSLATION:
					size = 3;
					break;
				case DaeTransformation::ROTATION:
					size = 4;
					break;
				}					
				if( sampler->output->floatArray.size() != _frameCount * size )
					_animNotSampled = true;
				else
					memcpy( node.transStack[i].animValues, &sampler->output->floatArray[frame * size], size * sizeof( float ) );
			}
		}
		else
		{
			// If no animation data is found, use standard transformation
			memcpy( node.transStack[i].animValues, node.transStack[i].values, 16 * sizeof( float ) );	
		}
	}

	return makeMatrix4f( node.assembleAnimMatrix().transposed().x, _daeDoc.y_up );
}


SceneNode *Converter::findNode( const char *name, SceneNode *ignoredNode )
{
	for( size_t i = 0, s = _joints.size(); i < s; ++i )
	{
		if( _joints[i] != ignoredNode && strcmp( _joints[i]->name, name ) == 0 )
			return _joints[i];
	}

	for( size_t i = 0, s = _meshes.size(); i < s; ++i )
	{
		if( _meshes[i] != ignoredNode && strcmp( _meshes[i]->name, name ) == 0 )
			return _meshes[i];
	}

	return 0x0;
}


void Converter::checkNodeName( SceneNode *node )
{
	// Check if a different node with the same name exists
	if( findNode( node->name, node ) != 0x0 )
	{
		// If necessary, cut name to make room for the postfix
		if( strlen( node->name ) > 240 ) node->name[240] = '\0';

		char newName[256];
		unsigned int index = 2;

		// Find a free name
		while( true )
		{
			sprintf( newName, "%s_%i", node->name, index++ );

			if( !findNode( newName, node ) )
			{
				char msg[1024];
				sprintf( msg, "Warning: Node with name '%s' already exists. "
				         "Node was renamed to '%s'.", node->name, newName );
				log( msg );
				
				strcpy( node->name, newName );
				break;
			}
		}
	}
}


bool Converter::validateInstance( const std::string &instanceId )
{
	string id = instanceId;
	
	DaeSkin *skin = _daeDoc.libControllers.findSkin( id );
	DaeMorph *morpher = _daeDoc.libControllers.findMorph( id );
	
	// Resolve controller stack
	if( skin != 0x0 )
	{	
		id = skin->ownerId;
		morpher = _daeDoc.libControllers.findMorph( id );
		if( morpher != 0x0 ) id = morpher->ownerId;
	}
	else if( morpher != 0x0 )
	{
		id = morpher->ownerId;
		skin = _daeDoc.libControllers.findSkin( id );
		if( skin != 0x0 ) id = skin->ownerId;
	}
	
	DaeGeometry *geo = _daeDoc.libGeometries.findGeometry( id );
	return geo != 0x0 && !geo->triGroups.empty();
}


SceneNode *Converter::processNode( DaeNode &node, SceneNode *parentNode,
                                   Matrix4f transAccum, vector< Matrix4f > animTransAccum )
{
	// Note: animTransAccum is used for pure transformation nodes of Collada that are no joints or meshes
	
	// Process instances and create nodes using the following rules:
	//		* DaeJoint and no Instance -> create joint
	//		* DaeJoint and one Instance -> create joint and mesh
	//		* DaeJoint and several Instances -> create joint and mesh with submeshes
	//		* DaeNode and no Instance -> forward transformation to children (pure transformation node)
	//		* DaeNode and one Instance -> create mesh
	//		* DaeNode and several Instances -> create mesh with submeshes
	
	if (node.reference )
	{
		DaeNode* n = _daeDoc.libNodes.findNode( node.name );
		if( n )	return processNode( *n, parentNode, transAccum, animTransAccum );
		else log( "Warning: undefined reference '"+ node.name +"' in instance_node" );
		return 0x0;
	}

	// Assemble matrix
	Matrix4f relMat = transAccum * makeMatrix4f( node.assembleMatrix().transposed().x, _daeDoc.y_up );
	
	SceneNode *oNode = 0x0;
	
	// Find valid instances
	std::vector< int > validInsts;
	for( unsigned int i = 0; i < node.instances.size(); ++i )
	{
		if( validateInstance( node.instances[i].url ) )
			validInsts.push_back( i );
	}
	
	// Create node
	if( node.joint )
	{
		oNode = new Joint();
		_joints.push_back( (Joint *)oNode );
	}
	else
	{
		if( !validInsts.empty() )
		{
			oNode = new Mesh();
			_meshes.push_back( (Mesh *)oNode );
			oNode->daeInstance = &node.instances[validInsts[0]];
		}
	}
	
	// Set node params
	if( oNode != 0x0 )
	{
		oNode->parent = parentNode;
		oNode->daeNode = &node;
		oNode->matRel = relMat;
		
		// Name
		if( node.name.length() > 255 )
		{
			log( "Warning: node name is too long" );
			node.name.erase( 255, node.name.length() - 255 );
		}
		strcpy( oNode->name, node.name.c_str() );

		// Check for duplicate node name
		checkNodeName( oNode );
			
		// Calculate absolute transformation
		if( parentNode != 0x0 ) oNode->matAbs = parentNode->matAbs * oNode->matRel;
		else oNode->matAbs = oNode->matRel;
	}

	// Create sub-nodes if necessary
	if( oNode != 0x0 )
	{
		if( node.joint && !validInsts.empty() )
		{
			SceneNode *oNode2 = new Mesh();
			_meshes.push_back( (Mesh *)oNode2 );
			oNode->children.push_back( _meshes.back() );
			
			*oNode2 = *oNode;
			oNode2->typeJoint = false;
			oNode2->matRel = Matrix4f();
			oNode2->daeInstance = &node.instances[validInsts[0]];
			oNode2->parent = oNode;
			oNode2->children.clear();
			for( unsigned int i = 0; i < oNode2->frames.size(); ++i )
				oNode2->frames[i] = Matrix4f();

			// Create submeshes if there are several instances
			for( unsigned int i = 1; i < validInsts.size(); ++i )
			{
				SceneNode *oNode3 = new Mesh();
				_meshes.push_back( (Mesh *)oNode3 );
				oNode2->children.push_back( _meshes.back() );

				*oNode3 = *oNode2;
				oNode3->daeInstance = &node.instances[validInsts[i]];
				oNode3->parent = oNode2;
			}
		}
		else if( !node.joint && validInsts.size() > 1 )
		{
			// Create submeshes
			for( unsigned int i = 1; i < validInsts.size(); ++i )
			{
				SceneNode *oNode2 = new Mesh();
				_meshes.push_back( (Mesh *)oNode2 );
				oNode->children.push_back( _meshes.back() );

				*oNode2 = *oNode;
				oNode2->matRel = Matrix4f();
				oNode2->daeInstance = &node.instances[validInsts[i]];
				oNode2->parent = oNode;
				oNode2->children.clear();
				for( unsigned int j = 0; j < oNode2->frames.size(); ++j )
					oNode2->frames[j] = Matrix4f();
			}
		}
	}

	if( oNode != 0x0 ) transAccum = Matrix4f();
	else transAccum = relMat;

	// Animation
	for( unsigned int i = 0; i < _frameCount; ++i )
	{
		Matrix4f mat = animTransAccum[i] * getNodeTransform( node, i );
		if( oNode != 0x0 )
		{
			oNode->frames.push_back( mat );
			animTransAccum[i] = Matrix4f();
		}
		else animTransAccum[i] = mat;	// Pure transformation node
	}
	
	// Process children
	for( unsigned int i = 0; i < node.children.size(); ++i )
	{
		SceneNode *parNode = oNode != 0x0 ? oNode : parentNode;
		
		SceneNode *childNode = processNode( *node.children[i], parNode,
		                                    transAccum, animTransAccum );
		if( childNode != 0x0 && parNode != 0x0 ) parNode->children.push_back( childNode );
	}

	return oNode;
}


void Converter::calcTangentSpaceBasis( vector<Vertex> &verts )
{
	for( unsigned int i = 0; i < verts.size(); ++i )
	{
		verts[i].normal = Vec3f( 0, 0, 0 );
		verts[i].tangent = Vec3f( 0, 0, 0 );
		verts[i].bitangent = Vec3f( 0, 0, 0 );
	}
	
	// Basic algorithm: Eric Lengyel, Mathematics for 3D Game Programming & Computer Graphics
	for( unsigned int i = 0; i < _meshes.size(); ++i )
	{
		for( unsigned int j = 0; j < _meshes[i]->triGroups.size(); ++j )
		{
			TriGroup *triGroup = _meshes[i]->triGroups[j];
			
			for( unsigned int k = triGroup->first; k < triGroup->first + triGroup->count; k += 3 )
			{
				// Compute basis vectors for triangle
				Vec3f edge1uv = verts[_indices[k + 1]].texCoords[0] - verts[_indices[k]].texCoords[0];
				Vec3f edge2uv = verts[_indices[k + 2]].texCoords[0] - verts[_indices[k]].texCoords[0];
				Vec3f edge1 = verts[_indices[k + 1]].pos - verts[_indices[k]].pos;
				Vec3f edge2 = verts[_indices[k + 2]].pos - verts[_indices[k]].pos;
				Vec3f normal = edge1.cross( edge2 );  // Normal weighted by triangle size (hence unnormalized)

				float r = 1.0f / (edge1uv.x * edge2uv.y - edge2uv.x * edge1uv.y); // UV area normalization
				Vec3f uDir = (edge1 * edge2uv.y - edge2 * edge1uv.y) * r;
				Vec3f vDir = (edge2 * edge1uv.x - edge1 * edge2uv.x) * r;

				// Accumulate basis for vertices
				for( unsigned int l = 0; l < 3; ++l )
				{
					verts[_indices[k + l]].normal += normal;
					verts[_indices[k + l]].tangent += uDir;
					verts[_indices[k + l]].bitangent += vDir;

					// Handle texture seams where vertices were split
					vector< unsigned int > &vertList =
						triGroup->posIndexToVertices[verts[_indices[k + l]].daePosIndex];
					for( unsigned int m = 0; m < vertList.size(); ++m )
					{
						if( vertList[m] != _indices[k + l] &&
						    verts[vertList[m]].storedNormal == verts[_indices[k + l]].storedNormal )
						{
							verts[vertList[m]].normal += normal;
							verts[vertList[m]].tangent += uDir;
							verts[vertList[m]].bitangent += vDir;
						}
					}
				}
			}
		}
	}

	// Normalize tangent space basis
	unsigned int numInvalidBasis = 0;
	for( unsigned int i = 0; i < verts.size(); ++i )
	{
		// Check if tangent space basis is invalid
		if( verts[i].normal.length() == 0 || verts[i].tangent.length() == 0 || verts[i].bitangent.length() == 0 )
			++numInvalidBasis;
		
		// Gram–Schmidt orthogonalization
		verts[i].normal.normalize();
		Vec3f &n = verts[i].normal;
		Vec3f &t = verts[i].tangent;
		verts[i].tangent = (t - n * n.dot( t )).normalized();
		
		// Calculate handedness (required to support mirroring) and final bitangent
		float handedness = n.cross( t ).dot( verts[i].bitangent ) < 0 ? -1.0f : 1.0f;
		verts[i].bitangent = n.cross( t ) * handedness;
	}

	if( numInvalidBasis > 0 )
	{
		log( "Warning: Geometry has zero-length basis vectors" );
		log( "   Maybe two faces point in opposite directions and share same vertices" );
	}
}


void Converter::processJoints()
{
	for( unsigned int i = 0; i < _joints.size(); ++i )
	{
		_joints[i]->index = i + 1;	// First index is identity matrix
		_joints[i]->invBindMat = _joints[i]->matAbs.inverted();
	}

	if( _joints.size() + 1 > 75 ) log( "Warning: Model has more than 75 joints" );
}


void Converter::processMeshes( bool optimize )
{
	// Note: At the moment the geometry for all nodes is copied and not referenced
	for( unsigned int i = 0; i < _meshes.size(); ++i )
	{
		// Interpret mesh LOD level
		if( strstr( _meshes[i]->name, "_lod1" ) == _meshes[i]->name + strlen( _meshes[i]->name ) - 5 )
			_meshes[i]->lodLevel = 1;
		else if( strstr( _meshes[i]->name, "_lod2" ) == _meshes[i]->name + strlen( _meshes[i]->name ) - 5 )
			_meshes[i]->lodLevel = 2;
		else if( strstr( _meshes[i]->name, "_lod3" ) == _meshes[i]->name + strlen( _meshes[i]->name ) - 5 )
			_meshes[i]->lodLevel = 3;
		else if( strstr( _meshes[i]->name, "_lod4" ) == _meshes[i]->name + strlen( _meshes[i]->name ) - 5 )
			_meshes[i]->lodLevel = 4;
		
		if( _meshes[i]->lodLevel > 0 )
		{
			if( _meshes[i]->lodLevel > _maxLodLevel ) _maxLodLevel = _meshes[i]->lodLevel;
			_meshes[i]->name[strlen( _meshes[i]->name ) - 5] = '\0';  // Cut off lod postfix from name
		}
		
		// Find geometry/controller for node
		string id = _meshes[i]->daeInstance->url;
		
		DaeSkin *skin = _daeDoc.libControllers.findSkin( id );
		DaeMorph *morpher = _daeDoc.libControllers.findMorph( id );
		
		// Resolve controller stack
		if( skin != 0x0 )
		{	
			id = skin->ownerId;
			morpher = _daeDoc.libControllers.findMorph( id );
			if( morpher != 0x0 ) id = morpher->ownerId;
		}
		else if( morpher != 0x0 )
		{
			id = morpher->ownerId;
			skin = _daeDoc.libControllers.findSkin( id );
			if( skin != 0x0 ) id = skin->ownerId;
		}

		// Check that skin has all required arrays
		if( skin != 0x0 )
		{
			if( skin->jointArray == 0x0 || skin->bindMatArray == 0x0 || skin->weightArray == 0x0 )
			{
				log( "Skin controller '" + skin->id + "' is missing information and is ignored" );
				skin = 0x0;
			}
		}
		
		DaeGeometry *geo = _daeDoc.libGeometries.findGeometry( id );
		ASSERT( geo != 0x0 );
		
		vector< Joint * > jointLookup;
		if( skin != 0x0 )
		{
			// Build lookup table
			for( unsigned int j = 0; j < skin->jointArray->stringArray.size(); ++j )
			{
				string sid = skin->jointArray->stringArray[j];

				jointLookup.push_back( 0x0 );
				
				bool found = false;
				for( unsigned int k = 0; k < _joints.size(); ++k )
				{
					if( _joints[k]->daeNode->sid == sid )
					{
						jointLookup[j] = _joints[k];
						found = true;
						break;
					}
				}

				if( !found )
				{
					log( "Warning: joint '" + sid + "' used in skin controller not found" );
				}
			}
			
			// Find bind matrices
			Matrix4f bindShapeMat = makeMatrix4f( &skin->bindShapeMat[0], _daeDoc.y_up );
			for( unsigned int j = 0; j < skin->jointArray->stringArray.size(); ++j )
			{
				if( jointLookup[j] != 0x0 )
				{
					jointLookup[j]->used = true;
					
					jointLookup[j]->daeInvBindMat =
						makeMatrix4f( &skin->bindMatArray->floatArray[j * 16], _daeDoc.y_up );

					// Multiply bind matrices with bind shape matrix
					jointLookup[j]->daeInvBindMat = jointLookup[j]->daeInvBindMat * bindShapeMat;
				}
			}
		}
		
		unsigned int firstGeoVert = (unsigned int)_vertices.size();
		
		for( unsigned int j = 0; j < geo->triGroups.size(); ++j )
		{
			DaeTriGroup &iTriGroup = geo->triGroups[j];
			TriGroup* oTriGroup = new TriGroup();

			DaeMaterial *mat = _daeDoc.libMaterials.findMaterial(
				_meshes[i]->daeInstance->materialBindings[iTriGroup.matId] );
			if( mat != 0x0 )
			{
				oTriGroup->matName = mat->name;
				mat->used = true;
			}
			else
				log( "Warning: Material '" + oTriGroup->matName + "' not found" );

			oTriGroup->first = (unsigned int)_indices.size();
			oTriGroup->count = (unsigned int)iTriGroup.indices.size();
			oTriGroup->vertRStart = (unsigned int)_vertices.size();

			// Add indices and vertices
			oTriGroup->numPosIndices = (unsigned int)iTriGroup.vSource->posSource->floatArray.size() /
			                          iTriGroup.vSource->posSource->paramsPerItem;
			oTriGroup->posIndexToVertices = new vector< unsigned int >[oTriGroup->numPosIndices];
			
			for( unsigned int k = 0; k < iTriGroup.indices.size(); ++k )
			{
				// Try to find vertex
				vector< unsigned int > &vertList = oTriGroup->posIndexToVertices[iTriGroup.indices[k].posIndex];
				bool found = false;
				unsigned int index = (unsigned int)_vertices.size();
				
				// Only check vertices that have the same Collada position index
				for( unsigned int l = 0; l < vertList.size(); ++l )
				{
					Vertex &v = _vertices[vertList[l]];
					
					if( v.storedPos == iTriGroup.getPos( iTriGroup.indices[k].posIndex ) &&
						v.storedNormal == iTriGroup.getNormal( iTriGroup.indices[k].normIndex ) &&
						v.texCoords[0] == iTriGroup.getTexCoords( iTriGroup.indices[k].texIndex[0], 0 ) &&
						v.texCoords[1] == iTriGroup.getTexCoords( iTriGroup.indices[k].texIndex[1], 1 ) &&
						v.texCoords[2] == iTriGroup.getTexCoords( iTriGroup.indices[k].texIndex[2], 2 ) &&
						v.texCoords[3] == iTriGroup.getTexCoords( iTriGroup.indices[k].texIndex[3], 3 ) )
	
					{
						found = true;
						index = vertList[l];
						break;
					}
				}

				if( found )
				{
					_indices.push_back( index );	
				}
				else
				{
					Vertex v;

					v.daePosIndex = iTriGroup.indices[k].posIndex;
					int normIndex = iTriGroup.indices[k].normIndex;

					// Position
					v.storedPos = iTriGroup.getPos( v.daePosIndex );
					v.pos = v.storedPos;
					if( !_daeDoc.y_up )
					{	
						swap( v.pos.y, v.pos.z );
						v.pos.z *= -1;
					}

					// Texture coordinates
					v.texCoords[0] = iTriGroup.getTexCoords( iTriGroup.indices[k].texIndex[0], 0 );
					v.texCoords[1] = iTriGroup.getTexCoords( iTriGroup.indices[k].texIndex[1], 1 );
					v.texCoords[2] = iTriGroup.getTexCoords( iTriGroup.indices[k].texIndex[2], 2 );
					v.texCoords[3] = iTriGroup.getTexCoords( iTriGroup.indices[k].texIndex[3], 3 );

					// Normal
					v.storedNormal = iTriGroup.getNormal( normIndex );

					// Skinning
					if( skin != 0x0 && v.daePosIndex < (int)skin->vertWeights.size() )
					{
						DaeVertWeights vertWeights = skin->vertWeights[v.daePosIndex];
						
						// Sort weights
						for( unsigned int xx = 0; xx < vertWeights.size(); ++xx )
						{
							for( unsigned int yy = 0; yy < xx; ++yy )
							{
								if( skin->weightArray->floatArray[(int)vertWeights[xx].weight] >
								    skin->weightArray->floatArray[(int)vertWeights[yy].weight] )
								{
									swap( vertWeights[xx], vertWeights[yy] );
								}
							}
						}
						
						// Take the four most significant weights
						for( unsigned int l = 0; l < vertWeights.size(); ++l )
						{
							if( l == 4 ) break;
							v.weights[l] = skin->weightArray->floatArray[vertWeights[l].weight];
							v.joints[l] = jointLookup[vertWeights[l].joint];
						}

						// Normalize weights
						float weightSum = v.weights[0] + v.weights[1] + v.weights[2] + v.weights[3];
						if( weightSum > Math::Epsilon )
						{
							v.weights[0] /= weightSum;
							v.weights[1] /= weightSum;
							v.weights[2] /= weightSum;
							v.weights[3] /= weightSum;
						}
						else
						{
							v.weights[0] = 1.0f;
							v.weights[1] = v.weights[2] = v.weights[3] = 0.0f;
						}

						// Apply skinning to vertex
						if( v.joints[0] != 0x0 || v.joints[1] != 0x0 || v.joints[2] != 0x0 || v.joints[3] != 0x0 )
						{
							Vec3f newPos( 0, 0, 0 );
							if( v.joints[0] != 0x0 )
								newPos += v.joints[0]->matAbs * v.joints[0]->daeInvBindMat * v.pos * v.weights[0];
							if( v.joints[1] != 0x0 )
								newPos += v.joints[1]->matAbs * v.joints[1]->daeInvBindMat * v.pos * v.weights[1];
							if( v.joints[2] != 0x0 )
								newPos += v.joints[2]->matAbs * v.joints[2]->daeInvBindMat * v.pos * v.weights[2];
							if( v.joints[3] != 0x0 )
								newPos += v.joints[3]->matAbs * v.joints[3]->daeInvBindMat * v.pos * v.weights[3];
							v.pos = newPos;
						}
					}
					
					_vertices.push_back( v );
					_indices.push_back( index );

					vertList.push_back( (unsigned int)_vertices.size() - 1 );
				}
			}

			oTriGroup->vertREnd = (unsigned int)_vertices.size() - 1;
			
			// Remove degenerated triangles
			unsigned int numDegTris = MeshOptimizer::removeDegeneratedTriangles( oTriGroup, _vertices, _indices );
			if( numDegTris > 0 )
			{
				stringstream ss;
				ss << numDegTris;
				log( "Removed " + ss.str() + " degenerated triangles from mesh " + _meshes[i]->daeNode->id );
			}
			
			_meshes[i]->triGroups.push_back( oTriGroup );			
		}

		unsigned int numGeoVerts = (unsigned int)_vertices.size() - firstGeoVert;

		// Morph targets
		if( morpher != 0x0 && morpher->targetArray != 0x0 )
		{
			for( unsigned int j = 0; j < morpher->targetArray->stringArray.size(); ++j )
			{
				string targetId = morpher->targetArray->stringArray[j];
				DaeGeometry *targetGeo = _daeDoc.libGeometries.findGeometry( targetId );
				if( targetGeo == 0x0 ) continue;
				
				// Try to find morph target
				int index = -1;
				for( unsigned int k = 0; k < _morphTargets.size(); ++k )
				{
					if( targetId == _morphTargets[k].name )
					{
						index = k;
						break;
					}
				}
				
				// Add new morph target if not found
				if( index < 0 )
				{
					MorphTarget mt;
					
					if( targetGeo->name != "" )
						strcpy( mt.name, targetGeo->name.c_str() );
					else
						strcpy( mt.name, targetId.c_str() );
					
					_morphTargets.push_back( mt );
					index = (unsigned int)_morphTargets.size() - 1;
				}

				// Loop over all vertices (Collada allows only exactly one vertex source per mesh)
				if( geo->vsources[0].posSource->floatArray.size() !=
				    targetGeo->vsources[0].posSource->floatArray.size() )
				{
					log( "Warning: morph target geometry differs from base geometry" );
					continue;
				}
				
				for( unsigned int k = 0; k < numGeoVerts; ++k )
				{
					Vertex &v = _vertices[firstGeoVert + k];
					
					Vec3f basePos = geo->getPos( v.daePosIndex );
					Vec3f targetPos = targetGeo->getPos( v.daePosIndex );
					
					if( targetPos != basePos )
					{
						if( !_daeDoc.y_up )
						{	
							swap( targetPos.y, targetPos.z );
							targetPos.z *= -1;
						}
						
						// Apply skinning to morph target vertex
						if( v.joints[0] != 0x0 || v.joints[1] != 0x0 || v.joints[2] != 0x0 || v.joints[3] != 0x0 )
						{
							Vec3f newPos( 0, 0, 0 );
							if( v.joints[0] != 0x0 )
								newPos += v.joints[0]->matAbs * v.joints[0]->daeInvBindMat * targetPos * v.weights[0];
							if( v.joints[1] != 0x0 )
								newPos += v.joints[1]->matAbs * v.joints[1]->daeInvBindMat * targetPos * v.weights[1];
							if( v.joints[2] != 0x0 )
								newPos += v.joints[2]->matAbs * v.joints[2]->daeInvBindMat * targetPos * v.weights[2];
							if( v.joints[3] != 0x0 )
								newPos += v.joints[3]->matAbs * v.joints[3]->daeInvBindMat * targetPos * v.weights[3];
							targetPos = newPos;
						}
						
						Vec3f diff = targetPos - v.pos;
						
						MorphDiff md;
						md.vertIndex = firstGeoVert + k;
						md.posDiff = diff;
						_morphTargets[index].diffs.push_back( md );
					}
				}
			}	
		}
	}

	// Calculate tangent space basis for base mesh
	calcTangentSpaceBasis( _vertices );

	// Calculate tangent space basis for morph targets
	vector< Vertex > verts( _vertices );
	for( unsigned int i = 0; i < _morphTargets.size(); ++i )
	{
		// Morph
		for( unsigned int j = 0; j < _morphTargets[i].diffs.size(); ++j )
		{	
			verts[_morphTargets[i].diffs[j].vertIndex].pos += _morphTargets[i].diffs[j].posDiff;
		}

		calcTangentSpaceBasis( verts );

		// Find basis differences and undo morphing
		for( unsigned int j = 0; j < _morphTargets[i].diffs.size(); ++j )
		{	
			MorphDiff &md = _morphTargets[i].diffs[j];
			
			verts[md.vertIndex].pos -= md.posDiff;
			md.normDiff = verts[md.vertIndex].normal - _vertices[md.vertIndex].normal;
			md.tanDiff = verts[md.vertIndex].tangent - _vertices[md.vertIndex].tangent;
			md.bitanDiff = verts[md.vertIndex].bitangent - _vertices[md.vertIndex].bitangent;
		}
	}

	// Optimization and clean up
	float optEffBefore = 0, optEffAfter = 0;
	unsigned int optNumCalls = 0;
	for( unsigned int i = 0; i < _meshes.size(); ++i )
	{
		for( unsigned int j = 0; j < _meshes[i]->triGroups.size(); ++j )
		{
			// Optimize order of indices for best vertex cache usage and remap vertices
			if( optimize )
			{
				map< unsigned int, unsigned int > vertMap;
				
				++optNumCalls;
				optEffBefore += MeshOptimizer::calcCacheEfficiency( _meshes[i]->triGroups[j], _indices );
				MeshOptimizer::optimizeIndexOrder( _meshes[i]->triGroups[j], _vertices, _indices, vertMap );
				optEffAfter += MeshOptimizer::calcCacheEfficiency( _meshes[i]->triGroups[j], _indices );

				// Update morph target vertex indices according to vertex remapping
				for( unsigned int k = 0; k < _morphTargets.size(); ++k )
				{
					for( unsigned int l = 0; l < _morphTargets[k].diffs.size(); ++l )
					{
						map< unsigned int, unsigned int >::iterator itr1 =
							vertMap.find( _morphTargets[k].diffs[l].vertIndex );

						if( itr1 != vertMap.end() )
						{
							_morphTargets[k].diffs[l].vertIndex = itr1->second;
						}
					}
				}
			}
			
			// Clean up
			delete[] _meshes[i]->triGroups[j]->posIndexToVertices;
			_meshes[i]->triGroups[j]->posIndexToVertices = 0x0;
		}
	}

	// Output info about optimization
	/*if( optNumCalls > 0 )
	{
		stringstream ss;
		ss << fixed << setprecision( 3 );
		ss << "Optimized geometry for vertex cache: from ATVR " << optEffBefore / optNumCalls;
		ss << " to ATVR " << optEffAfter / optNumCalls;
		log( ss.str() );
	}*/
}


bool Converter::convertModel( bool optimize )
{
	if( _daeDoc.scene == 0x0 ) return true;		// Nothing to convert
	
	_frameCount = _daeDoc.libAnimations.maxFrameCount;

	// Output default pose if no animation is available
	if( _frameCount == 0 ) _frameCount = 1;

	vector< Matrix4f > animTransAccum;
	animTransAccum.resize( _frameCount );
	
	// Process all nodes
	for( unsigned int i = 0; i < _daeDoc.scene->nodes.size(); ++i )
	{
		_nodes.push_back( processNode( *_daeDoc.scene->nodes[i], 0x0, Matrix4f(), animTransAccum ) );
	}

	if( _animNotSampled )
		log( "Warning: Animation is not sampled and will probably be wrong" );

	// Process joints and meshes
	processJoints();
	processMeshes( optimize );
	
	return true;
}


bool Converter::writeGeometry( const string &assetPath, const string &assetName )
{
	string fileName = _outPath + assetPath + assetName + ".geo";
	FILE *f = fopen( fileName.c_str(), "wb" );
	if( f == 0x0 )
	{	
		log( "Failed to write " + fileName + " file" );
		return false;
	}

	// Write header
	unsigned int version = 5;
	fwrite( "H3DG", 4, 1, f );
	fwrite( &version, sizeof( int ), 1, f ); 
	
	// Write joints
	unsigned int count = (unsigned int)_joints.size() + 1;
	fwrite( &count, sizeof( int ), 1, f );

	// Write default identity matrix
	for( unsigned int j = 0; j < 16; ++j )
		fwrite( &Matrix4f().x[j], sizeof( float ), 1, f );

	for( unsigned int i = 0; i < _joints.size(); ++i )
	{
		// Inverse bind matrix
		for( unsigned int j = 0; j < 16; ++j )
		{
			fwrite( &_joints[i]->invBindMat.x[j], sizeof( float ), 1, f );
		}
	}
	
	// Write vertex stream data
	if( _joints.empty() ) count = 6; else count = 8;	// Number of streams
	fwrite( &count, sizeof( int ), 1, f );
	count = (unsigned int)_vertices.size();
	fwrite( &count, sizeof( int ), 1, f );

	for( unsigned int i = 0; i < 8; ++i )
	{
		if( _joints.empty() && (i == 4 || i == 5) ) continue;
		
		unsigned char uc;
		short sh;
		unsigned int streamElemSize;
		
		switch( i )
		{
		case 0:		// Position
			fwrite( &i, sizeof( int ), 1, f );
			streamElemSize = 3 * sizeof( float ); fwrite( &streamElemSize, sizeof( int ), 1, f );
			for( unsigned int j = 0; j < count; ++j )
			{
				fwrite( &_vertices[j].pos.x, sizeof( float ), 1, f );
				fwrite( &_vertices[j].pos.y, sizeof( float ), 1, f );
				fwrite( &_vertices[j].pos.z, sizeof( float ), 1, f );
			}
			break;
		case 1:		// Normal
			fwrite( &i, sizeof( int ), 1, f );
			streamElemSize = 3 * sizeof( short ); fwrite( &streamElemSize, sizeof( int ), 1, f );
			for( unsigned int j = 0; j < count; ++j )
			{
				sh = (short)(_vertices[j].normal.x * 32767); fwrite( &sh, sizeof( short ), 1, f );
				sh = (short)(_vertices[j].normal.y * 32767); fwrite( &sh, sizeof( short ), 1, f );
				sh = (short)(_vertices[j].normal.z * 32767); fwrite( &sh, sizeof( short ), 1, f );
			}
			break;
		case 2:		// Tangent
			fwrite( &i, sizeof( int ), 1, f );
			streamElemSize = 3 * sizeof( short ); fwrite( &streamElemSize, sizeof( int ), 1, f );
			for( unsigned int j = 0; j < count; ++j )
			{
				sh = (short)(_vertices[j].tangent.x * 32767); fwrite( &sh, sizeof( short ), 1, f );
				sh = (short)(_vertices[j].tangent.y * 32767); fwrite( &sh, sizeof( short ), 1, f );
				sh = (short)(_vertices[j].tangent.z * 32767); fwrite( &sh, sizeof( short ), 1, f );
			}
			break;
		case 3:		// Bitangent
			fwrite( &i, sizeof( int ), 1, f );
			streamElemSize = 3 * sizeof( short ); fwrite( &streamElemSize, sizeof( int ), 1, f );
			for( unsigned int j = 0; j < count; ++j )
			{
				sh = (short)(_vertices[j].bitangent.x * 32767); fwrite( &sh, sizeof( short ), 1, f );
				sh = (short)(_vertices[j].bitangent.y * 32767); fwrite( &sh, sizeof( short ), 1, f );
				sh = (short)(_vertices[j].bitangent.z * 32767); fwrite( &sh, sizeof( short ), 1, f );
			}
			break;
		case 4:		// Joint indices
			fwrite( &i, sizeof( int ), 1, f );
			streamElemSize = 4 * sizeof( char ); fwrite( &streamElemSize, sizeof( int ), 1, f );
			for( unsigned int j = 0; j < count; ++j )
			{
				unsigned char jointIndices[4] = { 0, 0, 0, 0 };
				if( _vertices[j].joints[0] != 0x0 )
					jointIndices[0] = (unsigned char)_vertices[j].joints[0]->index;
				if( _vertices[j].joints[1] != 0x0 )
					jointIndices[1] = (unsigned char)_vertices[j].joints[1]->index;
				if( _vertices[j].joints[2] != 0x0 )
					jointIndices[2] = (unsigned char)_vertices[j].joints[2]->index;
				if( _vertices[j].joints[3] != 0x0 )
					jointIndices[3] = (unsigned char)_vertices[j].joints[3]->index;
				fwrite( &jointIndices[0], sizeof( char ), 1, f );
				fwrite( &jointIndices[1], sizeof( char ), 1, f );
				fwrite( &jointIndices[2], sizeof( char ), 1, f );
				fwrite( &jointIndices[3], sizeof( char ), 1, f );
			}
			break;
		case 5:		// Weights
			fwrite( &i, sizeof( int ), 1, f );
			streamElemSize = 4 * sizeof( char ); fwrite( &streamElemSize, sizeof( int ), 1, f );
			for( unsigned int j = 0; j < count; ++j )
			{
				uc = (unsigned char)(_vertices[j].weights[0] * 255); fwrite( &uc, sizeof( char ), 1, f );
				uc = (unsigned char)(_vertices[j].weights[1] * 255); fwrite( &uc, sizeof( char ), 1, f );
				uc = (unsigned char)(_vertices[j].weights[2] * 255); fwrite( &uc, sizeof( char ), 1, f );
				uc = (unsigned char)(_vertices[j].weights[3] * 255); fwrite( &uc, sizeof( char ), 1, f );
			}
			break;
		case 6:		// Texture Coord Set 1
			fwrite( &i, sizeof( int ), 1, f );
			streamElemSize = 2 * sizeof( float ); fwrite( &streamElemSize, sizeof( int ), 1, f );
			for( unsigned int j = 0; j < count; ++j )
			{
				fwrite( &_vertices[j].texCoords[0].x, sizeof( float ), 1, f );
				fwrite( &_vertices[j].texCoords[0].y, sizeof( float ), 1, f );
			}
			break;
		case 7:		// Texture Coord Set 2
			fwrite( &i, sizeof( int ), 1, f );
			streamElemSize = 2 * sizeof( float ); fwrite( &streamElemSize, sizeof( int ), 1, f );
			for( unsigned int j = 0; j < count; ++j )
			{
				fwrite( &_vertices[j].texCoords[1].x, sizeof( float ), 1, f );
				fwrite( &_vertices[j].texCoords[1].y, sizeof( float ), 1, f );
			}
			break;
		}
	}

	// Write triangle indices
	count = (unsigned int)_indices.size();
	fwrite( &count, sizeof( int ), 1, f );

	for( unsigned int i = 0; i < _indices.size(); ++i )
	{
		fwrite( &_indices[i], sizeof( int ), 1, f );
	}

	// Write morph targets
	count = (unsigned int)_morphTargets.size();
	fwrite( &count, sizeof( int ), 1, f );

	for( unsigned int i = 0; i < _morphTargets.size(); ++i )
	{
		fwrite( &_morphTargets[i].name, 256, 1, f );
		
		// Write vertex indices
		count = (unsigned int)_morphTargets[i].diffs.size();
		fwrite( &count, sizeof( int ), 1, f );

		for( unsigned int j = 0; j < count; ++j )
		{
			fwrite( &_morphTargets[i].diffs[j].vertIndex, sizeof( int ), 1, f );
		}
		
		// Write stream data
		unsigned int numStreams = 4, streamElemSize;
		fwrite( &numStreams, sizeof( int ), 1, f );

		for( unsigned int j = 0; j < 4; ++j )
		{
			switch( j )
			{
			case 0:		// Position
				fwrite( &j, sizeof( int ), 1, f );
				streamElemSize = 3 * sizeof( float ); fwrite( &streamElemSize, sizeof( int ), 1, f );
				for( unsigned int k = 0; k < count; ++k )
				{
					fwrite( &_morphTargets[i].diffs[k].posDiff.x, sizeof( float ), 1, f );
					fwrite( &_morphTargets[i].diffs[k].posDiff.y, sizeof( float ), 1, f );
					fwrite( &_morphTargets[i].diffs[k].posDiff.z, sizeof( float ), 1, f );
				}
				break;
			case 1:		// Normal
				fwrite( &j, sizeof( int ), 1, f );
				streamElemSize = 3 * sizeof( float ); fwrite( &streamElemSize, sizeof( int ), 1, f );
				for( unsigned int k = 0; k < count; ++k )
				{
					fwrite( &_morphTargets[i].diffs[k].normDiff.x, sizeof( float ), 1, f );
					fwrite( &_morphTargets[i].diffs[k].normDiff.y, sizeof( float ), 1, f );
					fwrite( &_morphTargets[i].diffs[k].normDiff.z, sizeof( float ), 1, f );
				}
				break;
			case 2:		// Tangent
				fwrite( &j, sizeof( int ), 1, f );
				streamElemSize = 3 * sizeof( float ); fwrite( &streamElemSize, sizeof( int ), 1, f );
				for( unsigned int k = 0; k < count; ++k )
				{
					fwrite( &_morphTargets[i].diffs[k].tanDiff.x, sizeof( float ), 1, f );
					fwrite( &_morphTargets[i].diffs[k].tanDiff.y, sizeof( float ), 1, f );
					fwrite( &_morphTargets[i].diffs[k].tanDiff.z, sizeof( float ), 1, f );
				}
				break;
			case 3:		// Bitangent
				fwrite( &j, sizeof( int ), 1, f );
				streamElemSize = 3 * sizeof( float ); fwrite( &streamElemSize, sizeof( int ), 1, f );
				for( unsigned int k = 0; k < count; ++k )
				{
					fwrite( &_morphTargets[i].diffs[k].bitanDiff.x, sizeof( float ), 1, f );
					fwrite( &_morphTargets[i].diffs[k].bitanDiff.y, sizeof( float ), 1, f );
					fwrite( &_morphTargets[i].diffs[k].bitanDiff.z, sizeof( float ), 1, f );
				}
				break;
			}
		}
	}
	
	fclose( f );

	return true;
}


void Converter::writeSGNode( const string &assetPath, const string &modelName, SceneNode *node, unsigned int depth, ofstream &outf )
{
	Vec3f trans, rot, scale;
	node->matRel.decompose( trans, rot, scale );
	rot.x = radToDeg( rot.x );
	rot.y = radToDeg( rot.y );
	rot.z = radToDeg( rot.z );

	// Write mesh
	if( !node->typeJoint )
	{
		Mesh *mesh = (Mesh *)node;
		
		// Write triangle groups as submeshes of first triangle group
		for( unsigned int i = 0; i < mesh->triGroups.size(); ++i )
		{
			for( unsigned int j = 0; j < depth + 1; ++j ) outf << "\t";
			if( i > 0 ) outf << "\t";
			outf << "<Mesh ";
			outf << "name=\"" << (i > 0 ? "#" : "") << mesh->name << "\" ";
			if( mesh->lodLevel > 0 ) outf << "lodLevel=\"" << mesh->lodLevel << "\" ";
			outf << "material=\"";
			outf << assetPath + modelName + mesh->triGroups[i]->matName + ".material.xml\" ";
			
			if( i == 0 )
			{
				if( trans != Vec3f( 0, 0, 0 ) )
					outf << "tx=\"" << trans.x << "\" ty=\"" << trans.y << "\" tz=\"" << trans.z << "\" ";
				if( rot != Vec3f( 0, 0, 0 ) )
					outf << "rx=\"" << rot.x << "\" ry=\"" << rot.y << "\" rz=\"" << rot.z << "\" ";
				if( scale != Vec3f( 1, 1, 1 ) )
					outf << "sx=\"" << scale.x << "\" sy=\"" << scale.y << "\" sz=\"" << scale.z << "\" ";
			}

			outf << "batchStart=\"";
			outf << mesh->triGroups[i]->first;
			outf << "\" batchCount=\"";
			outf << mesh->triGroups[i]->count;
			outf << "\" vertRStart=\"";
			outf << mesh->triGroups[i]->vertRStart;
			outf << "\" vertREnd=\"";
			outf << mesh->triGroups[i]->vertREnd;
			outf << "\"";
			
			if( i == 0 && mesh->triGroups.size() > 1 ) outf << ">\n";
			if( i > 0 ) outf << " />\n";
		}
	}
	else
	{
		Joint *joint = (Joint *)node;
		
		for( unsigned int i = 0; i < depth + 1; ++i ) outf << "\t";
		outf << "<Joint ";
		outf << "name=\"" << joint->name << "\" ";
		if( trans != Vec3f( 0, 0, 0 ) )
			outf << "tx=\"" << trans.x << "\" ty=\"" << trans.y << "\" tz=\"" << trans.z << "\" ";
		if( rot != Vec3f( 0, 0, 0 ) )
			outf << "rx=\"" << rot.x << "\" ry=\"" << rot.y << "\" rz=\"" << rot.z << "\" ";
		if( scale != Vec3f( 1, 1, 1 ) )
			outf << "sx=\"" << scale.x << "\" sy=\"" << scale.y << "\" sz=\"" << scale.z << "\" ";
		outf << "jointIndex=\"" << joint->index << "\"";
	}

	if( node->children.size() == 0 )
	{
		if( !node->typeJoint && ((Mesh *)node)->triGroups.size() > 1 )
		{
			for( unsigned int j = 0; j < depth + 1; ++j ) outf << "\t";
			outf << "</Mesh>\n";
		}
		else
		{
			outf << " />\n";
		}
	}
	else
	{
		outf << ">\n";
		for( unsigned int j = 0; j < node->children.size(); ++j )
			writeSGNode( assetPath, modelName, node->children[j], depth + 1, outf );

		// Closing tag
		for( unsigned int j = 0; j < depth + 1; ++j ) outf << "\t";
		if( !node->typeJoint ) outf << "</Mesh>\n";
		else outf << "</Joint>\n";	
	}
}


bool Converter::writeSceneGraph( const string &assetPath, const string &assetName, const string &modelName )
{
	ofstream outf;
	outf.open( (_outPath + assetPath + assetName + ".scene.xml").c_str(), ios::out );
	if( !outf.good() )
	{
		log( "Failed to write " + _outPath + assetPath + assetName + ".scene file" );
		return false;
	}
	
	outf << "<Model name=\"" << assetName << "\" geometry=\"" << assetPath << assetName << ".geo\"";
	if( _maxLodLevel >= 1 ) outf << " lodDist1=\"" << _lodDist1 << "\"";
	if( _maxLodLevel >= 2 ) outf << " lodDist2=\"" << _lodDist2 << "\"";
	if( _maxLodLevel >= 3 ) outf << " lodDist3=\"" << _lodDist3 << "\"";
	if( _maxLodLevel >= 4 ) outf << " lodDist4=\"" << _lodDist4 << "\"";
	outf << ">\n";

	// Output morph target names as comment
	if( !_morphTargets.empty() )
	{
		outf << "\t<!-- Morph targets: ";
		for( unsigned int i = 0; i < _morphTargets.size(); ++i )
		{
			outf << "\"" << _morphTargets[i].name << "\" ";
		}
		outf << "-->\n\n";
	}
	
	// Joints
	for( unsigned int i = 0; i < _joints.size(); ++i )
	{
		if( _joints[i]->parent == 0x0 ) writeSGNode( assetPath, modelName, _joints[i], 0, outf );
	}

	outf << "\n";
	
	// Meshes
	for( unsigned int i = 0; i < _meshes.size(); ++i )
	{
		if( _meshes[i]->parent == 0x0 ) writeSGNode( assetPath, modelName, _meshes[i], 0, outf );
	}

	outf << "</Model>\n";

	outf.close();
	
	return true;
}


bool Converter::writeModel( const std::string &assetPath, const std::string &assetName, const std::string &modelName )
{
	bool result = true;
	
	if( !writeGeometry( assetPath, assetName ) ) result = false;
	if( !writeSceneGraph( assetPath, assetName, modelName ) ) result = false;

	return result;
}


bool Converter::writeMaterials( const string &assetPath, const string &modelName, bool replace )
{
	for( unsigned int i = 0; i < _daeDoc.libMaterials.materials.size(); ++i )
	{
		DaeMaterial &material = *_daeDoc.libMaterials.materials[i];
		
		if( !material.used ) continue;
		
		string fileName = _outPath + assetPath + modelName + material.name + ".material.xml";
		
		if( !replace )
		{
			// Skip writing material file if it already exists
			ifstream inf( fileName.c_str() );
			if( inf.good() )
			{	
				log( "Skipping material '" + assetPath + modelName + material.name + ".material.xml'" );
				continue;
			}
		}

		ofstream outf;
		outf.open( fileName.c_str(), ios::out );
		if( !outf.good() )
		{
			log( "Failed writing .material.xml file" );
			continue;
		}

		outf << "<Material>\n";
		
		if( material.effect != 0x0 )
		{
			outf << "\t<Shader source=\"shaders/model.shader\" />\n";

			if( !_joints.empty() )
				outf << "\t<ShaderFlag name=\"_F01_Skinning\" />\n";
			outf << "\n";
			
			if( material.effect->diffuseMap != 0x0 )
			{
				outf << "\t<Sampler name=\"albedoMap\" map=\"";
				outf << assetPath << material.effect->diffuseMap->fileName;
				outf << "\" />\n";
			}
			else if( !material.effect->diffuseColor.empty() )
			{
				outf << "\t<Uniform name=\"matDiffuseCol\" ";				
				char value = 'a';
				std::istringstream iss(material.effect->diffuseColor);
				std::string token;
				while(std::getline(iss, token, ' '))
				{
					outf << value++ << "=\"" << token << "\" ";
				}				
				outf << "/>\n";						
			}

			if( !material.effect->specularColor.empty() )
			{
				outf << "\t<Uniform name=\"matSpecParams\" ";				
				char value = 'a';
				std::istringstream iss(material.effect->specularColor);
				std::string token;
				while(std::getline(iss, token, ' ') && value < 'd' )
				{
					outf << value++ << "=\"" << token << "\" ";
				}				
				outf << "d=\"" << material.effect->shininess << "\" ";
				outf << "/>\n";

			}		
		}
		
		outf << "</Material>\n";

		outf.close();
	}
	
	return true;
}


bool Converter::hasAnimation()
{
	return _frameCount > 0;
}


void Converter::writeAnimFrames( SceneNode &node, FILE *f )
{
	fwrite( &node.name, 256, 1, f );
		
	// Animation compression: just store a single frame if all frames are equal
	char canCompress = 0;
	if( node.frames.size() > 1 )
	{
		canCompress = 1;

		// Check if all frames are equal
		for( unsigned int i = 1; i < node.frames.size(); ++i )
		{
			if( memcmp( node.frames[0].x, node.frames[i].x, 16 * sizeof( float ) ) != 0 )
			{
				canCompress = 0;
				break;
			}
		}
	}
	fwrite( &canCompress, sizeof( char ), 1, f );
	
	for( size_t i = 0; i < (canCompress ? 1 : node.frames.size()); ++i )
	{
		Vec3f transVec, rotVec, scaleVec;
		node.frames[i].decompose( transVec, rotVec, scaleVec );
		Quaternion rotQuat( rotVec.x, rotVec.y, rotVec.z );
		
		fwrite( &rotQuat.x, sizeof( float ), 1, f );
		fwrite( &rotQuat.y, sizeof( float ), 1, f );
		fwrite( &rotQuat.z, sizeof( float ), 1, f );
		fwrite( &rotQuat.w, sizeof( float ), 1, f );
		fwrite( &transVec.x, sizeof( float ), 1, f );
		fwrite( &transVec.y, sizeof( float ), 1, f );
		fwrite( &transVec.z, sizeof( float ), 1, f );
		fwrite( &scaleVec.x, sizeof( float ), 1, f );
		fwrite( &scaleVec.y, sizeof( float ), 1, f );
		fwrite( &scaleVec.z, sizeof( float ), 1, f );
	}
}


bool Converter::writeAnimation( const string &assetPath, const string &assetName )
{
	FILE *f = fopen( (_outPath + assetPath + assetName + ".anim").c_str(), "wb" );
	if( f == 0x0 )
	{
		log( "Failed writing " + _outPath + assetPath + assetName + ".anim file" );
		return false;
	}

	// Write header
	unsigned int version = 3;
	fwrite( "H3DA", 4, 1, f );
	fwrite( &version, sizeof( int ), 1, f );
	
	// Write number of nodes
	unsigned int count = 0;
	for( unsigned int i = 0; i < _joints.size(); ++i )
		if( _joints[i]->frames.size() > 0 ) ++count;
	for( unsigned int i = 0; i < _meshes.size(); ++i )
		if( _meshes[i]->frames.size() > 0 ) ++count;
	fwrite( &count, sizeof( int ), 1, f );
	fwrite( &_frameCount, sizeof( int ), 1, f );

	for( unsigned int i = 0; i < _joints.size(); ++i )
	{
		if( _joints[i]->frames.size() == 0 ) continue;
		
		writeAnimFrames( *_joints[i], f );
	}

	for( unsigned int i = 0; i < _meshes.size(); ++i )
	{
		if( _meshes[i]->frames.size() == 0 ) continue;
		
		writeAnimFrames( *_meshes[i], f );
	}
	
	fclose( f );

	return true;
}
