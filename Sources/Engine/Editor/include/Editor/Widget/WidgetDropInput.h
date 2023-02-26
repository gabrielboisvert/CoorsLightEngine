#pragma once
#include "WidgetInspectorApp.h"
#include <QtWidgets/qlineedit.h>
#include <QtGui/qevent.h>
#include "Game/Component/AComponent.h"

namespace Editor::Widget
{
	class WidgetDropInput : public QLineEdit
	{
		public:
			Game::Component::AComponent& mComponent;
			QLabel& mName;


			WidgetDropInput(QLabel& pName, Game::Component::AComponent& pComponent);
			void dragEnterEvent(QDragEnterEvent* e) override;
			void dropEvent(QDropEvent* e) override;
	};
}