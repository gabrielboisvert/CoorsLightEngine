#include "Rendering/Data/DeletionQueue.h"

using namespace Rendering::Data;

void DeletionQueue::pushFunction(std::function<void()>&& pFunction)
{
	std::unique_lock lock(mMutex);
	mDeletors.push_back(pFunction);
}

void DeletionQueue::flush()
{
	std::unique_lock lock(mMutex);
	for (auto it = mDeletors.rbegin(); it != mDeletors.rend(); it++)
		(*it)();

	mDeletors.clear();
}