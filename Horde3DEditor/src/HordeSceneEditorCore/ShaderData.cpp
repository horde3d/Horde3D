#include <QtXml/QDomDocument>

#include "ShaderData.h"

typedef unsigned int uint32;

ShaderData::ShaderData( const QByteArray& data, QObject* parent ) : QObject(parent)
{	
	m_valid = loadShader( data );
}

ShaderData::~ShaderData()
{
}

bool ShaderData::loadShader( const QByteArray& data )
{
	// Parse sections
	const char *pData = data.constData();
	while( *pData != '\0' )
	{
		if( *pData++ == '[' && *pData++ == '[' )
		{
			// Parse section name
			const char *sectionNameStart = pData;
			while( *pData != ']' && *pData != '\n' && *pData != '\r' ) ++pData;
			const char *sectionNameEnd = pData++;

			// Check for correct closing of name
			if( *pData++ != ']' ) return raiseError( "Error in section name" );
			
			// Parse content
			const char *sectionContentStart = pData;
			while( *pData != '\0' && !(*pData == '[' && *(pData+1) == '[') ) ++pData;
			const char *sectionContentEnd = pData;
			

			if( sectionNameEnd - sectionNameStart != 2 || *sectionNameStart != 'F' || *(sectionNameStart+1) != 'X' )
			{				
				// Add section as private code resource which is not managed by resource manager
				QByteArray tmpCode0( sectionNameStart, sectionNameEnd - sectionNameStart );
								
				// Code section name currently not used 
				QByteArray tmpCode1( sectionContentStart, sectionContentEnd - sectionContentStart );
				loadCode( tmpCode1 );
			}
			// Parse FX section
			else if( sectionNameEnd - sectionNameStart == 2 && *sectionNameStart == 'F' && *(sectionNameStart + 1) == 'X' )
			{
				// Add section as private code resource which is not managed by resource manager
				QByteArray tmpCode0( sectionNameStart, sectionNameEnd - sectionNameStart );
								
				// Code section name currently not used 
				QByteArray tmpCode1( sectionContentStart, sectionContentEnd - sectionContentStart );
				loadFX( tmpCode1 );
			}
		}
	}

	return true;
}

bool ShaderData::loadCode( const QByteArray& data )
{
	const char* pData = data.constData();
	bool lineComment = false, blockComment = false;		
	const char* eof = pData + data.size();	
	// Parse code
	while( pData < eof )
	{				
		// Check for begin of comment
		if( pData < eof - 1 && !lineComment && !blockComment )
		{
			if( *pData == '/' && *(pData+1) == '/' )
				lineComment = true;
			else if( *pData == '/' &&  *(pData+1) == '*' )
				blockComment = true;
		}

		// Check for end of comment
		if( lineComment && (*pData == '\n' || *pData == '\r') )
			lineComment = false;
		else if( blockComment && pData < eof - 1 && *pData == '*' && *(pData+1) == '/' )
			blockComment = false;

		// Check for includes
		if( !lineComment && !blockComment && pData < eof - 7 )
		{
			if( *pData == '#' && *(pData+1) == 'i' && *(pData+2) == 'n' && *(pData+3) == 'c' &&
			    *(pData+4) == 'l' && *(pData+5) == 'u' && *(pData+6) == 'd' && *(pData+7) == 'e' )
			{
				pData += 6;
				
				// Parse resource name
				const char *nameBegin = 0x0, *nameEnd = 0x0;
				
				while( ++pData < eof )
				{
					if( *pData == '"' )
					{
						if( nameBegin == 0x0 )
							nameBegin = pData+1;
						else
							nameEnd = pData;
					}
					else if( *pData == '\n' || *pData == '\r' ) break;
				}

				if( nameBegin != 0x0 && nameEnd != 0x0 )
				{
					QString resName = QString::fromLocal8Bit( nameBegin, nameEnd - nameBegin );																	
					m_includedFiles.append( resName );
				}
			}
		}

		// Check for flags
		if( !lineComment && !blockComment && pData < eof - 4 )
		{
			if( *pData == '_' && *(pData+1) == 'F' && *(pData+4) == '_' &&
			    *(pData+2) >= 48 && *(pData+2) <= 57 && *(pData+3) >= 48 && *(pData+3) <= 57 )
			{
				ShaderFlag flag;
				// Set flag
				flag.Flag = (*(pData+2) - 48) * 10 + (*(pData+3) - 48);
							
				pData += 5;
				// Ignore rest of name
				while( pData < eof && *pData != ' ' && *pData != '\t' && *pData != '\n' && *pData != '\r' )
					flag.Name.append( *pData++ );
				if( !m_flags.contains( flag ) )
					m_flags.append( flag );
			}
		}
		
		++pData;
	}
	return true;
}


bool ShaderData::loadFX( const QByteArray& data )
{
	const char* pData = data.constData();
	bool lineComment = false, blockComment = false;		
	const char* eof = pData + data.size();	
	// Parse code
	while( pData < eof )
	{				
		// Check for begin of comment
		if( pData < eof - 1 && !lineComment && !blockComment )
		{
			if( *pData == '/' && *(pData+1) == '/' )
				lineComment = true;
			else if( *pData == '/' &&  *(pData+1) == '*' )
				blockComment = true;
		}

		// Check for end of comment
		if( lineComment && (*pData == '\n' || *pData == '\r') )
			lineComment = false;
		else if( blockComment && pData < eof - 1 && *pData == '*' && *(pData+1) == '/' )
			blockComment = false;		

		// Check for referenced textures
		if( !lineComment && !blockComment && pData < eof - 7 )
		{
			// Code section name currently not used 
			if( *pData == 'T' && *(pData+1) == 'e' && *(pData+2) == 'x' &&
				*(pData+3) =='t' && *(pData+4) == 'u' && *(pData+5) == 'r' && *(pData+6) == 'e' )
			{				
				QString file;				
				while( pData < eof && *pData != '\n' && *pData != '\r' )
				{
					if( *pData++ == '"' )
						break;
				}
				while( pData < eof && *pData != '\n' && *pData != '\r' && *pData != '"' )
					file.append( *pData++ );
				if( !m_textureFiles.contains( file ) )
					m_textureFiles.append( file );
				
			}
		}
		++pData;
	}
	return true;
}

bool ShaderData::raiseError( const QString& error )
{
	m_lastError = error;
	return false;
}
