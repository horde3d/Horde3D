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

#include "daeMain.h"
#include "converter.h"
#include "utPlatform.h"
#include <algorithm>

#ifdef PLATFORM_WIN
#   define WIN32_LEAN_AND_MEAN 1
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#	include <windows.h>
#	include <direct.h>
#else
#   include <unistd.h>
#   define _chdir chdir
#	include <sys/stat.h>
#	include <dirent.h>
#endif

using namespace std;


struct AssetTypes
{
	enum List
	{
		Unknown,
		Model,
		Animation
	};
};


void createAssetList( const string &basePath, const string &assetPath, vector< string > &assetList )
{
	vector< string >  directories;
	vector< string >  files;
	
// Find all files and subdirectories in current search path
#ifdef PLATFORM_WIN
	string searchString( basePath + assetPath + "*" );
	
	WIN32_FIND_DATA fdat;
	HANDLE h = FindFirstFile( searchString.c_str(), &fdat );
	if( h == INVALID_HANDLE_VALUE ) return;
	do
	{
		// Ignore hidden files
		if( strcmp( fdat.cFileName, "." ) == 0 || strcmp( fdat.cFileName, ".." ) == 0 ||
		    fdat.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN )
		{	
			continue;
		}
		
		if( fdat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			directories.push_back( fdat.cFileName );
		else
			files.push_back( fdat.cFileName );
	} while( FindNextFile( h, &fdat ) );
#else
	dirent *dirEnt;
	struct stat fileStat;
	string finalPath = basePath + assetPath;
	DIR *dir = opendir( finalPath.c_str() );
    if( dir == 0x0 ) return;

	while( (dirEnt = readdir( dir )) != 0x0 )
	{
		if( dirEnt->d_name[0] == '.' ) continue;  // Ignore hidden files

		lstat( (finalPath + dirEnt->d_name).c_str(), &fileStat );
		
		if( S_ISDIR( fileStat.st_mode ) )
			directories.push_back( dirEnt->d_name );
		else if( S_ISREG( fileStat.st_mode ) )
			files.push_back( dirEnt->d_name );
	}

    closedir( dir );

    sort( directories.begin(), directories.end() );
	sort( files.begin(), files.end() );
#endif

	// Check file extensions
	for( unsigned int i = 0; i < files.size(); ++i )
	{
		size_t len = files[i].length();

		if( len > 4 && _stricmp( files[i].c_str() + (len-4), ".dae" ) == 0 )
		{
			assetList.push_back( assetPath + files[i] );
		}
	}
	
	// Search in subdirectories
	for( unsigned int i = 0; i < directories.size(); ++i )
	{
		createAssetList( basePath, assetPath + directories[i] + "/", assetList );
	}
}


void printHelp()
{
	log( "Usage:" );
	log( "ColladaConv input [optional arguments]" );
	log( "" );
	log( "input             asset file or directory to be processed" );
	log( "-type model|anim  asset type to be processed (default: model)" );
	log( "-base path        base path where the repository root is located" );
	log( "-dest path        existing destination path where output is written" );
	log( "-noGeoOpt         disable geometry optimization" );
	log( "-overwriteMats    force update of existing materials" );
	log( "-addModelName     adds model name before material name" );
	log( "-lodDist1 dist    distance for LOD1" );
	log( "-lodDist2 dist    distance for LOD2" );
	log( "-lodDist3 dist    distance for LOD3" );
	log( "-lodDist4 dist    distance for LOD4" );
}


int main( int argc, char **argv )
{
	log( "Horde3D ColladaConv - 1.0.0 Beta5" );
	log( "" );
	
	if( argc < 2 )
	{
		printHelp();
		return 1;
	}
	
	// =============================================================================================
	// Parse arguments
	// =============================================================================================

	vector< string > assetList;
	string input = argv[1], basePath = "./", outPath = "./";
	AssetTypes::List assetType = AssetTypes::Model;
	bool geoOpt = true, overwriteMats = false, addModelName = false;
	float lodDists[4] = { 10, 20, 40, 80 };
	string modelName = "";	

	// Make sure that first argument ist not an option
	if( argv[1][0] == '-' )
	{
		log( "Missing input file or dir; use . for repository root" );
		return 1;
	}
	
	// Check optional arguments
	for( int i = 2; i < argc; ++i )
	{
		std::string arg = argv[i];
		arg.erase(remove_if(arg.begin(), arg.end(), ::isspace), arg.end());


		if( _stricmp( arg.c_str(), "-type" ) == 0 && argc > i + 1 )
		{
			if( _stricmp( argv[++i], "model" ) == 0 ) assetType = AssetTypes::Model;
			else if( _stricmp( argv[i], "anim" ) == 0 ) assetType = AssetTypes::Animation;
			else assetType = AssetTypes::Unknown;
		}
		else if( _stricmp( arg.c_str(), "-base" ) == 0 && argc > i + 1 )
		{
			basePath = cleanPath( argv[++i] ) + "/";
		}
		else if( _stricmp( arg.c_str(), "-dest" ) == 0 && argc > i + 1 )
		{
			outPath = cleanPath( argv[++i] ) + "/";
		}
		else if( _stricmp( arg.c_str(), "-noGeoOpt" ) == 0 )
		{
			geoOpt = false;
		}
		else if( _stricmp( arg.c_str(), "-overwriteMats" ) == 0 )
		{
			overwriteMats = true;
		}
		else if( (_stricmp( arg.c_str(), "-lodDist1" ) == 0 || _stricmp( arg.c_str(), "-lodDist2" ) == 0 ||
		          _stricmp( arg.c_str(), "-lodDist3" ) == 0 || _stricmp( arg.c_str(), "-lodDist4" ) == 0) && argc > i + 1 )
		{
			int index = 0;
			if( _stricmp( arg.c_str(), "-lodDist2" ) == 0 ) index = 1;
			else if( _stricmp( arg.c_str(), "-lodDist3" ) == 0 ) index = 2;
			else if( _stricmp( arg.c_str(), "-lodDist4" ) == 0 ) index = 3;
			
			lodDists[index] = (float)atof( argv[++i] );
		}
		else if( _stricmp( arg.c_str(), "-addModelName" ) == 0 )
		{
			addModelName = true;
		}
		else
		{
			log( std::string( "Invalid arguments: '" ) + arg.c_str() + std::string( "'" ) );
			printHelp();
			return 1;
		}
	}

	// Check whether input is single file or directory and create asset input list
	if( input.length() > 4 && _stricmp( input.c_str() + (input.length() - 4), ".dae" ) == 0 )
	{
		// Check if it's an absolute path
		if( input[0] == '/' || input[1] == ':' || input[0] == '\\' )
		{
			size_t index = input.find_last_of( "\\/" );
			_chdir( input.substr( 0, index ).c_str() );
			input = input.substr( index + 1, input.length() - index );
		}
		assetList.push_back ( input );
	}
	else
	{
		if( input == "." ) input = "";
		else input = cleanPath( input ) + "/";
		createAssetList( basePath, input, assetList );
	}

	// =============================================================================================
	// Batch conversion
	// =============================================================================================

	if( assetType == AssetTypes::Unknown )
	{
		log( "Error: Asset type not supported by ColladaConv" );
		return 1;
	}
	else
	{
		if( assetType == AssetTypes::Model )
			log( "Processing MODELS - Path: " + input );
		else if( assetType == AssetTypes::Animation )
			log( "Processing ANIMATIONS - Path: " + input );
		log( "" );
	}
	
	string tmpStr;
	tmpStr.reserve( 256 );
	
	for( unsigned int i = 0; i < assetList.size(); ++i )
	{
		if( assetType == AssetTypes::Model || assetType == AssetTypes::Animation )
		{
			string sourcePath = basePath + assetList[i];
			string assetName = extractFileName( assetList[i], false );
			if( addModelName )	
				modelName = assetName + "_";
			string assetPath = cleanPath( extractFilePath( assetList[i] ) );
			if( !assetPath.empty() ) assetPath += "/";
			
			ColladaDocument *daeDoc = new ColladaDocument();
			
			log( "Parsing dae asset '" + assetList[i] + "'..." );
			if( !daeDoc->parseFile( sourcePath ) )
				return 1;
			
			if( assetType == AssetTypes::Model )
			{
				log( "Compiling model data..." );
				Converter *converter = new Converter( *daeDoc, outPath, lodDists );
				converter->convertModel( geoOpt );
				
				createDirectories( outPath, assetPath );
				converter->writeModel( assetPath, assetName, modelName );
				converter->writeMaterials( assetPath, modelName, overwriteMats );

				delete converter; converter = 0x0;
			}
			else if( assetType == AssetTypes::Animation )
			{	
				log( "Compiling animation data..." );
				Converter *converter = new Converter( *daeDoc, outPath, lodDists );
				converter->convertModel( false );
				
				if( converter->hasAnimation() )
				{
					createDirectories( outPath, assetPath );
					converter->writeAnimation( assetPath, assetName );
				}
				else
				{
					log( "Skipping file (does not contain animation data)" );
				}

				delete converter; converter = 0x0;
			}
			
			delete daeDoc; daeDoc = 0x0;
		}

		log( "" );
	}
	
	return 0;
}
