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

#ifndef _daeLibAnimations_H_
#define _daeLibAnimations_H_

#include "utXML.h"
#include "daeCommon.h"
#include "utils.h"
#include <string>
#include <vector>
#include <algorithm>

using namespace Horde3D;


struct DaeSampler
{
	std::string  id;
	DaeSource    *input;  // Time values
	DaeSource    *output;  // Transformation data
};


struct DaeChannel
{
	DaeSampler   *source;
	std::string  nodeId;  // Target node
	std::string  transSid;  // Target transformation channel
	int          transValuesIndex;  // Index in values of node transformation (-1 for no index)
};


struct DaeAnimation
{
	std::string                    id;
	std::vector< DaeSource >       sources;
	std::vector< DaeSampler >      samplers;
	std::vector< DaeChannel >      channels;
	std::vector< DaeAnimation * >  children;


	~DaeAnimation()
	{
		for( unsigned int i = 0; i < children.size(); ++i ) delete children[i];
	}


	DaeSource *findSource( const std::string &id )
	{
		if( id == "" ) return 0x0;

		for( unsigned int i = 0; i < sources.size(); ++i )
		{
			if( sources[i].id == id ) return &sources[i];
		}

		return 0x0;
	}

	
	DaeSampler *findAnimForTarget( const std::string &nodeId, const std::string &transSid, int *transValuesIndex )
	{
		if( nodeId == "" || transSid == "" ) return 0x0;
		
		for( unsigned int i = 0; i < channels.size(); ++i )
		{
			if( channels[i].nodeId == nodeId && channels[i].transSid == transSid )
			{
				if( transValuesIndex != 0x0 ) *transValuesIndex = channels[i].transValuesIndex;
				return channels[i].source;
			}
		}
		
		// Parse children
		for( unsigned int i = 0; i < children.size(); ++i )
		{
			DaeSampler *sampler = children[i]->findAnimForTarget( nodeId, transSid, transValuesIndex );
			if( sampler != 0x0 ) return sampler;
		}

		return 0x0;
	}


