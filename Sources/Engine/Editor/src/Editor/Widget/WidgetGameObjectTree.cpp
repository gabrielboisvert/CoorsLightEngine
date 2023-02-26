#pragma once
#include <QtCore/qdiriterator.h>
#include <QtCore/qfileinfo.h>
#include <QtWidgets/qmenu.h>

#include "Editor/Widget/WidgetGameObjectTree.h"
#include "Editor/Widget/WidgetGameObjectTreeItem.h"
#include "Editor/Utils/Utils.h"
#include <QtWidgets/qheaderview.h>
#include "Editor/Widget/WidgetMenuSperarator.h"
#include "EngineCore/Thread/ThreadPool.h"
#include "Editor/Widget/WidgetConsole.h"
#include "EngineCore/Service/ServiceLocator.h"

using namespace Editor::Widget;


WidgetGameObjectTree::WidgetGameObjectTree(QSettings& pSetting, WidgetInspectorApp& pInspector) :
	QTreeView(), mSetting(pSetting), mInspector(pInspector)
{
	setSelectionMode(QAbstractItemView::SingleSelection);
	setSelectionBehavior(QAbstractItemView::SelectItems);

	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	connect(this, &QTreeView::collapsed, this, &WidgetGameObjectTree::collapse);
	connect(this, &QTreeView::expanded, this, &WidgetGameObjectTree::expand);

	setObjectName("Tree");
	setFocusPolicy(Qt::NoFocus);

	setModel(&mTreeModel);
	setAcceptDrops(true);
	viewport()->setAcceptDrops(true);
	setDropIndicatorShown(true);
	setDragEnabled(true);
	setDragDropMode(QAbstractItemView::DragDrop);
	setDefaultDropAction(Qt::MoveAction);

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QTreeView::customContextMenuRequested, this, &WidgetGameObjectTree::showContextMenu);
	connect(&mTreeModel, &WidgetGameObjectTreeModel::itemChanged, this, &WidgetGameObjectTree::onItemChanged);
}

WidgetGameObjectTree::~WidgetGameObjectTree()
{
}

void WidgetGameObjectTree::initHeader(const QString& pProjectName)
{
	mTreeModel.setHorizontalHeaderItem(0, new QStandardItem(pProjectName));
	header()->setSectionResizeMode(QHeaderView::Fixed);
}

void WidgetGameObjectTree::append(WidgetGameObjectTreeItem* pItem)
{
	mTreeModel.appendRow(pItem);
}

void WidgetGameObjectTree::append(WidgetGameObjectTreeItem* pParent, WidgetGameObjectTreeItem* pItem)
{
	pParent->appendRow(pItem);
	expand(mTreeModel.indexFromItem(pParent));
}

void WidgetGameObjectTree::showContextMenu(const QPoint& pPoint)
{
	QModelIndex index = indexAt(pPoint);
	if (index.isValid()) 
	{
		WidgetGameObjectTreeItem* item = (WidgetGameObjectTreeItem*)mTreeModel.itemFromIndex(index);
		item->showContextMenu(mSetting, pPoint, this);
		return;
	}
	
	QMenu contextMenu(tr("Context menu"), this);

	WidgetMenuSeperator sepaAssets(mSetting, mSetting.value("CategoryScene").toString(), mSetting.value("SeparatorIcon").toString(), &contextMenu);
	contextMenu.addAction(sepaAssets.mAction);

	QAction create(QIcon(mSetting.value("CategoryCreateIcon").toString()), mSetting.value("CategoryCreateName").toString(), this);
	connect(&create, &QAction::triggered, this, &WidgetGameObjectTree::create);
	contextMenu.addAction(&create);

	QString strIcon = (mCopiedActor == nullptr) ? mSetting.value("CategoryDataPasteDisabledIcon").toString() : mSetting.value("CategoryDataPasteIcon").toString();
	QAction paste(QIcon(strIcon), mSetting.value("CategoryDataPasteName").toString(), this);
	contextMenu.addAction(&paste);
	connect(&paste, &QAction::triggered, this, &WidgetGameObjectTree::paste);
	if (mCopiedActor == nullptr)
		paste.setDisabled(true);

	contextMenu.exec(mapToGlobal(pPoint));
}

void WidgetGameObjectTree::create()
{
	WidgetGameObjectTreeItem* item = new WidgetGameObjectTreeItem(mSetting, "GameObject");
	append(item);
	QModelIndex idx = mTreeModel.indexFromItem(item);

	
	setCurrentIndex(idx);
	item->rename(this);
}

