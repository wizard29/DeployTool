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
#ifndef DTDEPENDENCYMODEL_H
#define DTDEPENDENCYMODEL_H


#include <DTNamespace.h>
#include <qstandarditemmodel.h>
#include <qset.h>


class DTDependencyModel
        : public QStandardItemModel
{
    Q_OBJECT
    public:

        DTDependencyModel(QObject* pParent = nullptr);
        ~DTDependencyModel();

        QModelIndex AddDependency(const QString& name, const QString& path);
        void CleanupDependencies(const QSet<QString>& deps);
        QModelIndex GetIndex(const QModelIndex& index, DT::AttributeType type) const;
        QModelIndex SetAttribute(const QModelIndex& index, DT::AttributeType type,
                                 const QVariant& value);
        QVariant GetAttribute(const QModelIndex& index, DT::AttributeType type,
                              const QVariant& = QVariant()) const;
        bool Serialize(QIODevice* pOutput);
        bool Restore(QIODevice* pInput);

    private:

        QModelIndex GetNewItemIndex(const QModelIndex& root, int row);

    private:

        /// The dependency filter.
        QSet<QString> m_filter;
};//class DTDependencyModel
#endif // DTDEPENDENCYMODEL_H

