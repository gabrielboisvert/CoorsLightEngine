#include "Scripting/Binders/LuaActorBinder.h"
#include "Game/Data/Actor.h"
void Scripting::Binder::LuaActorBinder::bind(sol::state& pLuaState)
{
	pLuaState.new_usertype<Game::Data::Actor>("Actor",
		//Transform
		"GetTransform", &Game::Data::Actor::getTransform,
		"SetTransform", &Game::Data::Actor::setTransform,  //ResetsPhysics
		//Model
		"GetModel", &Game::Data::Actor::getModel,
		"GetCollider", &Game::Data::Actor::getCollider,
		//Tags
		"SetTag", &Game::Data::Actor::setTag,
		"GetTag", &Game::Data::Actor::getTag,
		"GetTagCount", &Game::Data::Actor::getNumtags,
		"ClearAllTags", &Game::Data::Actor::clearTags,
		"DeleteTag", &Game::Data::Actor::deleteTag,
		"ChangeTag", &Game::Data::Actor::changeTag,
		"GetCamera", &Game::Data::Actor::getCamera,
		"GetScript", &Game::Data::Actor::getScript,
		"GetSoundByName", &Game::Data::Actor::getSoundByName,
		"GetSound", &Game::Data::Actor::getSound,
		"GetUI", &Game::Data::Actor::getUI,
		"GetParticle", &Game::Data::Actor::getParticle,
		"GetAnimator", &Game::Data::Actor::getAnimator);
}