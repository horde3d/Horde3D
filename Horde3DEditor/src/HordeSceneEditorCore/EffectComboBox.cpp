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

#include "EffectComboBox.h"

#include <QtCore/QDir>
#include "HordeFileDialog.h"

EffectComboBox::EffectComboBox(QWidget* parent /*= 0*/) : QComboBox(parent)
{	
	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(currentChanged(int)));	
}


EffectComboBox::~EffectComboBox()
{
}

void EffectComboBox::init(const QString& resourcePath)
{
	clear();	
	m_resourcePath = resourcePath;
	blockSignals(true);
	addEffects(resourcePath, resourcePath);
	addItem(tr("Import from Repository"), QVariant((int) QVariant::UserType));
	if (count() == 1)
		setCurrentIndex(-1);
	blockSignals(false);
}

QString EffectComboBox::effectFile() const
{
	return currentText();
}

Effect EffectComboBox::effect() const
{
	return Effect(effectFile());
}

void EffectComboBox::setEffect(Effect effect)
{
	blockSignals(true);
	setCurrentIndex(findText(effect.FileName));
	blockSignals(false);
	m_init = effect.FileName;
}

void EffectComboBox::currentChanged(int index)
{
	if (itemData(index).isValid() && itemData(index) == QVariant((int)QVariant::UserType))
	{
		QString newEffect = HordeFileDialog::getResourceFile( H3DResTypes::ParticleEffect, m_resourcePath, this, tr("Select effect to import"));
		if (!newEffect.isEmpty())
		{
			if (findText(newEffect) == -1)
			{
				removeItem(index);
				addItem(newEffect);
				addItem(tr("Import from Repository"), QVariant(QVariant::UserType));
			}
			setCurrentIndex(findText(newEffect));
			return;
		}
		else
		{
			setCurrentIndex(findText(m_init));
			return;
		}
	}
	if (m_init != currentText())
	{
		emit editFinished();
		emit effectChanged();
	}
}

void EffectComboBox::addEffects(const QDir& base, const QString dir)
{	
	QList<QFileInfo> effects = QDir(dir).entryInfoList(QStringList("*.particle.xml"), QDir::Files | QDir::Readable);
	foreach(QFileInfo effect, effects)
	{
		addItem(base.relativeFilePath(effect.absoluteFilePath()));
	}	
	QList<QFileInfo> effectDirs = QDir(dir).entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
	foreach(QFileInfo directory, effectDirs)
	{
		addEffects(base, directory.absoluteFilePath());
	}
}
