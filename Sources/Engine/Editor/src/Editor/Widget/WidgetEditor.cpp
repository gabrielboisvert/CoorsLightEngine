
#include <QtCore/qstandardpaths.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qlabel.h>
#include <QtGui/qscreen.h>
#include <QtGui/qevent.h>
#include <QtCore/qsavefile.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qmessagebox.h>
#include <QtGui/qwindow.h>
#include <QtWidgets/qtoolbar.h>
#include <QtWidgets/qapplication.h>
#include <QtDocking/DockAreaWidget.h>


#include "Editor/Widget/WidgetEditor.h"
#include "Tools/Utils/Define.h"
#include "Editor/Resources/Loader/StyleSheetLoader.h"
#include "Editor/Widget/WidgetMenuSperarator.h"
#include "Editor/Widget/WidgetNewScene.h"
#include "Rendering/Data/UniformData.h"
#include "Maths/Utils.h"
#include "Maths/FVector3.h"
#include "Rendering/Renderer/VK/VKOffScreenRenderer.h"
#include "Rendering/Data/Material.h"
#include "Rendering/Resources/VK/PipeLineBuilder.h"
#include "Rendering/Resources/Texture.h"
#include "Rendering/Data/UniformData.h"
#include "Rendering/Buffers/VK/VKUniformBuffer.h"
#include "Rendering/Resources/Texture.h"
#include "Game/SceneSys/SceneManager.h"

using namespace Editor::Widget;

Editor::Widget::WidgetEditor::WidgetEditor(Core::EditorApp* pApp, const std::string& pProjectPath, const std::string& pProjectName) :
	mApp(pApp),
    QMainWindow(nullptr),
	mLocation(pProjectPath.c_str(), pProjectName.c_str()),
	mSettings("Resources/Editor/Config/EditorEN.ini", QSettings::IniFormat),
	mProjectSettings(QString(pProjectPath.c_str()) + "/" + QString(pProjectName.c_str()) + "." + Define::PROJECT_EXTENSION, QSettings::IniFormat),
    mEngineApp(800, 800)
{
    service(EngineCore::Thread::ThreadPool).queueJob([this] 
    {
        mEngineApp.run();
    });


	setTheme();
	setWindowDecoration();

	initWindowControlButtons();
    initSceneLabel();
    initMainToolBar();
    initCenterContent();

    readProjectSettings();
    show();

    provideService(Editor::Widget::WidgetEditor, *this);
    
    service(EngineCore::Thread::ThreadPool).queueJob([this] 
    {
        loadResources(mLocation.mFolder);
    });
}

WidgetEditor::~WidgetEditor()
{
    delete mDockManager;
}

void WidgetEditor::setTheme()
{
	setStyleSheet(Resources::Loaders::StyleSheetLoader::loadStyleSheet(mSettings.value("StyleSheet").toString()));
}

void WidgetEditor::setWindowDecoration()
{
	setMinimumSize(mSettings.value("MinWidth").toInt(), mSettings.value("MinHeight").toInt());
	setWindowFlags(Qt::Window | Qt::FramelessWindowHint);/*| Qt::CustomizeWindowHint);*/
}

void WidgetEditor::initWindowControlButtons()
{
	QLabel* icon = new QLabel();
	icon->setPixmap(QPixmap(mSettings.value("IconPath").toString()).scaled(mSettings.value("IconWidth").toInt(), mSettings.value("IconHeight").toInt()));
	icon->setObjectName("IconTitle");
	icon->setAlignment(Qt::AlignHCenter);
	menuBar()->setCornerWidget(icon, Qt::TopLeftCorner);

    
    createFileMenu();
    createViewMenu();


    QToolBar* windowManagement = addToolBar("windowManager");
    menuBar()->setCornerWidget(windowManagement);
    QAction* newAction = nullptr;

    QLabel* ProjetName = new QLabel(mLocation.mName, this);
    ProjetName->setObjectName("ProjectName");
    windowManagement->addWidget(ProjetName);

    const QIcon minimizeIcon(mSettings.value("MinimizeIcon").toString());
    newAction = createAction(minimizeIcon, mSettings.value("MinimizeName").toString(), mSettings.value("MinimizeTips").toString(), QKeySequence::UnknownKey, *windowManagement);
    connect(newAction, &QAction::triggered, this, &WidgetEditor::minimize);

    mScreenState.mScreenState.init(mSettings.value("ResizeMaxIcon").toString(), mSettings.value("ResizeRestoreIcon").toString(), createAction(QIcon(), mSettings.value("ResizeName").toString(), mSettings.value("ResizeTips").toString(), QKeySequence::UnknownKey, *windowManagement));
    connect(mScreenState.mScreenState.mAction, &QAction::triggered, this, &WidgetEditor::maximize);

    const QIcon quitIcon(mSettings.value("QuitIcon").toString());
    newAction = createAction(quitIcon, mSettings.value("QuitName").toString(), mSettings.value("QuitTips").toString(), QKeySequence::Quit, *windowManagement);
    connect(newAction, &QAction::triggered, this, &WidgetEditor::quit);
}

