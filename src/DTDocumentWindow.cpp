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
#include <DTDependencyManager.h>
#include <DTDependencyModel.h>
#include <qtconcurrentmap.h>
#include <qprogressdialog.h>
#include <qfuturewatcher.h>
#include <DTOutputModel.h>
#include <qinputdialog.h>
#include <qfiledialog.h>
#include <qgridlayout.h>
#include <qmessagebox.h>
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
 * @struct DependencyJob
 * @ingroup DeployTool
 * @brief The DeployJob struct contains data for copy job execution.
 */
//------------------------------------------------------------------------------
struct CopyJob
{
    public:

        /// A binary position in the model.
        QModelIndex m_position;
        /// A binary source path.
        QString m_source;
        /// A binary destination path.
        QString m_destination;
        /// The relocation map.
        QMap<QString, QString> m_relocationMap;
};//struct CopyJob

//------------------------------------------------------------------------------
/**
 * @struct CopyResult
 * @ingroup DeployTool
 * @brief The CopyResult struct contains data of copy job execution.
 */
//------------------------------------------------------------------------------
struct CopyResult
{
    public:

        /// A binary position in the model.
        QModelIndex m_position;
        /// A binary source path.
        QString m_source;
        /// A binary destination path.
        QString m_destination;
        /// The error code.
        int m_errors;
};//struct CopyResult

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
    args<<job.m_binary;
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
                if (line != job.m_binary)
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
inline void RelocateDependency(const QString& oldSource,
                               const QString& relocation,
                               const QString& destination)
{
#ifdef Q_OS_MAC
    QString oldPath = oldSource;
    QFileInfo fi(oldPath);
    QString newPath = fi.fileName();
    fi.setFile(destination);
    QString destinatioName = fi.fileName();
    fi.setFile(oldPath);
    QString oldPathName = fi.fileName();
    QProcess process;
    QStringList args;
    if (oldPathName == destinatioName)
    {
        args<<"-id";
        if (!relocation.isEmpty())
        {
            args<<(relocation + QString("/") + newPath);
        }
        else
        {
            args<<newPath;
        }
        args<<destination;
    }
    else
    {
        args<<"-change";
        args<<oldPath;
        if (!relocation.isEmpty())
        {
            args<<(relocation + QString("/") + newPath);
        }
        else
        {
            args<<newPath;
        }
        args<<destination;
    }
    process.start("install_name_tool", args);
    if (process.waitForStarted())
    {
        QByteArray processData;
        while (process.waitForReadyRead())
        {
            processData.append(process.readAll());
        }
    }
    process.waitForFinished();
#else
    Q_UNUSED(oldSource);
    Q_UNUSED(relocation);
    Q_UNUSED(destination);
#endif
}

//------------------------------------------------------------------------------
/**
 * @brief Copies a file and relocates if it needs.
 * @param job - a job data.
 */
