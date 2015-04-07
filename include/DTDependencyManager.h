//==============================================================================
// Copyright: Reksoft Ltd.
//==============================================================================
//
// Author:              $Author:$
// Creation date:       2015.04.07
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
#ifndef DTDEPENDENCYMANAGER_H
#define DTDEPENDENCYMANAGER_H


#include <qwidget.h>


class QTreeView;


class DTDependencyManager
        : public QWidget
{
    Q_OBJECT
    public:

        DTDependencyManager(QWidget* pParent = nullptr, Qt::WindowFlags f = 0);
        ~DTDependencyManager();

        void AddDependency(const QString& name, const QString& filePath);
        void Cleanup(const QSet<QString>& deps);
        bool Serialize(QIODevice* pOutput);
        bool Restore(QIODevice* pInput);

    signals:

        void Visible(bool);

    protected:

        void showEvent(QShowEvent*);
        void hideEvent(QHideEvent*);

    private:

        /// A pointer to the view.
        QTreeView* m_pDependencyView;
};//class DTDependencyManager
#endif // DTDEPENDENCYMANAGER_H