void WidgetEditor::createFileMenu()
{
    QMenu* mFileMenu = menuBar()->addMenu(mSettings.value("MenuFile").toString());
    QAction* newAction = nullptr;

    {
        WidgetMenuSeperator* sepa = new WidgetMenuSeperator(mSettings, mSettings.value("CategoryLevelOpen").toString(), mSettings.value("SeparatorIcon").toString(), mFileMenu);
        mFileMenu->addAction(sepa->mAction);
    }

    newAction = createAction(QIcon(mSettings.value("MenuFileNewMapIcon").toString()), mSettings.value("MenuFileNewMap").toString(), mSettings.value("MenuFileNewMapTips").toString(), QKeySequence::New, *mFileMenu);
    connect(newAction, &QAction::triggered, this, &WidgetEditor::newSceneDialog);

    newAction = createAction(QIcon(mSettings.value("MenuFileOpenMapIcon").toString()), mSettings.value("MenuFileOpenMap").toString(), mSettings.value("MenuFileOpenMapTips").toString(), QKeySequence::Open, *mFileMenu);
    connect(newAction, &QAction::triggered, this, &WidgetEditor::openScene);
    
    {
        WidgetMenuSeperator* sepa = new WidgetMenuSeperator(mSettings, mSettings.value("CategoryLevelSave").toString(), mSettings.value("SeparatorIcon").toString(), mFileMenu);
        mFileMenu->addAction(sepa->mAction);
    }

    newAction = createAction(QIcon(mSettings.value("MenuFileSaveMapIcon").toString()), mSettings.value("MenuFileSaveMap").toString(), mSettings.value("MenuFileSaveMapTips").toString(), QKeySequence::Save, *mFileMenu);
    connect(newAction, &QAction::triggered, this, &WidgetEditor::save);

    newAction = createAction(QIcon(mSettings.value("MenuFileSaveAsMapIcon").toString()), mSettings.value("MenuFileSaveAsMap").toString(), mSettings.value("MenuFileSaveAsMapTips").toString(), QKeySequence::SaveAs, *mFileMenu);
    connect(newAction, &QAction::triggered, this, &WidgetEditor::saveAs);

    newAction = createAction(QIcon(mSettings.value("MenuFileSaveAllMapIcon").toString()), mSettings.value("MenuFileSaveAllMap").toString(), mSettings.value("MenuFileSaveAllMapTips").toString(), QKeySequence::UnknownKey, *mFileMenu);
    connect(newAction, &QAction::triggered, this, &WidgetEditor::save);

    {
        WidgetMenuSeperator* sepa = new WidgetMenuSeperator(mSettings, mSettings.value("CategoryProject").toString(), mSettings.value("SeparatorIcon").toString(), mFileMenu);
        mFileMenu->addAction(sepa->mAction);
    }

    newAction = createAction(QIcon(mSettings.value("MenuFileNewProjectIcon").toString()), mSettings.value("MenuFileNewProject").toString(), mSettings.value("MenuFileNewProjectTips").toString(), QKeySequence::UnknownKey, *mFileMenu);
    connect(newAction, &QAction::triggered, this, &WidgetEditor::newProject);

    newAction = createAction(QIcon(mSettings.value("MenuFileOpenProjectIcon").toString()), mSettings.value("MenuFileOpenProject").toString(), mSettings.value("MenuFileOpenProjectTips").toString(), QKeySequence::UnknownKey, *mFileMenu);
    connect(newAction, &QAction::triggered, this, &WidgetEditor::openProject);

    {
        WidgetMenuSeperator* sepa = new WidgetMenuSeperator(mSettings, mSettings.value("CategoryExit").toString(), mSettings.value("SeparatorIcon").toString(), mFileMenu);
        mFileMenu->addAction(sepa->mAction);
    }

    newAction = createAction(QIcon(mSettings.value("MenuFileExitIcon").toString()), mSettings.value("MenuFileExit").toString(), mSettings.value("MenuFileExitTips").toString(), QKeySequence::UnknownKey, *mFileMenu);
    connect(newAction, &QAction::triggered, this, &WidgetEditor::quit);
}

