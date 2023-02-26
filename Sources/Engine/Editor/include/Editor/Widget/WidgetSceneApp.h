#include <QtDocking/DockWidget.h>
#include <QtCore/qsettings.h>
#include <QtGui/qwindow.h>
#include "EngineCore/Core/EngineApp.h"

namespace Editor::Widget
{
	class WidgetSceneApp : public ads::CDockWidget
	{
		public:
			HWND windowId;
			QWindow* qWindow;
			QWidget* renderingWindow;

			WidgetSceneApp(EngineCore::Core::EngineApp& mApp, QSettings& pSetting, QWidget* pParent = nullptr);
	};
}