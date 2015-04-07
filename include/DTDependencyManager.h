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
class QMenu;


class DTDependencyManager
        : public QWidget
{
    Q_OBJECT
    public:

        DTDependencyManager(QWidget* pParent = nullptr, Qt::WindowFlags f = 0);
        ~DTDependencyManager();

        void AddDependency(const QString& name, const QString& filePath);
        void Cleanup(const QSet<QString>& deps);
        QString GetRelocation(const QModelIndex&) const;
        QString GetPath(const QModelIndex&) const;
        QString GetName(const QModelIndex&) const;
        bool GetIsRelocate(const QModelIndex&) const;
        bool Serialize(QIODevice* pOutput);
        bool Restore(QIODevice* pInput);
        QModelIndex GetSelectedItem() const;

    signals:

        void Visible(bool);
        void ApplyForAll(const QModelIndex& id);
        void Copy(const QModelIndex& id);

    protected:

        void showEvent(QShowEvent*);
        void hideEvent(QHideEvent*);

    private slots:

        void OnApplyForAll();
        void OnCopy();
        void OnMenuRequest(const QPoint& pos);

    private:

        /// A pointer to the view.
        QTreeView* m_pDependencyView;
        /// A pointer to the context menu.
        QMenu* m_pContextMenu;
};//class DTDependencyManager
#endif // DTDEPENDENCYMANAGER_H

