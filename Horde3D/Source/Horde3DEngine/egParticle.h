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

#ifndef _egParticle_H_
#define _egParticle_H_

#include "egPrerequisites.h"
#include "utMath.h"
#include "egMaterial.h"
#include "egScene.h"


namespace Horde3D {

class XMLNode;


// =================================================================================================
// ParticleEffect Resource
// =================================================================================================

struct ParticleEffectResData
{
	enum List
	{
		ParticleElem = 800,
		ChanMoveVelElem,
		ChanRotVelElem,
		ChanSizeElem,
		ChanColRElem,
		ChanColGElem,
		ChanColBElem,
		ChanColAElem,
		PartLifeMinF,
		PartLifeMaxF,
		ChanStartMinF,
		ChanStartMaxF,
		ChanEndRateF,
		ChanDragElem    // TODO: Move behind ChanRotVelElem
	};
};

// =================================================================================================

struct ParticleChannel
{
	float  startMin, startMax;
	float  endRate;

	ParticleChannel();
	void reset();
	bool parse( XMLNode &node );
};

// =================================================================================================

class ParticleEffectResource : public Resource
{
public:
	static Resource *factoryFunc( const std::string &name, int flags )
		{ return new ParticleEffectResource( name, flags ); }
	
	ParticleEffectResource( const std::string &name, int flags );
	~ParticleEffectResource();
	
	void initDefault();
	void release();
	bool load( const char *data, int size );

	int getElemCount( int elem );
	float getElemParamF( int elem, int elemIdx, int param, int compIdx );
	void setElemParamF( int elem, int elemIdx, int param, int compIdx, float value );

private:
	bool raiseError( const std::string &msg, int line = -1 );

private:
	float            _lifeMin, _lifeMax;
	ParticleChannel  _moveVel, _rotVel, _drag;
	ParticleChannel  _size;
	ParticleChannel  _colR, _colG, _colB, _colA;

	friend class EmitterNode;
};

typedef SmartResPtr< ParticleEffectResource > PParticleEffectResource;


// =================================================================================================
// Emitter Node
// =================================================================================================

struct EmitterNodeParams
{
	enum List
	{
		MatResI = 700,
		PartEffResI,
		MaxCountI,
		RespawnCountI,
		DelayF,
		EmissionRateF,
		SpreadAngleF,
		ForceF3
	};
};

// =================================================================================================

struct EmitterNodeTpl : public SceneNodeTpl
{
	PMaterialResource        matRes;
	PParticleEffectResource  effectRes;
	uint32                   maxParticleCount;
	int                      respawnCount;
	float                    delay, emissionRate, spreadAngle;
	float                    fx, fy, fz;

	EmitterNodeTpl( const std::string &name, MaterialResource *materialRes,
		ParticleEffectResource *effectRes, uint32 maxParticleCount, int respawnCount) :
		SceneNodeTpl( SceneNodeTypes::Emitter, name ),
		matRes( materialRes ), effectRes( effectRes ), maxParticleCount( maxParticleCount ),
		respawnCount( respawnCount ), delay( 0 ), emissionRate( 0 ), spreadAngle( 0 ),
		fx( 0 ), fy( 0 ), fz( 0 )
	{
	}
};

// =================================================================================================

struct ParticleData
{
	float   life, maxLife;
	Vec3f   dir, dragVec;
	uint32  respawnCounter;

	// Start values
	float  moveVel0, rotVel0, drag0;
	float  size0;
	float  r0, g0, b0, a0;
};

// =================================================================================================

class EmitterNode : public SceneNode
{
public:
	static SceneNodeTpl *parsingFunc( std::map< std::string, std::string > &attribs );
	static SceneNode *factoryFunc( const SceneNodeTpl &nodeTpl );

	~EmitterNode();

	int getParamI( int param );
	void setParamI( int param, int value );
	float getParamF( int param, int compIdx );
	void setParamF( int param, int compIdx, float value );

	void update( float timeDelta );
	bool hasFinished();

protected:
	EmitterNode( const EmitterNodeTpl &emitterTpl );
	void setMaxParticleCount( uint32 maxParticleCount );

protected:
	// Emitter data
	float                    _emissionAccum;
	Matrix4f                 _prevAbsTrans;
	
	// Emitter params
	PMaterialResource        _materialRes;
	PParticleEffectResource  _effectRes;
	uint32                   _particleCount;
	int                      _respawnCount;
	float                    _delay, _emissionRate, _spreadAngle;
	Vec3f                    _force;

	// Particle data
	ParticleData             *_particles;
	float                    *_parPositions;
	float                    *_parSizesANDRotations;
	float                    *_parColors;

	std::vector< uint32 >    _occQueries;
	std::vector< uint32 >    _lastVisited;

	friend class SceneManager;
	friend class Renderer;
};

}
#endif // _egParticle_H_