CopyResult CopyFile(const CopyJob& job)
{
    CopyResult result;
    result.m_position = job.m_position;
    result.m_source = job.m_source;
    result.m_destination = job.m_destination;
    result.m_errors = 0;
    if (!QFile::copy(job.m_source, job.m_destination))
    {
        result.m_errors = 1;
    }
    else
    {
        // relocate
        if (job.m_relocationMap.size() != 0)
        {
            QMap<QString, QString>::ConstIterator pos = job.m_relocationMap.begin();
            QMap<QString, QString>::ConstIterator end = job.m_relocationMap.end();
            for (; pos != end; ++pos)
            {
                RelocateDependency(pos.key(), pos.value(), job.m_destination);
            }
        }
    }
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
    , m_projectFile()
    , m_pDependencies(nullptr)
{    
    m_pOutputView = new QTreeView(this);
    m_pOutputView->setAlternatingRowColors(true);
    m_pOutputView->setModel(new DTOutputModel(this));
    // setup dependency manager
    m_pDependencies = new DTDependencyManager(this, Qt::Window);
    m_pDependencies->setVisible(false);
    connect(m_pDependencies, SIGNAL(ApplyForAll(QModelIndex)),
            this, SLOT(OnApplyForAllDependencies(QModelIndex)));
    connect(m_pDependencies, SIGNAL(Copy(QModelIndex)),
            this, SLOT(OnCopyDependency(QModelIndex)));
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
    pAction = pMenu->addAction(tr("Deploy"));
    pAction->setShortcut(Qt::CTRL + Qt::Key_D);
    connect(pAction, SIGNAL(triggered()), this, SLOT(OnDeploy()));
    // form dependencies menu
    pMenu = pMenuBar->addMenu(tr("Dependencies"));
    pAction = pMenu->addAction(tr("Open"));
    connect(pAction, SIGNAL(triggered()), this, SLOT(OnOpenDependencies()));
    pAction = pMenu->addAction(tr("Save"));
    connect(pAction, SIGNAL(triggered()), this, SLOT(OnSaveDependencies()));
    pAction = pMenu->addSeparator();
    pAction = pMenu->addAction(tr("Refresh"));
    pAction->setShortcut(QKeySequence::Refresh);
    connect(pAction, SIGNAL(triggered()), this, SLOT(OnRefreshDependencies()));
    pAction = pMenu->addSeparator();
    pAction = pMenu->addAction(tr("Manage"));
    pAction->setCheckable(true);
    pAction->setChecked(false);
    connect(pAction, SIGNAL(toggled(bool)),
            m_pDependencies, SLOT(setVisible(bool)));
    connect(m_pDependencies, SIGNAL(Visible(bool)),
            pAction, SLOT(setChecked(bool)));
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
            else
            {
                m_projectFile = fileName;
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
    if (m_projectFile.isEmpty())
    {
        OnSaveAs();
    }
    else
    {
        Save(m_projectFile);
    }
}

//------------------------------------------------------------------------------
/**
 * @brief Save the project as.
 */
void DTDocumentWindow::OnSaveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save the project"),
                                                    QString(),
                                                    tr("Project files (*.deploy)"));
    Save(fileName);
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
 * @brief Starts deploy the project.
 */
void DTDocumentWindow::OnDeploy()
{
    QString rootPath = QFileDialog::getExistingDirectory(this,
                                                         tr("Select an output folder"));
    if (!rootPath.isEmpty())
    {
        // cleanup folder.
        if (!CleanFolder(rootPath))
        {
            if (QMessageBox::warning(this, tr("Warning"),
                                     tr("Clean destination folder error."),
                                     QMessageBox::Abort, QMessageBox::Ignore) !=
                QMessageBox::Ignore)
            {
                return;
            }
        }
        // build folder structure.
        if (!BuildFolderStructure(rootPath))
        {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Build an output folder structure failed."),
                                  QMessageBox::Close);
            return;
        }
        // copy files and relocate data for binaries.
        if (!CopyOutputData(rootPath))
        {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Copy files error."), QMessageBox::Close);
        }
        m_copyErrors.clear();
    }
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
        QString name = fi.fileName();
        pModel->AddDependency(result.m_position, name, fileName);
        m_pDependencies->AddDependency(name, fileName);
    });
}

//------------------------------------------------------------------------------
/**
 * @brief Calls when copy job ready.
 * @param index - a result index.
 */
void DTDocumentWindow::OnCopyReady(int index)
{
    QFutureWatcherBase* pWatcherBase = qobject_cast<QFutureWatcherBase*>(sender());
    Q_ASSERT(pWatcherBase);
    QFutureWatcher<CopyResult>* pWatcher = static_cast<QFutureWatcher<CopyResult>*>(
                pWatcherBase);
    Q_ASSERT(pWatcher);
    CopyResult result = pWatcher->resultAt(index);
    if (result.m_errors != 0)
    {
        m_copyErrors.append(result.m_source);
    }
}

//------------------------------------------------------------------------------
/**
 * @brief Loads dependencies from a file.
 */
void DTDocumentWindow::OnOpenDependencies()
{
}

//------------------------------------------------------------------------------
/**
 * @brief Saves dependencies to a file.
 */
void DTDocumentWindow::OnSaveDependencies()
{
}

//------------------------------------------------------------------------------
/**
 * @brief Refreshes a dependencies.
 */
