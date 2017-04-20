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

#include "HordeFileDialog.h"
#include "QTexUnit.h"
#include <QUniform.h>
#include "QHordeSceneEditorSettings.h"

#include <Horde3DUtils.h>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QtCore/QTextStream>
#include <QtXml/qdom.h>
#include <QtCore/QProcess>
#include <QtCore/QFileSystemWatcher>

#include "MaterialWidget.h"

MaterialWidget::MaterialWidget(QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : QWidget(parent, flags), m_shaderData(0), m_matHandle( 0 ),
m_shaderHandle( 0 )
{
	setupUi(this);
	connect(m_editShader, SIGNAL(clicked()), this, SLOT(editShader()));
	connect(m_texUnitCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(texUnitChanged(int)));
	connect(m_uniformCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(uniformChanged(int)));
	connect(m_linkMaterial, SIGNAL(currentIndexChanged(int)), this, SLOT(linkChanged(int)));
	connect(m_saveButton, SIGNAL(clicked()), this, SLOT(save()));
	connect(m_className, SIGNAL(textEdited(const QString&)), this, SLOT(classChanged()));
	m_texUnitProperties->registerCustomPropertyCB(CustomTypes::createCustomProperty);
	connect(m_texUnitProperties->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex& )), this, SLOT(texUnitDataChanged()));
	connect(m_uniformProperties->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex& )), this, SLOT(uniformDataChanged()));
	connect(m_shader, SIGNAL(shaderChanged()), this, SLOT(shaderChanged()));
	connect(m_shaderFlags, SIGNAL(stateChanged( int, bool ) ), this, SLOT( flagsChanged( int, bool ) ) );
	m_shaderFlags->setDisplayText( tr( "Shader Flags" ) );
	m_shaderWatcher = new QFileSystemWatcher( this );
	connect( m_shaderWatcher, SIGNAL( fileChanged( const QString& ) ), this, SLOT( syncWithShader() ) );
}


MaterialWidget::~MaterialWidget()
{
	release();
}

void MaterialWidget::init()
{
	m_shader->blockSignals(true);
	m_shader->init( QDir::currentPath() );
	m_linkMaterial->init( QDir::currentPath() );
	if( m_currentMaterialFile.isEmpty() ) 
		m_shader->setCurrentIndex(-1);
	else
	{
		QString shader = m_materialXml.documentElement().firstChildElement("Shader").attribute("source");
		m_shader->setCurrentIndex(m_shader->findText(shader));
	}
	m_shader->blockSignals(false);
}

void MaterialWidget::setCurrentMaterial(const QString &materialFileName)
{
	if( m_currentMaterialFile == materialFileName && !m_saveButton->isEnabled() )
		return;

	blockSignals(true);
	closeMaterial();
	m_matHandle = h3dFindResource( H3DResTypes::Material, qPrintable( materialFileName ) );	
	m_currentMaterialFile = materialFileName;			
	if (!materialFileName.isEmpty())
	{
		QFile file( QDir::current().absoluteFilePath(m_currentMaterialFile) );
		if (file.open(QIODevice::ReadOnly))
		{	
			QString errorMsg;
			int errorLine, errorColumn;
			// read material file
			if (!m_materialXml.setContent(&file, &errorMsg, &errorLine, &errorColumn))
				setStatusTip(tr("Error in line %1 column %2 when reading material file %3: %4").arg(errorLine).arg(errorColumn).arg(m_currentMaterialFile).arg(errorMsg));
			else
			{
				setStatusTip("");
				initValues();			
			}
			file.close();
		}
		else
			setStatusTip(file.errorString());
	}
	else
	{
		m_shader->setCurrentIndex(-1);
	}
	setEnabled(!materialFileName.isEmpty());
	blockSignals(false);
}

void MaterialWidget::closeMaterial()
{
	if (m_saveButton->isEnabled())
	{
		// Do a basic check for changes, not successfull in every case but better than nothing
		QFile file( QDir::current().absoluteFilePath(m_currentMaterialFile) );
		bool confirm = true;
		if( file.open(QIODevice::ReadOnly) )
		{	
			QDomDocument original;
			// read material file
			original.setContent(&file);
			file.close();
			if( original.toString() == m_materialXml.toString() )
				confirm = false;
		}
		
		if( confirm && QMessageBox::question(
			this, 
			tr("Save changes?"), 
			tr("Save previous changes to material %1?").arg(m_currentMaterialFile), 
			QMessageBox::Save | QMessageBox::Default, 
			QMessageBox::Ignore | QMessageBox::Escape)==QMessageBox::Save)
			save();				
	}
	m_saveButton->setEnabled(false);
	if (parentWidget())
		parentWidget()->setWindowTitle(tr("Material Settings"));

	release();

	m_materialXml.setContent( QString("<Material/>") );		
	m_shader->setCurrentIndex( -1 );	
}

