#include "Editor/Widget/WidgetInspectorApp.h"
#include <QtCore/qfileinfo.h>
#include "Tools/Utils/PathParser.h"
#include <QtCore/qmimedata.h>
#include "Editor/Utils/Utils.h"

using namespace Editor::Widget;

WidgetInspectorApp::WidgetInspectorApp(QSettings& pSetting, QWidget* pParent) : ads::CDockWidget("Inspector"), mSetting(pSetting)
{
	QWidget* content = new QWidget(pParent);
	setWidget(content);

	mLayout->setContentsMargins(5, 0, 5, 5);
	mLayout->setSpacing(0);
	content->setLayout(mLayout);

	setAcceptDrops(true);
}

void WidgetInspectorApp::dragEnterEvent(QDragEnterEvent* pEvent)
{
	pEvent->acceptProposedAction();
}

void WidgetInspectorApp::dropEvent(QDropEvent* pEvent)
{
	if (mSelectedItem == nullptr)
		return;

	QFileInfo info = QFileInfo(pEvent->mimeData()->text());
	if (Tools::Utils::PathParser::getFileType(info.suffix()) == Tools::Utils::PathParser::EFileType::MODEL)
		mSelectedItem->mDatas.setModel(info);
}

WidgetInspectorApp::~WidgetInspectorApp()
{
}


void WidgetInspectorApp::updateInspector(WidgetGameObjectTreeItem* pItem)
{	
	mSelectedItem = pItem;
	if (mData != nullptr)
	{
		mLayout->removeWidget(mData);
		mData->hide();
		mData = nullptr;
	}

	if (pItem == nullptr)
		return;


	mData = &pItem->mDatas;
	mLayout->addWidget(mData, 0, Qt::AlignTop);
	mData->show();
}