void WidgetEditor::createViewMenu()
{
    QMenu* mViewMenu = menuBar()->addMenu(mSettings.value("MenuView").toString());
    QAction* newAction = nullptr;

    newAction = createAction(QIcon(), mSettings.value("MenuViewViewport").toString(), mSettings.value("MenuViewViewportTips").toString(), QKeySequence::UnknownKey, *mViewMenu);
    connect(newAction, &QAction::triggered, this, [newAction, this] { mWindow->toggleView(newAction->isChecked()); });
    newAction->setCheckable(true);
    newAction->setChecked(true);
    viewActions["scene"] = newAction;

    newAction = createAction(QIcon(), mSettings.value("MenuViewBrowser").toString(), mSettings.value("MenuViewBrowserTips").toString(), QKeySequence::UnknownKey, *mViewMenu);
    connect(newAction, &QAction::triggered, this, [newAction, this] { mContent->toggleView(newAction->isChecked()); });
    newAction->setCheckable(true);
    newAction->setChecked(true);
    viewActions["browser"] = newAction;

    newAction = createAction(QIcon(), mSettings.value("MenuViewConsole").toString(), mSettings.value("MenuViewConsoleTips").toString(), QKeySequence::UnknownKey, *mViewMenu);
    connect(newAction, &QAction::triggered, this, [newAction, this] { mConsole->toggleView(newAction->isChecked()); });
    newAction->setCheckable(true);
    newAction->setChecked(true);
    viewActions["console"] = newAction;

    newAction = createAction(QIcon(), mSettings.value("MenuViewHierarchy").toString(), mSettings.value("MenuViewHierarchyTips").toString(), QKeySequence::UnknownKey, *mViewMenu);
    connect(newAction, &QAction::triggered, this, [newAction, this] { mHierarchy->toggleView(newAction->isChecked()); });
    newAction->setCheckable(true);
    newAction->setChecked(true);
    viewActions["hierarchy"] = newAction;

    newAction = createAction(QIcon(), mSettings.value("MenuViewInspector").toString(), mSettings.value("MenuViewInspectorTips").toString(), QKeySequence::UnknownKey, *mViewMenu);
    connect(newAction, &QAction::triggered, this, [newAction, this] { mInspector->toggleView(newAction->isChecked()); });
    newAction->setCheckable(true);
    newAction->setChecked(true);
    viewActions["inspector"] = newAction;
}

void WidgetEditor::initSceneLabel()
{
    QString scenePath = mLocation.mFolder + "/" + mProjectSettings.value("StartingMap").toString() + "." + Define::SCENE_EXTENSION;
    mLevelLabel.init(this, mSettings, menuBar()->size().height() + 1, scenePath, mProjectSettings.value("StartingMap").toString().split("/").last());
    
    service(Game::SceneSys::SceneManager).loadScene(Utils::qStringToStdString(scenePath).c_str());
}

