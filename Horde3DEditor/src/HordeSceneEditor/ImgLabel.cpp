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
#include "ImgLabel.h"

#include <QtGui/QMouseEvent>

ImgLabel::ImgLabel(QWidget* parent /*= 0*/) : QLabel(parent)
{
	setMouseTracking(true);	
	setCursor(Qt::CrossCursor);
}


ImgLabel::~ImgLabel()
{
}

void ImgLabel::mouseMoveEvent(QMouseEvent *event)
{
	if (!pixmap().isNull())
	{
		int x = event->pos().x() * pixmap().width() / width();
		int y = event->pos().y() * pixmap().height() / height();
		emit currentPixel(x, pixmap().height() - 1 - y);
	}
}

void ImgLabel::wheelEvent(QWheelEvent* event)
{
	if (event->pixelDelta().y() < 0)
		emit zoomOut();
	else
		emit zoomIn();
}


