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

#include "OpenGLWidget.h"
#include "AttachmentPlugIn.h"
#include "CustomTypes.h"
#include "QOneTimeDialog.h"
#include "QHordeSceneEditorSettings.h"

#include <QXmlTree/QXmlTreeNode.h>

#include <QListWidgetItem>
#include <QtCore/QDir>
#include <QtCore/QEvent>
#include <QtCore/QDateTime>
#include <QLabel>
#include <QtCore/QTimer>
#include <QApplication>
#include <QMessageBox>
#include <QWheelEvent>
#include <QOpenGLFunctions_3_1>
#include <QOpenGLVersionFunctionsFactory>

#include <math.h>
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <limits>

#include "im3d/im3d_h3d.h"

#include <Horde3D.h>
#include <Horde3DUtils.h>
#include <EditorLib/QRegExp.h>

static QOpenGLFunctions_3_1 *glf = nullptr;

OpenGLWidget::OpenGLWidget(QLabel* fpsLabel, QWidget* parent, Qt::WindowFlags flags) : QOpenGLWindow(NoPartialUpdate, nullptr),
    m_fpsLabel(fpsLabel), m_transformationMode(0), m_collisionCheck(false), m_navSpeed(5), m_fps(30.0), m_parentWidget(0),
    m_forward(false), m_backward(false), m_left(false), m_right(false), m_up(false), m_down(false),
    m_limitToAxis(0), m_axisVpX(1), m_axisVpY(1), m_gizmoSelection(0), m_debugInfo(0), m_gridScale(1),
    m_currentNode(0), m_attachmentPlugIn(0), m_activeCameraID(0), m_initialized(false), m_useLocalTransform( false )
{
    // setFocusPolicy(Qt::ClickFocus);
    // setAttribute(Qt::WA_DeleteOnClose);
    m_wheelTimer = new QTimer(this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    m_wheelTimer->setTimerType(Qt::PreciseTimer);
#endif
    m_wheelTimer->setSingleShot(true);
    connect(m_wheelTimer, SIGNAL(timeout()), this, SLOT(stopWheelMove()));
    loadButtonConfig();
}

OpenGLWidget::~OpenGLWidget()
{
    h3dRelease();
}

// Update the log widget
void OpenGLWidget::updateLog()
{
    int level;
    float time;
    QList<QListWidgetItem*> items;
    QString message;
    while(!(message = h3dGetMessage(&level, &time)).isEmpty())
    {
        // Create List Widget Item (removing HTML Tags from message since Horde3D still adds some tags to the warnings)
        message = message.remove( QRegularExpression( "(<(\\s*)?[a-z/]+(\\s*)?(/)?>)", QRegularExpression::CaseInsensitiveOption ) );
        QListWidgetItem* item = new QListWidgetItem(QString::number(time) + "\t" + message, 0, level);

        // QListWidgetItem* item = new QListWidgetItem(QString::number(time) + "\t" + message.remove(QRegExp("(<(\\s*)?[a-z/]+(\\s*)?(/)?>)",Qt::CaseInsensitive/*, QRegExp::RegExp2*/)), 0, level);
        items.push_back(item);
    }
    if (!items.isEmpty())
        emit logMessages(items);
}

void OpenGLWidget::loadButtonConfig()
{
    QHordeSceneEditorSettings settings(this);
    settings.beginGroup("General");
    m_cameraMoveButton = settings.value("CameraMoveButton", Qt::LeftButton).toInt();
    m_selectButton = settings.value("SelectButton", Qt::RightButton).toInt();
    m_resetSelectButton = settings.value("ResetSelectButton", Qt::MiddleButton).toInt();
    m_glClear = settings.value("glClear", true ).toBool();
    settings.endGroup();
}

void OpenGLWidget::setFullScreen(bool fullscreen, OpenGLWidget* widget /*= 0*/)
{
    /*
     *  Note that under Windows, the QGLContext belonging to a QOpenGLWidget has to be
     * recreated when the QOpenGLWidget is reparented. This is necessary due to limitations
     * on the Windows platform. This will most likely cause problems for users that have
     * subclassed and installed their own QGLContext on a QOpenGLWidget. It is possible to
     * work around this issue by putting the QOpenGLWidget inside a dummy widget and then reparenting the
     * dummy widget, instead of the QOpenGLWidget. This will side-step the issue altogether, and
     * is what we recommend for users that need this kind of functionality.
     */
    if( fullscreen)
    {
        m_parentWidget = m_containerWidget->parentWidget();
        m_containerWidget->setParent(0);
        m_containerWidget->showFullScreen();
        m_containerWidget->setFocus();
        setGeometry( QGuiApplication::primaryScreen()->geometry() );
    }
    else
    {
        m_containerWidget->showNormal();
        m_containerWidget->setParent(m_parentWidget);
        if( m_parentWidget->layout() )
            m_parentWidget->layout()->addWidget( m_containerWidget );

        m_containerWidget->setFocus();
        emit fullscreenActive(false);
    }
}

bool OpenGLWidget::underMouse()
{
    QPoint mousePos = QCursor::pos();

    return frameGeometry().contains( mousePos );
}

void OpenGLWidget::setTransformationMode(int mode)
{
    if( m_transformationMode == mode )	return;

    if (m_transformationMode != None)
        resetMode(false);

    if (m_currentNode != 0 && mode != None && !inSync())
    {
        resetMode(false);
        return;
    }

    if(!underMouse()) // Set mouse cursor if transformation mode was set by toolbar icon
    //    QCursor::setPos( frameGeometry().center() );
        QCursor::setPos(mapToGlobal(frameGeometry().center()));

    // Store mouse position when transformation mode was changed
    m_x = mapFromGlobal(QCursor::pos()).x();
    m_y = height() - mapFromGlobal(QCursor::pos()).y();

    // Store mouse position when transformation mode was changed
    const qreal scale = devicePixelRatio(); // support scaled display

    // m_x = mapFromGlobal(QCursor::pos()).x();
    // m_y = mapFromGlobal(QCursor::pos()).y();// * scale;

    m_transformationMode = mode;        // Set mode
    m_limitToAxis = 0;		// Reset Axis limitation
//    setMouseGrabEnabled( mode != None );
    // setMouseTracking(mode != None); // Enable mouse tracking if we have a transformation mode
    // setFocus(); 			// Ensure Widget focus
}

void OpenGLWidget::resetMode(bool accept)
{
    if( m_transformationMode != None )
    {
        applyChanges(accept);
        m_transformationMode = None;
        emit transformationMode(m_transformationMode);
        setCursor(Qt::ArrowCursor);
//        setMouseTracking(false);
        setMouseGrabEnabled( false );
    }
}

void OpenGLWidget::enableDebugView(bool debug)
{
   h3dSetOption(H3DOptions::DebugViewMode, debug ? 1 : 0);
}

void OpenGLWidget::setCurrentNode(QXmlTreeNode* node)
{
    m_currentNode = node;
}


void OpenGLWidget::initializeGL()
{
    if( ( m_initialized = h3dInit(H3DRenderDevice::OpenGL4) ) == false)
        QMessageBox::warning(nullptr, tr("Error"), tr("Error initializing Horde3D!"));

    glf = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_1>( context() );
    if ( !glf ) return;

    if ( !Im3d_Init( glf, this ) )
        QMessageBox::warning(nullptr, tr("Error"), tr("Failed to initialize Im3D!"));
}

void OpenGLWidget::resizeGL(int width, int height)
{
    const qreal scale = devicePixelRatio(); // support scaled display

    emit resized(width * scale, height * scale);
}


void OpenGLWidget::paintGL()
{
    static QTime initTime(QTime::currentTime());
    static int fps = 0;
    if( m_glClear )
    {
//        glClearColor( 1.0f, 1.0f, 0.0f, 1.0f );
        glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
    }

    fps++;
    int mseconds = initTime.msecsTo(QTime::currentTime());
    //qDebug("FPS: %d MSeconds %d", fps, mseconds);
    if (fps%5 == 0 && mseconds > 0)
    {
        // Calculate frame rate
        m_fps = fps * 1000.0f / mseconds;
        if (mseconds >= 2000)
        {
            // Update label (if available) every 2 seconds
            if (m_fpsLabel) m_fpsLabel->setText(QString("FPS %1").arg((int)m_fps));
            // Reset counter
            fps = 0;
            initTime = QTime::currentTime();
        }
    }

    Im3d_NewFrame( glf, m_activeCameraID );

    Im3d::Context& ctx = Im3d::GetContext();
    Im3d::AppData& ad  = Im3d::GetAppData();

    float x = 0.0f, y = 0.0f, z = 0.0f;
    if (m_fps > 0.0f)
    {
        float curVel = m_navSpeed/m_fps;
        // Fast mode?
        if (qApp->keyboardModifiers() & Qt::ShiftModifier) curVel *= 3;

        if (m_up)		y -= curVel;
        if (m_down)		y += curVel;
        if (m_forward)	z -= curVel;
        if (m_backward)	z += curVel;
        if (m_left)		x -= curVel;
        if (m_right)	x += curVel;
        // Apply camera transformation
        if (x != 0 || y != 0 || z != 0)
            cameraNavigationUpdate(x, y, z, 0.0f, 0.0f);
    }
    if (m_attachmentPlugIn)
    {
        //Horde3D::render(m_activeCameraID); // Render scene
        m_attachmentPlugIn->update();
        m_attachmentPlugIn->render( m_activeCameraID );
    }
    else
    {
        h3dRender(m_activeCameraID); // Render scene
        h3dFinalizeFrame();
    }

    if ( m_debugInfo & DRAW_GRID )
    {

    }

    renderEditorInfo();

    Im3d_EndFrame( glf, m_activeCameraID );

    // for some reason flushing is now required for opengl window
    glFlush();

    // Update Log if we are not the fullscreen widget
    updateLog();
}


void OpenGLWidget::wheelEvent(QWheelEvent* event)
{
    if (event->pixelDelta().y() > 0)
    {
        if( h3dGetNodeParamI(m_activeCameraID, H3DCamera::OrthoI ) == 1 )
        {
            const float* mat = 0;
            h3dGetNodeTransMats(m_activeCameraID, &mat, 0);
            QMatrix4f matrix(mat);
            matrix.scale(QVec3f(0.9f, 0.9f, 0.9f));
            matrix.x[12] = mat[12]; matrix.x[13] = mat[13]; matrix.x[14] = mat[14];
            h3dSetNodeTransMat(m_activeCameraID, matrix.x);
        }
        else
        {
            m_forward = true;
            m_backward= false;
            m_wheelTimer->start(event->pixelDelta().y());
        }
    }
    else
    {
        if( h3dGetNodeParamI(m_activeCameraID, H3DCamera::OrthoI) == 1 )
        {
            const float* mat = 0;
            h3dGetNodeTransMats(m_activeCameraID, &mat, 0);
            QMatrix4f matrix(mat);
            matrix.scale(QVec3f(1.0f / 0.9f, 1.0f / 0.9f, 1.0f / 0.9f));
            matrix.x[12] = mat[12]; matrix.x[13] = mat[13]; matrix.x[14] = mat[14];
            h3dSetNodeTransMat(m_activeCameraID, matrix.x);
        }
        else
        {
            m_forward = false;
            m_backward = true;
            m_wheelTimer->start(-event->pixelDelta().y());
        }
    }
    event->accept();
}


void OpenGLWidget::keyPressEvent(QKeyEvent* event)
{
    switch(event->key())
    {
    case Qt::Key_W:
        m_forward = true;
        break;
    case Qt::Key_S:
        m_backward = true;
        break;
    case Qt::Key_A:
        m_left = true;
        break;
    case Qt::Key_D:
        m_right = true;
        break;
    case Qt::Key_Escape:
        if (m_containerWidget->isFullScreen() )
            setFullScreen(false);

        resetMode(false);
        break;
    case Qt::Key_Return:
        resetMode(true);
        break;
    case Qt::Key_X:
	case Qt::Key_Y:
	case Qt::Key_Z:
//        if (m_transformationMode == MoveObject || m_transformationMode == RotateObject || m_transformationMode == ScaleObject)
        {
            m_useLocalTransform = !m_useLocalTransform;

//             switch( m_limitToAxis )
//             {
//             case Local_X: m_limitToAxis = 0; break;
//             case X: m_limitToAxis = Local_X; break;
//             default: m_limitToAxis = X; break;
//             }
            //applyChanges(false);
        }
        break;
//     case Qt::Key_Y:
//         if (m_transformationMode == MoveObject || m_transformationMode == RotateObject || m_transformationMode == ScaleObject)
//         {
//             switch( m_limitToAxis )
//             {
//             case Local_Y: m_limitToAxis = 0; break;
//             case Y: m_limitToAxis = Local_Y; break;
//             default: m_limitToAxis = Y; break;
// 
//             }
//             applyChanges(false);
//         }
//         break;
//     case Qt::Key_Z:
//         if (m_transformationMode == MoveObject || m_transformationMode == RotateObject || m_transformationMode == ScaleObject)
//         {
//             switch( m_limitToAxis )
//             {
//             case Local_Z: m_limitToAxis = 0; break;
//             case Z: m_limitToAxis = Local_Z; break;
//             default: m_limitToAxis = Z; break;
//             }
//             applyChanges(false);
//         }
//         break;
    };
    event->accept();
}



void OpenGLWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat())
    {
        QOpenGLWindow::keyReleaseEvent(event);
        return;
    }
    switch(event->key())
    {
    case Qt::Key_W:
        m_forward = false;
        break;
    case Qt::Key_A:
        m_left = false;
        break;
    case Qt::Key_S:
        m_backward = false;
        break;
    case Qt::Key_D:
        m_right = false;
        break;
    }
    event->accept();
}


