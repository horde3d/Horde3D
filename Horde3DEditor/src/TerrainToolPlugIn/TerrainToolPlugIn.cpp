// ****************************************************************************************
//
// Horde3D Scene Editor 
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// This file is part of the Horde3D Scene Editor.
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

#include "TerrainToolPlugIn.h"

#include "ConverterWizard.h"

#include <QFileDialog>
#include <QImageReader>
#include <QFileInfo>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QProgressDialog>
#include <QDir>

#include <math.h>

#include "SceneFile.h"

static int read_pbm_int(QIODevice *d)
{
    char c;
    int          val = -1;
    bool  digit;
    const int buflen = 100;
    char  buf[buflen];
    for (;;) {
        if (!d->getChar(&c))                // end of file
            break;
        digit = isdigit((uchar) c);
        if (val != -1) {
            if (digit) {
                val = 10*val + c - '0';
                continue;
            } else {
                if (c == '#')                        // comment
                    d->readLine(buf, buflen);
                break;
            }
        }
        if (digit)                                // first digit
            val = c - '0';
        else if (isspace((uchar) c))
            continue;
        else if (c == '#')
            d->readLine(buf, buflen);
        else
            break;
    }
    return val;
}

static int smooth8(const QImage* const image, int x, int y, int size)
{
	const uchar* pixel = image->scanLine(y) + x;

	const int minY = y - size < 0 ? -y : -size;
	const int maxY = y + size >= image->height() ? image->height() - y - 1 : size;

	const int minX = x - size < 0 ? -x : -size;
	const int maxX = x + size >= image->width() ? image->width() - x - 1 : size;

	const int lineStep = image->bytesPerLine();
	
	int val = 0;
	int count = 0;
	for (int i = minY; i <= maxY; ++i)
	{
		for (int j = minX; j <= maxX; ++j)
		{
			val += qRed(image->color(*(pixel + i * lineStep + j)));
			++count;
		}
	}
	return (val << 8) / count;
}

static int smooth32(const QImage* const image, int x, int y, int size)
{
	const QRgb* pixel = ((const QRgb*) image->scanLine(y)) + x;

	const int minY = y - size < 0 ? -y : -size;
	const int maxY = y + size >= image->height() ? image->height() - y - 1 : size;

	const int minX = x - size < 0 ? -x : -size;
	const int maxX = x + size >= image->width() ? image->width() - x - 1 : size;
	
	const int lineStep = image->bytesPerLine() >> 2;

	int val = 0;
	int count = 0;
	for (int i = minY; i <= maxY; ++i)
	{
		for (int j = minX; j <= maxX; ++j)
		{
			val += qRed(*(pixel + (i * lineStep) + j));
			++count;
		}
	}
	return (val << 8) / count;
}

TerrainToolPlugIn::TerrainToolPlugIn(QObject* parent /*= 0*/) : QObject(parent), m_scene(0), m_filterRadius(1)
{
	QAction* heightMapAction = new QAction(tr("Height Map Converter"), this);
	heightMapAction->setEnabled(false);
	connect(heightMapAction, SIGNAL(triggered()), this, SLOT(heightMapConverter()));
	m_actions.push_back(heightMapAction);
}


TerrainToolPlugIn::~TerrainToolPlugIn()
{
}

QString TerrainToolPlugIn::plugInName() const
{
	return "Terrain Tools PlugIn";
}

QList<QAction*> TerrainToolPlugIn::actions() const
{
	return m_actions;
}

void TerrainToolPlugIn::setSceneFile(SceneFile* scene)
{
	m_scene = scene;
	foreach(QAction* action, m_actions)
		action->setEnabled(scene != 0);
}

