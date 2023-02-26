#include "Rendering/Data/DeletionQueue.h"

using namespace Rendering::Data;

void DeletionQueue::pushFunction(std::function<void()>&& pFunction)
{
	mDeletors.push_back(pFunction);
}

void DeletionQueue::flush()
{
	for (auto it = mDeletors.rbegin(); it != mDeletors.rend(); it++)
		(*it)();

	mDeletors.clear();
}