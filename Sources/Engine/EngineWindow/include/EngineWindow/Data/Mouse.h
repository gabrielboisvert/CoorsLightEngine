#pragma once
#include "Maths/FVector2.h"
#include "EngineWindow/Data/Key.h"

namespace EngineWindow::Data
{
	struct Mouse
	{
		bool mCursorHidden = false;
		Maths::FVector2 mPosition;
		Maths::FVector2 mOldPosition;
		Key mLeft;
		Key mRight;
		float mScroll = 0;

		Maths::FVector2 getPositionDifference();
		bool isLeftPressed();
		bool isRightPressed();
		bool isScrolling();
	};
}