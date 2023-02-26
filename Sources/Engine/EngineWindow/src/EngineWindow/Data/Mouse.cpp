#include "EngineWindow/Data/Mouse.h"

using namespace EngineWindow::Data;

Maths::FVector2 Mouse::getPositionDifference()
{
	Maths::FVector2 res = mOldPosition - mPosition;
	mOldPosition = mPosition;
	return res;
}

bool Mouse::isLeftPressed()
{
	return mLeft.mPressed;
}

bool Mouse::isRightPressed()
{
	return mRight.mPressed;
}

bool Mouse::isScrolling()
{
	return mScroll != 0;
}