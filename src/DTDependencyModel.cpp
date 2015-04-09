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
#include <qtextstream.h>
#include <qdebug.h>
#include <qdom.h>


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
        SetAttribute(id, DT::RelocateAttribute, false);
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
                switch (static_cast<DT::OutputItemType>(itemType))
                {
                    case DT::OutputDependencyType:
                        if (value.type() == QVariant::Bool)
                        {
                            result = GetIndex(root, type);
                            if (!result.isValid())
                            {
                                result = GetNewItemIndex(root, rowCount(root));
                                setData(result, tr("Relocate"));
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
bool DTDependencyModel::Serialize(QIODevice* pOutput)
{
    if (pOutput)
    {
        QDomDocument xmlDocument(QString::fromLatin1("DependencyProject"));
        QModelIndex root;
        QDomElement xmlRoot = xmlDocument.createElement(QString::fromLatin1("root"));
        xmlDocument.appendChild(xmlRoot);
        QList<QModelIndex> unprocessedNodes;
        QList<QDomElement> unprocessedElements;
        unprocessedNodes.push_back(root);
        unprocessedElements.push_back(xmlRoot);
        while (!unprocessedNodes.isEmpty())
        {
            root = unprocessedNodes.front();
            xmlRoot = unprocessedElements.front();
            unprocessedNodes.pop_front();
            unprocessedElements.pop_front();
            int rows = rowCount(root);
            for (int i = 0; i < rows; ++i)
            {
                QModelIndex id = index(i, 0, root);
                int type = data(id, DT::ItemTypeRole).toInt();
                QString name = data(id).toString();
                QDomElement child = xmlDocument.createElement(QString::fromLatin1("item"));
                child.setAttribute(QString::fromLatin1("name"), name);
                child.setAttribute(QString::fromLatin1("type"), type);
                switch (static_cast<DT::OutputItemType>(type))
                {
                    case DT::OutputFolderType:
                        break;
                    case DT::OutputBinaryType:
                        break;
                    case DT::OutputOtherFileType:
                        break;
                    case DT::OutputAttributeType:
                    {
                        QModelIndex attrId = index(i, 1, root);
                        int attrType = data(id, DT::AttributeTypeRole).toInt();
                        child.setAttribute(QString::fromLatin1("attrType"),
                                           attrType);
                        switch (static_cast<DT::AttributeType>(attrType))
                        {
                            case DT::PathAttribute:
                                child.setAttribute(QString::fromLatin1("value"),
                                                   data(attrId).toString());
                                break;
                            case DT::RelocateAttribute:
                                child.setAttribute(QString::fromLatin1("value"),
                                                   data(attrId).toInt());
                                break;
                            case DT::RelocatePathAttribute:
                                child.setAttribute(QString::fromLatin1("value"),
                                                   data(attrId).toString());
                                break;
                        }
                    }
                        break;
                    case DT::OutputDependencyType:
                        break;
                    case DT::OutputDependencyFolderType:
                        break;
                }
                unprocessedElements.append(child);
                unprocessedNodes.append(id);
                xmlRoot.appendChild(child);
            }
        }
        QTextStream stream(pOutput);
        xmlDocument.save(stream, 4, QDomNode::EncodingFromTextStream);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
 * @brief Restores data from an input.
 * @param pInput - a pointer to the input.
 * @return True if the operation completes successfully.
 */
bool DTDependencyModel::Restore(QIODevice* pInput)
{
    if (pInput)
    {
        QDomDocument xmlDocument(QString::fromLatin1("DependencyProject"));
        QTextStream stream(pInput);
        QString msg;
        int line = -1;
        int column = -1;
        if (xmlDocument.setContent(stream.readAll(), &msg, &line, &column))
        {
            // remove all rows
            removeRows(0, rowCount());
            m_filter.clear();
            // make new content
            QDomElement xmlRoot = xmlDocument.documentElement();
            QModelIndex root;
            QList<QModelIndex> unprocessedNodes;
            QList<QDomElement> unprocessedElements;
            unprocessedElements.push_back(xmlRoot);
            unprocessedNodes.push_back(root);
            while (!unprocessedElements.isEmpty())
            {
                xmlRoot = unprocessedElements.front();
                root = unprocessedNodes.front();
                unprocessedElements.pop_front();
                unprocessedNodes.pop_front();
                QDomElement child = xmlRoot.firstChildElement();
                while (!child.isNull())
                {
                    int type = child.attribute(QString::fromLatin1("type"))
                            .toInt();
                    QString name = child.attribute(QString::fromLatin1("name"));
                    QModelIndex id = GetNewItemIndex(root, rowCount(root));
                    setData(id, name);
                    setData(id, type, DT::ItemTypeRole);
                    switch (static_cast<DT::OutputItemType>(type))
                    {
                        case DT::OutputFolderType:
                            setData(id, QPixmap(QString::fromLatin1(":/images/folder.png"))
                                    .scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                    Qt::DecorationRole);
                            break;
                        case DT::OutputBinaryType:
                            setData(id, QPixmap(QString::fromLatin1(":/images/binaryfile.png"))
                                    .scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                    Qt::DecorationRole);
                            break;
                        case DT::OutputOtherFileType:
                            break;
                        case DT::OutputAttributeType:
                        {
                            QModelIndex attrId = index(id.row(), 1, root);
                            int attrType = child.attribute(
                                        QString::fromLatin1("attrType"))
                                    .toInt();
                            setData(id, attrType, DT::AttributeTypeRole);
                            setData(id, QPixmap(QString::fromLatin1(":/images/attribute.png"))
                                    .scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                    Qt::DecorationRole);
                            switch (static_cast<DT::AttributeType>(attrType))
                            {
                                case DT::PathAttribute:
                                {
                                    int rootType = data(root, DT::ItemTypeRole)
                                            .toInt();
                                    QString value = child.attribute(
                                                QString::fromLatin1("value"));
                                    if (rootType == static_cast<int>(
                                                DT::OutputDependencyType))
                                    {
                                        m_filter.insert(value);
                                    }
                                    setData(attrId, value);
                                }
                                    break;
                                case DT::RelocateAttribute:
                                    setData(attrId, static_cast<bool>(
                                                child.attribute(
                                                    QString::fromLatin1("value"))
                                                .toInt()));
                                    break;
                                case DT::RelocatePathAttribute:
                                    setData(attrId, child.attribute(
                                                QString::fromLatin1("value")));
                                    break;
                            }
                        }
                            break;
                        case DT::OutputDependencyType:
                            break;
                        case DT::OutputDependencyFolderType:
                            setData(id, QPixmap(QString::fromLatin1(":/images/dependencyfolder.png"))
                                    .scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                    Qt::DecorationRole);
                            break;
                    }
                    unprocessedElements.append(child);
                    unprocessedNodes.push_back(id);
                    child = child.nextSiblingElement();
                }
            }
            return true;
        }
        else
        {
            qDebug()<<QString::fromLatin1("Read project error: \"%1\"(line: %2, column: %3)")
                      .arg(msg).arg(line).arg(column);
        }
    }
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
