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

#include <QApplication>
#include <QtGui/QSurfaceFormat>
#include <QSplashScreen>
#include "HordeSceneEditor.h"

#include "utPlatform.h"

#include "cstdlib"

#ifdef MEMORYCHECK
#include <vld.h>
#endif

int main(int argc, char** argv)
{
    #ifdef __linux__
    // Force X11 backend to fix Wayland keyboard focus issues
    // The '1' parameter ensures it overwrites existing environment variables
    setenv("QT_QPA_PLATFORM", "xcb", 1);
    // 2. Prevent Wayland from mismatching your scale factor coordinates
    setenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1", 1);
    setenv("QT_SCALE_FACTOR_ROUNDING_POLICY", "PassThrough", 1);
    #endif

	// From Qt docs:
	// Calling QSurfaceFormat::setDefaultFormat() before constructing the QApplication instance is mandatory on some platforms (for
	// example, macOS) when an OpenGL core profile context is requested. This is to ensure that resource sharing between contexts
	// stays functional as all internal contexts are created using the correct version and profile.
	// Set default GL Format
	QSurfaceFormat format;
#ifdef PLATFORM_MAC
	format.setVersion(4, 1); 
	format.setProfile( QSurfaceFormat::CoreProfile ); 
#else
	format.setProfile( QSurfaceFormat::CompatibilityProfile ); 
#endif

	format.setDepthBufferSize( 32 );
	format.setSwapBehavior( QSurfaceFormat::DoubleBuffer );
//	format.setAlphaBufferSize( 8 );
	QSurfaceFormat::setDefaultFormat( format );

	QApplication app( argc, argv );
	app.addLibraryPath(app.applicationDirPath()+QDir::separator()+"plugins");
	app.setApplicationName("HordeSceneEditor");
    QPixmap pixmap(":/Icons/Resources/logo.png");
    QSplashScreen splash(pixmap);
    splash.show();
    app.processEvents();
	// create a new instance of HordeSceneEditor (will be automatically deleted on close)
    HordeSceneEditor* mainWindow = new HordeSceneEditor();
	mainWindow->show();
	splash.finish(mainWindow);
	if (argc > 1 && !QString(argv[1]).isEmpty())
		mainWindow->openScene(argv[1]);
	// Enters the main event loop and waits until exit() is called
	// or the main widget is destroyed, and Returns the value that
	// was set via to exit() (which is 0 if exit() is called via quit()).
	return app.exec();
}
