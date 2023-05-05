#include <QtWidgets/qwidget.h>
#include <QtWidgets/qboxlayout.h>
#include "Editor/Widget/WidgetSceneApp.h"
#include "Tools/Time/Clock.h"
#include "EngineCore/Service/ServiceLocator.h"
#include "Editor/Data/Camera.h"
#include "EngineCore/EventSystem/InputManager.h"
#include "Rendering/Data/Material.h"
#include "Rendering/Resources/Texture.h"
#include "Game/Data/Actor.h"
#include "Game/SceneSys/SceneManager.h"
#include "Rendering/Resources/VK/PipeLineBuilder.h"
#include "Physics/Core/BulPhysicsEngine.h"
#include "Scripting/ScriptInterpreter.h"
#include "Physics/Resources/IRigidbody.h"
#include "Game/Utils/ComponentType.h"


using namespace Editor::Widget;

WidgetSceneApp::WidgetSceneApp() :
	ads::CDockWidget(""), mWindow("", 0, 0), mCamera(0, 0)
{
}

WidgetSceneApp::WidgetSceneApp(QSettings& pSetting, WidgetGameObjectTree& pTreeItem, QWidget* pParent) :
	ads::CDockWidget("Scene"), mWindow("", 800, 800), mTreeItem(&pTreeItem), mCamera(800, 800)
{
	QWidget* content = new QWidget(pParent);
	setWidget(content);

	QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, nullptr);
	layout->setContentsMargins(5, 0, 5, 5);
	layout->setSpacing(0);
	content->setLayout(layout);

	setAcceptDrops(true);

	windowId = mWindow.getWindowId();
	qWindow = QWindow::fromWinId((WId)windowId);

	renderingWindow = QWidget::createWindowContainer(qWindow);
	renderingWindow->installEventFilter(this);
	layout->addWidget(renderingWindow);

	provideService(Editor::Widget::WidgetSceneApp, *this);
	renderer = new Rendering::Renderer::VK::VKRenderer();
	renderer->init(mWindow);
	offScreenRenderer = new Rendering::Renderer::VK::VKOffScreenRenderer(renderingWindow->width(), renderingWindow->height());

	pickingArrow = new Editor::Data::PickingArrow(renderer->mRenderPass, offScreenRenderer->mRenderPass);
}

void WidgetSceneApp::run()
{
	mLightManager = new EngineCore::Light::LightManager();
	initLight();

	initSceneMat();
	initSkyBox();
	initInput();

	service(Physics::Core::BulPhysicsEngine).createDebugDrawer(renderer);
	
	renderer->mFramebufferResized = true;
	bool newSelection = false;
	while (!mWindow.shouldClose())
	{
		eventUpdate();
		checkResizeWindow();
		checkMousePress(newSelection);
		drawScene(newSelection);
	}

	cleanupScene();
}

void WidgetSceneApp::close()
{
	mWindow.close();
}

WidgetSceneApp::~WidgetSceneApp()
{
	for (auto actor : mActors)
		delete actor;
}

void Editor::Widget::WidgetSceneApp::addActor(Editor::Data::Actor* pActor)
{
	mainThreadAction.pushFunction([this, pActor]
	{
		mActors.push_back(pActor);
	});
}

void WidgetSceneApp::removeActor(Editor::Data::Actor* pActor)
{
	mainThreadAction.pushFunction([this, pActor]
	{
		renderer->waitForCleanUp();
		offScreenRenderer->waitForCleanUp();

		mActors.remove(pActor);
		delete pActor;
	});
}

