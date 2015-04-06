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
#include "DTDocumentWindow.h"
#include <qtconcurrentmap.h>
#include <qprogressdialog.h>
#include <qfuturewatcher.h>
#include <DTOutputModel.h>
#include <qinputdialog.h>
#include <qfiledialog.h>
#include <qgridlayout.h>
#include <qtreeview.h>
#include <qmenubar.h>
#include <functional>
#include <qprocess.h>
#include <algorithm>
#include <qdebug.h>


//------------------------------------------------------------------------------
/**
 * @struct DependencyJob
 * @ingroup DeployTool
 * @brief The DeployJob struct contains data of dependency discovery.
 */
//------------------------------------------------------------------------------
struct DependencyJob
{
    public:

        /// A binary position in the model.
        QModelIndex m_position;
        /// A binary file path.
        QString m_binary;
};//struct DependencyJob

//------------------------------------------------------------------------------
/**
 * @struct DependencyResult
 * @ingroup DeployTool
 * @brief The DependencyResult struct contains data of dependencies.
 */
//------------------------------------------------------------------------------
struct DependencyResult
{
    public:

        /// A binary position in the model.
        QModelIndex m_position;
        /// A binary file path list.
        QStringList m_binaries;
};//struct DependencyResult

//------------------------------------------------------------------------------
/**
 * @brief Scans a dependencies.
 * @param job - job data.
 * @return List of dependencies.
 */
DependencyResult ScanDependencies(const DependencyJob& job)
{
    DependencyResult result;
    result.m_position = job.m_position;
#ifdef Q_OS_MAC
    QProcess process;
    QStringList args;
    args<<"-L";
    args<<data.m_binary;
    process.start("otool", args);
    if (process.waitForStarted())
    {
        QByteArray processData;
        while (process.waitForReadyRead())
        {
            processData.append(process.readAll());
        }
        QStringList lines = QString::fromLocal8Bit(processData)
                .split(QString::fromLatin1("\n\t"));
        processData.clear();
        int count = lines.size();
        for (int i = 0; i < count; ++i)
        {
            QString line = lines[i];
            line.replace(QRegExp("\\(.*\\)"), QString());
            line.replace(QRegExp("^\\( *\\)"), QString());
            line.replace(QRegExp("\\( *\\)$"), QString());
            line.replace(QRegExp(":"), QString());
            line.replace(QString::fromLatin1("\t"), QString());
            line.replace(QRegExp("[^A-Za-z0-9\\.\\*-+]$"), QString());
            if (!line.isEmpty())
            {
                if (line != data.m_binary)
                {
                    result.m_binaries.append(line);
                }
            }
        }
    }
#endif
    return result;
}


//------------------------------------------------------------------------------
/**
 * @class DTDocumentWindow
 * @ingroup DeployTool
 * @brief The DTDocumentWindow class provides a view of a deployment project.
 */
//------------------------------------------------------------------------------
/**
 * @brief Creates a deployment view.
 * @param pParent - a pointer to a parent widget.
 * @param f - a window flags.
 */
