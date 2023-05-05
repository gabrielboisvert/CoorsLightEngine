#include "Game/Component/CPParticle.h"
#include <Game/Utils/ComponentType.h>
#include <EngineCore/Service/ServiceLocator.h>
#ifdef NSHIPPING
	#include "Editor/Widget/WidgetEditor.h"
#endif
#include "Game/SceneSys/SceneManager.h"
#include "Game/Data/Actor.h"
#include "EngineCore/ResourceManagement/ResourceManager.h"
#include <filesystem>
#include "Tools/Utils/PathParser.h"

using namespace Game::Component;

CPParticle::~CPParticle()
{
	bool a = 0;
}

AComponent* CPParticle::clone()
{
	return new CPParticle(*this);
}

void CPParticle::setParticle(const std::string& pName, const char* pPath)
{
	mName = pName;
	mPath = pPath;

	Rendering::Resources::ParticleSystem* master = service(EngineCore::ResourcesManagement::ResourceManager).create<Rendering::Resources::ParticleSystem>(pPath, pPath);

	mPat = master->getInstance();

#ifdef NSHIPPING
	mPatEditor = master->getInstance(true);
#endif
}

void CPParticle::setParticle(const char* pPath)
{
	if (pPath == nullptr)
	{
		mName = "";
		mPath = "";
		mPat = nullptr;
		return;
	}
	
	mPath = pPath;
	Rendering::Resources::ParticleSystem* master = service(EngineCore::ResourcesManagement::ResourceManager).create<Rendering::Resources::ParticleSystem>(pPath, pPath);

	mPat = master->getInstance();

#ifdef NSHIPPING
	mPatEditor = master->getInstance(true);
#endif
}

void CPParticle::setParticleWithLua(const char* pPath)
{
	std::string currentPath = service(Game::SceneSys::SceneManager).mProjectPath + "/";
	mPath = currentPath + pPath;

	Rendering::Resources::ParticleSystem* master = service(EngineCore::ResourcesManagement::ResourceManager).create<Rendering::Resources::ParticleSystem>(mPath, mPath);

	mPat = master->getInstance();

#ifdef NSHIPPING
	mPatEditor = master->getInstance(true);
#endif
}

void CPParticle::OnStart()
{
	if (!playOnAwake || mPat == nullptr)
		return;
	
	playing = true;
}

void CPParticle::OnUpdate(float pDeltaTime)
{
	if (!playing || mPat == nullptr || mStop)
		return;

	Game::Data::Actor* actor = (Game::Data::Actor*)mOwner;
	
	if (actor != nullptr && Game::Component::CPCamera::mWorldCamera != nullptr)
		mPat->update(pDeltaTime * mPlaybackSpeed, actor->getTransform()->mWorldPosition, Game::Component::CPCamera::mWorldCamera->mPosition, loop);
}

void CPParticle::OnUpdateEditor(float pDeltaTime, Maths::FVector3& pCamPos)
{
#ifdef NSHIPPING
	if (mPatEditor == nullptr || !playInEditor || mStop)
		return;

	Game::Data::Actor* actor = (Game::Data::Actor*)mOwner;
	if (actor != nullptr)
		mPatEditor->update(pDeltaTime * mPlaybackSpeed, actor->getTransform()->mWorldPosition, pCamPos, true);
#endif
}

void CPParticle::OnPaused()
{
	playing = !playing;
}

void CPParticle::OnDestroy()
{
	OnUpdate(0);
	playing = false;
}

void CPParticle::play()
{
	paused = false;
	mStop = false;
	playing = true;
}

void CPParticle::pause()
{
	paused = true;
	playing = false;
}

void CPParticle::stop()
{
	mStop = true;
}

void CPParticle::draw(void* pCmd, Rendering::Data::Material& pPipeLine, Maths::FMatrix4& pViewProj, Maths::FMatrix4& pView)
{
	if (mPat == nullptr || mStop)
		return;

	mPat->draw(VkCommandBuffer(pCmd), pPipeLine, pViewProj, pView);
}

void CPParticle::drawEditor(void* pCmd, Rendering::Data::Material& pPipeLine, Maths::FMatrix4& pViewProj, Maths::FMatrix4& pView)
{
#ifdef NSHIPPING
	if (mPatEditor == nullptr || mStop)
		return;

	mPatEditor->draw(VkCommandBuffer(pCmd), pPipeLine, pViewProj, pView);
#endif
}

void CPParticle::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter)
{
	pWriter.StartObject();

	pWriter.Key("Type");
	pWriter.Int((int)Game::Utils::ComponentType::Particle);

	pWriter.Key("Name");
	pWriter.String(mName.c_str());

#ifdef NSHIPPING
	std::string currentPath = Editor::Utils::qStringToStdString(service(Editor::Widget::WidgetEditor).mLocation.mFolder);
	pWriter.Key("Path");
	pWriter.String(mPath.empty() ? "" : mPath.substr(currentPath.length()).c_str());
#endif

	pWriter.Key("PlayOnAwake");
	pWriter.Bool(playOnAwake);

	pWriter.Key("Loop");
	pWriter.Bool(loop);

	pWriter.Key("PlayInEditor");
	pWriter.Bool(playInEditor);

	pWriter.Key("PlayBackSpeed");
	pWriter.Double(mPlaybackSpeed);

	pWriter.EndObject();
}