void OpenGLWidget::mousePressEvent ( QMouseEvent * event )
{
    // if we have a transformation pending, return ( should never be the case, since mouseReleaseEvent should reset m_transformationMode ? )
    if ( m_transformationMode != None )
    {
        event->accept();
        m_selectButtonPressed = true;
        return;
    }
    // if we currently are not in a transformation mode and left button pressed, check for GIZMO selection
    if (event->button() == Qt::LeftButton && m_gizmoSelection != 0)
    {
        //setTransformationMode(MoveObject);
        if ( m_gizmoSelection == MoveObject ) emit transformationMode(MoveObject);
        else if ( m_gizmoSelection == RotateObject ) emit transformationMode(RotateObject);
        else if ( m_gizmoSelection == ScaleObject ) emit transformationMode(ScaleObject);

        m_limitToAxis = m_gizmoSelection;
        m_selectButtonPressed = true;
        event->accept();
        return;
    }
    // Camera Move Button pressed?
    if (event->buttons() & m_cameraMoveButton)// Do Camera movement
    {
        // Change Cursor Shape
        setCursor(Qt::SizeAllCursor);
        cameraNavigationStart();
    }
    // Selection Button pressed (and no other button)
    else if (event->buttons() == m_selectButton)
    {
        // Normalize viewport coordinates
        const qreal scale = devicePixelRatio(); // support scaled display
        float scaledWidth = width() * scale;
        float scaledHeight = height() * scale;

        float normalized_x( float(event->pos().x()) / scaledWidth );
        float normalized_y( float((scaledHeight - event->pos().y())) / scaledHeight );

        // Select node under the mouse cursor
        H3DNode node = h3dutPickNode(m_activeCameraID, normalized_x, normalized_y);
        // If it's a mesh node select it's parent model node
        if (node != 0 && h3dGetNodeType(node) == H3DNodeTypes::Mesh)
        {
            while ( node != 0 && h3dGetNodeType(node) != H3DNodeTypes::Model )
                node = h3dGetNodeParent(node);
        }
        if (m_currentNode && node == m_currentNode->property("ID").toInt() )
            emit nodeSelected(0);
        else
            emit nodeSelected(node);
    }
    // Reset Selection button ?
    if (event->button() == m_resetSelectButton)
    {
        // Deselect any node
        emit nodeSelected(0);
    }
    event->accept();
}

void OpenGLWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == m_selectButton)
    {
        // Normalize viewport coordinates
        const qreal scale = devicePixelRatio(); // support scaled display
        float scaledWidth = width() * scale;
        float scaledHeight = height() * scale;

        float normalized_x( float(event->pos().x() * scale) / scaledWidth );
        float normalized_y( float( (scaledHeight - (event->pos().y() * scale)) ) / scaledHeight );

        // Select mesh node under mouse cursor
        H3DNode node = h3dutPickNode(m_activeCameraID, normalized_x, normalized_y);
        if (m_currentNode && m_currentNode->property("ID").toInt() == node)
            emit nodeSelected(0); // Deselect node
        else
            emit nodeSelected(node); // Select Mesh
    }
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
    // If we were in object manipulation mode...
    if (m_transformationMode == MoveObject || m_transformationMode == RotateObject || m_transformationMode == ScaleObject)
    {
        // Check if we have to confirm the change
        if (event->button() == Qt::LeftButton)
            resetMode(true); // apply transformation if left mouse button was pressed
        else // on any other than the left mouse button we reject it
            resetMode(false); // reset transformation if any other mouse button was pressed
        emit transformationMode(None); // Reset mode
    }
    else // Reset cursor state
        setCursor(Qt::ArrowCursor);
    m_limitToAxis = 0;
//    m_useLocalTransform = false;

    m_selectButtonPressed = false;
    event->accept();
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_transformationMode == None && event->buttons() & m_cameraMoveButton)
    {
        bool controlPressed =
                ( ( ((qApp->keyboardModifiers() & Qt::ControlModifier)) != 0 ) | ( event->buttons() & m_selectButton ) ) ^
                (h3dGetNodeParamI(m_activeCameraID, H3DCamera::OrthoI) == 1);
        QPointF globalPos = event->globalPosition();
        float diffX = globalPos.x() - m_navOrigin.x();
        float diffY = globalPos.y() - m_navOrigin.y();
        m_navOrigin = globalPos;
        if (controlPressed) // Strafe
            cameraNavigationUpdate(diffX * (m_navSpeed / 200), diffY * (m_navSpeed / 200), 0, 0, 0);
        else // Rotate Camera
            cameraNavigationUpdate(0, 0, 0, diffX * (m_navSpeed / 50), diffY * (m_navSpeed / 50));
    }
    else if( m_currentNode )
    {
        // Move object
        // if (m_transformationMode == MoveObject)
        //     translateObject(event->position().x(), (height() - event->position().y()));
        // Rotate Object
        // else if (m_transformationMode == RotateObject)
        //     rotateObject(event->position().x(), height() - event->position().y());
        // // Scale Object
        // else if (m_transformationMode == ScaleObject)
        //     scaleObject(event->position().x(), height() - event->position().y());
    }
    event->accept();
}


void OpenGLWidget::focusInEvent(QFocusEvent* event)
{
    // QFrame* frame = qobject_cast<QFrame*>(parentWidget());
    // if( frame )
    //     frame->setFrameShadow(QFrame::Sunken);
    event->accept();
}

void OpenGLWidget::focusOutEvent(QFocusEvent* event)
{
    // QFrame* frame = qobject_cast<QFrame*>(parentWidget());
    // if( frame )
    //     frame->setFrameShadow(QFrame::Raised);
    event->accept();
}


void OpenGLWidget::cameraNavigationStart()
{
    // The start and update approach is suffering less from gimble lock than the implementation before,
    // but it still is not a perfekt solution
    h3dGetNodeTransform(m_activeCameraID, 0,0,0, &m_navRx, &m_navRy, 0,0,0,0);
    m_navOrigin = QCursor::pos();
}


void OpenGLWidget::cameraNavigationUpdate(float x, float y, float z, float rx, float ry)
{
    unsigned int cameraID = m_activeCameraID;
    if( x != 0.0f || y != 0.0f || z != 0.0f )
    {
        const float* transMat = 0;
        h3dGetNodeTransMats(cameraID, &transMat, 0);
        if( !transMat ) return;

        float newx, newy, newz;
        float scale = QVec3f(transMat[0], transMat[1], transMat[2]).length();
        newx = transMat[0] * x  +  transMat[4] * (-y) + transMat[ 8] * z / scale + transMat[12];
        newy = transMat[1] * x  +  transMat[5] * (-y) + transMat[ 9] * z / scale + transMat[13];
        newz = transMat[2] * x  +  transMat[6] * (-y) + transMat[10] * z / scale + transMat[14];
        if (!m_collisionCheck || h3dCastRay(H3DRootNode, transMat[12], transMat[13], transMat[14], newx - transMat[12], newy - transMat[13], newz - transMat[14], 1) == 0)
        {
            const_cast<float*>(transMat)[12] = newx;
            const_cast<float*>(transMat)[13] = newy;
            const_cast<float*>(transMat)[14] = newz;
            h3dSetNodeTransMat(cameraID, transMat);
        }
    }
    if( rx != 0.0f || ry != 0.0f)
    {
        float t_px, t_py, t_pz, t_sx, t_sy, t_sz;
        h3dGetNodeTransform(cameraID, &t_px, &t_py, &t_pz, 0,0,0, &t_sx,&t_sy,&t_sz);
        h3dSetNodeTransform(cameraID, t_px, t_py, t_pz, m_navRx - ry, m_navRy - rx, 0, t_sx, t_sy, t_sz);
        m_navRx = m_navRx - ry; m_navRy = m_navRy - rx;
    }
}

