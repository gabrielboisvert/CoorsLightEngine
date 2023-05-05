#pragma once
#include <functional>
#include <deque>
#include <mutex>

namespace Rendering::Data
{
	struct DeletionQueue
	{
		std::deque<std::function<void()>> mDeletors;
		std::mutex mMutex;

		void pushFunction(std::function<void()>&& pFunction);

		void flush();
	};
}
