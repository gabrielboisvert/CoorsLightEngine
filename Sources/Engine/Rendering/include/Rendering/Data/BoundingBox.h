#pragma once
#include "Maths/FVector3.h"

namespace Rendering::Data
{
	struct BoundingBox
	{
		Maths::FVector3 mMin;
		Maths::FVector3 mMax;
		Maths::FVector3 mSize;
	};
}