void OpenGLWidget::translateObject(int x, int y)
{
    QVariant transProp = m_currentNode->property("__AbsoluteTransformation");
    if( !transProp.isValid() ) return;

    // Get the currently active scene node transformation
    QMatrix4f nodeTrans = transProp.value<QMatrix4f>();

    H3DNode camera = m_activeCameraID;
    const float* cameraTrans = 0;
    h3dGetNodeTransMats(camera, 0, &cameraTrans);
    if ( !cameraTrans ) return;

    const qreal scale = devicePixelRatio(); // support scaled display
    float scaledWidth = width() * scale;
    float scaledHeight = height() * scale;

    const float camScale = QVec3f(cameraTrans[0], cameraTrans[1], cameraTrans[2]).length();
    const float frustumHeight = h3dGetNodeParamF(camera, H3DCamera::TopPlaneF, 0) - h3dGetNodeParamF(camera, H3DCamera::BottomPlaneF, 0);
    const float frustumWidth  = h3dGetNodeParamF(camera, H3DCamera::RightPlaneF, 0) - h3dGetNodeParamF(camera, H3DCamera::LeftPlaneF, 0);
    float diffX = (x - m_x) * frustumWidth * camScale / scaledWidth;
    float diffY = (y - m_y) * frustumHeight * camScale / scaledHeight;

    if ( camera == m_currentNode->property("ID").toInt() )
    {
        diffX = (x - m_x) / m_navSpeed;
        diffY = (y - m_y) / m_navSpeed;
    }
    else if( h3dGetNodeParamI( camera, H3DCamera::OrthoI ) == 0 )
    {
        const float nearPlane = h3dGetNodeParamF(camera, H3DCamera::NearPlaneF, 0);
        const float dist = QVec3f( cameraTrans[12] - nodeTrans.x[12], cameraTrans[13] - nodeTrans.x[13], cameraTrans[14] - nodeTrans.x[14] ).length();
        diffX *= dist / nearPlane;
        diffY *= dist / nearPlane;
    }

    // Synchronize Axis and Mouse Movement
    diffX *= m_axisVpX;
    diffY *= m_axisVpY;

    float tx = 0, ty = 0, tz = 0;
    // Calculate translation parallel to the camera plane if no axis limitation is present
    switch(m_limitToAxis)
    {
    case 0:
        // move parallel to the camera plane
        tx = cameraTrans[0] * diffX  +  cameraTrans[4] * diffY  /*+   cameraTrans[8]  * z  +   cameraTrans[12]*/;
        ty = cameraTrans[1] * diffX  +  cameraTrans[5] * diffY  /*+   cameraTrans[9]  * z  +   cameraTrans[13]*/;
        tz = cameraTrans[2] * diffX  +  cameraTrans[6] * diffY  /*+   cameraTrans[10] * z  +   cameraTrans[14]*/;
        break;
    case X:
        tx = diffX + diffY;
        break;
    case Y:
        ty = diffX + diffY;
        break;
    case Z:
        tz = diffX + diffY;
        break;
    case Local_X:
    {
        float sx = sqrtf( nodeTrans.x[0]*nodeTrans.x[0] + nodeTrans.x[1]*nodeTrans.x[1] + nodeTrans.x[2]*nodeTrans.x[2] );
        tx = nodeTrans.x[0] * (diffX + diffY) / sx;
        ty = nodeTrans.x[1] * (diffX + diffY) / sx;
        tz = nodeTrans.x[2] * (diffX + diffY) / sx;
    }
        break;
    case Local_Y:
    {
        float sy = sqrtf( nodeTrans.x[4]*nodeTrans.x[4] + nodeTrans.x[5]*nodeTrans.x[5] + nodeTrans.x[6]*nodeTrans.x[6] );
        tx = nodeTrans.x[4] * (diffX + diffY) / sy;
        ty = nodeTrans.x[5] * (diffX + diffY) / sy;
        tz = nodeTrans.x[6] * (diffX + diffY) / sy;
    }
        break;
    case Local_Z:
    {
        float sz = sqrtf( nodeTrans.x[8]*nodeTrans.x[8] + nodeTrans.x[9]*nodeTrans.x[9] + nodeTrans.x[10]*nodeTrans.x[10] );
        tx = nodeTrans.x[8] * (diffX + diffY) / sz;
        ty = nodeTrans.x[9] * (diffX + diffY) / sz;
        tz = nodeTrans.x[10] * (diffX + diffY) / sz;
    }
        break;
    }
    // avoid zero transformations since this is used to confirm transformation change
    if (tx != 0 || ty != 0 || tz != 0)
    {
        emit moveObject(tx, ty, tz);
        emit statusMessage(tr("Translation change: %1, %2, %3").arg(tx).arg(ty).arg(tz), 2000);
    }
}

void OpenGLWidget::rotateObject(int x, int y)
{
    QVariant transProp = m_currentNode->property("__AbsoluteTransformation");
    if( !transProp.isValid() ) return;

    // Get the currently active scene node transformation
    QMatrix4f nodeTrans = transProp.value<QMatrix4f>();

    // Get the currently active camera transformation
    H3DNode camera = m_activeCameraID;
    const float* cameraTrans = 0;
    h3dGetNodeTransMats(camera, 0, &cameraTrans);
    if ( !cameraTrans ) return;

    // Activate OpenGL Context to use gluProject
    makeCurrent();
    // Get viewport coordinate of the currently active scene node's center of gravity by using gluProject
    double cogX, cogY, cogZ;
    getViewportProjection(0, 0, 0, cogX, cogY, cogZ, nodeTrans.x);
    // Avoid division by zero
    if (cogX == x && cogY == y)	return;
    // Calculate angle between current mouse position and CoG
    float angle1 = asin(float(y - cogY) / sqrt((cogX - x ) * (cogX - x) + (cogY - y) * (cogY - y)));
    if (x < cogX) angle1 = 3.1415926f - angle1;
    // Calculate angle between entry mouse position and CoG
    float angle2 = asin(float(m_y - cogY) / sqrt((cogX - m_x ) * (cogX - m_x) + (cogY - m_y) * (cogY - m_y)));
    if (m_x < cogX)	angle2 = 3.1415926f - angle2;

    // Calculate angle change between entry and current mouse position
    double alpha = angle2 - angle1;
    if (alpha < 0) alpha += 6.2831853f;
    QVec3f t, r, s;
    QMatrix4f rot, nodeMat(QMatrix4f::RotMat( m_currentNode->property("Rotation").value<QVec3f>() * (3.1415926f / 180.0f)));
    switch ( m_limitToAxis )
    {
    // rotate parallel to the camera plane
    case 0:
        rot = QMatrix4f::RotMat((QVec3f(nodeTrans.x[12], nodeTrans.x[13], nodeTrans.x[14]) - QVec3f (cameraTrans[12], cameraTrans[13], cameraTrans[14])).normalized(), alpha);
        (rot * nodeMat).decompose(t, r, s);
        r *= 180.0f / 3.1415926f;
        r = r - m_currentNode->property("Rotation").value<QVec3f>();
        emit statusMessage(tr("Rotation change: %1, %2, %3").arg(r.X).arg(r.Y).arg(r.Z), 2000);
        break;
        // Rotate in global coordinate system TODO Check this
    case X:
        rot = QMatrix4f::RotMat(alpha, 0, 0);
        (rot * nodeMat).decompose(t, r, s);
        r *= 180.0f / 3.1415926f;
        r = r - m_currentNode->property("Rotation").value<QVec3f>();
        emit statusMessage(tr("World X-Axis: Rotation change: %1, %2, %3").arg(r.X).arg(r.Y).arg(r.Z), 2000);
        break;
    case Y:
        rot = QMatrix4f::RotMat(0, alpha, 0);
        (rot * nodeMat).decompose(t, r, s);
        r *= 180.0f / 3.1415926f;
        r = r - m_currentNode->property("Rotation").value<QVec3f>();
        emit statusMessage(tr("World Y-Axis: Rotation change: %1, %2, %3").arg(r.X).arg(r.Y).arg(r.Z), 2000);
        break;
    case Z:
        rot = QMatrix4f::RotMat(0, 0, alpha);
        (rot * nodeMat).decompose(t, r, s);
        r *= 180.0f / 3.1415926f;
        r = r - m_currentNode->property("Rotation").value<QVec3f>();
        emit statusMessage(tr("World Z-Axis: Rotation change: %1, %2, %3").arg(r.X).arg(r.Y).arg(r.Z), 2000);
        break;
        // Rotate in local coordinate system
    case Local_X:
        //r.X = alpha;
        rot = QMatrix4f::RotMat(alpha, 0, 0);
        (nodeMat * rot).decompose(t, r, s);
        r *= 180.0f / 3.1415926f;
        r = r - m_currentNode->property("Rotation").value<QVec3f>();
        emit statusMessage(tr("Local X-Axis: Rotation change: %1, %2, %3").arg(r.X).arg(r.Y).arg(r.Z), 2000);
        break;
    case Local_Y:
        rot = QMatrix4f::RotMat(0, alpha, 0);
        (nodeMat * rot).decompose(t, r, s);
        r *= 180.0f / 3.1415926f;
        r = r - m_currentNode->property("Rotation").value<QVec3f>();
        emit statusMessage(tr("Local Y-Axis: Rotation change: %1, %2, %3").arg(r.X).arg(r.Y).arg(r.Z), 2000);
        break;
    case Local_Z:
        rot = QMatrix4f::RotMat(0, 0, alpha);
        (nodeMat * rot).decompose(t, r, s);
        r *= 180.0f / 3.1415926f;
        r = r - m_currentNode->property("Rotation").value<QVec3f>();
        emit statusMessage(tr("Local Z-Axis: Rotation change: %1, %2, %3").arg(r.X).arg(r.Y).arg(r.Z), 2000);
        break;
    }
    // avoid zero transformations since this is used to confirm transformation change
    if (r.X != 0 || r.Y != 0 || r.Z != 0)
        emit rotateObject(r.X, r.Y, r.Z);
}

