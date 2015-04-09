//==============================================================================
// Copyright: Reksoft Ltd.
//==============================================================================
//
// Author:              $Author:$
// Creation date:       2015.04.03
// Modification date:   $Date$
// Module:              DeployTool
// Platform:            MS Windows 7/8, MAC OS X 10.10.x
//
//------------------------------------------------------------------------------
// Note:
//------------------------------------------------------------------------------
// Warning:
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#ifndef DTDOCUMENTWINDOW_H
#define DTDOCUMENTWINDOW_H


#include <qwidget.h>
#include <qmap.h>


class QTreeView;
class DTDependencyManager;


class DTDocumentWindow
        : public QWidget
{
    Q_OBJECT
    public:

        DTDocumentWindow(QWidget* pParent = nullptr, Qt::WindowFlags f = 0);
        ~DTDocumentWindow();

    public slots:

        void OnOpen();
        void OnSave();
        void OnSaveAs();
        void OnAddFolder();
        void OnAddFiles();
        void OnAddNewFolder();
        void OnDeploy();
        void OnDeleteRow();
        void OnDependencyReady(int);
        void OnCopyReady(int);
        void OnOpenDependencies();
        void OnSaveDependencies();
        void OnSaveAsDependencies();
        void OnRefreshDependencies();
        void OnApplyForAllDependencies(const QModelIndex&);
        void OnCopyDependency(const QModelIndex&);        
        void OnCreateOSXBundleStructure();

    private:

        QModelIndex GetCurrentIndex() const;
        void Save(const QString& fileName);
        void SaveDependencies(const QString& fileName);
        bool BuildFolderStructure(const QString& rootPath);
        bool CopyOutputData(const QString& rootPath);
        void ReplaceToRelative(const QString& oldRoot, const QString& newRoot);
        QString GetAbsoluteInputPath(const QString& relative) const;
        static bool CleanFolder(const QString& path);

    private:

        /// A pointer to the output view.
        QTreeView* m_pOutputView;
        /// The project file name.
        QString m_projectFile;
        /// The copy error list.
        QStringList m_copyErrors;
        /// The relocation error map.
        QMap<QString, QStringList> m_relocationErrors;
        /// A pointer to the dependency manager.
        DTDependencyManager* m_pDependencies;
        /// The dependency file name.
        QString m_dependencyFile;
};//class DTDocumentWindow
#endif // DTDOCUMENTWINDOW_H
