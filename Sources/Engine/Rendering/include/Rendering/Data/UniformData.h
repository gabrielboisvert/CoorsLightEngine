#pragma once
#include "Maths/FMatrix4.h"

namespace Rendering::Data
{
	struct UniformData
	{
		alignas(16) Maths::FMatrix4 mModel;
		alignas(16) Maths::FMatrix4 mViewProjection;
	};
}