void OpenGLWidget::scaleObject(int x, int y)
{
    QVariant transProp = m_currentNode->property("__AbsoluteTransformation");
    if( !transProp.isValid() ) return;

    // Get the currently active scene node transformation
    QMatrix4f nodeTrans = transProp.value<QMatrix4f>();

    // Activate OpenGL Context to use gluProject
    makeCurrent();
    // Get viewport coordinate of the currently active scene node's center of gravity by using gluProject
    double cogX, cogY, cogZ;
    getViewportProjection(0, 0, 0, cogX, cogY, cogZ, nodeTrans.x);

    float distance1 = QVec3f(cogX - m_x, cogY - m_y, 0).length();
    float distance2 = QVec3f(cogX - x, cogY - y, 0).length();

    if (distance1 == 0 || distance2 == 0) // Avoid zero scaling and division by zero
        return;

    float sx = 1, sy = 1, sz = 1;
    switch (m_limitToAxis)
    {
    case 0:
        sx = sy = sz = distance2 / distance1;
        emit statusMessage(tr("Scale change: %1, %2, %3").arg(sx).arg(sy).arg(sz), 2000);
        break;
    case X:
    {
        QVec3f scale = (QMatrix4f::ScaleMat(distance2 / distance1, 1, 1) *
                        QMatrix4f::RotMat(m_currentNode->property("Rotation").value<QVec3f>().toRad() )).getScale();
        sx = scale.X; sy = scale.Y; sz = scale.Z;
        emit statusMessage(tr("World X-Axis: Scale change: %1, %2, %3").arg(sx).arg(sy).arg(sz), 2000);
    }
        break;
    case Y:
    {
        QVec3f scale = (QMatrix4f::ScaleMat(1, distance2 / distance1, 1) *
                        QMatrix4f::RotMat(m_currentNode->property("Rotation").value<QVec3f>().toRad() )).getScale();
        sx = scale.X; sy = scale.Y; sz = scale.Z;
        emit statusMessage(tr("World Y-Axis: Scale change: %1, %2, %3").arg(sx).arg(sy).arg(sz), 2000);
    }
        break;
    case Z:
    {
        QVec3f scale = (QMatrix4f::ScaleMat(1, 1, distance2 / distance1) *
                        QMatrix4f::RotMat(m_currentNode->property("Rotation").value<QVec3f>().toRad() )).getScale();
        sx = scale.X; sy = scale.Y; sz = scale.Z;
        emit statusMessage(tr("World Z-Axis: Scale change: %1, %2, %3").arg(sx).arg(sy).arg(sz), 2000);
    }
        break;
    case Local_X:
        sx = distance2 / distance1;
        emit statusMessage(tr("Local X-Axis: Scale change: %1, %2, %3").arg(sx).arg(sy).arg(sz), 2000);
        break;
    case Local_Y:
        sy = distance2 / distance1;
        emit statusMessage(tr("Local Y-Axis: Scale change: %1, %2, %3").arg(sx).arg(sy).arg(sz), 2000);
        break;
    case Local_Z:
        sz = distance2 / distance1;
        emit statusMessage(tr("Local Z-Axis: Scale change: %1, %2, %3").arg(sx).arg(sy).arg(sz), 2000);
        break;
    }
    emit scaleObject(sx, sy, sz);
}

void OpenGLWidget::applyChanges(bool save)
{
    switch (m_transformationMode)
    {
    case MoveObject: // if we were in move mode
        if( save )
            emit moveObject(0, 0, 0); // send acknowledge command
        else
            emit moveObject(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()); // send reset command
        break;
    case RotateObject: // if we were in rotate mode
        if( save )
            emit rotateObject(0,0,0);
        else
            emit rotateObject(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()); // send reset command
        break;
    case ScaleObject:
        if( save )
            emit scaleObject(0,0,0);
        else
            emit scaleObject(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()); // send reset command
        break;
    }
}

