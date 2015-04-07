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


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
DTDependencyManager::DTDependencyManager(QWidget* pParent, Qt::WindowFlags f)
    : QWidget(pParent, f)
    , m_pDependencyView(nullptr)
{
    setWindowTitle(tr("Dependencies"));
    DTDependencyModel* pModel = new DTDependencyModel(this);
    m_pDependencyView = new QTreeView(this);
    m_pDependencyView->setModel(pModel);
    QGridLayout* pLayout = new QGridLayout(this);
    pLayout->setMargin(0);
    pLayout->addWidget(m_pDependencyView, 0, 0);
}

//------------------------------------------------------------------------------
DTDependencyManager::~DTDependencyManager()
{
}

//------------------------------------------------------------------------------
void DTDependencyManager::AddDependency(const QString& name,
                                        const QString& filePath)
{
    DTDependencyModel* pModel = qobject_cast<DTDependencyModel*>(
                m_pDependencyView->model());
    Q_ASSERT(pModel);
    pModel->AddDependency(name, filePath);
}

//------------------------------------------------------------------------------
void DTDependencyManager::Cleanup(const QSet<QString>& deps)
{
    DTDependencyModel* pModel = qobject_cast<DTDependencyModel*>(
                m_pDependencyView->model());
    Q_ASSERT(pModel);
    pModel->CleanupDependencies(deps);
}

//------------------------------------------------------------------------------
bool DTDependencyManager::Serialize(QIODevice* pOutput)
{
    DTDependencyModel* pModel = qobject_cast<DTDependencyModel*>(
                m_pDependencyView->model());
    Q_ASSERT(pModel);
    return pModel->Serialize(pOutput);
}

//------------------------------------------------------------------------------
bool DTDependencyManager::Restore(QIODevice* pInput)
{
    DTDependencyModel* pModel = qobject_cast<DTDependencyModel*>(
                m_pDependencyView->model());
    Q_ASSERT(pModel);
    return pModel->Restore(pInput);
}

//------------------------------------------------------------------------------
void DTDependencyManager::showEvent(QShowEvent* pEvent)
{
    QWidget::showEvent(pEvent);
    emit Visible(true);
}

//------------------------------------------------------------------------------
void DTDependencyManager::hideEvent(QHideEvent* pEvent)
{
    QWidget::hideEvent(pEvent);
    emit Visible(false);
}
//------------------------------------------------------------------------------
