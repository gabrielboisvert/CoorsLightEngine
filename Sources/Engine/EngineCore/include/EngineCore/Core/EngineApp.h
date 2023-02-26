#pragma once
#include "EngineWindow/Core/Window.h"

namespace EngineCore::Core
{
	class EngineApp
	{
	public:
		bool mPlaying = false;

		EngineWindow::Core::Window mWindow;

		EngineApp(unsigned int pWidth, unsigned int pHeight);
		void run();
		void close();
	};
}