void OpenGLWidget::renderEditorInfo()
{
    const float* camera = 0;
    // Retrieve camera position...
    h3dGetNodeTransMats(m_activeCameraID, 0, &camera);

    // In case of an invalid camera (e.g. pipeline not set) return
    if ( !camera ) return;

    // ... and projection matrix
    float projMat[16];
    h3dGetCameraProjMat( m_activeCameraID, projMat );

    // Save OpenGL States
    // glPushAttrib(GL_ALL_ATTRIB_BITS);
    // glDisable(GL_LIGHTING);
    // glDisable(GL_BLEND);
    // glMatrixMode( GL_PROJECTION );
    // glLoadMatrixf( projMat );
    // glMatrixMode( GL_MODELVIEW );
    // QMatrix4f transMat( camera );
    // glLoadMatrixf( transMat.inverted().x );

    // glEnable(GL_DEPTH_TEST);
    if (m_debugInfo & DRAW_GRID)
        drawBaseGrid(camera[12], camera[13], camera[14]);

    // glDisable(GL_DEPTH_TEST);

    if (m_currentNode != 0 && m_currentNode->property("ID").toInt() != m_activeCameraID )
    {
        QVariant transProp = m_currentNode->property("__AbsoluteTransformation");
        if( !transProp.isValid() ) return;

        // Get the currently active scene node transformation
        QMatrix4f nodeTrans = transProp.value<QMatrix4f>();

        // select gizmo based on transformation mode
        if ( m_transformationMode == MoveObject )
        {
			Im3d::GetContext().m_gizmoMode = Im3d::GizmoMode_Translation;
        }
        else if ( m_transformationMode == RotateObject )
        {
            Im3d::GetContext().m_gizmoMode = Im3d::GizmoMode_Rotation;
        }
        else if ( m_transformationMode == ScaleObject )
        {
            Im3d::GetContext().m_gizmoMode = Im3d::GizmoMode_Scale;
        }

        Im3d::GetContext().m_gizmoLocal = m_useLocalTransform;

        Im3d::Mat4 transform;
        memcpy( &transform.m, &nodeTrans.x, sizeof( float ) * 16 );

		// The ID passed to Gizmo() should be unique during a frame - to create gizmos in a loop use PushId()/PopId().
        if ( Im3d::Gizmo( "GizmoUnified", transform ) )
        {
            m_gizmoSelection = m_transformationMode;
            memcpy( &nodeTrans.x, &transform.m, sizeof( float ) * 16 );
            emit transformObject( nodeTrans );
        }

        if ( Im3d::GetHotId() == Im3d::MakeId( "GizmoUnified" ) )
        {
            // save transformationMode in gizmo selection for further use in mouse press/move
            if ( Im3d::GetContext().m_gizmoMode == Im3d::GizmoMode_Translation ) m_gizmoSelection = MoveObject;
            else if ( Im3d::GetContext().m_gizmoMode == Im3d::GizmoMode_Rotation ) m_gizmoSelection = RotateObject;
            else if ( Im3d::GetContext().m_gizmoMode == Im3d::GizmoMode_Scale ) m_gizmoSelection = ScaleObject;
        }
        else m_gizmoSelection = 0;

        // drawGizmo(nodeTrans.x, camera);
        // Get Center of Gravity of selected node
//        double cogX, cogY, cogZ;
        // getViewportProjection(0, 0, 0, cogX, cogY, cogZ, nodeTrans.x);

        // if (m_transformationMode == RotateObject || m_transformationMode == ScaleObject)
        // {
        //     // Draw line between Center of Gravity and Mouse Position
        //     glColor3f(1.0f, 1.0f, 1.0f);
        //     QPoint mousePosition = mapFromGlobal(QCursor::pos());
        //     mousePosition.setY(height() - mousePosition.y());
        //     drawViewportLine(mousePosition, QPoint(cogX, cogY));
        //     if (m_transformationMode == RotateObject)
        //     {
        //         glEnable(GL_LINE_STIPPLE);
        //         glLineStipple(1, 0xFF00);
        //         glColor3f(0.6f, 0.6f, 0.6f);
        //         drawViewportCircle(cogX, cogY, QVec3f(mousePosition.x() - cogX, mousePosition.y() - cogY, 0).length());
        //         glDisable(GL_LINE_STIPPLE);
        //     }
        // }

        // Use depth buffer (if not disabled by the pipeline config)
        // glEnable(GL_DEPTH_TEST);

        double axisVpX = 0, axisVpY = 0, axisVpZ = 0;
        // Draw axis information
        // switch (m_limitToAxis)
        // {
        // case X:
        //     drawAxis(QVec3f(nodeTrans.x[12], nodeTrans.x[13], nodeTrans.x[14]), QVec3f(1, 0, 0), QVec3f(1, 0, 0));
        //     getViewportProjection(nodeTrans.x[12] + 1, nodeTrans.x[13], nodeTrans.x[14], axisVpX, axisVpY, axisVpZ);
        //     break;
        // case Y:
        //     drawAxis(QVec3f(nodeTrans.x[12], nodeTrans.x[13], nodeTrans.x[14]), QVec3f(0, 1, 0), QVec3f(0, 1, 0));
        //     getViewportProjection(nodeTrans.x[12], nodeTrans.x[13] + 1, nodeTrans.x[14], axisVpX, axisVpY, axisVpZ);
        //     break;
        // case Z:
        //     drawAxis(QVec3f(nodeTrans.x[12], nodeTrans.x[13], nodeTrans.x[14]), QVec3f(0, 0, 1), QVec3f(0, 0, 1));
        //     getViewportProjection(nodeTrans.x[12], nodeTrans.x[13], nodeTrans.x[14] + 1, axisVpX, axisVpY, axisVpZ);
        //     break;
        // case Local_X:
        //     drawAxis(QVec3f(0, 0, 0), QVec3f(1, 0, 0), QVec3f(1, 0, 0), nodeTrans.x);
        //     getViewportProjection(1, 0, 0, axisVpX, axisVpY, axisVpZ, nodeTrans.x);
        //     break;
        // case Local_Y:
        //     drawAxis(QVec3f(0, 0, 0), QVec3f(0, 1, 0), QVec3f(0, 1, 0), nodeTrans.x);
        //     getViewportProjection(0, 1, 0, axisVpX, axisVpY, axisVpZ, nodeTrans.x);
        //     break;
        // case Local_Z:
        //     drawAxis(QVec3f(0, 0, 0), QVec3f(0, 0, 1), QVec3f(0, 0, 1), nodeTrans.x);
        //     getViewportProjection(0, 0, 1, axisVpX, axisVpY, axisVpZ, nodeTrans.x);
        //     break;
        // }
        // if (m_limitToAxis == 0)
        // {
        //     m_axisVpX = 1;
        //     m_axisVpY = 1;
        // }
        // else
        // {
        //     m_axisVpX = (axisVpX - cogX);
        //     m_axisVpY = (axisVpY - cogY);
        //     float norm = sqrt(m_axisVpX * m_axisVpX + m_axisVpY * m_axisVpY);
        //     m_axisVpX /= norm;
        //     m_axisVpY /= norm;
        // }

        if (m_debugInfo & DRAW_BOUNDING_BOX)
            drawBoundingBox( m_currentNode->property("ID").toInt() );
    }
//    glPopAttrib(); // Restore old OpenGL States
}

void OpenGLWidget::drawAxis(const QVec3f& start, const QVec3f& dir, const QVec3f& color, const float* modelView /*= 0*/)
{
    glMatrixMode(GL_MODELVIEW);
    if( modelView )
    {
        glPushMatrix();
        glMultMatrixf(modelView);
    }
    glLineWidth(1.0);
    glColor3f(color.X, color.Y, color.Z);
    glBegin(GL_LINES);
    glVertex3f( start.X, start.Y, start.Z );
    glVertex3f( start.X + dir.X * INT_MAX, start.Y + dir.Y * INT_MAX, start.Z + dir.Z * INT_MAX );
    glVertex3f( start.X, start.Y, start.Z );
    glVertex3f( start.X - dir.X * INT_MAX, start.Y - dir.Y * INT_MAX, start.Z - dir.Z * INT_MAX );
    glEnd();
    if( modelView )
        glPopMatrix();
}

void OpenGLWidget::drawBoundingBox(unsigned int hordeID)
{
    if( hordeID != 0 )
    {
        // Draw AABB of selected object
        float minX, minY, minZ, maxX, maxY, maxZ;
        h3dGetNodeAABB(hordeID, &minX, &minY, &minZ, &maxX, &maxY, &maxZ);
        Im3d::SetColor( 0.7f, 0.7f, 0.7f );
        Im3d::BeginLineStrip();
        Im3d::Vertex(minX, minY, minZ);
        Im3d::Vertex(maxX, minY, minZ);
        Im3d::Vertex(maxX, maxY, minZ);
        Im3d::Vertex(minX, maxY, minZ);
        Im3d::Vertex(minX, minY, minZ);
        Im3d::Vertex(minX, minY, maxZ);
        Im3d::Vertex(maxX, minY, maxZ);
        Im3d::Vertex(maxX, maxY, maxZ);
        Im3d::Vertex(minX, maxY, maxZ);
        Im3d::Vertex(minX, minY, maxZ);
        Im3d::End();
        Im3d::BeginLines();
        Im3d::Vertex(minX, maxY, minZ);
        Im3d::Vertex(minX, maxY, maxZ);
        Im3d::Vertex(maxX, minY, minZ);
        Im3d::Vertex(maxX, minY, maxZ);
        Im3d::Vertex(maxX, maxY, minZ);
        Im3d::Vertex(maxX, maxY, maxZ);
        Im3d::End();
    }
}

