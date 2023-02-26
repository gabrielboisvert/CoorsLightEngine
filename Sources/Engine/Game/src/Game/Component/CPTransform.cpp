#pragma once
#include "Game/Component/CPTransform.h"

using namespace Game::Component;

AComponent* CPTransform::clone()
{
	CPTransform* clone = new CPTransform(*this);
	clone->notifier.m_notificationHandlers.clear();
	clone->notifier.m_availableHandlerID = 0;
	clone->mParent = nullptr;

	return clone;
}