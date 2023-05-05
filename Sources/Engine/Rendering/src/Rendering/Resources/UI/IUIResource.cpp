#include "Rendering/Resources/UI/IUIResource.h"
#include "Game/Component/CPCamera.h"
#include "EngineCore/Core/EngineApp.h"
using namespace Rendering::Resources::UIResource;

bool Rect::contain(float x, float y)
{
	return contain({ x, y });
}

bool Rect::contain(Maths::FVector2 p)
{
	return p.x >= corner.x && p.y >= corner.y && p.x <= corner.x + size.x && p.y <= corner.y + size.y;
}

bool IUIResource::contain(float x, float y)
{
	if (isHidden)
		return false;

	return contain({ x, y });
}

bool IUIResource::contain(Maths::FVector2 p)
{
	if (isHidden)
		return false;

	return position.contain(p);
}

void Rendering::Resources::UIResource::IUIResource::setColor(Maths::FVector4 pNewColor)
{
	current->color = pNewColor;
}

void Rendering::Resources::UIResource::IUIResource::setVisible(bool pIsVisible)
{
	isHidden = !pIsVisible;
}

void Rendering::Resources::UIResource::IUIResource::setScale(Maths::FVector2 pNewScale)
{
	EngineCore::Core::EngineApp& app = service(EngineCore::Core::EngineApp);

	position.ratio = pNewScale;
	updateData(app.uiProj, app.mWidth, app.mHeight);
}

Maths::FVector2 Rendering::Resources::UIResource::IUIResource::getScale()
{
	return position.ratio;
}