void TerrainToolPlugIn::heightMapConverter()
{
	QDir textureDir = QDir::current();
	QWizard wizard;
	wizard.addPage(new ConverterWizard(&wizard));
	if (wizard.exec() == QDialog::Accepted)
	{		
		m_filterRadius = wizard.field("filter_radius").toInt();
		QFileInfo heightMap( wizard.field("heightmap").toString() );		

		QImage output, heightMapSrcData;
		
		QProgressDialog dlg(tr("Converting..."), tr("Abort"), 0, 2);
		dlg.setMinimumDuration(0);
		dlg.setValue(0);
		QImageReader reader( heightMap.absoluteFilePath() );
		if ( reader.format().toUpper() != "PGM" )
		{
			QSize size = reader.size();
			if ( size.isValid() )
				dlg.setMaximum( size.height() * 3 );
			if ( !reader.read(&heightMapSrcData) )
			{
				QMessageBox::warning(0, tr("Error"), 
					tr("Error reading heightmap %1 !\n").arg(heightMap.absoluteFilePath()).arg(reader.errorString()));		
				return;
			}
			output = smoothInputImage( heightMapSrcData, &dlg );			
			dlg.setValue(1);
		}
		else  
			output = readPGM( heightMap.absoluteFilePath(), &dlg );
		dlg.setValue(dlg.maximum() * 2 / 3);
		calculateNormalMap(output, &dlg);		
		QString format = wizard.field("format").toString();
		output.save(
			textureDir.absoluteFilePath(wizard.field("target").toString()+"."+format), 
			qPrintable(format) );
		dlg.setValue(dlg.maximum());
	}
}


QImage TerrainToolPlugIn::readPGM( const QString& fileName, QProgressDialog* dlg /* = 0*/ )
{
	QFile file(fileName);
	if ( !file.open(QIODevice::ReadOnly) )
		return QImage();

	char magicBytes[2];
	file.read(magicBytes, 2);
	if ( magicBytes[0] == 'P' && magicBytes[1] == '2' )
	{
		// get image size
	    int width = read_pbm_int(&file);                        
	    int height = read_pbm_int(&file);            
		// get maximum gray level
		int max = read_pbm_int(&file);               
		if (max > 256 && max <= 0xFFFF)
		{
			if (dlg) dlg->setMaximum(height * 3);
			QRgb* p;
			int val;
			QImage output(width, height, QImage::Format_ARGB32);
			for( int y=0; y < height; ++y )
			{
				p = (QRgb*) output.scanLine(y);
				if (dlg) dlg->setValue(y * 2);
				for( int x=0; x < width; ++x, ++p)
				{
					val = read_pbm_int(&file);
					*p = qRgb(val >> 8, val % 256, 0);
				}
			}
			return output;
		}		
	}	
	return smoothInputImage(QImage(fileName), dlg);
}