DTDocumentWindow::DTDocumentWindow(QWidget* pParent, Qt::WindowFlags f)
    : QWidget(pParent, f)
    , m_pOutputView(nullptr)
{
    m_pOutputView = new QTreeView(this);
    m_pOutputView->setModel(new DTOutputModel(this));
    QMenuBar* pMenuBar = new QMenuBar(this);
    // form the "File" menu
    QMenu* pMenu = pMenuBar->addMenu(tr("File"));
    QAction* pAction = pMenu->addAction(tr("Open"));
    pAction->setShortcut(QKeySequence::Open);
    connect(pAction, SIGNAL(triggered()), this, SLOT(OnOpen()));
    pAction = pMenu->addAction(tr("Save"));
    pAction->setShortcut(QKeySequence::Save);
    connect(pAction, SIGNAL(triggered()), this, SLOT(OnSave()));
    pAction = pMenu->addSeparator();
    pAction = pMenu->addAction(tr("Exit"));
    connect(pAction, SIGNAL(triggered()), this, SLOT(close()));
    // form the "Project" menu
    pMenu = pMenuBar->addMenu(tr("Project"));
    pAction = pMenu->addAction(tr("Add folder"));
    pAction->setShortcut(Qt::CTRL + Qt::Key_P);
    connect(pAction, SIGNAL(triggered()), this, SLOT(OnAddFolder()));
    pAction = pMenu->addAction(tr("Add files"));
    pAction->setShortcut(Qt::CTRL + Qt::Key_F);
    connect(pAction, SIGNAL(triggered()), this, SLOT(OnAddFiles()));
    pAction = pMenu->addAction(tr("Add new folder"));
    pAction->setShortcut(Qt::CTRL + Qt::Key_N);
    connect(pAction, SIGNAL(triggered()), this, SLOT(OnAddNewFolder()));
    pAction = pMenu->addSeparator();
    pAction = pMenu->addAction(tr("Refresh dependencies"));
    pAction->setShortcut(QKeySequence::Refresh);
    connect(pAction, SIGNAL(triggered()), this, SLOT(OnRefreshDependencies()));
    pAction = pMenu->addSeparator();
    pAction = pMenu->addAction(tr("Deploy"));
    pAction->setShortcut(Qt::CTRL + Qt::Key_D);
    connect(pAction, SIGNAL(triggered()), this, SLOT(OnDeploy()));
    // form other actions
    pAction = new QAction(tr("Delete item"), m_pOutputView);
    m_pOutputView->addAction(pAction);
    pAction->setShortcut(Qt::Key_Delete);
    pAction->setShortcutContext(Qt::WidgetShortcut);
    connect(pAction, SIGNAL(triggered()), this, SLOT(OnDeleteRow()));
    // setup layout
    QGridLayout* pLayout = new QGridLayout(this);
    pLayout->addWidget(pMenuBar);
    pLayout->addWidget(m_pOutputView, 1, 0);
    pLayout->setMargin(0);
}

//------------------------------------------------------------------------------
/**
 * @brief Destructor.
 */
DTDocumentWindow::~DTDocumentWindow()
{
}


//------------------------------------------------------------------------------
/**
 * @brief Loads a project form a file.
 */
