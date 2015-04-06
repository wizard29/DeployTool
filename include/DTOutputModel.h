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
#ifndef DTOUTPUTMODEL_H
#define DTOUTPUTMODEL_H


#include <DTNamespace.h>
#include <qstandarditemmodel.h>


class DTOutputModel
        : public QStandardItemModel
{
    Q_OBJECT
    public:

        DTOutputModel(QObject* pParent = nullptr);
        ~DTOutputModel();

        QModelIndex AddFolder(const QModelIndex& root, const QString& name);
        QModelIndex AddFile(const QModelIndex& root, const QString& filePath);
        QModelIndex SetAttribute(const QModelIndex& root, DT::AttributeType type,
                                 const QVariant& value);
        QVariant GetAttribute(const QModelIndex& root, DT::AttributeType type,
                              const QVariant& = QVariant()) const;
        QModelIndex AddDependency(const QModelIndex& root, const QString& name,
                                  const QString& path);
        void CleanupDependencies(const QModelIndex& root, const QStringList& deps);
        QMap<QString, QString> GetRelocations(const QModelIndex& root) const;
        QModelIndex GetIndex(const QModelIndex& root, DT::AttributeType type) const;
        bool Serialize(QIODevice* pOutput);
        bool Restore(QIODevice* pInput);

    private:

        QModelIndex GetNewItemIndex(const QModelIndex& root, int row);
        QModelIndex GetDependencyFolder(const QModelIndex& root);
        QModelIndex GetDependencyFolder(const QModelIndex& root) const;
        QModelIndex GetDependency(const QModelIndex& root,
                                  const QString& fileName) const;
};//class DTOutputModel
#endif // DTOUTPUTMODEL_H

