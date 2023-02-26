#include "Editor/Widget/WidgetGameObjectTreeItem.h"
#include <QtWidgets/qmenu.h>
#include <QtGui/qicon.h>
#include <QtCore/qobject.h>
#include <QtGui/qwindow.h>

#include "Editor/Widget/WidgetMenuSperarator.h"
#include "Editor/Widget/WidgetGameObjectTree.h"
#include "EngineCore/Service/ServiceLocator.h"
#include "Game/SceneSys/SceneManager.h"
#include "EngineCore/Thread/ThreadPool.h"
#include "Editor/Widget/WidgetConsole.h"
#include "Editor/Utils/Utils.h"
#include <QtWidgets/qlabel.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qpushbutton.h>


using namespace Editor::Widget;

WidgetGameObjectTreeItem::WidgetGameObjectTreeItem(QSettings& pSetting, const QString& pText) 
	: QStandardItem(pText), mDatas(pSetting, this), mSetting(pSetting)
{
	service(Game::SceneSys::SceneManager).mCurrentScene->addActor(mActor);
	service(EngineCore::Thread::ThreadPool).queueJob([pText]
	{
		WidgetConsole::infoPrint("%s Created", Utils::qStringToStdString(pText));
	});
}

WidgetGameObjectTreeItem::WidgetGameObjectTreeItem(const WidgetGameObjectTreeItem& pOther) 
	: QStandardItem(pOther.text()), mActor(pOther.mActor), mDatas(mSetting, this), mSetting(pOther.mSetting)
{
	QString str = pOther.text();

	service(Game::SceneSys::SceneManager).mCurrentScene->addActor(mActor);
	service(EngineCore::Thread::ThreadPool).queueJob([str]
	{
		WidgetConsole::infoPrint("%s Created", Utils::qStringToStdString(str));
	});
}

WidgetGameObjectTreeItem::~WidgetGameObjectTreeItem()
{
	//TODO remove item from scene
	service(Game::SceneSys::SceneManager).mCurrentScene->mActors.remove(&mActor);
}

void WidgetGameObjectTreeItem::onItemChanged()
{
	if (mModel == nullptr)
		return;

	((WidgetGameObjectTreeModel*)mModel)->mShouldEdit = false;
	mModel = nullptr;

	mDatas.mGameObjectName->setText(text());
}

void WidgetGameObjectTreeItem::showContextMenu(QSettings& pSetting, const QPoint& pPoint, QWidget* pParent)
{
	QMenu contextMenu(pParent);

	WidgetMenuSeperator sepaAssets(pSetting, pSetting.value("CategoryData").toString(), pSetting.value("SeparatorIcon").toString(), &contextMenu);
	contextMenu.addAction(sepaAssets.mAction);

	QAction create(QIcon(pSetting.value("CategoryCreateIcon").toString()), pSetting.value("CategoryCreateName").toString());
	pParent->connect(&create, &QAction::triggered, pParent, [pParent, this] { this->create(pParent);  });
	contextMenu.addAction(&create);

	QAction rename(QIcon(pSetting.value("CategoryRenameIcon").toString()), pSetting.value("CategoryRenameName").toString());
	contextMenu.addAction(&rename);
	pParent->connect(&rename, &QAction::triggered, pParent, [pParent, this] { this->rename(pParent); });

	QAction copy(QIcon(pSetting.value("CategoryDataCopieIcon").toString()), pSetting.value("CategoryDataCopieName").toString());
	contextMenu.addAction(&copy);
	pParent->connect(&copy, &QAction::triggered, pParent, [pParent, this] { this->copy(pParent); });


	QString strIcon = (((WidgetGameObjectTree*)pParent)->mCopiedActor == nullptr) ? pSetting.value("CategoryDataPasteDisabledIcon").toString() : pSetting.value("CategoryDataPasteIcon").toString();
	QAction paste(QIcon(strIcon), pSetting.value("CategoryDataPasteName").toString());
	contextMenu.addAction(&paste);
	pParent->connect(&paste, &QAction::triggered, pParent, [pParent, this] { this->paste(pParent); });
	if (((WidgetGameObjectTree*)pParent)->mCopiedActor == nullptr)
		paste.setDisabled(true);


	QAction aDelete(QIcon(pSetting.value("CategoryDeleteIcon").toString()), pSetting.value("CategoryDeleteName").toString());
	contextMenu.addAction(&aDelete);
	pParent->connect(&aDelete, &QAction::triggered, pParent, [pParent, this] { this->deleteItem(pParent); });
	
	contextMenu.exec(pParent->mapToGlobal(pPoint));
}

