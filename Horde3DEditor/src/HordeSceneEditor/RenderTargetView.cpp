// ****************************************************************************************
//
// Horde3D Scene Editor 
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// This file is part of the Horde3D Scene Editor.
// It uses parts of the Image Viewer Example from Trolltech
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

#include "RenderTargetView.h"
#include "ImgLabel.h"

#include "HordeSceneEditor.h"
#include "OpenGLWidget.h"

#include <QtCore/QEvent>
#include <QtGui/QBitmap>
#include <QFileDialog>
#include <QtGui/QImageWriter>
#include <QMessageBox>
#include <QtGui/QImage>
#include <QScrollArea>
#include <QScrollBar>
#include <QtCore/QVector>
#include <QApplication>
#include <QtGui/QClipboard>
#include <QCloseEvent>
#include <QMouseEvent>

#include <Horde3D.h>

RenderTargetView::RenderTargetView(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QWidget(parent, flags),
m_pipelineID(0), m_scaleFactor(1.0), m_imgData(0), m_imgDepth(0)
{
	setFocusPolicy(Qt::ClickFocus);
	setupUi(this);
	m_imageLabel = new ImgLabel(this);
	//m_imageLabel->setBackgroundRole(QPalette::Base);
	m_imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	m_imageLabel->setScaledContents(true);
	connect(m_imageLabel, SIGNAL(currentPixel(int, int)), this, SLOT(showPixel(int, int)));

	m_scrollArea = new QScrollArea(m_frame);
	m_scrollArea->setBackgroundRole(QPalette::Dark);
	m_scrollArea->setWidget(m_imageLabel);

	m_frame->setLayout(new QHBoxLayout(m_frame));
	m_frame->layout()->setContentsMargins(1,1,1,1);
	m_frame->layout()->addWidget(m_scrollArea);
	m_frame->addAction(m_actionZoomIn);
	m_frame->addAction(m_actionZoomOut);
	m_frame->addAction(m_actionNormalSize);

	addAction(m_actionClose);

	connect(m_actionZoomIn, SIGNAL(triggered()), this, SLOT(zoomIn()));
	connect(m_actionZoomOut, SIGNAL(triggered()), this, SLOT(zoomOut()));
	connect(m_imageLabel, SIGNAL(zoomIn()), this, SLOT(zoomIn()));
	connect(m_imageLabel, SIGNAL(zoomOut()), this, SLOT(zoomOut()));
	connect(m_actionNormalSize, SIGNAL(triggered()), this, SLOT(normalSize()));
	connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(close()));
	connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(saveBuffer()));
	connect(m_buffer, SIGNAL(currentIndexChanged(int)), this, SLOT(showBuffer(int)));
	connect(m_updateButton, SIGNAL(clicked()), this, SLOT(showBuffer()));	
}


RenderTargetView::~RenderTargetView()
{
	delete[] m_imgData;
}

void RenderTargetView::closeEvent(QCloseEvent* event)
{
	emit closed(this);
	event->accept();
}

void RenderTargetView::mousePressEvent(QMouseEvent* event)
{	
	QClipboard *clipboard = QApplication::clipboard();
	if (event->button() == Qt::LeftButton)
	{
		clipboard->setText(m_imageLabel->toolTip());	
		event->accept();
	}
	else
		QWidget::mousePressEvent(event);
}

