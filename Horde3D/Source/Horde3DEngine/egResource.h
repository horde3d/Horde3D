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

#ifndef _egResource_H_
#define _egResource_H_

#include "egPrerequisites.h"
#include "utMath.h"
#include <string>
#include <vector>
#include <map>


namespace Horde3D {

// =================================================================================================
// Resource
// =================================================================================================

struct ResourceTypes
{
	enum List
	{
		Undefined = 0,
		SceneGraph,
		Geometry,
		Animation,
		Material,
		Code,
		Shader,
		Texture,
		ParticleEffect,
		Pipeline
	};
};

struct ResourceFlags
{
	enum Flags
	{
		NoQuery = 1,
		NoTexCompression = 2,
		NoTexMipmaps = 4,
		TexCubemap = 8,
		TexDynamic = 16,
		TexRenderable = 32,
		TexSRGB = 64
	};
};

// =================================================================================================

class Resource
{
public:
	Resource( int type, const std::string &name, int flags );
	virtual ~Resource();
	virtual Resource *clone();  // TODO: Implement this for all resource types
	
	virtual void initDefault();
	virtual void release();
	virtual bool load( const char *data, int size );
	void unload();
	
	int findElem( int elem, int param, const char *value );
	virtual int getElemCount( int elem );
	virtual int getElemParamI( int elem, int elemIdx, int param );
	virtual void setElemParamI( int elem, int elemIdx, int param, int value );
	virtual float getElemParamF( int elem, int elemIdx, int param, int compIdx );
	virtual void setElemParamF( int elem, int elemIdx, int param, int compIdx, float value );
	virtual const char *getElemParamStr( int elem, int elemIdx, int param );
	virtual void setElemParamStr( int elem, int elemIdx, int param, const char *value );
	virtual void *mapStream( int elem, int elemIdx, int stream, bool read, bool write );
	virtual void unmapStream();

	int &getType() { return _type; }
	int getFlags() { return _flags; }
	const std::string &getName() { return _name; }
	ResHandle getHandle() { return _handle; }
	bool isLoaded() { return _loaded; }
	void addRef() { ++_refCount; }
	void subRef() { --_refCount; }

protected:
	int                  _type;
	std::string          _name;
	ResHandle            _handle;
	int                  _flags;
	
	uint32               _refCount;  // Number of other objects referencing this resource
	uint32               _userRefCount;  // Number of handles created by user

	bool                 _loaded;
	bool                 _noQuery;

	friend class ResourceManager;
};

// =================================================================================================

template< class T > class SmartResPtr
{
public:
	SmartResPtr( T *ptr = 0x0 ) : _ptr( ptr ) { addRef(); }
	SmartResPtr( const SmartResPtr &smp ) : _ptr( smp._ptr ) { addRef(); }
	~SmartResPtr() { subRef(); }

	T &operator*() const { return *_ptr; }
    T *operator->() const { return _ptr; }
	operator T*() const { return _ptr; }
    operator const T*() const { return _ptr; }
	operator bool() const { return _ptr != 0x0; }
	T *getPtr() { return _ptr; }
	
	SmartResPtr &operator=( const SmartResPtr &smp ) { return *this = smp._ptr; }
	SmartResPtr &operator=( T *ptr )
	{
		subRef(); _ptr = ptr; addRef();
		return *this;
	}

private:
	void addRef() { if( _ptr != 0x0 ) _ptr->addRef(); }
	void subRef() { if( _ptr != 0x0 ) _ptr->subRef(); }

private:
    T  *_ptr;
};

typedef SmartResPtr< Resource > PResource;


// =================================================================================================
// Resource Manager
// =================================================================================================

typedef void (*ResTypeInitializationFunc)();
typedef void (*ResTypeReleaseFunc)();
typedef Resource *(*ResTypeFactoryFunc)( const std::string &name, int flags );

struct ResourceRegEntry
{
	std::string                typeString;
	ResTypeInitializationFunc  initializationFunc;  // Called when type is registered
	ResTypeReleaseFunc         releaseFunc;  // Called when type is unregistered
	ResTypeFactoryFunc         factoryFunc;  // Factory to create resource object
};

// =================================================================================================

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void registerResType( int resType, const std::string &typeString, ResTypeInitializationFunc inf,
	                      ResTypeReleaseFunc rf, ResTypeFactoryFunc ff );
	
	Resource *getNextResource( int type, ResHandle start );
	Resource *findResource( int type, const std::string &name );
	ResHandle addResource( int type, const std::string &name, int flags, bool userCall );
	ResHandle addNonExistingResource( Resource &resource, bool userCall );
	ResHandle cloneResource( Resource &sourceRes, const std::string &name );
	int removeResource( Resource &resource, bool userCall );
	void clear();
	ResHandle queryUnloadedResource( int index );
	void releaseUnusedResources();

	Resource *resolveResHandle( ResHandle handle )
		{ return (handle != 0 && (unsigned)(handle - 1) < _resources.size()) ? _resources[handle - 1] : 0x0; }

	std::vector < Resource * > &getResources() { return _resources; }

protected:
	ResHandle addResource( Resource &res );

protected:
	std::vector < Resource * >         _resources;
	std::map< int, ResourceRegEntry >  _registry;  // Registry of resource types
};

}
#endif // _egResource_H_