void OpenGLWidget::drawViewportLine(const QPoint& start, const QPoint& end)
{
    // Save current projection matrix
    GLdouble projection[16];							// Where The 16 Doubles Of The Projection Matrix Are To Be Stored
    glGetDoublev(GL_PROJECTION_MATRIX, projection);	// Retrieve The Projection Matrix
    // Save current modelview matrix
    GLdouble modelview[16];					// Where The 16 Doubles Of The Modelview Matrix Are To Be Stored
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);		// Retrieve The Modelview Matrix

    const qreal scale = devicePixelRatio(); // support scaled display
    float scaledWidth = width() * scale;
    float scaledHeight = height() * scale;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, scaledWidth, 0, scaledHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPointSize(5);
    glBegin(GL_POINTS);
    glVertex2f(start.x(), start.y());
    glVertex2d(end.x(), end.y());
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(start.x(), start.y());
    glVertex2d(end.x(), end.y());
    glEnd();

    // Restore old projection and modelview stack
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(projection);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(modelview);
}

void OpenGLWidget::drawViewportCircle(float x, float y, float radius)
{
    // Save current projection matrix
    GLdouble projection[16];							// Where The 16 Doubles Of The Projection Matrix Are To Be Stored
    glGetDoublev(GL_PROJECTION_MATRIX, projection);	// Retrieve The Projection Matrix
    // Save current modelview matrix
    GLdouble modelview[16];					// Where The 16 Doubles Of The Modelview Matrix Are To Be Stored
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);		// Retrieve The Modelview Matrix

    const qreal scale = devicePixelRatio(); // support scaled display
    float scaledWidth = width() * scale;
    float scaledHeight = height() * scale;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, scaledWidth, 0, scaledHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glLineWidth(1);
    glBegin(GL_LINE_LOOP);
    float degInRad = 0;
    for (int i=0; i<360; ++i)
    {
        degInRad = i * 3.141592f/180.0f;
        glVertex2f(cos(degInRad)*radius + x,sin(degInRad)*radius + y);
    }
    glEnd();

    // Restore old projection and modelview stack
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(projection);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(modelview);
}

void OpenGLWidget::glGizmo()
{
    glShadeModel(GL_SMOOTH);
    // x-axis
    glColor3f( 1, 0, 0);
    glPushMatrix ();
    glLoadName(8);
    glRotatef ((GLfloat) 90.0, (GLfloat)0.0, (GLfloat)1.0, (GLfloat)0.0);
    GLUquadricObj *quadObj = gluNewQuadric ();
    gluQuadricDrawStyle (quadObj, GLU_FILL);
    gluQuadricNormals (quadObj, GLU_SMOOTH);
    gluCylinder (quadObj, 0.250, 0.250, 3.75, 8, 2);
    gluDeleteQuadric(quadObj);
    glTranslatef ((GLfloat)0.0, (GLfloat)0.0, (GLfloat)3.5);
    quadObj = gluNewQuadric ();
    gluQuadricDrawStyle (quadObj, GLU_FILL);
    gluQuadricNormals (quadObj, GLU_SMOOTH);
    gluCylinder(quadObj, 0.5, 0.0, 3, 10, 6);
    gluDeleteQuadric(quadObj);
    glPopMatrix ();

    //// y-axis
    glColor3f( 0, 1, 0);
    glPushMatrix ();
    glLoadName(16);
    glRotatef ((GLfloat) -90.0, (GLfloat)1.0, (GLfloat)0.0, (GLfloat)0.0);
    quadObj = gluNewQuadric ();
    gluQuadricDrawStyle (quadObj, GLU_FILL);
    gluQuadricNormals (quadObj, GLU_SMOOTH);
    gluCylinder (quadObj, 0.250, 0.250, 3.75, 8, 2);
    gluDeleteQuadric(quadObj);
    glTranslatef ((GLfloat)0.0, (GLfloat)0.0, (GLfloat)3.5);
    quadObj = gluNewQuadric ();
    gluQuadricDrawStyle (quadObj, GLU_FILL);
    gluQuadricNormals (quadObj, GLU_SMOOTH);
    gluCylinder(quadObj, 0.5, 0.0, 3, 10, 6);
    gluDeleteQuadric(quadObj);
    glPopMatrix ();

    //// z-axis
    glColor3f( 0, 0, 1);
    glPushMatrix ();
    glLoadName(32);
    //glRotatef ((GLfloat) -90.0, (GLfloat)0.0, (GLfloat)0.0, (GLfloat)1.0);
    quadObj = gluNewQuadric ();
    gluQuadricDrawStyle (quadObj, GLU_FILL);
    gluQuadricNormals (quadObj, GLU_SMOOTH);
    gluCylinder (quadObj, 0.250, 0.250, 3.75, 8, 2);
    gluDeleteQuadric(quadObj);
    glTranslatef ((GLfloat)0.0, (GLfloat)0.0, (GLfloat)3.5);
    quadObj = gluNewQuadric ();
    gluQuadricDrawStyle (quadObj, GLU_FILL);
    gluQuadricNormals (quadObj, GLU_SMOOTH);
    gluCylinder(quadObj, 0.5, 0.0, 3, 10, 6);
    gluDeleteQuadric(quadObj);
    glPopMatrix ();
}

void OpenGLWidget::drawGizmo(const float* nodeTransform, const float* cam)
{
    // Save current projection matrix
    GLdouble projection[16];							// Where The 16 Doubles Of The Projection Matrix Are To Be Stored
    glGetDoublev(GL_PROJECTION_MATRIX, projection);	// Retrieve The Projection Matrix


    // Set model view matrix of selected node
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(nodeTransform);	// Load scene node matrix

    glPushMatrix(); // Push matrix before applying the scaling for the gizmo

    // Calculate scale for gizmo with constant size
    QVec3f camScale = QMatrix4f(cam).getScale();
    QVec3f nodeScale = QMatrix4f(nodeTransform).getScale();

    //const float frustumHeight = h3dGetNodeParamF(m_activeCameraID, H3DCamera::TopPlane) - h3dGetNodeParamF(m_activeCameraID, H3DCamera::BottomPlane);
    const float frustumWidth  = h3dGetNodeParamF(m_activeCameraID, H3DCamera::RightPlaneF, 0) - h3dGetNodeParamF(m_activeCameraID, H3DCamera::LeftPlaneF, 0);
    // first calculate scale depending on the viewing frustum and viewport settings (10 is a constant factor to make the gizmo big enough and is not
    // part of the formula
    float scale = 10 * frustumWidth * camScale.X / width();

    // If we have an perspective projection
    if( h3dGetNodeParamI( m_activeCameraID, H3DCamera::OrthoI ) == 0 )
    {
        // Take node distance and nearPlane distance into account
        const float nearPlane = h3dGetNodeParamF(m_activeCameraID, H3DCamera::NearPlaneF, 0);
        const float distance = QVec3f(nodeTransform[12] - cam[12], nodeTransform[13] - cam[13], nodeTransform[14] - cam[14]).length(); // Calculate distance between scene node and camera
        scale *= distance / nearPlane;
    }
    // finally remove any scale part of the current modelview matrix
    glScalef(scale / nodeScale.X, scale / nodeScale.Y, scale / nodeScale.Z);

    // Draw Gizmo
    glGizmo();

    // Check for Gizmo Selection
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport); 	// Get viewport settings for gluPickMatrix
    QPoint cursorPos = mapFromGlobal(QCursor::pos()); // Retrieve mouse cursor position

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPickMatrix(cursorPos.x() ,viewport[3]-cursorPos.y(), 1, 1, viewport); // Use Pickmatrix to render only the scene under the mouse cursor
    glMultMatrixd(projection);
    glMatrixMode(GL_MODELVIEW);

    GLuint selectBuf[512];
    glSelectBuffer (512, selectBuf);
    glRenderMode (GL_SELECT);
    glInitNames();
    glPushName(0);
    glGizmo();	// Render Gizmo to retrieve the glName hits
    glFlush();
    GLint hits = glRenderMode (GL_RENDER);
    glPopMatrix(); // Remove scale for Gizmo from stack
    GLuint names, *ptr = selectBuf;
    float minAxisZ = float(INT_MAX);
    int axis = 0;
    for (int i = 0; i < hits; ++i) /*  for each hit  */
    {
        names = *ptr++;	// Get number of names for this hit
        float z = float(*ptr)/0x7fffffff; // Get nearest Z Value
        ptr += 2; // Skip z2
        if (z < minAxisZ && names > 0)
        {
            minAxisZ = z;
            axis = *ptr; // Set selected axis
        }
        ptr += names;
    }
    m_gizmoSelection = axis;

    glPopMatrix(); // Restore old model view matrix (Only Camera Transformation applied)

    // Restore old projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(projection);
    glMatrixMode(GL_MODELVIEW);
}