void WidgetGameObjectTree::paste()
{
	WidgetGameObjectTreeItem* item = new WidgetGameObjectTreeItem(*mCopiedActor);
	append(item);
	QModelIndex idx = mTreeModel.indexFromItem(item);
	setCurrentIndex(idx);
	mCopiedActor = nullptr;
}

void WidgetGameObjectTree::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
	QTreeView::currentChanged(current, previous);

	mTreeModel.mShouldEdit = false;
}

void WidgetGameObjectTree::dragEnterEvent(QDragEnterEvent* e)
{
	if (e->mimeData()->hasFormat("Actor"))
		e->acceptProposedAction();
}

void WidgetGameObjectTree::dropEvent(QDropEvent* e)
{
	QModelIndex idx = indexAt(e->pos());
	if (!idx.isValid())
		return;

	WidgetGameObjectTreeItem* destinationItem = dynamic_cast<WidgetGameObjectTreeItem*>(mTreeModel.itemFromIndex(idx));
	if (destinationItem == nullptr)
		return;

	WidgetGameObjectTreeItem* sourceItem = nullptr;
	if (e->mimeData()->hasFormat("Actor"))
	{
		sourceItem = (WidgetGameObjectTreeItem*)mTreeModel.dropingItem;
	}

	DropIndicatorPosition dropIndicator = dropIndicatorPosition();
	switch (dropIndicator)
	{
		case QAbstractItemView::AboveItem: reorder(-1, sourceItem, destinationItem);
			break;
		case QAbstractItemView::BelowItem: reorder(1, sourceItem, destinationItem);
			break;
		case QAbstractItemView::OnItem: reorder(0, sourceItem, destinationItem);
			break;
	}

	QWidget::dropEvent(e);
}

void WidgetGameObjectTree::insertRow(int pRow, QList<QStandardItem*>& pItems)
{
	for (QStandardItem* item : pItems)
	{
		Maths::FTransform* transform = ((WidgetGameObjectTreeItem*)item)->mActor.getTransform();
		if (!transform->hasParent())
			continue;

		transform->removeParent();
		service(EngineCore::Thread::ThreadPool).queueJob([=]
		{
			WidgetConsole::infoPrint("Remove parent from %s", Utils::qStringToStdString(item->text()));
		});
	}

	mTreeModel.insertRow(pRow, pItems);
}

void WidgetGameObjectTree::insertRow(int pRow, QStandardItem* pParent, QList<QStandardItem*>& pItems)
{
	for (QStandardItem* item : pItems)
	{
		Maths::FTransform* itemTransform = ((WidgetGameObjectTreeItem*)item)->mActor.getTransform();
		Maths::FTransform* parentTransform = ((WidgetGameObjectTreeItem*)pParent)->mActor.getTransform();

		itemTransform->setParent(*parentTransform);
		service(EngineCore::Thread::ThreadPool).queueJob([=]
		{
			WidgetConsole::infoPrint("%s became child of %s", Utils::qStringToStdString(item->text()), Utils::qStringToStdString(pParent->text()));
		});
	}
	pParent->insertRow(pRow, pItems);
}

void WidgetGameObjectTree::reorder(int pSide, WidgetGameObjectTreeItem* pSrc, WidgetGameObjectTreeItem* pDist)
{
	QModelIndex idxSrc = mTreeModel.indexFromItem(pSrc);
	QList<QStandardItem*> itemList = pSrc->parent() == nullptr? mTreeModel.takeRow(idxSrc.row()): pSrc->parent()->takeRow(idxSrc.row());
	QModelIndex idxdist = mTreeModel.indexFromItem(pDist);
	
	if (pSide < 0)
		pDist->parent() == nullptr ? insertRow(idxdist.row(), itemList) : insertRow(idxdist.row(), pDist->parent(), itemList);
	else if (pSide > 0)
		pDist->parent() == nullptr ? insertRow(idxdist.row() + 1, itemList) : insertRow(idxdist.row() + 1, pDist->parent(), itemList);
	else
	{
		pDist->appendRow(itemList);
		expand(idxdist);
	}
}

void WidgetGameObjectTree::dragMoveEvent(QDragMoveEvent* e)
{
	QTreeView::dragMoveEvent(e);
}

void WidgetGameObjectTree::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QTreeView::selectionChanged(selected, deselected);


	QModelIndexList indexes = selected.indexes();

	if (indexes.size() == 0)
		mInspector.updateInspector(nullptr);

	for(QModelIndex& index: indexes)
		mInspector.updateInspector((WidgetGameObjectTreeItem*)mTreeModel.itemFromIndex(index));
}

void WidgetGameObjectTree::onItemChanged(QStandardItem* pItem)
{
	((WidgetGameObjectTreeItem*)pItem)->onItemChanged();
}