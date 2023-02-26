#pragma once

namespace EngineWindow::Data
{
	struct Key
	{
		bool mPressed = false;
		bool mRelease = !mRelease;
	};
}