void MaterialWidget::save()
{
	if (!m_currentMaterialFile.isEmpty() && m_saveButton->isEnabled())
	{
		QDomDocument cleanMaterial = m_materialXml.cloneNode( true ).toDocument();
		QDomNodeList samplers = cleanMaterial.documentElement().elementsByTagName( "Sampler" );
		for( int i = 0; i < samplers.count();  )
		{
			if( samplers.at( i ).toElement().attribute( "map" ).isEmpty() )
				cleanMaterial.documentElement().removeChild( samplers.at( i ) );				
			else
				++i;
		}		

		QDomNodeList shaderFlags = cleanMaterial.documentElement().elementsByTagName( "ShaderFlag" );
		for( int i = 0; i < shaderFlags.count(); )
		{
			if( shaderFlags.at( i ).toElement().attribute( "name" ).isEmpty() )
				cleanMaterial.documentElement().removeChild( shaderFlags.at( i ) );
			else
				++i;
		}

		if (m_className->text().isEmpty())
			cleanMaterial.documentElement().removeAttribute("class");
		else
			cleanMaterial.documentElement().setAttribute("class", m_className->text());		
		QFile file( QDir::current().absoluteFilePath(m_currentMaterialFile));
		if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		{
			QTextStream stream(&file);
			cleanMaterial.save(stream, 4);
			file.flush();
			file.close();
		}
		else
		{
			QMessageBox::warning(this, tr("Error"), tr("Error opening file %1 for writing:\n\n%2").arg(m_currentMaterialFile).arg(file.errorString()));
			return;
		}
	}
	m_saveButton->setEnabled(false);
}

void MaterialWidget::editShader()
{
	QHordeSceneEditorSettings settings(this);
	settings.beginGroup("General");
    QString executable = settings.value("ShaderEditor", "notepad.exe").toString();
    QStringList arguments( QDir::current().absoluteFilePath( m_shader->currentText() ) );
#ifdef __APPLE__
    QFileInfo info( executable );
    if( info.isDir() && executable.endsWith(".app") )
    {
        arguments.insert(0, info.baseName());
        arguments.insert(0, "-a");
        executable = "open";
    }
    qDebug( "%s %s", qPrintable(executable), qPrintable(arguments.join(" ")) );
#endif
	QProcess::startDetached(
		executable, 
		arguments 
	);
	settings.endGroup();	
}

void MaterialWidget::initValues()
{
	m_className->setText(m_materialXml.documentElement().attribute("class"));
	m_linkMaterial->setCurrentIndex( m_linkMaterial->findText( m_materialXml.documentElement().attribute( "link", "No material" ) ) );
	QString shader = m_materialXml.documentElement().firstChildElement("Shader").attribute("source");
	m_shader->setCurrentIndex(m_shader->findText(shader));
	if (m_shader->currentIndex() == -1)
		QMessageBox::warning(this, tr("Error"), tr("Shader \"%1\" not found!").arg(shader));

	if (parentWidget())
		parentWidget()->setWindowTitle(tr("Material - %1").arg(m_currentMaterialFile));	
	m_saveButton->setEnabled(false);
}

void MaterialWidget::texUnitChanged(int /*index*/)
{
	QTexUnit* unit = static_cast<QTexUnit*>(m_texUnitCombo->itemData(m_texUnitCombo->currentIndex()).value<void*>());
	m_texUnitProperties->setObject(unit);
}

void MaterialWidget::texUnitDataChanged()
{
	//QTexUnit* unit = static_cast<QTexUnit*>(m_texUnitCombo->itemData(m_texUnitCombo->currentIndex()).value<void*>());
	m_saveButton->setEnabled(true);	
}

void MaterialWidget::uniformChanged(int)
{
	QUniform* uniform = static_cast<QUniform*>(m_uniformCombo->itemData(m_uniformCombo->currentIndex()).value<void*>());	
	m_uniformProperties->setObject(uniform);
}

void MaterialWidget::uniformDataChanged()
{
        //QUniform* uniform = static_cast<QUniform*>(m_uniformCombo->itemData(m_uniformCombo->currentIndex()).value<void*>());
	m_saveButton->setEnabled(true);	
}