void WidgetSceneApp::initSceneMat()
{
	mSceneMat = new Rendering::Data::Material(Rendering::Renderer::Resources::VK::PipeLineBuilder()
		.initPipeLine("Resources/Engine/Shaders/ModelVertex.vert.spv", "Resources/Engine/Shaders/ModelFrag.frag.spv", renderer->mRenderPass, false, false, true, false));

	mOutline = new Rendering::Data::Material(Rendering::Renderer::Resources::VK::PipeLineBuilder()
		.initPipeLine("Resources/Editor/Shaders/outline.vert.spv", "Resources/Editor/Shaders/outline.frag.spv", renderer->mRenderPass, false, false, false, true));

	mPickingMat = new Rendering::Data::Material(Rendering::Renderer::Resources::VK::PipeLineBuilder()
		.initPipeLine("Resources/Editor/Shaders/IdItem.vert.spv", "Resources/Editor/Shaders/IdItem.frag.spv", offScreenRenderer->mRenderPass, false, false, true, false));

	skyMat = new Rendering::Data::Material(Rendering::Renderer::Resources::VK::PipeLineBuilder()
		.initPipeLine("Resources/Engine/Shaders/SkyVertex.vert.spv", "Resources/Engine/Shaders/SkyFrag.frag.spv", renderer->mRenderPass, false));

	ParticleMat = new Rendering::Data::Material(Rendering::Renderer::Resources::VK::PipeLineBuilder()
		.initPipeLine("Resources/Engine/Shaders/ParticleVertex.vert.spv", "Resources/Engine/Shaders/ParticleFrag.frag.spv", renderer->mRenderPass, false, false, true, false, true));
}

void WidgetSceneApp::initSkyBox()
{
	skyBox = new Rendering::Renderer::SkyBox();
	skyBox->setDefault();
}

void WidgetSceneApp::initInput()
{
	service(EngineCore::EventSystem::InputManager).bindContinuousActionCallBack(EngineCore::EventSystem::Key::W,
		[this] {if (mInput->mMouse.isRightPressed()) mCamera.update(Maths::FVector3::Up, mClock.getDeltaTime()); });

	service(EngineCore::EventSystem::InputManager).bindContinuousActionCallBack(EngineCore::EventSystem::Key::S,
		[this] {if (mInput->mMouse.isRightPressed()) mCamera.update(-Maths::FVector3::Up, mClock.getDeltaTime()); });

	service(EngineCore::EventSystem::InputManager).bindContinuousActionCallBack(EngineCore::EventSystem::Key::A,
		[this] {if (mInput->mMouse.isRightPressed()) mCamera.update(Maths::FVector3::Right, mClock.getDeltaTime()); });

	service(EngineCore::EventSystem::InputManager).bindContinuousActionCallBack(EngineCore::EventSystem::Key::D,
		[this] {if (mInput->mMouse.isRightPressed()) mCamera.update(Maths::FVector3::Left, mClock.getDeltaTime()); });

	service(EngineCore::EventSystem::InputManager).bindContinuousActionCallBack(EngineCore::EventSystem::Key::E,
		[this] {if (mInput->mMouse.isRightPressed()) mCamera.update(Maths::FVector3::Forward, mClock.getDeltaTime()); });

	service(EngineCore::EventSystem::InputManager).bindContinuousActionCallBack(EngineCore::EventSystem::Key::Q,
		[this] {if (mInput->mMouse.isRightPressed()) mCamera.update(Maths::FVector3::Backward, mClock.getDeltaTime()); });

	service(EngineCore::EventSystem::InputManager).bindActionCallBack(EngineCore::EventSystem::KState::PRESS, EngineCore::EventSystem::Key::E,
		[this] { if (!mInput->mMouse.isRightPressed())pickingArrow->setMode(Editor::Data::ArrowMode::ETranslation); });

	service(EngineCore::EventSystem::InputManager).bindActionCallBack(EngineCore::EventSystem::KState::PRESS, EngineCore::EventSystem::Key::R,
		[this] {if (!mInput->mMouse.isRightPressed()) pickingArrow->setMode(Editor::Data::ArrowMode::ERotation); });

	service(EngineCore::EventSystem::InputManager).bindActionCallBack(EngineCore::EventSystem::KState::PRESS, EngineCore::EventSystem::Key::T,
		[this] {if (!mInput->mMouse.isRightPressed()) pickingArrow->setMode(Editor::Data::ArrowMode::EScale); });

	mInput = &service(EngineCore::EventSystem::InputManager);
	mWindow.setInputManager(mInput);
}