void OpenGLWidget::drawBaseGrid(const float camX, const float camY, const float camZ)
{
    static int gridSize = 200;
    const float gridHalf = (float)gridSize * 0.5f;
    Im3d::SetAlpha(1.0f);
    Im3d::SetSize(1.0f);
    Im3d::BeginLines();

    Im3d::Color color;
        for (int x = 0; x <= gridSize; ++x)
        {
            if ( x == (int) gridHalf ) color = Im3d::Color( 0.3f, 0.3f, 0.9f );
            else color = Im3d::Color(0.5f, 0.5f, 0.5f);
            Im3d::Vertex(-gridHalf, 0.0f, (float)x - gridHalf, color /*Im3d::Color(0.5f, 0.5f, 0.5f)*/);
            Im3d::Vertex( gridHalf, 0.0f, (float)x - gridHalf, color /*Im3d::Color(1.0f, 0.5f, 0.5f)*/);
        }
        for (int z = 0; z <= gridSize; ++z)
        {
            if( z == (int) gridHalf ) color = Im3d::Color(0.9f, 0.3f, 0.3f);
            else color = Im3d::Color(0.5f, 0.5f, 0.5f);
            Im3d::Vertex((float)z - gridHalf, 0.0f, -gridHalf, color /*Im3d::Color(0.5f, 0.5f, 0.5f)*/);
            Im3d::Vertex((float)z - gridHalf, 0.0f,  gridHalf, color /*Im3d::Color(0.0f, 0.0f, 1.0f)*/);
        }
    Im3d::End();
}


bool OpenGLWidget::inSync()
{
    // Check if node transformation is in sync with stored transformation
    QVec3f pos( m_currentNode->property("Position").value<QVec3f>() );
    QVec3f rot( m_currentNode->property("Rotation").value<QVec3f>() );
    QVec3f scale( m_currentNode->property("Scale").value<QVec3f>() );

    QVariant matProp = m_currentNode->property("__RelativeTransformation");
    if( !matProp.isValid() ) return true;

    QMatrix4f relTrans = matProp.value<QMatrix4f>();
    QVec3f actPos( relTrans.getTranslation() );
    QVec3f actRot( relTrans.getRotation().toDeg() );
    QVec3f actScale( relTrans.getScale() );

    if((actPos - pos).length() > 0.001f || (actRot - rot).length() > 0.001f || ( actScale - scale).length() > 0.001f )
    {
        QHordeSceneEditorSettings settings(this);
        settings.beginGroup("ConfirmDialogs");
        int syncMode = settings.value("TransformationMismatchMode", 0).toInt();
        if (syncMode == 0)
        {
            syncMode = QOneTimeDialog::question( nullptr, tr("Adjust transformation?"),
                                                tr("The selected node's current transformation is not in sync\n"
                                                   "with the transformation in the XML file!\n\n\n"
                                                   "Do want to save the current transformation or reset the node's\n"
                                                   "transformation to the one stored in the XML file?"), QDialogButtonBox::Save, QDialogButtonBox::Reset, QDialogButtonBox::Cancel);
            if 	(syncMode & QDialogButtonBox::Cancel)
                return false;
            if (syncMode & QOneTimeDialog::ApplyAlways)
                settings.setValue("TransformationMismatchMode", syncMode & ~QOneTimeDialog::ApplyAlways);
        }
        settings.endGroup();

        switch( syncMode & ~QOneTimeDialog::ApplyAlways )
        {
        case QDialogButtonBox::Save:
        {
            QVec3f worldScale = QVec3f(1, 1, 1);
            int hordeID = m_currentNode->property("ID").toInt();
            // Move translation from local to global coordinate system
            if( hordeID != 0 )
            {
                const float* mat = 0;
                h3dGetNodeTransMats(h3dGetNodeParent(hordeID), 0, &mat);
                worldScale = QMatrix4f(mat).getScale();
            }
            emit moveObject((actPos.X - pos.X) * worldScale.X, (actPos.Y - pos.Y) * worldScale.Y, (actPos.Z - pos.Z) * worldScale.Z);
            emit moveObject(0,0,0);
            emit rotateObject(actRot.X - rot.X, actRot.Y - rot.Y, actRot.Z - rot.Z );
            emit rotateObject(0,0,0);
            emit scaleObject(scale.X / actScale.X, scale.Y / actScale.Y, scale.Z / actScale.Z);
            emit scaleObject(0,0,0);
        }
            break;
        case QDialogButtonBox::Reset:
            m_currentNode->setProperty("__RelativeTransformation",
                                       QVariant::fromValue(
                                           QMatrix4f::TransMat( pos.X, pos.Y, pos.Z ) *
                                           QMatrix4f::RotMat( QVec3f(rot.X, rot.Y, rot.Z).toRad() ) *
                                           QMatrix4f::ScaleMat( scale.X, scale.Y, scale.Z ) ) );
            break;
        }
    }
    return true;
}

void OpenGLWidget::getViewportProjection(const double px, const double py, const double pz, double &vx, double& vy, double& vz, const float* mat /*=0*/)
{
    if( mat )
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glMultMatrixf(mat);
    }
    // Get projection matrix for gluProject
    GLdouble projection[16];							// Where The 16 Doubles Of The Projection Matrix Are To Be Stored
    glGetDoublev(GL_PROJECTION_MATRIX, projection);	// Retrieve The Projection Matrix
    // Get modelview matrix for gluProject
    GLdouble modelview[16];					// Where The 16 Doubles Of The Modelview Matrix Are To Be Stored
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);		// Retrieve The Modelview Matrix
    // Get viewport settings for gluProject
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    // Get viewport coordinate of the specified 3D-point by using gluProject
    gluProject(px, py, pz, modelview, projection, viewport, &vx, &vy, &vz);
    if( mat )
        glPopMatrix();
}


