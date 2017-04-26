#ifndef HORDEMODELDIALOG_H_
#define HORDEMODELDIALOG_H_

#include <HordeFileDialog.h>

class GLWidget;
class HordeSceneEditor;

class HordeModelDialog : public HordeFileDialog
{
	Q_OBJECT
public:
	HordeModelDialog(const QString& targetPath, QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~HordeModelDialog();

		/**
	 * Returns the selected model filename relative to the currently set Horde3D scenegraph path
     * @param parent widget the HordeFileDialog will be child of
	 * @param caption string to be displayed as window caption
	 * @return QString filename of the selected model
	 */
	static QString getModelFile(const QString& targetPath, QWidget* parent, const QString& caption);

protected slots:
	virtual void itemChanged(QListWidgetItem* current, QListWidgetItem* previous);

private slots:
	void importCollada();

private:
	void initModelViewer();

	void loadModel(const QString& fileName, bool repoFile);

	GLWidget*				m_glWidget;
	QPushButton*			m_importButton;
    HordeSceneEditor*       m_editorInstance;
    QWidget*                m_glParentOriginal;
    int                     m_oldCameraID;

	unsigned int			m_oldScene;
	unsigned int			m_newScene;
	unsigned int			m_currentModel;
	unsigned int			m_cameraID;

};
#endif
