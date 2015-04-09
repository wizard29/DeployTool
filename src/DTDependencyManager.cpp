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
#include "DTDependencyManager.h"
#include <DTDependencyModel.h>
#include <qgridlayout.h>
#include <qtreeview.h>
#include <qaction.h>
#include <qmenu.h>


//------------------------------------------------------------------------------
/**
 * @class DTDependencyManager
 * @ingroup DeployTool
 * @brief The DTDependencyManager class implements a widgets that represents information
 * about a binary dependencies of a deploy project.
 */
//------------------------------------------------------------------------------
/**
 * @brief Constructor.
 * @param pParent - a pointer to a parent widget.
 * @param f - the window flags.
 */
DTDependencyManager::DTDependencyManager(QWidget* pParent, Qt::WindowFlags f)
    : QWidget(pParent, f)
    , m_pDependencyView(nullptr)
    , m_pContextMenu(nullptr)
{
    setWindowTitle(tr("Dependencies"));
    DTDependencyModel* pModel = new DTDependencyModel(this);
    m_pDependencyView = new QTreeView(this);
    m_pDependencyView->setModel(pModel);
    QGridLayout* pLayout = new QGridLayout(this);
    pLayout->setMargin(0);
    pLayout->addWidget(m_pDependencyView, 0, 0);
    // create context menu
    m_pContextMenu = new QMenu(m_pDependencyView);
    QAction* pAction = m_pContextMenu->addAction(tr("Apply for all"));
    connect(pAction, SIGNAL(triggered()), this, SLOT(OnApplyForAll()));
    pAction = m_pContextMenu->addAction(tr("Copy to project"));
    connect(pAction, SIGNAL(triggered()), this, SLOT(OnCopy()));
    connect(m_pDependencyView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(OnMenuRequest(QPoint)));
    m_pDependencyView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_pDependencyView->setSortingEnabled(true);
    // create delete action
    pAction = new QAction(this);
    pAction->setShortcut(QKeySequence::Delete);
    pAction->setShortcutContext(Qt::WindowShortcut);
    m_pDependencyView->addAction(pAction);
    connect(pAction, SIGNAL(triggered()), this, SLOT(OnDelete()));
}

//------------------------------------------------------------------------------
/**
 * @brief Destructor.
 */
DTDependencyManager::~DTDependencyManager()
{
}

//------------------------------------------------------------------------------
/**
 * @brief Adds/updates dependency to a list.
 * @param name - the name of the dependency.
 * @param filePath - the file path of the dependency.
 */
void DTDependencyManager::AddDependency(const QString& name,
                                        const QString& filePath)
{
    DTDependencyModel* pModel = qobject_cast<DTDependencyModel*>(
                m_pDependencyView->model());
    Q_ASSERT(pModel);
    pModel->AddDependency(name, filePath);
}

//------------------------------------------------------------------------------
/**
 * @brief Refreshes the dependency list.
 * @param deps - the set of dependencies.
 */
void DTDependencyManager::Cleanup(const QSet<QString>& deps)
{
    DTDependencyModel* pModel = qobject_cast<DTDependencyModel*>(
                m_pDependencyView->model());
    Q_ASSERT(pModel);
    pModel->CleanupDependencies(deps);
}

//------------------------------------------------------------------------------
/**
 * @brief Returns relocation string by an index.
 * @param id - the dependency index.
 */
QString DTDependencyManager::GetRelocation(const QModelIndex& id) const
{
    DTDependencyModel* pModel = qobject_cast<DTDependencyModel*>(
                m_pDependencyView->model());
    Q_ASSERT(pModel);
    return pModel->GetAttribute(id, DT::RelocatePathAttribute).toString();
}

//------------------------------------------------------------------------------
/**
 * @brief Returns a dependency path by an index.
 * @param id - the dependency index.
 */
QString DTDependencyManager::GetPath(const QModelIndex& id) const
{
    DTDependencyModel* pModel = qobject_cast<DTDependencyModel*>(
                m_pDependencyView->model());
    Q_ASSERT(pModel);
    return pModel->GetAttribute(id, DT::PathAttribute).toString();
}

//------------------------------------------------------------------------------
/**
 * @brief Returns a dependency name by an index.
 * @param id - the dependency index.
 */