void WidgetEditor::initMainToolBar()
{
    QToolBar* mainToolBar = addToolBar("toolbar");
    mainToolBar->setFloatable(false);
    mainToolBar->setMovable(false);
    mainToolBar->setFixedHeight(30);
    mainToolBar->setObjectName("MainToolBar");

    QAction* action = nullptr;
    QAction* separator = nullptr;

    const QIcon SaveIcon(mSettings.value("SaveIcon").toString());
    action = mainToolBar->addAction(SaveIcon, "");
    connect(action, &QAction::triggered, this, &WidgetEditor::save);

    mainToolBar->addSeparator();

    QToolBar* playToolBar = new QToolBar();
    playToolBar->setObjectName("PlayButtonToolBar");
    mainToolBar->addWidget(playToolBar);


    mPlayState.init(mSettings.value("PlayIcon").toString(), mSettings.value("PauseIcon").toString(), playToolBar->addAction(""));
    mPlayState.setTips(mSettings.value("PlayTips").toString());
    connect(mPlayState.mAction, &QAction::triggered, this, &WidgetEditor::play);

    mPlayFrameState.init(mSettings.value("PlayFrameIcon").toString(), mSettings.value("PlayFrameIconActif").toString(), playToolBar->addAction(""));
    mPlayFrameState.setTips(mSettings.value("PlayFrameTips").toString());
    mPlayFrameState.setDisabled(true);
    connect(mPlayFrameState.mAction, &QAction::triggered, this, &WidgetEditor::playFrame);

    mStopState.init(mSettings.value("StopIcon").toString(), mSettings.value("StopActifIcon").toString(), playToolBar->addAction(""));
    mStopState.setTips(mSettings.value("StopTips").toString());
    mStopState.setDisabled(true);
    connect(mStopState.mAction, &QAction::triggered, this, &WidgetEditor::stop);

    const QIcon ToogleCameraIcon(mSettings.value("ToogleCameraIcon").toString());
    action = playToolBar->addAction(ToogleCameraIcon, "");
    action->setDisabled(true);

    mainToolBar->addSeparator();
}

void WidgetEditor::initCenterContent()
{
    QFlags<ads::CDockManager::eConfigFlag> flags = ads::CDockManager::eConfigFlag::DragPreviewIsDynamic | ads::CDockManager::eConfigFlag::DockAreaHasUndockButton;
    ads::CDockManager::setConfigFlags(flags);
    mDockManager = new ads::CDockManager(this);
    mDockManager->setStyleSheet(Resources::Loaders::StyleSheetLoader::loadStyleSheet(mSettings.value("DockingStyleSheet").toString()));

    //Window
    mWindow = new WidgetSceneApp(mEngineApp, mSettings);
    connect(mWindow, &ads::CDockWidget::closed, this, [this]{ viewActions["scene"]->setChecked(false); });

    mInspector = new WidgetInspectorApp(mSettings);
    connect(mInspector, &ads::CDockWidget::closed, this, [this] { viewActions["inspector"]->setChecked(false); });

    mHierarchy = new WidgetSceneHierarchyApp(mSettings, *mInspector);
    connect(mHierarchy, &ads::CDockWidget::closed, this, [this] { viewActions["hierarchy"]->setChecked(false); });

    mContent = new WidgetContentBrowserApp(mSettings, mLocation);
    connect(mContent, &ads::CDockWidget::closed, this, [this] { viewActions["browser"]->setChecked(false); });

    mConsole = new WidgetConsoleApp(mSettings);
    connect(mConsole, &ads::CDockWidget::closed, this, [this] { viewActions["console"]->setChecked(false); });
    
    mModelView = new WidgetModelViewerApp(mSettings);
    
    
    
    //Centering
    ads::CDockAreaWidget* areaCentral = mDockManager->addDockWidget(ads::CenterDockWidgetArea, mWindow);
    ads::CDockAreaWidget* areaLeft = mDockManager->addDockWidget(ads::LeftDockWidgetArea, mHierarchy, areaCentral);
    ads::CDockAreaWidget* areaRight = mDockManager->addDockWidget(ads::RightDockWidgetArea, mInspector, areaCentral);
    ads::CDockAreaWidget* areaBottom = mDockManager->addDockWidget(ads::BottomDockWidgetArea, mContent);

    mDockManager->addDockWidgetTabToArea(mModelView, areaCentral);
    mDockManager->addDockWidgetTabToArea(mConsole, areaBottom);

    // Resizewindow
    mDockManager->setSplitterSizes(areaCentral, { (int)(width() * 0.15), (int)(width() * 0.55), (int)(width() * 0.3) });
    mDockManager->setSplitterSizes(areaBottom, { (int)(height() * 0.70f), (int)(height() * 0.30f)});

    //Disabled window
    mContent->toggleView(true);
    mModelView->toggleView(false);
}

void WidgetEditor::readProjectSettings()
{
    const QByteArray geometry = mProjectSettings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty())
    {
        const QRect availableGeometry = screen()->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
            (availableGeometry.height() - height()) / 2);
    }
    else
        restoreGeometry(geometry);

    const bool maximized = mProjectSettings.value("Maximized", QVariant()).toBool();
    if (maximized)
        maximize();
}

