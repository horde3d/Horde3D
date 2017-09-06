// ****************************************************************************************
//
// Horde3D Scene Editor 
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// This file is part of the Horde3D Scene Editor.
// The code is based on the psd loader code by Thatcher Ulrich and the Qt tga loader plugin
// by Dominik Seichter and Ignacio Castaño 
//
// The Horde3D Scene Editor is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 3 of the License 
//
// The Horde3D Scene Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ****************************************************************************************

#include "qpsdhandler.h"

#include <assert.h>

#include <QtGui/QImage>
#include <QtCore/QDataStream>
#include <QtCore/QVariant>

typedef quint32 uint;
typedef quint16 ushort;
typedef quint8 uchar;

namespace { // Private.

	struct PsdHeader {
		qint32 identifier;
		qint16 version;
		char   reserved[6];
		qint16 channelCount;
		qint32 height;
		qint32 width;
		qint16 depth;
		qint16 colormode;
		qint16 compression;

		enum { SIZE = 26 }; // const static size 26
	};

	typedef struct _ResolutionInfo
	{
		qint32 hRes;              /* Fixed-point number: pixels per inch */
		qint16 hResUnit;          /* 1=pixels per inch, 2=pixels per centimeter */
		qint16 WidthUnit;         /* 1=in, 2=cm, 3=pt, 4=picas, 5=columns */
		qint32 vRes;              /* Fixed-point number: pixels per inch */
		qint16 vResUnit;          /* 1=pixels per inch, 2=pixels per centimeter */
		qint16 HeightUnit;        /* 1=in, 2=cm, 3=pt, 4=picas, 5=columns */
	} RESOLUTIONINFO;

	typedef struct _DisplayInfo
	{
		qint16  ColorSpace;
		qint16  Color[4];         
		qint16  Opacity;          /* 0-100 */
		qint8  Kind;             /* 0=selected, 1=protected */
		qint8  Padding;          /* Always zero */
	} DISPLAYINFO;

	static QDataStream & operator>> ( QDataStream & s, PsdHeader & head )
	{
		s >> head.identifier;
		s >> head.version;
		s.readRawData(head.reserved, 6);
		s >> head.channelCount;
		s >> head.height;
		s >> head.width;
		s >> head.depth;
		s >> head.colormode;
		quint32 skip;
		// Skip the Mode Data.  (It's the palette for indexed color; other info for other modes.)
		s >> skip;
		s.skipRawData(skip);
		// Skip the image resources.  (resolution, pen tool paths, etc)
		s >> skip;
		char* data = new char[skip];
		s.readRawData(data, skip);
		delete[] data;
		//s.skipRawData(skip);
		// Skip the reserved data.
		s >> skip;
		s.skipRawData(skip);
		s >> head.compression;
		return s;
	}

	static QString IsSupported( const PsdHeader & head )
	{
		if( head.identifier != 0x38425053 )
			return "Not a psd image! Header corrupt";
		if( head.version != 1 )
			return QObject::tr("Wrong version %1").arg(head.version);
		if( head.channelCount < 0 || head.channelCount > 16 ) 
			return QObject::tr("Unsupported number of channels in PSD image (%1)").arg(head.channelCount);
		if( head.depth != 8 )
			return QObject::tr("PSD bit depth is not 8 (%1)").arg(head.depth);

		// Make sure the color mode is RGB.
		// Valid options are:
		//   0: Bitmap
		//   1: Grayscale
		//   2: Indexed color
		//   3: RGB color
		//   4: CMYK color
		//   7: Multichannel
		//   8: Duotone
		//   9: Lab color
		if( head.colormode != 3 )
			return QObject::tr("Color format is not RGB");

		// Find out if the data is compressed.
		// Known values:
		//   0: no compression
		//   1: RLE compressed
		if( head.compression > 1 )
			return QObject::tr("Unkown compression format");
		
		return QString();
	}

