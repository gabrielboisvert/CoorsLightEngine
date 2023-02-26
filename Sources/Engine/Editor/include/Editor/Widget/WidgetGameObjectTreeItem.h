#pragma once
#include <QtGui/qstandarditemmodel.h>
#include <QtCore/qpoint.h>
#include <QtCore/qsettings.h>
#include "Game/Data/Actor.h"
#include "Editor/Widget/WidgetInspectorData.h"

namespace Editor::Widget
{
	class WidgetGameObjectTreeItem : public QStandardItem
	{
		public:
			QStandardItemModel* mModel = nullptr;
			Game::Data::Actor mActor;
			WidgetInspectorData mDatas;
			
			QSettings& mSetting;

			WidgetGameObjectTreeItem(QSettings& pSetting, const QString& pText);
			WidgetGameObjectTreeItem(const WidgetGameObjectTreeItem& pOther);
			~WidgetGameObjectTreeItem();
			void onItemChanged();
			void showContextMenu(QSettings& pSetting, const QPoint& pPoint, QWidget* pParent);

			void create(QWidget* pParent);
			void rename(QWidget* pParent);
			void copy(QWidget* pParent);
			void paste(QWidget* pParent);
			void deleteItem(QWidget* pParent);

			void appendRow(WidgetGameObjectTreeItem* pItem);
			void appendRow(QList<QStandardItem*>& pItems);


	};
}