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


class QTreeView;
class QProgressBar;


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
        void OnRefreshDependencies();
        void OnDeploy();
        void OnDeleteRow();
        void OnDependencyReady(int);

    private:

        QModelIndex GetCurrentIndex() const;
        void Save(const QString& fileName);

    private:

        /// A pointer to the output view.
        QTreeView* m_pOutputView;
        /// A project file name.
        QString m_projectFile;
};//class DTDocumentWindow
#endif // DTDOCUMENTWINDOW_H