QImage TerrainToolPlugIn::smoothInputImage( const QImage& image, QProgressDialog* dlg /*= 0*/ )
{
	if (image.isNull())
		return image;
	QImage output(image.size(), QImage::Format_ARGB32);

	const int height = image.height();
	const int width = image.width();
	const int start = dlg != 0 ? dlg->value() : 0;

	if (image.format() == QImage::Format_Indexed8)
	{
		for( int y = 0; y < height; ++y )
		{
			QRgb* target = (QRgb*) output.scanLine(y);
			//const uchar* center = image.scanLine(y);
			//const uchar* top   = y > 0          ? image.scanLine(y - 1) : center;
			//const uchar* bot   = y < height - 1 ? image.scanLine(y + 1) : center;
			//const uchar* left  = 0;
			//const uchar* right = 0;
			//const uchar* topleft  = 0;
			//const uchar* topright = 0;
			//const uchar* botleft  = 0;
			//const uchar* botright = 0;			
			if (dlg) dlg->setValue( start + y );
			for( int x = 0; x < width; ++x, ++target/*, ++center, ++top, ++bot, ++left, ++right, ++botleft, ++botright, ++topleft, ++topright*/)
			{
				//botleft     = x > 0    ? bot - 1 : bot;
				//left        = x > 0    ? center - 1 : center;
				//topleft     = x > 0    ? top - 1 : top;
				//botright    = x < width - 1 ? bot + 1 : bot;
				//right       = x < width - 1 ? center + 1 : center;
				//topright    = x < width - 1 ? top + 1 : top;
				//
				//const int val =
				//	( ( qRed(image.color(*topleft)) + qRed(image.color(*top)) + qRed(image.color(*topright)) +
				//	    qRed(image.color(*left)) + qRed(image.color(*center)) + qRed(image.color(*right)) +
				//	    qRed(image.color(*botleft)) + qRed(image.color(*bot)) + qRed(image.color(*botright)) ) << 8 ) / 9;				

				const int val = smooth8(&image, x, y, m_filterRadius);
				*target = qRgb( val >> 8, val % 256, 0);
			}
		}
	}
	else
	{
		for( int y = 0; y < height; ++y )
		{
			QRgb* target = (QRgb*) output.scanLine(y);
			//const QRgb* center = (const QRgb*) image.scanLine(y);
			//const QRgb* top   = y > 0          ? (QRgb*) image.scanLine(y - 1) : center;
			//const QRgb* bot   = y < height - 1 ? (QRgb*) image.scanLine(y + 1) : center;
			//const QRgb* left  = 0;
			//const QRgb* right = 0;
			//const QRgb* topleft  = 0;
			//const QRgb* topright = 0;
			//const QRgb* botleft  = 0;
			//const QRgb* botright = 0;
			if (dlg) dlg->setValue( start + y );
			for( int x = 0; x < width; ++x, ++target/*, ++center, ++top, ++bot, ++left, ++right, ++botleft, ++botright, ++topleft, ++topright*/)
			{
				//botleft     = x > 0    ? bot - 1 : bot;
				//left        = x > 0    ? center - 1 : center;
				//topleft     = x > 0    ? top - 1 : top;
				//botright    = x < width - 1 ? bot + 1 : bot;
				//right       = x < width - 1 ? center + 1 : center;
				//topright    = x < width - 1 ? top + 1 : top;
				
				//const int val = 
				//    ((qRed(*topleft) + qRed(*top) + qRed(*topright) +
				//	  qRed(*left) + qRed(*center) + qRed(*right) +
				//	  qRed(*botleft) + qRed(*bot) + qRed(*botright)) << 8 ) / 9;
				const int val = smooth32(&image, x, y, m_filterRadius);

				*target = qRgb( val >> 8, val % 256, 0 );
			}
		}
	}
	return output;
}

void TerrainToolPlugIn::calculateNormalMap(QImage& image, QProgressDialog* dlg /*= 0*/)
{
	const int height = image.height();
	const int width = image.width();

	const float normalY2 = ( 2.0f / width ) * ( 2.0f / width );

	const float scale = 1.0f/65535.0f;

	const int start = dlg != 0 ? dlg->value() : 0;

	for( int y = 0; y < height; ++y )
	{
		QRgb* center = (QRgb*) image.scanLine(y);
		QRgb* top   = y > 0          ? (QRgb*) image.scanLine(y - 1) : center;
		QRgb* bot   = y < height - 1 ? (QRgb*) image.scanLine(y + 1) : center;
		QRgb* left  = 0;
		QRgb* right = 0;
		if (dlg) dlg->setValue( start + y );
		for( int x = 0; x < width; ++x, ++center, ++left, ++right, ++bot, ++top)
		{							
			left  = x > 0         ? center - 1 : center;
			right = x < width - 1 ? center + 1 : center;
	
			const int t = (qRed(*top) << 8) + qGreen(*top);
			const int b = (qRed(*bot) << 8) + qGreen(*bot);			
			const int l = (qRed(*left) << 8) + qGreen(*left);
			const int r = (qRed(*right) << 8) + qGreen(*right);

			float nx = ( l - r ) * scale;			
			float nz = ( b - t ) * scale;

			const float length = sqrtf( (nx * nx) + (normalY2) + (nz * nz) );

			nx /= length;
			nz /= length;

			*center = qRgba(qRed(*center), qGreen(*center), int( (nx + 1.0f) * 255.0f ) >> 1 , int( (nz + 1.0f) * 255.0f ) >> 1);
		}
	}
}
