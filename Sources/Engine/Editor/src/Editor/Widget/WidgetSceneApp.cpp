#include <QtWidgets/qboxlayout.h>

#include "Editor/Widget/WidgetSceneApp.h"

using namespace Editor::Widget;

WidgetSceneApp::WidgetSceneApp(EngineCore::Core::EngineApp& mApp, QSettings& pSetting, QWidget* pParen) :
	ads::CDockWidget("Scene")
{
	QWidget* content = new QWidget(pParen);
	setWidget(content);

	QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, nullptr);
	layout->setContentsMargins(5, 0, 5, 5);
	layout->setSpacing(0);
	content->setLayout(layout);


	windowId = mApp.mWindow.getWindowId();
	qWindow = QWindow::fromWinId((WId)windowId);
	renderingWindow = QWidget::createWindowContainer(qWindow, nullptr);

	layout->addWidget(renderingWindow);
}