void DTDocumentWindow::OnOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open a project"),
                                                    QString(),
                                                    tr("Project files (*.deploy)"));
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly))
        {
            DTOutputModel* pModel = qobject_cast<DTOutputModel*>(
                        m_pOutputView->model());
            Q_ASSERT(pModel);
            if (!pModel->Restore(&file))
            {
                qDebug()<<QString::fromLatin1("Load project error. file: \"%1\" line:%2")
                          .arg(__FILE__).arg(__LINE__);
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
 * @brief Saves the project.
 */
void DTDocumentWindow::OnSave()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save the project"),
                                                    QString(),
                                                    tr("Project files (*.deploy)"));
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (file.open(QFile::WriteOnly | QFile::Truncate))
        {
            DTOutputModel* pModel = qobject_cast<DTOutputModel*>(
                        m_pOutputView->model());
            Q_ASSERT(pModel);
            if (!pModel->Serialize(&file))
            {
                qDebug()<<QString::fromLatin1("Save project error. file: \"%1\" line:%2")
                          .arg(__FILE__).arg(__LINE__);
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
 * @brief Adds a folder.
 */
void DTDocumentWindow::OnAddFolder()
{
}

//------------------------------------------------------------------------------
/**
 * @brief Adds a files.
 */
void DTDocumentWindow::OnAddFiles()
{
    QModelIndex id = GetCurrentIndex();
    DTOutputModel* pModel = qobject_cast<DTOutputModel*>(
                m_pOutputView->model());
    Q_ASSERT(pModel);
    int type = -1;
    if (id.isValid())
    {
        type = pModel->data(id, DT::ItemTypeRole).toInt();
    }
    else
    {
        type = static_cast<int>(DT::OutputFolderType);
    }
    if (type == static_cast<int>(DT::OutputFolderType))
    {
        QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Select files"),
                                                              QString(),
                                                              tr("Files (*)"));
        int count = fileNames.size();
        for (int i = 0; i < count; ++i)
        {
            QString fileName = fileNames[i];
            QFileInfo fi(fileName);
            if (!fileName.isEmpty() && fi.exists())
            {
                pModel->AddFile(id, fileName);
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
 * @brief Adds new empty folder.
 */
void DTDocumentWindow::OnAddNewFolder()
{
    QModelIndex id = GetCurrentIndex();
    DTOutputModel* pModel = qobject_cast<DTOutputModel*>(
                m_pOutputView->model());
    Q_ASSERT(pModel);
    int type = -1;
    if (id.isValid())
    {
        type = pModel->data(id, DT::ItemTypeRole).toInt();
    }
    else
    {
        type = static_cast<int>(DT::OutputFolderType);
    }
    if (type == static_cast<int>(DT::OutputFolderType))
    {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Enter folder name"),
                                            tr("Folder name:"), QLineEdit::Normal,
                                            tr("New folder"), &ok);
        if (ok)
        {
            pModel->AddFolder(id, name);
        }
    }
}

//------------------------------------------------------------------------------
/**
 * @brief Refreshes a dependencies.
 */
void DTDocumentWindow::OnRefreshDependencies()
{
    QProgressDialog progress(this);
    QList<DependencyJob> jobs;
    QFutureWatcher<DependencyResult> watcher;
    connect(&watcher, SIGNAL(progressRangeChanged(int,int)),
            &progress, SLOT(setRange(int,int)));
    connect(&watcher, SIGNAL(progressValueChanged(int)),
            &progress, SLOT(setValue(int)));
    connect(&watcher, SIGNAL(resultReadyAt(int)),
            this, SLOT(OnDependencyReady(int)));
    watcher.setFuture(QtConcurrent::mapped(jobs, &ScanDependencies));
    progress.exec();
}

//------------------------------------------------------------------------------
/**
 * @brief Starts deploy the project.
 */
void DTDocumentWindow::OnDeploy()
{
}

//------------------------------------------------------------------------------
/**
 * @brief Deletes a selected row.
 */
void DTDocumentWindow::OnDeleteRow()
{
    QModelIndex id = GetCurrentIndex();
    if (id.isValid())
    {
        DTOutputModel* pModel = qobject_cast<DTOutputModel*>(
                    m_pOutputView->model());
        Q_ASSERT(pModel);
        int type = pModel->data(id, DT::ItemTypeRole).toInt();
        switch (static_cast<DT::OutputItemType>(type))
        {
            case DT::OutputFolderType:
            case DT::OutputBinaryType:
            case DT::OutputOtherFileType:
                pModel->removeRow(id.row(), id.parent());
                break;
            case DT::OutputAttributeType:
                break;
            case DT::OutputDependencyType:
                break;
            case DT::OutputDependencyFolderType:
                break;
        }
    }
}

//------------------------------------------------------------------------------
/**
 * @brief Calls when dependency job ready.
 * @param index - a result index.
 */
void DTDocumentWindow::OnDependencyReady(int index)
{
    QFutureWatcherBase* pWatcherBase = qobject_cast<QFutureWatcherBase*>(sender());
    Q_ASSERT(pWatcherBase);
    QFutureWatcher<DependencyResult>* pWatcher = static_cast<QFutureWatcher<DependencyResult>*>(
                pWatcherBase);
    Q_ASSERT(pWatcher);
    DependencyResult result = pWatcher->resultAt(index);
    DTOutputModel* pModel = qobject_cast<DTOutputModel*>(m_pOutputView->model());
    Q_ASSERT(pModel);
    pModel->CleanupDependencies(result.m_position, result.m_binaries);
    std::for_each(result.m_binaries.begin(), result.m_binaries.end(),
                  [&](const QString& fileName)
    {
        QFileInfo fi(fileName);
        pModel->AddDependency(result.m_position, fi.fileName(), fileName);
    });
}

//------------------------------------------------------------------------------
/**
 * @brief Returns a selected index.
 */
QModelIndex DTDocumentWindow::GetCurrentIndex() const
{
    QItemSelectionModel* pSel = m_pOutputView->selectionModel();
    Q_ASSERT(pSel);
    QModelIndexList ids = pSel->selectedRows();
    if (ids.size() == 1)
    {
        return ids[0];
    }
    return QModelIndex();
}
//------------------------------------------------------------------------------