	static QImage LoadPSD( QDataStream & s, const PsdHeader & psd )
	{
		int channel;
		quint32 i, count;
		quint8 len;
		quint32 pixelCount = psd.width * psd.height;
		unsigned char* out = new unsigned char[4 * psd.width * psd.height];

		if( psd.compression )
		{
			// RLE as used by .PSD and .TIFF
			// Loop until you get the number of unpacked bytes you are expecting:
			//     Read the next source byte into n.
			//     If n is between 0 and 127 inclusive, copy the next n+1 bytes literally.
			//     Else if n is between -127 and -1 inclusive, copy the next byte -n+1 times.
			//     Else if n is 128, noop.
			// Endloop

			// The RLE-compressed data is preceeded by a 2-byte data count for each row in the data,
			// which we're going to just skip.
			s.skipRawData(psd.height * psd.channelCount * 2);
			// Read the RLE data by channel.
			for (channel = 0; channel < 4; channel++) 
			{
				quint8 *p = out + channel;

				if (channel >= psd.channelCount) 
				{
					quint8 alpha = (channel == 3 ? 255 : 0);
					// Fill this channel with default data.
					for (i = 0; i < pixelCount; ++i) 
					{
						*p = alpha;
						p += 4;
					}
				} 
				else 
				{
					// Read the RLE data.
					count = 0;
					while (count < pixelCount)
					{
						s.readRawData( (char*) &len, 1);
						if (len == 128) 
						{
							// No-op.
						} 
						else if (len < 128) 
						{
							// Copy next len+1 bytes literally.
							++len;
							count += len;
							while (len--) 
							{
								s.readRawData( (char*) p, 1);
								p += 4;								
							}
						} 
						else if (len > 128) 
						{
							quint8	val;
							// Next -len+1 bytes in the dest are replicated from next source byte.
							// (Interpret len as a negative 8-bit int.)
							len ^= 0x0FF;
							len += 2;
							s.readRawData((char*) &val, 1);
							count += len;
							while (len--) 
							{
								*p = val;
								p += 4;								
							}
						}
					}
				}
			}

		} else {
			// We're at the raw image data.  It's each channel in order (Red, Green, Blue, Alpha, ...)
			// where each channel consists of an 8-bit value for each pixel in the image.

			// Read the data by channel.
			for (channel = 0; channel < 4; channel++) 
			{
				quint8 *p;
				p = out + channel;
				if (channel > psd.channelCount) 
				{
					quint8 alpha = (channel == 3 ? 255 : 0);
					// Fill this channel with default data.
					for (i = 0; i < pixelCount; i++) *p = alpha, p += 4;
				} 
				else
				{
					// Read the data.
					count = 0;
					for (i = 0; i < pixelCount; i++)
						s.readRawData( (char*) p, 1), p += 4;
				}
			}
		}		

		// Create image.
		QImage img( out, psd.width, psd.height, QImage::Format_ARGB32 );
		delete[] out;
		return img.rgbSwapped();
	}

} // namespace


QPsdHandler::QPsdHandler()
{
}

bool QPsdHandler::canRead() const
{
	return canRead(device());
}

bool QPsdHandler::read(QImage *outImage)
{
	//kDebug("Loading PSD file!");

	QDataStream s( device() );
	s.setByteOrder( QDataStream::BigEndian );


	// Read image header.
	PsdHeader psd;
	s >> psd;

	// Check supported file types.
	QString error = IsSupported(psd);
	if ( !error.isEmpty() )
	{
		qDebug("This PSD file is not supported. %s", qPrintable(error));
		return false;
	}

	QImage img = LoadPSD(s, psd);

	if( img.isNull() ) {
		qDebug("Error loading PSD file.");
		return false;
	}

	*outImage = img;
	return true;
}

