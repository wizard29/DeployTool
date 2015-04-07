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
#include "DTDependencyModel.h"


//------------------------------------------------------------------------------
/**
 * @class DTDependencyModel
 * @ingroup DeployTool
 * @brief The DTDependencyModel class  provides methods of dependency project list.
 */
//------------------------------------------------------------------------------
/**
 * @brief Constructor.
 * @param pParent - a pointer to a parent object.
 */
DTDependencyModel::DTDependencyModel(QObject* pParent)
    : QStandardItemModel(pParent)
{
    insertColumns(0, 2);
    setHeaderData(0, Qt::Horizontal, tr("Name"));
    setHeaderData(1, Qt::Horizontal, tr("Value"));
}

//------------------------------------------------------------------------------
/**
 * @brief Destructor.
 */
DTDependencyModel::~DTDependencyModel()
{
}

//------------------------------------------------------------------------------
/**
 * @brief Adds/updates a binary dependency.
 * @param name - the dependency name.
 * @param path - the dependency path.
 * @return An index of result or invalid index.
 */
QModelIndex DTDependencyModel::AddDependency(const QString& name,
                                             const QString& path)
{
    if (!m_filter.contains(path))
    {
        QModelIndex id = GetNewItemIndex(QModelIndex(), rowCount());
        setData(id, name);
        setData(id, DT::OutputDependencyType, DT::ItemTypeRole);
        SetAttribute(id, DT::PathAttribute, path);
        SetAttribute(id, DT::RelocatePathAttribute, QString());
        m_filter.insert(path);
        return id;
    }
    return QModelIndex();
}

//------------------------------------------------------------------------------
/**
 * @brief Cleanups binary dependencies that is not contained in the given dependency list.
 * @param root - a binary index.
 * @param deps - new dependency set.
 */
void DTDependencyModel::CleanupDependencies(const QSet<QString>& deps)
{
    int count = rowCount();
    for (int i = 0; i < count;)
    {
        QModelIndex id = index(i, 0);
        if (!deps.contains(GetAttribute(id, DT::PathAttribute).toString()))
        {
            m_filter.remove(GetAttribute(id, DT::PathAttribute).toString());
            removeRow(i);
            --count;
        }
        else
        {
            ++i;
        }
    }
}

//------------------------------------------------------------------------------
/**
 * @brief Returns a relocation string.
 * @param index - an index.
 */
QString DTDependencyModel::GetRelocation(const QModelIndex& index) const
{
    return GetAttribute(index, DT::RelocatePathAttribute).toString();
}

//------------------------------------------------------------------------------
/**
 * @brief Returns an attribute index.
 * @param id - the dependency index.
 * @param type - the attribute type.
 * @return Index of the attribute or invalid index.
 */
QModelIndex DTDependencyModel::GetIndex(const QModelIndex& id,
                                        DT::AttributeType type) const
{
    int count = rowCount(id);
    for (int i = 0; i < count; ++i)
    {
        QModelIndex childId = index(i, 0, id);
        if (childId.isValid())
        {
            int itemType = data(childId, DT::ItemTypeRole).toInt();
            if (itemType == static_cast<int>(DT::OutputAttributeType))
            {
                int attrType = data(childId, DT::AttributeTypeRole).toInt();
                if (attrType == static_cast<int>(type))
                {
                    return childId;
                }
            }
        }
    }
    return QModelIndex();
}

//------------------------------------------------------------------------------
/**
 * @brief Sets an item attribute and returns an index of result.
 * @param root - a root item of attribute.
 * @param type - an attribute type.
 * @param value - an attribute value.
 */
QModelIndex DTDependencyModel::SetAttribute(const QModelIndex& root,
                                            DT::AttributeType type,
                                            const QVariant& value)
{
    QModelIndex result;
    if (root.isValid())
    {
        int itemType = data(root, DT::ItemTypeRole).toInt();
        switch (type)
        {
            case DT::PathAttribute:
                switch (static_cast<DT::OutputItemType>(itemType))
                {
                    case DT::OutputDependencyType:
                        if (value.type() == QVariant::String)
                        {
                            result = GetIndex(root, type);
                            if (!result.isValid())
                            {
                                result = GetNewItemIndex(root, rowCount(root));
                                setData(result, tr("Path"));
                                setData(result, static_cast<int>(
                                            DT::OutputAttributeType),
                                        DT::ItemTypeRole);
                                setData(result, type, DT::AttributeTypeRole);
                                setData(result, QPixmap(QString::fromLatin1(":/images/attribute.png"))
                                        .scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                        Qt::DecorationRole);
                            }
                            QModelIndex id = index(result.row(), 1, root);
                            setData(id, value);
                        }
                        break;
                    default:
                        break;
                }
                break;
            case DT::RelocateAttribute:
                break;
            case DT::RelocatePathAttribute:
                switch (static_cast<DT::OutputItemType>(itemType))
                {
                    case DT::OutputDependencyType:
                        if (value.type() == QVariant::String)
                        {
                            result = GetIndex(root, type);
                            if (!result.isValid())
                            {
                                result = GetNewItemIndex(root, rowCount(root));
                                setData(result, tr("Relocation path"));
                                setData(result, static_cast<int>(
                                            DT::OutputAttributeType),
                                        DT::ItemTypeRole);
                                setData(result, type, DT::AttributeTypeRole);
                                setData(result, QPixmap(QString::fromLatin1(":/images/attribute.png"))
                                        .scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                        Qt::DecorationRole);
                            }
                            QModelIndex id = index(result.row(), 1, root);
                            setData(id, value);
                        }
                        break;
                    default:
                        break;
                }
                break;
        }
    }
    return result;
}

//------------------------------------------------------------------------------
/**
 * @brief Returns an attribute value.
 * @param root - an element index.
 * @param type - an attribute type.
 * @param defaultValue - the default value.
 */
QVariant DTDependencyModel::GetAttribute(const QModelIndex& root,
                                         DT::AttributeType type,
                                         const QVariant& defaultValue) const
{
    QModelIndex id = GetIndex(root, type);
    if (id.isValid())
    {
        id = index(id.row(), 1, root);
        Q_ASSERT(id.isValid());
        return data(id);
    }
    return defaultValue;
}

//------------------------------------------------------------------------------
/**
 * @brief Serializes a model.
 * @param pOutput - a pointer to an output.
 * @return True if the operation completes successfully.
 */
bool DTDependencyModel::Serialize(QIODevice* /*pOutput*/)
{
    return false;
}

//------------------------------------------------------------------------------
/**
 * @brief Restores data from an input.
 * @param pInput - a pointer to the input.
 * @return True if the operation completes successfully.
 */
bool DTDependencyModel::Restore(QIODevice* /*pInput*/)
{
    return false;
}

//------------------------------------------------------------------------------
/**
 * @brief Returns an index of new element.
 * @param root - a root index.
 * @param row - a destination row number.
 */
QModelIndex DTDependencyModel::GetNewItemIndex(const QModelIndex& root, int row)
{
    int count = rowCount(root);
    if (row > count)
    {
        row = count;
    }
    if (columnCount(root) == 0)
    {
        insertColumns(0, 2, root);
    }
    insertRow(row, root);
    return index(row, 0, root);
}
//------------------------------------------------------------------------------