void WidgetGameObjectTreeItem::appendRow(WidgetGameObjectTreeItem* pItem)
{
	Maths::FTransform* transform = ((WidgetGameObjectTreeItem*)pItem)->mActor.getTransform();
	if (transform->mParent == this->mActor.getTransform())
		return;

	transform->setParent(*mActor.getTransform());
	service(EngineCore::Thread::ThreadPool).queueJob([=]
	{
		WidgetConsole::infoPrint("%s became child of %s", Utils::qStringToStdString(pItem->text()), Utils::qStringToStdString(text()));
	});
	QStandardItem::appendRow(pItem);
}

void WidgetGameObjectTreeItem::appendRow(QList<QStandardItem*>& pItems)
{
	for (QStandardItem* item : pItems)
	{
		Maths::FTransform* transform = ((WidgetGameObjectTreeItem*)item)->mActor.getTransform();
		if (transform->mParent == this->mActor.getTransform())
			continue;

		transform->setParent(*mActor.getTransform());
		service(EngineCore::Thread::ThreadPool).queueJob([=]
		{
			WidgetConsole::infoPrint("%s became child of %s", Utils::qStringToStdString(item->text()), Utils::qStringToStdString(text()));
		});
	}
	QStandardItem::appendRow(pItems);
}

void WidgetGameObjectTreeItem::create(QWidget* pParent)
{
	WidgetGameObjectTree* tree = ((WidgetGameObjectTree*)pParent);

	WidgetGameObjectTreeItem* item = new WidgetGameObjectTreeItem(mSetting, "GameObject");
	WidgetGameObjectTreeItem::appendRow(item);

	QModelIndex idx = tree->mTreeModel.indexFromItem(item);

	tree->expand(tree->mTreeModel.indexFromItem(this));
	tree->setCurrentIndex(idx);
	
	item->rename(pParent);
}

void WidgetGameObjectTreeItem::rename(QWidget* pParent)
{
	WidgetGameObjectTree* tree = ((WidgetGameObjectTree*)pParent);
	tree->mTreeModel.mShouldEdit = true;

	QModelIndex idx = tree->mTreeModel.indexFromItem(this);
	tree->edit(idx);

	mModel = &tree->mTreeModel;
}

void WidgetGameObjectTreeItem::copy(QWidget* pParent)
{
	((WidgetGameObjectTree*)pParent)->mCopiedActor = this;
}

void WidgetGameObjectTreeItem::paste(QWidget* pParent)
{
	WidgetGameObjectTree* tree = ((WidgetGameObjectTree*)pParent);

	WidgetGameObjectTreeItem* item = new WidgetGameObjectTreeItem(*tree->mCopiedActor);
	appendRow(item);
	QModelIndex idx = tree->mTreeModel.indexFromItem(item);
	tree->setCurrentIndex(idx);
	tree->expand(tree->mTreeModel.indexFromItem(this));
	tree->mCopiedActor = nullptr;
}

void WidgetGameObjectTreeItem::deleteItem(QWidget* pParent)
{
	QString str = text();
	service(EngineCore::Thread::ThreadPool).queueJob([str]
	{
		WidgetConsole::infoPrint("%s is deleted", Utils::qStringToStdString(str));
	});

	WidgetGameObjectTree* tree = ((WidgetGameObjectTree*)pParent);
	QModelIndex idx = tree->mTreeModel.indexFromItem(this);
	tree->mTreeModel.removeRow(idx.row(), idx.parent());
}