bool QPsdHandler::write(const QImage &image)
{
	QDataStream s( device() );
	s.setByteOrder( QDataStream::BigEndian );

	const QImage& img = image;
	const bool hasAlpha = (img.format() == QImage::Format_ARGB32);
	
	// write header
	s << quint32( 0x38425053 ); // '8BPS' identifier
	s << quint16( 1 ); // version is always 1
	s.writeRawData("\0\0\0\0\0\0", 6); // Reserved, must be zeroed 
	switch( img.format() )
	{
	case QImage::Format_ARGB32:
		s << qint16( 4 );
		break;
	case QImage::Format_RGB32:
		s << qint16( 3 );
		break;
	default:
		return false;
	}
	s << qint32( img.height() );
	s << qint32( img.width() );
	s << qint16( 8 ); // QImage supports only 8 bit or 1 bit, but we don't support monochrome images 
	s << qint16( 3 ); // We support only RGB mode
	s << qint32( 0 ); // No Mode Data // TODO support gray scale
	
	s << qint32( 12 + sizeof(RESOLUTIONINFO) /*+ 8 + sizeof(DISPLAYINFO)*/ ); // Default image resource data
	
	s.writeRawData("8BIM", 4);
	s << qint16( 0x03ed ); // Resolution data
	s.writeRawData("\0\0", 2); // Even-length Pascal-format string, 2 bytes or longer
	s << qint32( sizeof(RESOLUTIONINFO) ); // Length of resource data following, in bytes 

	RESOLUTIONINFO resInfo;
	resInfo.hRes = 72;
	resInfo.hResUnit = 1;
	resInfo.WidthUnit = 2;
	resInfo.vRes = 72;
	resInfo.vResUnit = 1;
	resInfo.HeightUnit = 2;	
	s.writeRawData( (char*) &resInfo, sizeof(RESOLUTIONINFO) );

	//s.writeRawData("8BIM", 4);
	//s << qint16( 0x03ef );
	//s.writeRawData("\0\0", 2);
	//s << qint32( sizeof(DISPLAYINFO) );

	//DISPLAYINFO disInfo;
	//disInfo.ColorSpace = 	

	s << qint32( 0 ); // No reserved data
	s << qint16( 0 ); // no compression to keep it simple

	s << quint16( img.width() ); // width
	s << quint16( img.height() ); // height
	s << quint8( hasAlpha ? 32 : 24 ); // depth (24 bit RGB + 8 bit alpha)
	s << quint8( hasAlpha ? 0x24 : 0x20 ); // top left image (0x20) + 8 bit alpha (0x4)
	
	// slow but simple
	for( int y = 0; y < img.height(); ++y )
	{
		QRgb* line = (QRgb*) img.scanLine(y);
		for( int x = 0; x < img.width(); ++x, ++line ) 
		{
			s << (quint8) qBlue( *line);
		}
	}
	for( int y = 0; y < img.height(); ++y )
	{
		QRgb* line = (QRgb*) img.scanLine(y);
		for( int x = 0; x < img.width(); ++x, ++line ) 
		{
			s << (quint8) qRed( *line);
		}
	}
	for( int y = 0; y < img.height(); ++y )
	{
		QRgb* line = (QRgb*) img.scanLine(y);
		for( int x = 0; x < img.width(); ++x, ++line ) 
		{
			s << (quint8) qGreen( *line);
		}
	}
	if (hasAlpha)
	{
		for( int y = 0; y < img.height(); ++y )
		{
			QRgb* line = (QRgb*) img.scanLine(y);
			for( int x = 0; x < img.width(); ++x, ++line ) 
			{
				s << (quint8) qAlpha( *line);
			}
		}
	}
	return true;
}

QByteArray QPsdHandler::name() const
{
	return "psd";
}

bool QPsdHandler::canRead(QIODevice *device)
{
	if (!device) {
		qWarning("QPsdHandler::canRead() called with no device");
		return false;
	}
	qint64 oldPos = device->pos();
	char head[26];
	qint64 readBytes = device->read(head, sizeof(head));
	if (readBytes != sizeof(head)) {
		if (device->isSequential()) {
			while (readBytes > 0)
				device->ungetChar(head[readBytes-- - 1]);
		} else {
			device->seek(oldPos);
		}
		return false;
	}

	if (device->isSequential()) {
		while (readBytes > 0)
			device->ungetChar(head[readBytes-- - 1]);
	} else {
		device->seek(oldPos);
	}

	return  *( (quint32*)head ) == 0x53504238;
}


bool QPsdHandler::supportsOption(ImageOption option) const
{
    return option == Size;// || option == Parameters;
}

QVariant QPsdHandler::option(ImageOption option) const
{
    if (option == Size) {
        if (canRead() && !device()->isSequential()) {
            qint64 pos = device()->pos();
			QDataStream s( device() );
			s.setByteOrder( QDataStream::BigEndian );
			// Read image header.
			PsdHeader psd;
			s >> psd;
			device()->seek(pos);
			return QSize(psd.width, psd.height);
        }
    }
    return QVariant();
}

void QPsdHandler::setOption(ImageOption /*option*/, const QVariant &/*value*/)
{
//    else if (option == Parameters)
//        parameters = value.toByteArray();
}

