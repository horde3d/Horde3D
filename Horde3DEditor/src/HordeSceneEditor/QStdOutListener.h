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

#ifndef QSTDOUTLISTENER_H_
#define QSTDOUTLISTENER_H_

#include <QtCore/QObject>

#ifdef _WIN32
#include <windows.h>
#endif

class QTextStream;

/**
 * Class to redirect the standard output to a QWidget window
 */
class QStdOutListener : public QObject
{
	Q_OBJECT
public:
	QStdOutListener(const QString& pipeName = QString("stdoutredirection"), QObject* parent = 0);
	virtual ~QStdOutListener();

	void start(int msecs);
	void stop();

signals:
	void received(const QString& output);

protected:
	void timerEvent(QTimerEvent* event);

private:
	bool			m_running;
	int				m_timerID;
#ifdef _WIN32
	HANDLE			m_pipe; 
#else
    int             m_pipe;
    int             m_oldPipe;
#endif


};
#endif