void RenderTargetView::setRenderTarget(const QDomElement &renderTarget, unsigned int pipelineID)
{
	m_buffer->clear();

	m_pipelineID = pipelineID;
	setWindowTitle(renderTarget.attribute("id"));

	m_buffer->blockSignals(true);

	int buffers = renderTarget.attribute("numColBufs", "1").toInt();
	for (int i=0; i<buffers; ++i)
		m_buffer->addItem(tr("Color Buffer %1").arg(i), i);

	bool depthBuf = 
		(renderTarget.attribute("depthBuf", "true").compare("true", Qt::CaseInsensitive) == 0 || 
		renderTarget.attribute("depthBuf", "true").compare("1")==0);
	if( depthBuf )
		m_buffer->addItem(tr("Depth Buffer"), 32);

	m_buffer->blockSignals(false);

	QString toolTip(tr("Depth Buffer: %1").arg(depthBuf ? tr("enabled") : tr("disabled")));
	toolTip+=tr("\nFormat: %1").arg(renderTarget.attribute("format", "RGBA8"));
	bool bilinear = 
		renderTarget.attribute("bilinear", "false").compare("true", Qt::CaseInsensitive) == 0 || 
		renderTarget.attribute("bilinear", "0").compare("1")==0;
	toolTip+=tr("\nBilinear: %1").arg(bilinear ? tr("enabled") : tr("disabled"));
	int width = renderTarget.attribute("width","0").toInt();
	int height = renderTarget.attribute("height","0").toInt();
	float scale = renderTarget.attribute("scale","1.0").toFloat();
	if (width == 0 && height == 0)
	{
        HordeSceneEditor* editorInstance = static_cast<HordeSceneEditor*>(qApp->property("SceneEditorInstance").value<void*>());
        width = editorInstance->glContext()->width();
        height = editorInstance->glContext()->height();
	}
	width *= scale;
	height *= scale;
	toolTip+=tr("\nSize: %1x%2").arg(width).arg(height);
	toolTip+=tr("\nScale: %1").arg(scale);	
	m_buffer->setToolTip(toolTip);

	showBuffer(m_buffer->currentIndex());

	if (!m_imageLabel->pixmap().isNull())
		resize(m_imageLabel->pixmap().size() + QSize(30, 65));
}

void RenderTargetView::showBuffer(int index)
{
	// This can take some time
	setCursor(Qt::BusyCursor);
	// Release any previously allocated buffer memory
	delete[] m_imgData;
	m_imgData = 0;

        if( index == -2 )
		index = m_buffer->currentIndex();
	if (index >= 0)
	{
		int bufferIndex = m_buffer->itemData(index).toUInt();
		int width = 0, height = 0;		

		h3dGetRenderTargetData(m_pipelineID, qPrintable(windowTitle()), bufferIndex, &width, &height, &m_imgDepth, 0, 0);
		
		unsigned int bufferSize = width * height * m_imgDepth;
		m_imgData = new float[bufferSize];
		if (h3dGetRenderTargetData(m_pipelineID, qPrintable(windowTitle()), bufferIndex, &width, &height, &m_imgDepth, m_imgData, bufferSize * sizeof(float)))
		{					
			m_imageLabel->setPixmap(QPixmap::fromImage(convertImageData(m_imgData, width, height, m_imgDepth, true)));
			scaleImage(1.0);
			m_imageLabel->setMouseTracking(true);
		}
		else
			m_imageLabel->setText("Failed getting buffer");

	}
	else
		m_imageLabel->setText("No Buffer available");

	bool valid = !m_imageLabel->pixmap().isNull();
	if( valid )
		m_buttonBox->setStandardButtons(QDialogButtonBox::Close | QDialogButtonBox::Save);
	else
	{
		m_buttonBox->setStandardButtons(QDialogButtonBox::Close);
		m_imageLabel->adjustSize();
	}

	m_actionZoomIn->setEnabled(valid);
	m_actionZoomOut->setEnabled(valid);
	m_actionNormalSize->setEnabled(valid);
	unsetCursor();
}

void RenderTargetView::showPixel(int x, int y)
{
	if (m_imgData && x > 0 && y > 0)
	{
		const int width = m_imageLabel->pixmap().width();
		if (m_imgDepth == 1)
		{
			float v = m_imgData[y * width + x];
			m_imageLabel->setToolTip(tr("(%1,%2)\n\n %3").arg(x).arg(y).arg(v));
		}
		else if (m_imgDepth == 4)
		{
			float r = m_imgData[y * width * 4 + x * 4];
			float g = m_imgData[y * width * 4 + x * 4 + 1];
			float b = m_imgData[y * width * 4 + x * 4 + 2];
			float a = m_imgData[y * width * 4 + x * 4 + 3];
			m_imageLabel->setToolTip(tr("(%1,%2)\n\nR: %3\nG: %4\nB: %5\nA: %6").arg(x).arg(y).arg(r).arg(g).arg(b).arg(a));
		}
	}
}