void WidgetEditor::writeProjectSettings()
{
    mProjectSettings.setValue("geometry", saveGeometry());
    mProjectSettings.setValue("Maximized", mScreenState.mIsFullScreen);
}

QAction* WidgetEditor::createAction(const QIcon& pIcon, const QString& pTitle, const QString& pStatusTip, const QKeySequence::StandardKey& pShortcut, QWidget& pWidget)
{
    QAction* newAction = new QAction(pIcon, pTitle, this);
    newAction->setShortcuts(pShortcut);
    newAction->setStatusTip(pStatusTip);
    pWidget.addAction(newAction);
    return newAction;
}

bool WidgetEditor::maybeSave()
{
    if (!mProjectModified)
        return true;

    const QMessageBox::StandardButton ret  = QMessageBox::warning(this, tr("Application"), tr("The document has been modified.\n" "Do you want to save your changes?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch (ret)
    {
        case QMessageBox::Save:
            return save();
        case QMessageBox::Cancel:
            return false;
        default:
            return true;
    }

    return true;
}

void WidgetEditor::newSceneDialog()
{
    mApp->removeEventFilter(this);

    WidgetNewScene dialog(mLocation.mFolder);
    mApp->installEventFilter(&dialog);
    dialog.exec();
    if (dialog.result() == 1)
    {
        mLevelLabel.mFileName = dialog.mNewScene;
        mLevelLabel.mMapTitle->setText(QFileInfo(dialog.mNewScene).baseName());
    }

    mApp->installEventFilter(this);
}

void WidgetEditor::openScene()
{
    std::string filter = "Level File (*." + std::string(Define::SCENE_EXTENSION) + ")";
    QFileDialog dialog(this, tr("Open Level"), mLocation.mFolder, tr(filter.c_str()));
    connect(&dialog, &QFileDialog::directoryEntered, [&dialog, this](QString path) 
        {
            if (!path.contains(mLocation.mFolder))
                dialog.setDirectory(mLocation.mFolder);
        });

    if (dialog.exec() != QDialog::Accepted)
        return;

    QString fileName = dialog.selectedFiles().first();
    if (!fileName.isEmpty())
    {
        mLevelLabel.mFileName = fileName;
        mLevelLabel.mMapTitle->setText(QFileInfo(fileName).baseName());
    }
}

bool WidgetEditor::save()
{
    if (mLevelLabel.mFileName.isEmpty())
        return saveAs();

    writeProjectSettings();
    return saveFile(mLevelLabel.mFileName);
}

bool WidgetEditor::saveAs()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);

    QString filter = "Level File (*." + QString(Define::SCENE_EXTENSION) + ")";
    dialog.setNameFilter(filter);
    dialog.setDefaultSuffix(Define::SCENE_EXTENSION);

    connect(&dialog, &QFileDialog::directoryEntered, [&dialog, this](QString path) {
        if (!path.contains(mLocation.mFolder))
            dialog.setDirectory(mLocation.mFolder);
        });

    if (dialog.exec() != QDialog::Accepted)
        return false;

    writeProjectSettings();
    return saveFile(dialog.selectedFiles().first());
}

bool WidgetEditor::saveFile(const QString& pFileName)
{
    QDir dir("Map");
    if (!dir.exists())
        dir.mkpath(".");

    QString errorMessage;
    QSaveFile file(QUrl::fromLocalFile(pFileName).toLocalFile());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        file.write("Cock");

        if (!file.commit())
        {
            errorMessage = tr("Cannot write file %1:\n%2.")
                .arg(QDir::toNativeSeparators(pFileName), file.errorString());
        }
    }
    else
    {
        errorMessage = tr("Cannot open file %1 for writing:\n%2.")
            .arg(QDir::toNativeSeparators(pFileName), file.errorString());
    }

    if (!errorMessage.isEmpty())
    {
        QMessageBox::warning(this, tr("Application"), errorMessage);
        return false;
    }

    return true;
}

void WidgetEditor::newProject()
{
    if (maybeSave())
    {
        writeProjectSettings();

        
        mEngineApp.close();
        mModelView->close();
        close();
        QApplication::exit(Define::EEditorState::OPEN_HUB);
        service(EngineCore::Thread::ThreadPool).stop();
    }
}

