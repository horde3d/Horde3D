/****************************************************************************
**
** Copyright (C) 1992-2006 Trolltech AS. All rights reserved.
**
** This file is part of the plugins of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui/QImageIOHandler>
#include <QtCore/QStringList>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#ifdef QT_NO_IMAGEFORMAT_TGA
#undef QT_NO_IMAGEFORMAT_TGA
#endif
#include "qtgahandler.h"

class QTgaPlugin : public QImageIOPlugin
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID QImageIOHandlerFactoryInterface_iid)
#endif
public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

QStringList QTgaPlugin::keys() const
{
    return QStringList() << "tga" << "tga";
}

QImageIOPlugin::Capabilities QTgaPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "tga" || format == "tga")
        return Capabilities(CanRead | CanWrite);
    if (!format.isEmpty())
        return (QImageIOPlugin::Capabilities) 0;
    if (!device->isOpen())
        return (QImageIOPlugin::Capabilities) 0;

    Capabilities cap;
    if (device->isReadable() && QTgaHandler::canRead(device))
        cap |= CanRead;
    if (device->isWritable())
        cap |= CanWrite;
    return cap;
}

QImageIOHandler *QTgaPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = (QImageIOHandler *) new QTgaHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#endif // QT_NO_IMAGEFORMATPLUGIN