void MaterialWidget::linkChanged(int index)
{
	if( index == 0 ) // No material index
		m_materialXml.documentElement().removeAttribute( "link" );
	else
		m_materialXml.documentElement().setAttribute( "link", m_linkMaterial->currentText() );
	m_saveButton->setEnabled(true);
}

void MaterialWidget::shaderChanged()
{
	QDomElement shader = m_materialXml.documentElement().firstChildElement("Shader");
	if( shader.isNull() && !m_shader->currentText().isEmpty() )
		shader = m_materialXml.insertBefore( m_materialXml.createElement("Shader"), QDomNode()).toElement();
	shader.setAttribute("source", m_shader->currentText());

	if( !m_shaderWatcher->files().isEmpty() )
		m_shaderWatcher->removePaths( m_shaderWatcher->files() );
	if ( m_shader->currentText().isEmpty() )
	{
		m_materialXml.documentElement().removeChild(shader);
		m_shaderWatcher->addPath( QDir::current().absoluteFilePath( m_shader->currentText() ) );
	}
	syncWithShader();
	m_saveButton->setEnabled(true);
}

void MaterialWidget::classChanged()
{
	m_saveButton->setEnabled(true);
}

void MaterialWidget::flagsChanged( int index, bool checked )
{
	m_saveButton->setEnabled(true);
	QDomNodeList shaderFlags = m_materialXml.elementsByTagName( "ShaderFlag" );
	for( int i = 0; i < m_shaderFlags->count(); ++i )
	{
		qDebug( "Flag %d", m_shaderFlags->itemData( i, Qt::UserRole + 1 ).toInt() );
	}
	int flag = m_shaderFlags->itemData( index, Qt::UserRole + 1 ).toInt();
	QString flagName = 
		QString( "_F%1_%2" ).arg( flag, 2, 10, QChar::fromLatin1('0') ).arg( m_shaderFlags->itemText( index ) );
	for( int i = 0; i < shaderFlags.size(); ++i )
	{		
		if( shaderFlags.at( i ).toElement().attribute( "name" ) == flagName )
		{
			if( !checked )
				m_materialXml.documentElement().removeChild( shaderFlags.at( i ) );
			return;
		}
	}
	QDomNode shaderFlag = m_materialXml.documentElement().appendChild( m_materialXml.createElement( "ShaderFlag" ) );
	shaderFlag.toElement().setAttribute( "name", flagName );
}

void MaterialWidget::release()
{
	// Release all QUniform instances 
	for( int i = 0; i < m_uniformCombo->count(); ++i )
	{
		QUniform* uniform = static_cast<QUniform*>( m_uniformCombo->itemData( i ).value<void*>() );
		delete uniform;			
	}
	m_uniformCombo->clear();

	// Delete the shader data
	delete m_shaderData;
	m_shaderData = 0;

	// Release all QTexUnit instances
	for( int i = 0; i < m_texUnitCombo->count(); ++i )
	{
		QTexUnit* unit = static_cast<QTexUnit*>( m_texUnitCombo->itemData( i ).value<void*>() );
		delete unit;
	}
	m_texUnitCombo->clear();
	
	m_shaderFlags->clear();	
	if( m_shaderHandle )
	{
		h3dRemoveResource( m_shaderHandle );
		m_shaderHandle = 0;
		h3dReleaseUnusedResources();
	}
}