void WidgetEditor::openProject()
{
    QString filter = "Project Files (*." + QString(Define::PROJECT_EXTENSION) + ")";
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Project"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), filter);
    
    if (!filePath.isEmpty())
    {
        QFileInfo fileOpen(filePath);
        mNewProjectOpen.mFolder = fileOpen.absolutePath();
        mNewProjectOpen.mName = fileOpen.baseName();

        writeProjectSettings();

        mEngineApp.close();
        mModelView->close();
        close();
        QApplication::exit(Define::LOAD_EDITOR);
    }
}

void WidgetEditor::quit()
{
    if (maybeSave())
    {
        writeProjectSettings();

        mEngineApp.close();
        mModelView->close();
        
        service(EngineCore::Thread::ThreadPool).stop();

        close();
        QApplication::exit(Define::CLOSE_EDITOR);
    }
}

void WidgetEditor::minimize()
{
    windowHandle()->showMinimized();
}

void WidgetEditor::maximize()
{
    mScreenState.mIsFullScreen = !mScreenState.mIsFullScreen;
    if (mScreenState.mIsFullScreen)
    {
        mScreenState.mOldPos = mapToGlobal(rect().topLeft());
        mScreenState.mOldSize = size();
        setWindowState(Qt::WindowMaximized);
        mScreenState.mScreenState.switchIcon(2);
    }
    else
    {
        move(mScreenState.mOldPos);
        resize(mScreenState.mOldSize);
        setWindowState(Qt::WindowActive);
        mScreenState.mScreenState.switchIcon(1);
    }
}

void WidgetEditor::play()
{
    if (mPaused)
    {
        mPlayState.switchIcon(2);
        mEngineApp.mPlaying = true;
        mPaused = false;

        mPlayFrameState.switchIcon(1);
        mPlayFrameState.setDisabled(true);

        mPlayState.setTips(mSettings.value("PauseTips").toString());
    }
    else
    {
        mPlayState.switchIcon(1);
        mEngineApp.mPlaying = false;
        mPaused = true;

        mPlayFrameState.switchIcon(2);
        mPlayFrameState.setDisabled(false);
        mPlayState.setTips(mSettings.value("ResumeTips").toString());
    }

    if (!mPlaying)
    {
        mConsole->toggleView(true);
        mConsole->play();

        mPlaying = true;

        mStopState.switchIcon(2);
        mStopState.setDisabled(false);

        /*for (unsigned int i = 0; i < mEngine.mGameObjects.size(); i++)
            mEngine.mGameObjects[i]->compile();

        for (unsigned int i = 0; i < mEngine.mGameObjects.size(); i++)
            mEngine.mGameObjects[i]->beginPlay();*/
    }
}

void WidgetEditor::playFrame()
{
    /*for (unsigned int i = 0; i < mEngine.mGameObjects.size(); i++)
        mEngine.mGameObjects[i]->update(mEngine.mTime.mDeltaTime);*/
}

void WidgetEditor::stop()
{
    mPlayState.switchIcon(1);
    mPlaying = false;
    mPaused = true;

    mPlayFrameState.switchIcon(1);
    mPlayFrameState.setDisabled(true);

    mStopState.switchIcon(1);
    mStopState.setDisabled(true);

    mPlayState.setTips(mSettings.value("PlayTips").toString());

    mEngineApp.mPlaying = false;
    //for (unsigned int i = 0; i < mEngine.mGameObjects.size(); i++)
    //    mEngine.mGameObjects[i]->stopPlay();
}

bool WidgetEditor::eventFilter(QObject* obj, QEvent* event)
{

    if (obj == menuBar() && event->type() == QEvent::MouseButtonPress)
    {
        if (mScreenState.mIsFullScreen)
        {
            mScreenState.mIsFullScreen = false;
            mScreenState.mScreenState.switchIcon(1);
        }
        windowHandle()->startSystemMove();
    }
    else if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = (QKeyEvent*)event;
        if (keyEvent->key() == Qt::Key_Escape)
            quit();
    }
    
    return QObject::eventFilter(obj, event);
}