void WidgetSceneApp::initLight()
{
	if (mLightManager != nullptr)
	{
		Game::SceneSys::SceneManager& scene = service(Game::SceneSys::SceneManager);
		if (scene.mCurrentScene != nullptr)
			mLightManager->mLights = &scene.mCurrentScene->mLights;
	}
}

void WidgetSceneApp::eventUpdate()
{
	clearThreadAction.flush();
	mainThreadAction.flush();
	mClock.update();
	mWindow.pollEvents();
	mInput->processInput();
}

void WidgetSceneApp::checkResizeWindow()
{
	if (renderer->mFramebufferResized)
	{
		int width = renderingWindow->width();
		int height = renderingWindow->height();;
		mWindow.mWidth = width;
		mWindow.mHeight = height;
		mCamera.updateProjection(width, height);

		renderer->mFramebufferResized = false;
		offScreenRenderer->recreateSwapChain(width, height);
	}
}

void WidgetSceneApp::checkMousePress(bool& pSelectedActor)
{
	Maths::FVector2 dif = mInput->mMouse.getPositionDifference();
	if (mInput->mMouse.isRightPressed())
		mCamera.updateView(dif);
	
	static bool mSingleClick = false;
	if (mInput->mMouse.isLeftPressed() && !mSingleClick)
	{
		pickupPhase();
		pSelectedActor = true;
		mSingleClick = true;
	}
	else if (!mInput->mMouse.isLeftPressed())
	{
		mSingleClick = false;
		pickingArrow->endDrag();
	}

	if (pickingArrow->isDrag)
		pickingArrow->moveDrag(dif);
}

void WidgetSceneApp::pickupPhase()
{
	QPoint StartPoint = renderingWindow->mapFromGlobal(QCursor::pos());
	if (StartPoint.x() <= 0 || StartPoint.y() <= 0)
		return;

	offScreenRenderer->beginFrame();
	for (Editor::Data::Actor* actor : mActors)
	{
		mPickingMat->bindDescriptor("ubo", actor->mUniformBufferid.mDescriptorSets);

		actor->mUniformBufferid.mData.id = actor->id;
		actor->mUniformBufferid.mData.mViewProjection = mCamera.viewProj();
		actor->mUniformBufferid.mData.mModel = actor->getTransform()->getWorldMatrix();
		actor->mUniformBufferid.updateData();

		mPickingMat->bindPipeLine(offScreenRenderer->getCurrentCommandBuffer());
		
		actor->draw(offScreenRenderer->getCurrentCommandBuffer());

		#ifdef NSHIPPING
			actor->drawCameraOffscreen(offScreenRenderer->getCurrentCommandBuffer());
		#endif
	}

	if (mTreeItem->mSelectedActor != nullptr)
		pickingArrow->drawPicking(offScreenRenderer->getCurrentCommandBuffer(), mCamera.viewProj(), mCamera.mPosition);

	offScreenRenderer->endFrame();
	const unsigned char* data = ((const unsigned char*)offScreenRenderer->pixelToArray());


	int yPos = StartPoint.y() * (offScreenRenderer->subResourceLayout.rowPitch / offScreenRenderer->ColorChannel);
	int PosInArray = ((StartPoint.x()) + yPos) * offScreenRenderer->ColorChannel;

	int id = data[PosInArray];
	if (id == 0 && data[PosInArray + 1] == 255)
	{
		pickingArrow->beginDrag(data[PosInArray + 2], mCamera);
		return;
	}

	if (id != mTreeItem->mIdSelected)
	{
		mTreeItem->mIdSelected = id;

		if (mTreeItem->mSelectedActor != nullptr)
		{
			mTreeItem->mSelectedActor = nullptr;
		}

		if (id == 0)
			mTreeItem->selectionModel()->clearSelection();
	}
}

void WidgetSceneApp::drawScene(bool& pSelectedActor)
{
	renderer->beginFrame();
		
		drawSky();
		drawActors(pSelectedActor);
		drawCollider();
		drawParticle();
		drawArrow();
		
	renderer->endFrame();
}

