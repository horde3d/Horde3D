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

#include "egParticle.h"
#include "egMaterial.h"
#include "egModules.h"
#include "egCom.h"
#include "egRenderer.h"
#include "utXML.h"

#include "utDebug.h"


namespace Horde3D {

using namespace std;

// *************************************************************************************************
// ParticleChannel
// *************************************************************************************************

ParticleChannel::ParticleChannel()
{
	reset();
}


void ParticleChannel::reset()
{
	startMin = 0; startMax = 0; endRate = 0;
}


bool ParticleChannel::parse( XMLNode &node )
{
	if( node.getAttribute( "startMin" ) == 0x0 ) return false;
	startMin = (float)atof( node.getAttribute( "startMin" ) );

	if( node.getAttribute( "startMax" ) != 0x0 )
		startMax = (float)atof( node.getAttribute( "startMax" ) );
	else
		startMax = startMin;
	
	endRate = (float)atof( node.getAttribute( "endRate", "1" ) );

	return true;
}


// *************************************************************************************************
// ParticleEffectResource
// *************************************************************************************************

ParticleEffectResource::ParticleEffectResource( const string &name, int flags ) :
	Resource( ResourceTypes::ParticleEffect, name, flags )
{
	initDefault();	
}


ParticleEffectResource::~ParticleEffectResource()
{
	release();
}


void ParticleEffectResource::initDefault()
{
	_lifeMin = 0; _lifeMax = 0;
	_moveVel.reset();
	_rotVel.reset();
	_drag.reset();
	_size.reset();
	_colR.reset();
	_colG.reset();
	_colB.reset();
	_colA.reset();
}


void ParticleEffectResource::release()
{
}


bool ParticleEffectResource::raiseError( const string &msg, int line )
{
	// Reset
	release();
	initDefault();

	if( line < 0 )
		Modules::log().writeError( "ParticleEffect resource '%s': %s", _name.c_str(), msg.c_str() );
	else
		Modules::log().writeError( "ParticleEffect resource '%s' in line %i: %s", _name.c_str(), line, msg.c_str() );
	
	return false;
}


bool ParticleEffectResource::load( const char *data, int size )
{
	if( !Resource::load( data, size ) ) return false;

	XMLDoc doc;
	doc.parseBuffer( data, size );
	if( doc.hasError() )
		return raiseError( "XML parsing error" );

	XMLNode rootNode = doc.getRootNode();
	if( strcmp( rootNode.getName(), "ParticleEffect" ) != 0 )
		return raiseError( "Not a particle effect resource file" );
	
	if( rootNode.getAttribute( "lifeMin" ) == 0x0 ) return raiseError( "Missing ParticleConfig attribute 'lifeMin'" );
	if( rootNode.getAttribute( "lifeMax" ) == 0x0 ) return raiseError( "Missing ParticleConfig attribute 'lifeMax'" );

	_lifeMin = (float)atof( rootNode.getAttribute( "lifeMin" ) );
	_lifeMax = (float)atof( rootNode.getAttribute( "lifeMax" ) );

	XMLNode node1 = rootNode.getFirstChild( "ChannelOverLife" );
	while( !node1.isEmpty() )
	{
		if( node1.getAttribute( "channel" ) != 0x0 )
		{
			if( _stricmp( node1.getAttribute( "channel" ), "moveVel" ) == 0 ) _moveVel.parse( node1 );
			else if( _stricmp( node1.getAttribute( "channel" ), "rotVel" ) == 0 ) _rotVel.parse( node1 );
			else if( _stricmp( node1.getAttribute( "channel" ), "drag" ) == 0 ) _drag.parse( node1 );
			else if( _stricmp( node1.getAttribute( "channel" ), "size" ) == 0 ) _size.parse( node1 );
			else if( _stricmp( node1.getAttribute( "channel" ), "colR" ) == 0 ) _colR.parse( node1 );
			else if( _stricmp( node1.getAttribute( "channel" ), "colG" ) == 0 ) _colG.parse( node1 );
			else if( _stricmp( node1.getAttribute( "channel" ), "colB" ) == 0 ) _colB.parse( node1 );
			else if( _stricmp( node1.getAttribute( "channel" ), "colA" ) == 0 ) _colA.parse( node1 );
		}
		else
			return raiseError( "Missing ChannelOverLife attribute 'channel'" );
		
		node1 = node1.getNextSibling( "ChannelOverLife" );
	}
	
	return true;
}


int ParticleEffectResource::getElemCount( int elem )
{
	switch( elem )
	{
	case ParticleEffectResData::ParticleElem:
	case ParticleEffectResData::ChanMoveVelElem:
	case ParticleEffectResData::ChanRotVelElem:
	case ParticleEffectResData::ChanDragElem:
	case ParticleEffectResData::ChanSizeElem:
	case ParticleEffectResData::ChanColRElem:
	case ParticleEffectResData::ChanColGElem:
	case ParticleEffectResData::ChanColBElem:
	case ParticleEffectResData::ChanColAElem:
		return 1;
	default:
		return Resource::getElemCount( elem );
	}
}


float ParticleEffectResource::getElemParamF( int elem, int elemIdx, int param, int compIdx )
{
	ParticleChannel *chan = 0x0;
	
	switch( elem )
	{
	case ParticleEffectResData::ParticleElem:
		switch( param )
		{
		case ParticleEffectResData::PartLifeMinF:
			return _lifeMin;
		case ParticleEffectResData::PartLifeMaxF:
			return _lifeMax;
		}
		break;
	case ParticleEffectResData::ChanMoveVelElem:
		chan = &_moveVel;
		break;
	case ParticleEffectResData::ChanRotVelElem:
		chan = &_rotVel;
		break;
	case ParticleEffectResData::ChanDragElem:
		chan = &_drag;
		break;
	case ParticleEffectResData::ChanSizeElem:
		chan = &_size;
		break;
	case ParticleEffectResData::ChanColRElem:
		chan = &_colR;
		break;
	case ParticleEffectResData::ChanColGElem:
		chan = &_colG;
		break;
	case ParticleEffectResData::ChanColBElem:
		chan = &_colB;
		break;
	case ParticleEffectResData::ChanColAElem:
		chan = &_colA;
		break;
	}

	if( chan != 0x0 )
	{
		switch( param )
		{
		case ParticleEffectResData::ChanStartMinF:
			return chan->startMin;
		case ParticleEffectResData::ChanStartMaxF:
			return chan->startMax;
		case ParticleEffectResData::ChanEndRateF:
			return chan->endRate;
		}
	}

	return Resource::getElemParamF( elem, elemIdx, param, compIdx );
}


void ParticleEffectResource::setElemParamF( int elem, int elemIdx, int param, int compIdx, float value )
{
	ParticleChannel *chan = 0x0;
	
	switch( elem )
	{
	case ParticleEffectResData::ParticleElem:
		switch( param )
		{
		case ParticleEffectResData::PartLifeMinF:
			_lifeMin = value;
			return;
		case ParticleEffectResData::PartLifeMaxF:
			_lifeMax = value;
			return;
		}
		break;
	case ParticleEffectResData::ChanMoveVelElem:
		chan = &_moveVel;
		break;
	case ParticleEffectResData::ChanRotVelElem:
		chan = &_rotVel;
		break;
	case ParticleEffectResData::ChanDragElem:
		chan = &_drag;
		break;
	case ParticleEffectResData::ChanSizeElem:
		chan = &_size;
		break;
	case ParticleEffectResData::ChanColRElem:
		chan = &_colR;
		break;
	case ParticleEffectResData::ChanColGElem:
		chan = &_colG;
		break;
	case ParticleEffectResData::ChanColBElem:
		chan = &_colB;
		break;
	case ParticleEffectResData::ChanColAElem:
		chan = &_colA;
		break;
	}

	if( chan != 0x0 )
	{
		switch( param )
		{
		case ParticleEffectResData::ChanStartMinF:
			chan->startMin = value;
			return;
		case ParticleEffectResData::ChanStartMaxF:
			chan->startMax = value;
			return;
		case ParticleEffectResData::ChanEndRateF:
			chan->endRate = value;
			return;
		}
	}

	Resource::setElemParamF( elem, elemIdx, param, compIdx, value );
}


// *************************************************************************************************
// EmitterNode
// *************************************************************************************************

EmitterNode::EmitterNode( const EmitterNodeTpl &emitterTpl ) :
	SceneNode( emitterTpl )
{
	_renderable = true;
	_materialRes = emitterTpl.matRes;
	_effectRes = emitterTpl.effectRes;
	_particleCount = emitterTpl.maxParticleCount;
	_respawnCount = emitterTpl.respawnCount;
	_delay = emitterTpl.delay;
	_emissionRate = emitterTpl.emissionRate;
	_spreadAngle = emitterTpl.spreadAngle;
	_force = Vec3f( emitterTpl.fx, emitterTpl.fy, emitterTpl.fz );

	_emissionAccum = 0;
	_prevAbsTrans = _absTrans;

	_particles = 0x0;
	_parPositions = 0x0;
	_parSizesANDRotations = 0x0;
	_parColors = 0x0;

	setMaxParticleCount( _particleCount );
}


EmitterNode::~EmitterNode()
{
	for( uint32 i = 0; i < _occQueries.size(); ++i )
	{
		if( _occQueries[i] != 0 )
			gRDI->destroyQuery( _occQueries[i] );
	}
	
	delete[] _particles;
	delete[] _parPositions;
	delete[] _parSizesANDRotations;
	delete[] _parColors;
}


SceneNodeTpl *EmitterNode::parsingFunc( map< string, string > &attribs )
{
	bool result = true;
	
	map< string, string >::iterator itr;
	EmitterNodeTpl *emitterTpl = new EmitterNodeTpl( "", 0x0, 0x0, 0, 0 );

	itr = attribs.find( "material" );
	if( itr != attribs.end() )
	{
		uint32 res = Modules::resMan().addResource( ResourceTypes::Material, itr->second, 0, false );
		if( res != 0 )
			emitterTpl->matRes = (MaterialResource *)Modules::resMan().resolveResHandle( res );
	}
	else result = false;
	itr = attribs.find( "particleEffect" );
	if( itr != attribs.end() )
	{
		uint32 res = Modules::resMan().addResource( ResourceTypes::ParticleEffect, itr->second, 0, false );
		if( res != 0 )
			emitterTpl->effectRes = (ParticleEffectResource *)Modules::resMan().resolveResHandle( res );
	}
	else result = false;
	itr = attribs.find( "maxCount" );
	if( itr != attribs.end() ) emitterTpl->maxParticleCount = atoi( itr->second.c_str() );
	else result = false;
	itr = attribs.find( "respawnCount" );
	if( itr != attribs.end() ) emitterTpl->respawnCount = atoi( itr->second.c_str() );
	else result = false;
	itr = attribs.find( "delay" );
	if( itr != attribs.end() ) emitterTpl->delay = (float)atof( itr->second.c_str() );
	itr = attribs.find( "emissionRate" );
	if( itr != attribs.end() ) emitterTpl->emissionRate = (float)atof( itr->second.c_str() );
	itr = attribs.find( "spreadAngle" );
	if( itr != attribs.end() ) emitterTpl->spreadAngle = (float)atof( itr->second.c_str() );
	itr = attribs.find( "forceX" );
	if( itr != attribs.end() ) emitterTpl->fx = (float)atof( itr->second.c_str() );
	itr = attribs.find( "forceY" );
	if( itr != attribs.end() ) emitterTpl->fy = (float)atof( itr->second.c_str() );
	itr = attribs.find( "forceZ" );
	if( itr != attribs.end() ) emitterTpl->fz = (float)atof( itr->second.c_str() );
	
	if( !result )
	{
		delete emitterTpl; emitterTpl = 0x0;
	}
	
	return emitterTpl;
}


SceneNode *EmitterNode::factoryFunc( const SceneNodeTpl &nodeTpl )
{
	if( nodeTpl.type != SceneNodeTypes::Emitter ) return 0x0;

	return new EmitterNode( *(EmitterNodeTpl *)&nodeTpl );
}


void EmitterNode::setMaxParticleCount( uint32 maxParticleCount )
{
	// Delete particles
	delete[] _particles; _particles = 0x0;
	delete[] _parPositions; _parPositions = 0x0;
	delete[] _parSizesANDRotations; _parSizesANDRotations = 0x0;
	delete[] _parColors; _parColors = 0x0;
	
	// Initialize particles
	_particleCount = maxParticleCount;
	_particles = new ParticleData[_particleCount];
	_parPositions = new float[_particleCount * 3];
	_parSizesANDRotations = new float[_particleCount * 2];
	_parColors = new float[_particleCount * 4];
	for( uint32 i = 0; i < _particleCount; ++i )
	{
		_particles[i].life = 0;
		_particles[i].respawnCounter = 0;
		
		_parPositions[i*3+0] = 0.0f;
		_parPositions[i*3+1] = 0.0f;
		_parPositions[i*3+2] = 0.0f;
		_parSizesANDRotations[i*2+0] = 0.0f;
		_parSizesANDRotations[i*2+1] = 0.0f;
		_parColors[i*4+0] = 0.0f;
		_parColors[i*4+1] = 0.0f;
		_parColors[i*4+2] = 0.0f;
		_parColors[i*4+3] = 0.0f;
	}
}


int EmitterNode::getParamI( int param )
{
	switch( param )
	{
	case EmitterNodeParams::MatResI:
		if( _materialRes != 0x0 ) return _materialRes->getHandle();
		else return 0;
	case EmitterNodeParams::PartEffResI:
		if( _effectRes != 0x0 ) return _effectRes->getHandle();
		else return 0;
	case EmitterNodeParams::MaxCountI:
		return (int)_particleCount;
	case EmitterNodeParams::RespawnCountI:
		return _respawnCount;
	}

	return SceneNode::getParamI( param );
}


void EmitterNode::setParamI( int param, int value )
{
	Resource *res;
	
	switch( param )
	{
	case EmitterNodeParams::MatResI:
		res = Modules::resMan().resolveResHandle( value );
		if( res != 0x0 && res->getType() == ResourceTypes::Material )
			_materialRes = (MaterialResource *)res;
		else
			Modules::setError( "Invalid handle in h3dSetNodeParamI for H3DEmitter::MatResI" );
		return;
	case EmitterNodeParams::PartEffResI:
		res = Modules::resMan().resolveResHandle( value );
		if( res != 0x0 && res->getType() == ResourceTypes::ParticleEffect )
			_effectRes = (ParticleEffectResource *)res;
		else
			Modules::setError( "Invalid handle in h3dSetNodeParamI for H3DLight::PartEffResI" );
		return;
	case EmitterNodeParams::MaxCountI:
		setMaxParticleCount( (uint32)value );
		return;
	case EmitterNodeParams::RespawnCountI:
		_respawnCount = value;
		return;
	}

	SceneNode::setParamI( param, value );
}


float EmitterNode::getParamF( int param, int compIdx )
{
	switch( param )
	{
	case EmitterNodeParams::DelayF:
		return _delay;
	case EmitterNodeParams::EmissionRateF:
		return _emissionRate;
	case EmitterNodeParams::SpreadAngleF:
		return _spreadAngle;
	case EmitterNodeParams::ForceF3:
		if( (unsigned)compIdx < 3 ) return _force[compIdx];
		break;
	}

	return SceneNode::getParamF( param, compIdx );
}


void EmitterNode::setParamF( int param, int compIdx, float value )
{
	switch( param )
	{
	case EmitterNodeParams::DelayF:
		_delay = value;
		return;
	case EmitterNodeParams::EmissionRateF:
		_emissionRate = value;
		return;
	case EmitterNodeParams::SpreadAngleF:
		_spreadAngle = value;
		return;
	case EmitterNodeParams::ForceF3:
		if( (unsigned)compIdx < 3 )
		{
			_force[compIdx] = value;
			return;
		}
		break;
	}

	SceneNode::setParamF( param, compIdx, value );
}


float randomF( float min, float max )
{
	return (rand() / (float)RAND_MAX) * (max - min) + min;
}


void EmitterNode::update( float timeDelta )
{
	if( timeDelta == 0 || _effectRes == 0x0 ) return;
	
	// Update absolute transformation
	updateTree();
	
	Timer *timer = Modules::stats().getTimer( EngineStats::ParticleSimTime );
	if( Modules::config().gatherTimeStats ) timer->setEnabled( true );
	
	Vec3f bBMin( Math::MaxFloat, Math::MaxFloat, Math::MaxFloat );
	Vec3f bBMax( -Math::MaxFloat, -Math::MaxFloat, -Math::MaxFloat );
	
	if( _delay <= 0 )
		_emissionAccum += _emissionRate * timeDelta;
	else
		_delay -= timeDelta;

	Vec3f motionVec = _absTrans.getTrans() - _prevAbsTrans.getTrans();

	// Check how many particles will be spawned
	float spawnCount = 0;
	for( uint32 i = 0; i < _particleCount; ++i )
	{
		ParticleData &p = _particles[i];
		if( p.life <= 0 && ((int)p.respawnCounter < _respawnCount || _respawnCount < 0) )
		{
			spawnCount += 1.0f;
			if( spawnCount >= _emissionAccum ) break;
		}
	}

	// Particles are distributed along emitter's motion vector to avoid blobs when fps is low
	float curStep = 0, stepWidth = 0.5f;
	if( spawnCount > 2.0f ) stepWidth = motionVec.length() / spawnCount;
	
	for( uint32 i = 0; i < _particleCount; ++i )
	{
		ParticleData &p = _particles[i];
		
		// Create particle
		if( p.life <= 0 && ((int)p.respawnCounter < _respawnCount || _respawnCount < 0) )
		{
			if( _emissionAccum >= 1.0f )
			{
				// Respawn
				p.maxLife = randomF( _effectRes->_lifeMin, _effectRes->_lifeMax );
				p.life = p.maxLife;
				float angle = degToRad( _spreadAngle / 2 );
				Matrix4f m = _absTrans;
				m.c[3][0] = 0; m.c[3][1] = 0; m.c[3][2] = 0;
				m.rotate( randomF( -angle, angle ), randomF( -angle, angle ), randomF( -angle, angle ) );
				p.dir = (m * Vec3f( 0, 0, -1 )).normalized();
				p.dragVec = motionVec / timeDelta;
				++p.respawnCounter;

				// Generate start values
				p.moveVel0 = randomF( _effectRes->_moveVel.startMin, _effectRes->_moveVel.startMax );
				p.rotVel0 = randomF( _effectRes->_rotVel.startMin, _effectRes->_rotVel.startMax );
				p.drag0 = randomF( _effectRes->_drag.startMin, _effectRes->_drag.startMax );
				p.size0 = randomF( _effectRes->_size.startMin, _effectRes->_size.startMax );
				p.r0 = randomF( _effectRes->_colR.startMin, _effectRes->_colR.startMax );
				p.g0 = randomF( _effectRes->_colG.startMin, _effectRes->_colG.startMax );
				p.b0 = randomF( _effectRes->_colB.startMin, _effectRes->_colB.startMax );
				p.a0 = randomF( _effectRes->_colA.startMin, _effectRes->_colA.startMax );
				
				// Update arrays
				_parPositions[i * 3 + 0] = _absTrans.c[3][0] - motionVec.x * curStep;
				_parPositions[i * 3 + 1] = _absTrans.c[3][1] - motionVec.y * curStep;
				_parPositions[i * 3 + 2] = _absTrans.c[3][2] - motionVec.z * curStep;
				_parSizesANDRotations[i * 2 + 0] = p.size0;
				_parSizesANDRotations[i * 2 + 1] = randomF( 0, 360 );
				_parColors[i * 4 + 0] = p.r0;
				_parColors[i * 4 + 1] = p.g0;
				_parColors[i * 4 + 2] = p.b0;
				_parColors[i * 4 + 3] = p.a0;

				// Update emitter
				_emissionAccum -= 1.f;
				if( _emissionAccum < 0 ) _emissionAccum = 0.f;

				curStep += stepWidth;
			}
		}
		
		// Update particle
		if( p.life > 0 )
		{
			// Interpolate data
			float fac = 1.0f - (p.life / p.maxLife);
			
			float moveVel = p.moveVel0 * (1.0f + (_effectRes->_moveVel.endRate - 1.0f) * fac);
			float rotVel = p.rotVel0 * (1.0f + (_effectRes->_rotVel.endRate - 1.0f) * fac);
			float drag = p.drag0 * (1.0f + (_effectRes->_drag.endRate - 1.0f) * fac);
			_parSizesANDRotations[i * 2 + 0] = p.size0 * (1.0f + (_effectRes->_size.endRate - 1.0f) * fac);
			_parSizesANDRotations[i * 2 + 0] *= 2;  // Keep compatibility with old particle vertex shader
			_parColors[i * 4 + 0] = p.r0 * (1.0f + (_effectRes->_colR.endRate - 1.0f) * fac);
			_parColors[i * 4 + 1] = p.g0 * (1.0f + (_effectRes->_colG.endRate - 1.0f) * fac);
			_parColors[i * 4 + 2] = p.b0 * (1.0f + (_effectRes->_colB.endRate - 1.0f) * fac);
			_parColors[i * 4 + 3] = p.a0 * (1.0f + (_effectRes->_colA.endRate - 1.0f) * fac);

			// Update particle position and rotation
			_parPositions[i * 3 + 0] += (p.dir.x * moveVel + p.dragVec.x * drag + _force.x) * timeDelta;
			_parPositions[i * 3 + 1] += (p.dir.y * moveVel + p.dragVec.y * drag + _force.y) * timeDelta;
			_parPositions[i * 3 + 2] += (p.dir.z * moveVel + p.dragVec.z * drag + _force.z) * timeDelta;
			_parSizesANDRotations[i * 2 + 1] += degToRad( rotVel ) * timeDelta;

			// Decrease lifetime
			p.life -= timeDelta;
			
			// Check if particle is dying
			if( p.life <= 0 )
			{
				_parSizesANDRotations[i * 2 + 0] = 0.0f;
			}
		}

		// Update bounding box
		Vec3f vertPos( _parPositions[i*3+0], _parPositions[i*3+1], _parPositions[i*3+2] );
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
	
	_bBox.min = bBMin;
	_bBox.max = bBMax;

	_prevAbsTrans = _absTrans;

	timer->setEnabled( false );
}


bool EmitterNode::hasFinished()
{
	if( _respawnCount < 0 ) return false;

	for( uint32 i = 0; i < _particleCount; ++i )
	{	
		if( _particles[i].life > 0 || (int)_particles[i].respawnCounter < _respawnCount )
		{
			return false;
		}
	}
	
	return true;
}

}  // namespace