	bool parse( const XMLNode &animNode, unsigned int &maxFrameCount, float &maxAnimTime )
	{
		id = animNode.getAttribute( "id", "" );
		
		// Sources
		XMLNode node1 = animNode.getFirstChild( "source" );
		while( !node1.isEmpty() )
		{
			sources.push_back( DaeSource() );
			if( !sources.back().parse( node1 ) ) sources.pop_back();

			node1 = node1.getNextSibling( "source" );
		}
		
		// Samplers
		node1 = animNode.getFirstChild( "sampler" );
		while( !node1.isEmpty() )
		{
			samplers.push_back( DaeSampler() );
			DaeSampler &sampler = samplers.back();

			sampler.id = node1.getAttribute( "id" );
			
			XMLNode node2 = node1.getFirstChild( "input" );
			while( !node2.isEmpty() )
			{
				if( strcmp( node2.getAttribute( "semantic", "" ), "INPUT" ) == 0x0 )
				{
					std::string id = node2.getAttribute( "source", "" );
					removeGate( id );
					sampler.input = findSource( id );
				}
				else if( strcmp( node2.getAttribute( "semantic", "" ), "OUTPUT" ) == 0x0 )
				{
					std::string id = node2.getAttribute( "source", "" );
					removeGate( id );
					sampler.output = findSource( id );
				}
				
				node2 = node2.getNextSibling( "input" );
			}
			
			if( sampler.input == 0x0 || sampler.output == 0x0 )
				samplers.pop_back();
			else
			{
				unsigned int frameCount = (unsigned int)sampler.input->floatArray.size();
				maxFrameCount = std::max( maxFrameCount, frameCount );
				
				for( unsigned int i = 0; i < frameCount; ++i )
					maxAnimTime = std::max( maxAnimTime, sampler.input->floatArray[i] );
			}

			node1 = node1.getNextSibling( "sampler" );
		}

		// Channels
		node1 = animNode.getFirstChild( "channel" );
		while( !node1.isEmpty() )
		{
			channels.push_back( DaeChannel() );
			DaeChannel &channel = channels.back();
			channel.transValuesIndex = -1;

			// Parse target
			std::string s = node1.getAttribute( "target", "" );
			size_t pos = s.find( "/" );
			if( pos != std::string::npos && pos != s.length() - 1 )
			{
				channel.nodeId = s.substr( 0, pos );
				channel.transSid = s.substr( pos + 1, s.length() - pos );				
				if( channel.transSid.find( ".X" ) != std::string::npos )
				{
					channel.transValuesIndex = 0;
					channel.transSid = channel.transSid.substr( 0, channel.transSid.find(".") );
				}
				else if( channel.transSid.find( ".Y" ) != std::string::npos )
				{
					channel.transValuesIndex = 1;
					channel.transSid = channel.transSid.substr( 0, channel.transSid.find(".") );
				}
				else if( channel.transSid.find( ".Z" ) != std::string::npos )
				{
					channel.transValuesIndex = 2;
					channel.transSid = channel.transSid.substr( 0, channel.transSid.find(".") );
				}
				else if( channel.transSid.find( ".ANGLE" ) != std::string::npos )
				{
					channel.transValuesIndex = 3;
					channel.transSid = channel.transSid.substr( 0, channel.transSid.find(".") );
				}
				else if( channel.transSid.find( '(' ) != std::string::npos )
				{
					size_t index1 = channel.transSid.find( '(' );
					size_t index2 = channel.transSid.find( '(', index1 + 1 );
					if( index2 == std::string::npos )  // We got a vector index
					{
						channel.transValuesIndex = atoi( 
							channel.transSid.substr( index1 + 1, channel.transSid.find(')', index1) - ( index1 + 1) ).c_str() );
					}
					else  // We got an array index
					{
						int x = atoi( channel.transSid.substr( 
							index1 + 1, channel.transSid.find( ')', index1 ) - (index1 + 1) ).c_str() );
						int y = atoi( channel.transSid.substr( 
							index2 + 1, channel.transSid.find( ')', index2 ) - (index2 + 1) ).c_str() );
						// TODO: Is this the correct access order? Maybe collada defines it transposed
						channel.transValuesIndex = y * 4 + x; 
					}
					channel.transSid = channel.transSid.substr( 0, index1 );					
				}
			}
			
			// Find source
			s = node1.getAttribute( "source", "" );
			removeGate( s );
			if( s != "" )
			{
				for( unsigned int i = 0; i < samplers.size(); ++i )
				{
					if( samplers[i].id == s )
					{
						channel.source = &samplers[i];
						break;
					}
				}
			}

			if( channel.nodeId == "" || channel.transSid == "" || channel.source == 0x0 )
			{
				log( "Warning: Missing channel attributes or sampler not found" );
				channels.pop_back();
			}

			node1 = node1.getNextSibling( "channel" );
		}
		
		// Parse children
		node1 = animNode.getFirstChild( "animation" );
		while( !node1.isEmpty() )
		{
			DaeAnimation *anim = new DaeAnimation();
			if( anim->parse( node1, maxFrameCount, maxAnimTime ) ) children.push_back( anim );
			else delete anim;

			node1 = node1.getNextSibling( "animation" );
		}
		
		return true;
	}
};


struct DaeLibAnimations
{
	std::vector< DaeAnimation * >  animations;
	unsigned int                   maxFrameCount;
	float                          maxAnimTime;
	

	~DaeLibAnimations()
	{
		for( unsigned int i = 0; i < animations.size(); ++i ) delete animations[i];
	}
	

	DaeSampler *findAnimForTarget( const std::string &nodeId, std::string const &transSid, int *index )
	{
		for( unsigned int i = 0; i < animations.size(); ++i )
		{
			DaeSampler *sampler = animations[i]->findAnimForTarget( nodeId, transSid, index );
			if( sampler != 0x0 ) return sampler;
		}

		return 0x0;
	}

	
	bool parse( const XMLNode &rootNode )
	{
		maxFrameCount = 0;
		
		XMLNode node1 = rootNode.getFirstChild( "library_animations" );
		if( node1.isEmpty() ) return true;

		XMLNode node2 = node1.getFirstChild( "animation" );
		while( !node2.isEmpty() )
		{
			DaeAnimation *anim = new DaeAnimation();
			if( anim->parse( node2, maxFrameCount, maxAnimTime ) ) animations.push_back( anim );
			else delete anim;

			node2 = node2.getNextSibling( "animation" );
		}
		
		return true;
	}
};

#endif // _daeLibAnimations_H_