void WidgetEditor::loadResources(const QString& pPath)
{
    const QList<QFileInfo> dirFileList = QDir(pPath).entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);
    for (unsigned int i = 0; i < dirFileList.size(); i++)
    {
        if (dirFileList[i].isDir())
        {
            loadResources(dirFileList[i].absoluteFilePath());
            continue;
        }

        Tools::Utils::PathParser::EFileType type = Tools::Utils::PathParser::getFileType(dirFileList[i].suffix());
        switch (type)
        {
            case Tools::Utils::PathParser::EFileType::UNKNOWN:
                break;

            case Tools::Utils::PathParser::EFileType::MODEL:
                loadFile<Rendering::Resources::Model>(dirFileList[i].absoluteFilePath(), dirFileList[i].baseName());
                break;

            case Tools::Utils::PathParser::EFileType::TEXTURE:
                loadFile<Rendering::Resources::Texture>(dirFileList[i].absoluteFilePath(), dirFileList[i].baseName());
                break;

            case Tools::Utils::PathParser::EFileType::SHADER:
                //loadFile<Shader>(dirFileList[i].absoluteFilePath());
                break;

            case Tools::Utils::PathParser::EFileType::MATERIAL:
                //loadFile<Material>(dirFileList[i].absoluteFilePath());
                break;

            case Tools::Utils::PathParser::EFileType::SOUND:
                //loadFile<Sound>(dirFileList[i].absoluteFilePath());
                break;

            case Tools::Utils::PathParser::EFileType::SCENE:
                service(Game::SceneSys::SceneManager).addScene(Utils::qStringToStdString(dirFileList[i].absoluteFilePath()).c_str());
                break;

            case Tools::Utils::PathParser::EFileType::SCRIPT:
                //loadFile<script>(dirFileList[i].absoluteFilePath());
                break;

            case Tools::Utils::PathParser::EFileType::FONT:
                //loadFile<Font>(dirFileList[i].absoluteFilePath());
                break;
        }
    }
}

void WidgetEditor::createPreview(std::string pFile, Rendering::Resources::Model* pModel)
{
    QDir dir(mLocation.mFolder + mSettings.value("previewFolder").toString());
    if (!dir.exists())
        dir.mkpath(".");
    std::string location = Utils::qStringToStdString(dir.absolutePath());
    SetFileAttributes(location.c_str(), FILE_ATTRIBUTE_HIDDEN);
    location += "/" + pFile + ".png";

    if (QFile::exists(location.c_str()))
    {
        pModel->previewCreated = true;
        return;
    }

    //Create renderer and material, texture and uniform buffer
    Rendering::Renderer::VK::VKOffScreenRenderer renderer(800, 800);
    Rendering::Data::Material mat = Rendering::Renderer::Resources::VK::PipeLineBuilder()
        .initPipeLine("Resources/Editor/Shaders/ModelVertex.vert.spv", "Resources/Editor/Shaders/ModelFrag.frag.spv", renderer.mRenderPass, false);
    Rendering::Resources::Texture text("Resources/Editor/Textures/default.png");
    Rendering::Buffers::VK::VKUniformBuffer<Rendering::Data::UniformData> unibuffer(VK_SHADER_STAGE_VERTEX_BIT);

    //Bind to material
    mat.bindDescriptor("texSampler", text.mTextureSets);
    mat.bindDescriptor("ubo", unibuffer.mDescriptorSets);

    // Settup data view
    float max = Maths::fmax(Maths::fmax(pModel->mBox.mSize.x, pModel->mBox.mSize.y), pModel->mBox.mSize.z);
    float dist = max / tan(Maths::degreesToRadians(45) / 2);
    Maths::FVector3 position(0, pModel->mBox.mMin.y + (pModel->mBox.mSize.y / 2), dist);
    unibuffer.mData = {
                        Maths::FMatrix4::createTransformMatrix({ 0, 0, 0 }, { 25, 0, 0 }, { 1, 1, 1 }),
                            Maths::FMatrix4::createPerspective(-45, float(renderer.mWindowExtent.width) / float(renderer.mWindowExtent.height), 0.01f, 1000.0f)
                            * Maths::FMatrix4::lookAt(position, position + Maths::FVector3::Forward, Maths::FVector3::Up)
                        };
    unibuffer.updateData();

    //Draw content
    renderer.beginFrame();
        mat.bindPipeLine(renderer.getCurrentCommandBuffer());
        pModel->draw(renderer.getCurrentCommandBuffer());
    renderer.endFrame();

    //Create image
    renderer.saveToPNG(location.c_str());


    pModel->previewCreated = true;
}

void WidgetEditor::openModel(QString pPath)
{
    mModelView->open(service(EngineCore::ResourcesManagement::ResourceManager).get<Rendering::Resources::Model>(Utils::qStringToStdString(pPath)));
}