void DTDocumentWindow::OnRefreshDependencies()
{
    QList<DependencyJob> jobs;
    // create job list
    DTOutputModel* pModel = qobject_cast<DTOutputModel*>(m_pOutputView->model());
    Q_ASSERT(pModel);
    // go througth all binary items.
    QList<QModelIndex> unprocessedNodes;
    unprocessedNodes.push_back(QModelIndex());
    while (!unprocessedNodes.isEmpty())
    {
        QModelIndex root = unprocessedNodes.front();
        unprocessedNodes.pop_front();
        int rows = pModel->rowCount(root);
        for (int i = 0; i < rows; ++i)
        {
            QModelIndex id = pModel->index(i, 0, root);
            int type = pModel->data(id, DT::ItemTypeRole).toInt();
            if (type == static_cast<int>(DT::OutputBinaryType))
            {
                QString fileName = pModel->GetAttribute(id, DT::PathAttribute).toString();
                if (!fileName.isEmpty())
                {
                    DependencyJob job;
                    job.m_position = id;
                    job.m_binary = fileName;
                    jobs.append(job);
                }
            }
            else if (type == static_cast<int>(DT::OutputFolderType))
            {
                unprocessedNodes.append(id);
            }
        }
    }
    if (jobs.size())
    {
        QProgressDialog progress(this);
        progress.setWindowTitle(tr("Dependency discovering"));
        progress.setCancelButton(nullptr);
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
}

//------------------------------------------------------------------------------
/**
 * @brief Applies dependency settings for all matches in the project.
 * @param depId - the dependency index.
 */
void DTDocumentWindow::OnApplyForAllDependencies(const QModelIndex& depId)
{
    QString name = m_pDependencies->GetName(depId);
    QString path = m_pDependencies->GetPath(depId);
    QString relocationPath = m_pDependencies->GetRelocation(depId);
    bool relocate = m_pDependencies->GetIsRelocate(depId);
    // update output model.
    DTOutputModel* pModel = qobject_cast<DTOutputModel*>(m_pOutputView->model());
    Q_ASSERT(pModel);
    // go througth all binary items.
    QList<QModelIndex> unprocessedNodes;
    unprocessedNodes.push_back(QModelIndex());
    while (!unprocessedNodes.isEmpty())
    {
        QModelIndex root = unprocessedNodes.front();
        unprocessedNodes.pop_front();
        int rows = pModel->rowCount(root);
        for (int i = 0; i < rows; ++i)
        {
            QModelIndex id = pModel->index(i, 0, root);
            int type = pModel->data(id, DT::ItemTypeRole).toInt();
            switch (static_cast<DT::OutputItemType>(type))
            {
                case DT::OutputAttributeType:
                case DT::OutputOtherFileType:
                    break;
                case DT::OutputFolderType:
                case DT::OutputBinaryType:
                case DT::OutputDependencyFolderType:
                    unprocessedNodes.append(id);
                    break;
                case DT::OutputDependencyType:
                    if (pModel->GetAttribute(id, DT::PathAttribute).toString() ==
                        path)
                    {
                        pModel->SetAttribute(id, DT::RelocateAttribute, relocate);
                        pModel->SetAttribute(id, DT::RelocatePathAttribute,
                                             relocationPath);
                        pModel->setData(id, name);
                    }
                    break;
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
 * @brief Copy dependency to the project.
 * @param depId - the dependency index.
 */
void DTDocumentWindow::OnCopyDependency(const QModelIndex& depId)
{
    DTOutputModel* pModel = qobject_cast<DTOutputModel*>(m_pOutputView->model());
    Q_ASSERT(pModel);
    QModelIndex id = GetCurrentIndex();
    if (id.isValid())
    {
        if (pModel->data(id, DT::ItemTypeRole).toInt() !=
            static_cast<int>(DT::OutputFolderType))
        {
            QMessageBox::warning(this, tr("Copy dependency"),
                                 tr("Folder selection required to complete the operation."),
                                 QMessageBox::Close);
            return;
        }
    }
    QString path = m_pDependencies->GetPath(depId);
    if (!path.isEmpty())
    {
        if (!QFile::exists(path))
        {
            int btn = QMessageBox::warning(
                        this, tr("Copy dependency"),
                        tr("Dependency file \"%1\" does not exist.\nDo you want to proceed?")
                        .arg(path), QMessageBox::Yes, QMessageBox::No);
            if (btn != static_cast<int>(QMessageBox::Yes))
            {
                return;
            }
        }
        pModel->AddFile(id, path);
    }
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
/**
 * @brief Saves project into a file.
 * @param fileName - the file name.
 */
void DTDocumentWindow::Save(const QString& fileName)
{
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
            else
            {
                m_projectFile = fileName;
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
 * @brief Builds an output folder structure.
 * @param rootPath - the output folder.
 * @return True if the operation completes successfully.
 */
bool DTDocumentWindow::BuildFolderStructure(const QString& rootPath)
{
    QDir dir(rootPath);
    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {
            qDebug()<<QString::fromLatin1("Make path error line:%1").arg(__LINE__);
            return false;
        }
    }
    DTOutputModel* pModel = qobject_cast<DTOutputModel*>(m_pOutputView->model());
    Q_ASSERT(pModel);
    // go througth all path items.
    QList<QModelIndex> unprocessedNodes;
    QList<QString> paths;
    unprocessedNodes.push_back(QModelIndex());
    paths.append(rootPath);
    while (!unprocessedNodes.isEmpty())
    {
        QModelIndex root = unprocessedNodes.front();
        unprocessedNodes.pop_front();
        QString path = paths.front();
        paths.pop_front();
        int rows = pModel->rowCount(root);
        for (int i = 0; i < rows; ++i)
        {
            QModelIndex id = pModel->index(i, 0, root);
            int type = pModel->data(id, DT::ItemTypeRole).toInt();
            if (type == static_cast<int>(DT::OutputFolderType))
            {
                unprocessedNodes.append(id);
                QString name = pModel->data(id).toString();
                paths.append(path + QString::fromLatin1("/") + name);
                QDir dir(path);
                if (!dir.exists(name))
                {
                    if (!dir.mkpath(name))
                    {
                        qDebug()<<QString::fromLatin1("Make path error line:%1").arg(__LINE__);
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

//------------------------------------------------------------------------------
/**
 * @brief Copies and relocates an output data.
 * @param rootPath - deploy root path.
 * @return True if the operation completes successfully.
 */
bool DTDocumentWindow::CopyOutputData(const QString& rootPath)
{
    m_copyErrors.clear();
    QList<CopyJob> jobs;
    // create job list
    DTOutputModel* pModel = qobject_cast<DTOutputModel*>(m_pOutputView->model());
    Q_ASSERT(pModel);
    // go througth all binary items.
    QList<QModelIndex> unprocessedNodes;
    QList<QString> paths;
    unprocessedNodes.push_back(QModelIndex());
    paths.append(rootPath);
    while (!unprocessedNodes.isEmpty())
    {
        QModelIndex root = unprocessedNodes.front();
        unprocessedNodes.pop_front();
        QString path = paths.front();
        paths.pop_front();
        int rows = pModel->rowCount(root);
        for (int i = 0; i < rows; ++i)
        {
            QModelIndex id = pModel->index(i, 0, root);
            int type = pModel->data(id, DT::ItemTypeRole).toInt();
            if (type == static_cast<int>(DT::OutputBinaryType))
            {
                QString fileName = pModel->GetAttribute(id, DT::PathAttribute).toString();
                QString name = pModel->data(id).toString();
                if (!fileName.isEmpty() && !name.isEmpty())
                {
                    CopyJob job;
                    job.m_position = id;
                    job.m_source = fileName;
                    job.m_destination = path + QString::fromLatin1("/") + name;
                    job.m_relocationMap = pModel->GetRelocations(id);
                    jobs.append(job);
                }
            }
            else if (type == static_cast<int>(DT::OutputFolderType))
            {
                unprocessedNodes.append(id);
                QString name = pModel->data(id).toString();
                paths.append(path + QString::fromLatin1("/") + name);
            }
        }
    }
    if (jobs.size())
    {
        QProgressDialog progress(this);
        progress.setWindowTitle(tr("Copy binaries"));
        progress.setCancelButton(nullptr);
        QFutureWatcher<CopyResult> watcher;
        connect(&watcher, SIGNAL(progressRangeChanged(int,int)),
                &progress, SLOT(setRange(int,int)));
        connect(&watcher, SIGNAL(progressValueChanged(int)),
                &progress, SLOT(setValue(int)));
        connect(&watcher, SIGNAL(resultReadyAt(int)),
                this, SLOT(OnCopyReady(int)));
        watcher.setFuture(QtConcurrent::mapped(jobs, &CopyFile));
        progress.exec();
    }
    return m_copyErrors.size() == 0;
}

//------------------------------------------------------------------------------
/**
 * @brief Removes all contents from the path.
 * @param path - the source path.
 */
bool DTDocumentWindow::CleanFolder(const QString& path)
{
    QDir dir(path);
    return dir.removeRecursively();
}
//------------------------------------------------------------------------------