void MaterialWidget::syncWithShader()
{	
	if( m_shader->currentText().isEmpty() )
		return;

	QFile shaderFile( QDir::current().absoluteFilePath( m_shader->currentText() ) );
	if( !shaderFile.open( QIODevice::ReadOnly ) )
	{
		QMessageBox::warning( this, tr("Error"), tr("Error opening shader file\n\n%1").arg( QDir::current().absoluteFilePath( m_shader->currentText() ) ) );
		return;
	}
	release();

	QByteArray shaderData = shaderFile.readAll();
	shaderData.append( '\0' );
	m_shaderData = new ShaderData( shaderData );
	if( !m_shaderData->isValid() )
	{
		QMessageBox::warning( 
			this, 
			tr("Error"), 
			tr("Error reading shader file\n\n%1\n\n%2").arg( QDir::current().absoluteFilePath( m_shader->currentText() ) ).arg( m_shaderData->lastError() ) 
		);
		delete m_shaderData;
		m_shaderData = 0;
		return;
	}

	m_shaderHandle = h3dAddResource( H3DResTypes::Shader, qPrintable( m_shader->currentText() ), 0 );
	
	h3dLoadResource( m_shaderHandle, shaderData.constData(), shaderData.size() );

	QDomNodeList flags = m_materialXml.elementsByTagName( "ShaderFlag" );
	m_shaderFlags->blockSignals( true );
	for( int i = 0; i < m_shaderData->flags().size(); ++i)
	{
		bool set = false;
		for( int j = 0; j < flags.size(); ++j )
		{
			QString flag = flags.at(j).toElement().attribute( "name" );
			if( flag.compare( 
				QString( "_F%1_%2" ).arg( m_shaderData->flags().at(i).Flag, 2, 10, QChar::fromLatin1('0') ).arg( m_shaderData->flags().at(i).Name ),
				Qt::CaseInsensitive ) == 0 )
			{
				set = true;
				break;
			}
		}
		m_shaderFlags->addItem( m_shaderData->flags().at(i).Name, set );
		m_shaderFlags->setItemData( m_shaderFlags->count() - 1, m_shaderData->flags().at(i).Flag, Qt::UserRole + 1 );
	}
	m_shaderFlags->blockSignals( false );

	QDomNodeList samplers = m_materialXml.documentElement().elementsByTagName("Sampler");
	int numSamplers = h3dGetResElemCount( m_shaderHandle, H3DShaderRes::SamplerElem );	
	for( int i = 0; i < numSamplers; ++i )
	{
		QString sampler = h3dGetResParamStr( m_shaderHandle, H3DShaderRes::SamplerElem, i, H3DShaderRes::SampNameStr );
		QString tip("Sampler %1");		
		tip = tip.arg( sampler );
		QDomElement samplerXML;
		for( int j = 0;  j < samplers.count(); ++j)
		{			
			if( samplers.at(j).toElement().attribute("name") == sampler )
			{
				samplerXML = samplers.at(j).toElement();
				break;
			}	
		}
		if( samplerXML.isNull() )
		{
			samplerXML = m_materialXml.documentElement().appendChild( m_materialXml.createElement("Sampler") ).toElement();
			samplerXML.setAttribute( "name", sampler );
			H3DRes texRes = h3dGetResParamI( m_shaderHandle, H3DShaderRes::SamplerElem, i, H3DShaderRes::SampDefTexResI );
			if( texRes )
				samplerXML.setAttribute( "map", h3dGetResName( texRes ) );
		}
		QTexUnit *unit = new QTexUnit( samplerXML, m_texUnitCombo );
		m_texUnitCombo->addItem( sampler, QVariant::fromValue<void*>( unit ) );
		m_texUnitCombo->setItemData( m_texUnitCombo->count()-1, tip, Qt::ToolTipRole );
		m_texUnitCombo->setItemData( m_texUnitCombo->count()-1, tip, Qt::StatusTipRole);
	}

	QDomNodeList uniforms = m_materialXml.documentElement().elementsByTagName("Uniform");
	int numUniforms = h3dGetResElemCount( m_shaderHandle, H3DShaderRes::UniformElem );	
	for( int i = 0; i < numUniforms; ++i )
	{
		QString uniformName = h3dGetResParamStr( m_shaderHandle, H3DShaderRes::UniformElem, i, H3DShaderRes::UnifNameStr );
		QUniform* uni = 0;		
		for( int j = 0; j < uniforms.count(); ++j)
		{
			QDomElement uniform = uniforms.at(j).toElement();
			if( uniform.attribute("name") == uniformName )
			{
				uni = new QUniform( uniform, false );
				break;
			}	
		}
		if( !uni )
		{
			QDomElement uniform = m_materialXml.createElement( "Uniform" );
			uniform.setAttribute( "name", uniformName );
			uniform.setAttribute( "a", h3dGetResParamF( m_shaderHandle, H3DShaderRes::UniformElem, i, H3DShaderRes::UnifDefValueF4, 0 ) );
			uniform.setAttribute( "b", h3dGetResParamF( m_shaderHandle, H3DShaderRes::UniformElem, i, H3DShaderRes::UnifDefValueF4, 1 ) );
			uniform.setAttribute( "c", h3dGetResParamF( m_shaderHandle, H3DShaderRes::UniformElem, i, H3DShaderRes::UnifDefValueF4, 2 ) );
			uniform.setAttribute( "d", h3dGetResParamF( m_shaderHandle, H3DShaderRes::UniformElem, i, H3DShaderRes::UnifDefValueF4, 3 ) );
			uni = new QUniform( uniform, true, m_uniformCombo );
		}
		m_uniformCombo->addItem( uni->name(), QVariant::fromValue<void*>( uni ) );
	}


}