void WidgetSceneApp::bindLights()
{
	if (mLightManager == nullptr)
		return;

	if (mLightManager->mLights == nullptr)
	{
		initLight();
		return;
	}

	mLightManager->mUniformBuffer.mData.mNumLight = mLightManager->mLights->size();
	
	{
		int i = 0;
		for (auto& it : *mLightManager->mLights)
		{
			it->fillUniform(i, mLightManager->mUniformBuffer.mData);
			i++;
		}
	}

	mLightManager->mUniformBuffer.mData.mEyePos = mCamera.mPosition;
	mLightManager->mUniformBuffer.updateData();

	mSceneMat->bindDescriptor("uboFrag", mLightManager->mUniformBuffer.mDescriptorSets);
}

void WidgetSceneApp::drawActors(bool& pSelectedActor)
{
	bindLights();

	for (Editor::Data::Actor* actor : mActors)
	{
		if (actor->id == mTreeItem->mIdSelected && pSelectedActor)
		{
			mTreeItem->mSelectedActor = actor;

			QModelIndex idx = mTreeItem->mTreeModel.indexFromItem((Editor::Widget::WidgetGameObjectTreeItem*)actor->mItemOwner);
			mTreeItem->setCurrentIndex(idx);

			pSelectedActor = false;
		}

		actor->updateProj(mCamera.viewProj());
		actor->draw(renderer->getCurrentCommandBuffer(), mSceneMat);

#ifdef NSHIPPING
		actor->drawCamera(renderer->getCurrentCommandBuffer());
#endif
	}
}

void WidgetSceneApp::drawCollider()
{
	service(Physics::Core::BulPhysicsEngine).debugDrawWorld(mCamera.viewProj());
}

void WidgetSceneApp::drawSky()
{
	skyBox->updateUniform(mCamera.viewProj(), mCamera.mPosition, mCamera.mFar - 50);
	skyBox->draw(renderer->getCurrentCommandBuffer(), *skyMat);
}

void WidgetSceneApp::drawArrow()
{
	if (mTreeItem->mSelectedActor != nullptr)
	{
		mTreeItem->mSelectedActor->mUniformBuferOutLine.mData.mViewProjection = mCamera.viewProj();
		Maths::FMatrix4 model = mTreeItem->mSelectedActor->getTransform()->getWorldMatrix();
		model.data[0][0] += outLineWidth;
		model.data[1][1] += outLineWidth;
		model.data[2][2] += outLineWidth;
		mTreeItem->mSelectedActor->mUniformBuferOutLine.mData.mModel = model;
		mTreeItem->mSelectedActor->mUniformBuferOutLine.updateData();


		mOutline->bindDescriptor("ubo", mTreeItem->mSelectedActor->mUniformBuferOutLine.mDescriptorSets);
		mOutline->bindPipeLine(renderer->getCurrentCommandBuffer());
		mTreeItem->mSelectedActor->draw(renderer->getCurrentCommandBuffer());

#ifdef NSHIPPING
		mTreeItem->mSelectedActor->drawCamera(renderer->getCurrentCommandBuffer());
#endif

		pickingArrow->mActor = mTreeItem->mSelectedActor;
		pickingArrow->drawScene(renderer->getCurrentCommandBuffer(), mCamera.viewProj(), mCamera.mPosition);
	}
}

void WidgetSceneApp::drawParticle()
{
	for (Editor::Data::Actor* actor : mActors)
	{
		actor->updateParticle(mClock.getDeltaTime(), mCamera.mPosition);
		actor->drawParticle(renderer->getCurrentCommandBuffer(), *ParticleMat, mCamera.viewProj(), mCamera.mView);
	}
}

void WidgetSceneApp::cleanupScene()
{	
	renderer->waitForCleanUp();
	offScreenRenderer->waitForCleanUp();

	std::unique_lock<std::mutex> lck(closeMut);
	closed = true;
	cv.notify_all();
	
	delete mSceneMat;
	delete mOutline;
	delete mPickingMat;
	delete skyMat;
	delete ParticleMat;

	delete pickingArrow;
	delete skyBox;

	if (mLightManager != nullptr)
		delete mLightManager;

	delete offScreenRenderer;
}