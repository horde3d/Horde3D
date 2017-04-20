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

#include "QStdOutListener.h"

#ifndef _WIN32
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#endif

QStdOutListener::QStdOutListener(const QString& pipeName /*= QString("stdoutredirection")*/, QObject* parent /*= 0*/) : QObject(parent), m_timerID(0)
{
#ifdef _WIN32
#ifdef UNICODE	
	m_pipe = CreateNamedPipe((LPCWSTR) (QString("\\\\.\\pipe\\")+pipeName).utf16(),	PIPE_ACCESS_INBOUND, 0,	1,	2048, 2048,	500, NULL); 	
#else
	m_pipe = CreateNamedPipe(qPrintable(QString("\\\\.\\pipe\\")+pipeName),	PIPE_ACCESS_INBOUND, 0,	1,	2048, 2048,	500, NULL); 	
#endif
	if( m_pipe == INVALID_HANDLE_VALUE )
	{
		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
			);
		// Process any inserts in lpMsgBuf.
		// ...
		// Display the string.
		MessageBox( NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK | MB_ICONINFORMATION );
		// Free the buffer.
		LocalFree( lpMsgBuf );
	}
#if _MSC_VER > 1300
	FILE* file = 0;
	freopen_s(&file, qPrintable(QString("\\\\.\\pipe\\")+pipeName), "w", stdout); 
#else
	freopen(qPrintable(QString("\\\\.\\pipe\\")+pipeName), "w", stdout); 
#endif
	// This call is necessary and will FAIL. This is NORMAL. GetLastError() should
	// return ERROR_PIPE_CONNECTED
	ConnectNamedPipe(m_pipe, NULL);
#else
    int pipePair[2];
    m_oldPipe = dup(STDOUT_FILENO);
    pipe(pipePair);
    dup2(pipePair[1], STDOUT_FILENO);
    close (pipePair[1]);
    long flags = fcntl(pipePair[0], F_GETFL); 
    flags |= O_NONBLOCK; 
    fcntl(pipePair[0], F_SETFL, flags);
    m_pipe = pipePair[0];
#endif
}


QStdOutListener::~QStdOutListener()
{
#ifdef _WIN32
	CloseHandle(m_pipe);
#else
    dup2(m_oldPipe, STDOUT_FILENO);
    close(m_pipe);
#endif
}

void QStdOutListener::start(int msecs)
{
	if (m_timerID == 0)
		m_timerID = startTimer(msecs);
}

void QStdOutListener::stop()
{
	if (m_timerID != 0)
		killTimer(m_timerID);
	m_timerID = 0;
}

void QStdOutListener::timerEvent(QTimerEvent* /*event*/)
{
#ifdef _WIN32
	char buf[256];
	unsigned long bytesRead = 0;		
	do
    {
        fflush(stdout); // necessary, or you could wait long before seeing lines
        bytesRead = 0;		
		// look first in the pipe to prevent ReadFile from blocking
		if (PeekNamedPipe(m_pipe, buf, sizeof(buf)-1, &bytesRead, NULL, NULL) && bytesRead) 
		{
			ReadFile(m_pipe, buf, bytesRead, &bytesRead, NULL);
			buf[bytesRead] = 0;
			emit received(QString(buf));
		}
    } while (bytesRead);	  
#else
    fflush(stdout); // necessary, or you could wait long before seeing lines
    
    char c;
    QString buffer;
    while( read( m_pipe, &c, 1 ) > 0 ) 
    {
        buffer += c;
    }
    emit received(buffer);
#endif
}