QString DTDependencyManager::GetName(const QModelIndex& id) const
{
    DTDependencyModel* pModel = qobject_cast<DTDependencyModel*>(
                m_pDependencyView->model());
    Q_ASSERT(pModel);
    return pModel->data(id).toString();
}

//------------------------------------------------------------------------------
/**
 * @brief Returns a dependency relocation state by an index.
 * @param id - the dependency index.
 */
bool DTDependencyManager::GetIsRelocate(const QModelIndex& id) const
{
    DTDependencyModel* pModel = qobject_cast<DTDependencyModel*>(
                m_pDependencyView->model());
    Q_ASSERT(pModel);
    return pModel->GetAttribute(id, DT::RelocateAttribute, false).toBool();
}

//------------------------------------------------------------------------------
/**
 * @brief Serializes the list.
 * @param pOutput - a pointer to an output device.
 * @return True if the operation completes successfully.
 */
bool DTDependencyManager::Serialize(QIODevice* pOutput)
{
    DTDependencyModel* pModel = qobject_cast<DTDependencyModel*>(
                m_pDependencyView->model());
    Q_ASSERT(pModel);
    return pModel->Serialize(pOutput);
}

//------------------------------------------------------------------------------
/**
 * @brief Restores the list.
 * @param pInput - a pointer to an input device.
 * @return True if the operation completes successfully.
 */
bool DTDependencyManager::Restore(QIODevice* pInput)
{
    DTDependencyModel* pModel = qobject_cast<DTDependencyModel*>(
                m_pDependencyView->model());
    Q_ASSERT(pModel);
    return pModel->Restore(pInput);
}

//------------------------------------------------------------------------------
/**
 * @brief Returns an index of the selected item.
 */
QModelIndex DTDependencyManager::GetSelectedItem() const
{
    QItemSelectionModel* pSel = m_pDependencyView->selectionModel();
    Q_ASSERT(pSel);
    QModelIndexList ids = pSel->selectedRows();
    if (ids.size() == 1)
    {
        return ids[0];
    }
    return QModelIndex();
}

//------------------------------------------------------------------------------
/**
 * @brief Show event handler.
 * emits the Visible(true) signal.
 */
void DTDependencyManager::showEvent(QShowEvent* pEvent)
{
    QWidget::showEvent(pEvent);
    emit Visible(true);
}

//------------------------------------------------------------------------------
/**
 * @brief Hide event handler.
 * emits the Visible(false) signal.
 */
void DTDependencyManager::hideEvent(QHideEvent* pEvent)
{
    QWidget::hideEvent(pEvent);
    emit Visible(false);
}

//------------------------------------------------------------------------------
/**
 * @brief Emits ApplyToAll signal.
 */
void DTDependencyManager::OnApplyForAll()
{
    emit ApplyForAll(GetSelectedItem());
}

//------------------------------------------------------------------------------
/**
 * @brief Emits Copy signal.
 */
void DTDependencyManager::OnCopy()
{
    emit Copy(GetSelectedItem());
}

//------------------------------------------------------------------------------
/**
 * @brief Shows the context menu if the request position is on the dependency item.
 * @param pos - the request position.
 */
void DTDependencyManager::OnMenuRequest(const QPoint& pos)
{
    QModelIndex id = m_pDependencyView->indexAt(pos);
    if (id.isValid())
    {
        DTDependencyModel* pModel = qobject_cast<DTDependencyModel*>(
                    m_pDependencyView->model());
        Q_ASSERT(pModel);
        int type = pModel->data(id, DT::ItemTypeRole).toInt();
        if (type == static_cast<int>(DT::OutputDependencyType))
        {
            m_pContextMenu->popup(m_pDependencyView->mapToGlobal(pos));
        }
    }
}

//------------------------------------------------------------------------------
/**
 * @brief Deletes a selected dependency or does nothing.
 */
void DTDependencyManager::OnDelete()
{
    QModelIndex id = GetSelectedItem();
    if (id.isValid())
    {
        DTDependencyModel* pModel = qobject_cast<DTDependencyModel*>(
                    m_pDependencyView->model());
        Q_ASSERT(pModel);
        int type = pModel->data(id, DT::ItemTypeRole).toInt();
        if (type == static_cast<int>(DT::OutputDependencyType))
        {
            pModel->removeRow(id.row(), id.parent());
        }
    }
}

//------------------------------------------------------------------------------