QImage RenderTargetView::convertImageData(float *data, unsigned int width, unsigned int height, unsigned int depth, bool ignoreAlpha)
{
	if (depth == 1)
	{
		QImage img(width, height, QImage::Format_Indexed8);
		QVector<QRgb> colors;
		for (int i=0; i<256; ++i)
			colors.push_back(qRgb(i,i,i));
		img.setColorTable(colors);
		// Slow implementation
		for (int y = int(height)-1; y >= 0; y--)
			for (int x = 0; x < int(width); ++x)
				img.setPixel(x, y, qMax(qMin(1.0f,*data++) * 255, 0.0f));
		return img;
	}
	else if (depth == 4)
	{
		QImage img;
		if( ignoreAlpha )
			img = QImage(width, height, QImage::Format_RGB32);
		else
			img = QImage(width, height, QImage::Format_ARGB32);
		// Slow implementation
		for (int y = int(height)-1; y >= 0; y--)
		{
			for (int x = 0; x < int(width); ++x)
			{		
				float r = qMax(qMin(1.0f,*data++), 0.0f);
				float g = qMax(qMin(1.0f,*data++), 0.0f);
				float b = qMax(qMin(1.0f,*data++), 0.0f);
				if( ignoreAlpha )
				{
					++data;
					img.setPixel(x, y, qRgb( r * 255, g * 255, b * 255));
				}
				else
				{
					float a = qMax(qMin(1.0f,*data++), 0.0f);
					img.setPixel(x, y, qRgba( r * 255, g * 255, b * 255, a * 255));
				}
			}
		}
		return img;		
	}	
	return QImage();
}

void RenderTargetView::zoomIn()
{
	scaleImage(1.25);
}

void RenderTargetView::zoomOut()
{
	scaleImage(0.8);
}

void RenderTargetView::normalSize()
{
	m_imageLabel->adjustSize();
	m_scaleFactor = 1.0;
}

void RenderTargetView::saveBuffer()
{
	QString filters;
	QList<QByteArray> formats = QImageWriter::supportedImageFormats();
	for (int i=0; i<formats.size(); ++i)
		filters += tr("%1 Image (*.%2);;").arg(QString(formats.at(i)).toUpper()).arg(QString(formats.at(i)));			
	filters += tr("All files (*.*)");

	QString fileName = QFileDialog::getSaveFileName(this, tr("Select filename"), QDir::currentPath(), filters);
	if (!fileName.isEmpty())
	{
		QImage image(convertImageData(m_imgData, m_imageLabel->pixmap().width(), m_imageLabel->pixmap().height(), m_imgDepth, false));
		QImageWriter writer(fileName);
		if (!writer.write(image))
			QMessageBox::warning(this, tr("Error"), writer.errorString());
	}
}

void RenderTargetView::scaleImage(double factor)
{
	Q_ASSERT(!m_imageLabel->pixmap().isNull());
	m_scaleFactor *= factor;
	m_imageLabel->resize(m_scaleFactor * m_imageLabel->pixmap().size());

	adjustScrollBar(m_scrollArea->horizontalScrollBar(), factor);
	adjustScrollBar(m_scrollArea->verticalScrollBar(), factor);

	m_actionZoomIn->setEnabled(m_scaleFactor < 3.0);
	m_actionZoomOut->setEnabled(m_scaleFactor > 0.333);
}

void RenderTargetView::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
	scrollBar->setValue(int(factor * scrollBar->value()	+ ((factor - 1) * scrollBar->pageStep()/2)));
}
