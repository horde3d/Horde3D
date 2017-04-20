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
#include <QSplashScreen>
#include "HordeSceneEditor.h"


#ifdef MEMORYCHECK
#include <vld.h>
#endif

int main(int argc, char** argv)
{
	QApplication app( argc, argv );
	app.addLibraryPath(app.applicationDirPath()+QDir::separator()+"plugins");
	app.setApplicationName("HordeSceneEditor");
    QPixmap pixmap(":/Icons/Resources/logo.png");
    QSplashScreen splash(pixmap);
    splash.show();
    app.processEvents();
	// create a new instance of HordeSceneEditor (will be automatically deleted on close)
	HordeSceneEditor* mainWindow = HordeSceneEditor::instance();	
	mainWindow->show();
	splash.finish(mainWindow);
	if (argc > 1 && !QString(argv[1]).isEmpty())
		mainWindow->openScene(argv[1]);	
	// Enters the main event loop and waits until exit() is called 
	// or the main widget is destroyed, and Returns the value that 
	// was set via to exit() (which is 0 if exit() is called via quit()). 
	return app.exec();
}
