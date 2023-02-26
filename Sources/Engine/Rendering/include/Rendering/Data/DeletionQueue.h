#pragma once
#include <functional>
#include <deque>

namespace Rendering::Data
{
	struct DeletionQueue
	{
		std::deque<std::function<void()>> mDeletors;

		void pushFunction(std::function<void()>&& pFunction);

		void flush();
	};
}
