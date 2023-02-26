#include "Editor/Widget/WidgetInspectorData.h"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qmenu.h>
#include "Editor/Widget/WidgetGameObjectTreeItem.h"
#include "Editor/Widget/WidgetCollapse.h"
#include "Editor/Widget/WidgetDropInput.h"
#include "Editor/Widget/WidgetMenuSperarator.h"
#include <QtWidgets/qfiledialog.h>
#include "Editor/Utils/Utils.h"
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qformlayout.h>
#include "Game/Utils/ComponentType.h"
#include <QtWidgets/qcombobox.h>
#include "Physics/Data/TypeRigidBody.h"

using namespace Editor::Widget;

WidgetInspectorData::WidgetInspectorData(QSettings& pSetting, QStandardItem* pItem)
	: mItem(pItem), mSetting(pSetting)
{
	setObjectName("Inspector");

	mLayout->setSpacing(5);
	mLayout->setContentsMargins(0, 0, 0, 5);
	setLayout(mLayout);

	setObjectNameEdit();
	setCreateButton();

	initTransforms();
}

void WidgetInspectorData::setObjectNameEdit()
{
	mGameObjectName = new QLineEdit();
	mGameObjectName->setObjectName("GameObject");
	mGameObjectName->setText(mItem->text());
	connect(mGameObjectName, &QLineEdit::textChanged, this, [=]
	{ 
		mItem->setText(mGameObjectName->text());
	});

	mLayout->addWidget(mGameObjectName, 0, Qt::AlignTop);
}

void WidgetInspectorData::setCreateButton()
{
	// Add Component
	mCreateComponent = new QPushButton("Add Component", nullptr);
	mCreateComponent->setObjectName("AddComponent");
	mLayout->addWidget(mCreateComponent, 0, Qt::AlignTop);


	QMenu* menu = new QMenu();
	menu->setStyleSheet("Qmenu:item { background-color: #383838; }");
	menu->installEventFilter(this);
	mCreateComponent->setMenu(menu);


	//Renderer
	QAction* newAction = nullptr;
	{
		WidgetMenuSeperator* sepa = new WidgetMenuSeperator(mSetting, mSetting.value("CategoryInspectorModel").toString(), mSetting.value("SeparatorIcon").toString(), menu);
		menu->addAction(sepa->mAction);
	}

	newAction = createAction(QIcon(mSetting.value("MenuInspectorModelIcon").toString()), mSetting.value("MenuInspectorModel").toString(), mSetting.value("MenuInspectorModelTips").toString(), QKeySequence::UnknownKey, *menu);
	connect(newAction, &QAction::triggered, this, &WidgetInspectorData::createModel);


	//Physic
	{
		WidgetMenuSeperator* sepa = new WidgetMenuSeperator(mSetting, mSetting.value("CategoryInspectorPhysic").toString(), mSetting.value("SeparatorIcon").toString(), menu);
		menu->addAction(sepa->mAction);
	}

	newAction = createAction(QIcon(mSetting.value("MenuInspectorPhysicBoxColliderIcon").toString()), mSetting.value("MenuInspectorPhysicBoxCollider").toString(), mSetting.value("MenuInspectorPhysicBoxColliderTips").toString(), QKeySequence::UnknownKey, *menu);
	connect(newAction, &QAction::triggered, this, &WidgetInspectorData::createBoxCollider);
}

QAction* WidgetInspectorData::createAction(const QIcon& pIcon, const QString& pTitle, const QString& pStatusTip, const QKeySequence::StandardKey& pShortcut, QWidget& pWidget)
{
	QAction* newAction = new QAction(pIcon, pTitle, this);
	newAction->setShortcuts(pShortcut);
	newAction->setStatusTip(pStatusTip);
	pWidget.addAction(newAction);
	return newAction;
}

bool WidgetInspectorData::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::Show && obj == mCreateComponent->menu())
	{
		QPoint pos = mapToGlobal(mCreateComponent->parentWidget()->rect().bottomLeft());
		pos.setX(pos.x() + ((mCreateComponent->width() / 2) - (mCreateComponent->menu()->width() / 2)));

		mCreateComponent->menu()->move(pos);
		return true;
	}
	return false;
}

void WidgetInspectorData::initTransforms()
{
	WidgetGameObjectTreeItem* item = (WidgetGameObjectTreeItem*)mItem;

	std::vector<Game::Component::AComponent*>& transforms = item->mActor.mComponents[Game::Utils::ComponentType::Transform];
	for (unsigned int i = 0; i < transforms.size(); i++)
	{
		Game::Component::CPTransform* transform = (Game::Component::CPTransform*)transforms[i];

		WidgetCollapse* wiz = new WidgetCollapse(QIcon(mSetting.value("InspectorTransformIcon").toString()), mSetting.value("InspectorTransformTitle").toString());
		mLayout->insertWidget(mLayout->count() - 1, wiz, 0, Qt::AlignTop);
		wiz->setObjectName("Component");

		QGridLayout* grid = new QGridLayout();
		grid->setContentsMargins(15, 5, 5, 5);
		grid->setVerticalSpacing(1);

		//Position
		{
			QLabel* position = new QLabel("Position");
			grid->addWidget(position, 0, 0, Qt::AlignLeft);

			setIndividualTransformAxisScrollbar("X", transform->mLocalPosition.x, grid, *transform, 0, 1);
			setIndividualTransformAxisScrollbar("Y", transform->mLocalPosition.y, grid, *transform, 0, 3);
			setIndividualTransformAxisScrollbar("Z", transform->mLocalPosition.z, grid, *transform, 0, 5);
		}

		//Rotation
		{
			QLabel* position = new QLabel("Rotation");
			grid->addWidget(position, 1, 0, Qt::AlignLeft);
			Maths::FVector3 vec = transform->mLocalRotation.eulerAngles();
			
			setIndividualTransformAxisScrollbarWithRotation("X", vec.x, 0, grid, *transform, 1, 1);

			/*setIndividualTransformAxisScrollbarWithRotation("X", vec.x, 0, grid, *transform, 1, 1);
			setIndividualTransformAxisScrollbarWithRotation("Y", vec.y, 1, grid, *transform, 1, 3);
			setIndividualTransformAxisScrollbarWithRotation("Z", vec.z, 2, grid, *transform, 1, 5);*/
		}

		//Scale
		{
			QLabel* position = new QLabel("Scale");
			grid->addWidget(position, 2, 0, Qt::AlignLeft);

			setIndividualTransformAxisScrollbar("X", transform->mLocalScale.x, grid, *transform, 2, 1);
			setIndividualTransformAxisScrollbar("Y", transform->mLocalScale.y, grid, *transform, 2, 3);
			setIndividualTransformAxisScrollbar("Z", transform->mLocalScale.z, grid, *transform, 2, 5);
		}

		wiz->setContentLayout(*grid);
	}

}

void WidgetInspectorData::setIndividualTransformAxisScrollbar(std::string pText, float& pAxisVal, QGridLayout* pLayout, Game::Component::CPTransform& pTransform, int row, int column)
{
	QLabel* axis = new QLabel(pText.c_str());
	pLayout->addWidget(axis, row, column, Qt::AlignRight);

	WidgetDoubleSpinBox* SpinBox = new WidgetDoubleSpinBox(pAxisVal, INT64_MIN, INT64_MAX);
	pLayout->addWidget(SpinBox, row, column + 1, Qt::AlignLeft);
	connect(SpinBox, &WidgetDoubleSpinBox::valueChanged, this, [SpinBox, &pAxisVal, &pTransform, this]
	{
		pAxisVal = SpinBox->value();
		((WidgetGameObjectTreeItem*)mItem)->mActor.updateWorldTransformEditor(&pTransform);
	});
}

void Editor::Widget::WidgetInspectorData::setIndividualTransformAxisScrollbarWithRotation(std::string pText, float pAxisVal, int AxisNum, QGridLayout* pLayout, Game::Component::CPTransform& pTransform, int row, int column)
{

	QLabel* axisX = new QLabel("X");
	pLayout->addWidget(axisX, 1, 1, Qt::AlignRight);

	WidgetDoubleSpinBox* SpinBoxX = new WidgetDoubleSpinBox(0, INT64_MIN, INT64_MAX);
	pLayout->addWidget(SpinBoxX, 1, 2, Qt::AlignLeft);

	QLabel* axisY = new QLabel("Y");
	pLayout->addWidget(axisY, 1, 3, Qt::AlignRight);

	WidgetDoubleSpinBox* SpinBoxY = new WidgetDoubleSpinBox(0, INT64_MIN, INT64_MAX);
	pLayout->addWidget(SpinBoxY, 1, 4, Qt::AlignLeft);

	QLabel* axisZ = new QLabel("Z");
	pLayout->addWidget(axisZ, 1, 5, Qt::AlignRight);

	WidgetDoubleSpinBox* SpinBoxZ = new WidgetDoubleSpinBox(0, INT64_MIN, INT64_MAX);
	pLayout->addWidget(SpinBoxZ, 1, 6, Qt::AlignLeft);


	connect(SpinBoxX, &WidgetDoubleSpinBox::valueChanged, this, [=, &pTransform]
	{
		Maths::FVector3 vec = {(float)SpinBoxX->value(), (float)SpinBoxY->value(), (float)SpinBoxZ->value() };
		pTransform.mLocalRotation = Maths::FQuaternion(vec);
		((WidgetGameObjectTreeItem*)mItem)->mActor.updateWorldTransformEditor(&pTransform);
	});

	connect(SpinBoxY, &WidgetDoubleSpinBox::valueChanged, this, [=, &pTransform]
	{
		Maths::FVector3 vec = { (float)SpinBoxX->value(), (float)SpinBoxY->value(),(float)SpinBoxZ->value() };
		pTransform.mLocalRotation = Maths::FQuaternion(vec);
		((WidgetGameObjectTreeItem*)mItem)->mActor.updateWorldTransformEditor(&pTransform);
	});

	connect(SpinBoxZ, &WidgetDoubleSpinBox::valueChanged, this, [=, &pTransform]
	{
		Maths::FVector3 vec = { (float)SpinBoxX->value(), (float)SpinBoxY->value(), (float)SpinBoxZ->value() };
		pTransform.mLocalRotation = Maths::FQuaternion(vec);
		((WidgetGameObjectTreeItem*)mItem)->mActor.updateWorldTransformEditor(&pTransform);
	});
}

void WidgetInspectorData::createModel()
{
	Game::Component::CPModel* newModel = new Game::Component::CPModel();
	((WidgetGameObjectTreeItem*)mItem)->mActor.addComponent(Game::Utils::ComponentType::MeshRenderer, newModel);

	addModel(newModel);
}

void WidgetInspectorData::addModel(Game::Component::CPModel* pModel)
{
	WidgetCollapse* wiz = new WidgetCollapse(QIcon(mSetting.value("MenuInspectorModelIcon").toString()), mSetting.value("InspectorModelTitle").toString());
	mLayout->insertWidget(mLayout->count() - 1, wiz, 0, Qt::AlignTop);
	wiz->setObjectName("Component");

	QBoxLayout* widLayout = new QBoxLayout(QBoxLayout::TopToBottom, nullptr);
	widLayout->setSpacing(0);

	// Model name
	mModelName = new QLabel(pModel->mName.c_str());
	widLayout->addWidget(mModelName);

	// Model path
	mModelPath = new WidgetDropInput(*mModelName, *pModel);
	mModelPath->setText(pModel->mPath.c_str());
	mModelPath->setReadOnly(true);
	widLayout->addWidget(mModelPath);

	const QIcon folderIcon(mSetting.value("FolderIcon").toString());
	QAction* browseAction = new QAction(folderIcon, mSetting.value("FolderTip").toString());
	mModelPath->addAction(browseAction, QLineEdit::TrailingPosition);
	connect(browseAction, &QAction::triggered, this, [=]
	{
		setModel(pModel, mModelName, mModelPath);
	});

	wiz->setContentLayout(*widLayout);
}

void WidgetInspectorData::setModel(Game::Component::AComponent* pComponent, QLabel* name, QLineEdit* mProjectPath)
{
	std::string filter = "Model Files (*.fbx);;Model Files (*.obj);;Model Files (*.dae)";
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Model"), "", tr(filter.c_str()));
	if (!fileName.isEmpty())
	{
		QFileInfo info(fileName);

		name->setText(info.baseName());
		mProjectPath->setText(fileName);
		((Game::Component::CPModel*)pComponent)->setModel(Utils::qStringToStdString(info.baseName())
			, Utils::qStringToStdString(info.absoluteFilePath()).c_str());

	}
}

void WidgetInspectorData::setModel(QFileInfo& info)
{
	Game::Component::CPModel* newModel = new Game::Component::CPModel();
	((WidgetGameObjectTreeItem*)mItem)->mActor.addComponent(Game::Utils::ComponentType::MeshRenderer, newModel);

	addModel(newModel);

	mModelName->setText(info.baseName());
	mModelPath->setText(info.absoluteFilePath());
	newModel->setModel(Utils::qStringToStdString(info.baseName())
		, Utils::qStringToStdString(info.absoluteFilePath()).c_str());
}

void WidgetInspectorData::createBoxCollider()
{
	WidgetGameObjectTreeItem* item = ((WidgetGameObjectTreeItem*)mItem);

	Maths::FVector3 position = item->mActor.getTransform()->mWorldPosition;
	Maths::FVector3 size = item->mActor.getTransform()->mWorldScale;

	Game::Component::CPBoxCollider* newBody = new Game::Component::CPBoxCollider(position, size);
	item->mActor.addComponent(Game::Utils::ComponentType::BoxCollider, newBody);

	addBoxCollider(newBody);
}

void WidgetInspectorData::addBoxCollider(Game::Component::CPBoxCollider* pBody)
{
	WidgetCollapse* wiz = new WidgetCollapse(QIcon(mSetting.value("MenuInspectorPhysicBoxColliderIcon").toString()), mSetting.value("InspectorPhysicBoxColliderTitle").toString());
	mLayout->insertWidget(mLayout->count() - 1, wiz, 0, Qt::AlignTop);
	wiz->setObjectName("Component");

	QGridLayout* widLayout = new QGridLayout();
	widLayout->setContentsMargins(15, 5, 5, 5);
	widLayout->setVerticalSpacing(1);

	// check box
	{
		QCheckBox* box = new QCheckBox();
		widLayout->addWidget(new QLabel("Is Trigger"), 0, 0, Qt::AlignLeft);
		widLayout->addWidget(new QLabel(" "), 0, 1, Qt::AlignRight);
		widLayout->addWidget(box, 0, 2, 1, 4, Qt::AlignLeft);
		connect(box, &QCheckBox::stateChanged, this, [box]
		{
				box->clearFocus();
		});
	}

	// Combo box
	{
		QComboBox* typeCollision = new QComboBox();
		
		typeCollision->addItem("Dynamic", QVariant((int)Physics::Data::TypeRigidBody::E_DYNAMIC));
		typeCollision->addItem("Static", QVariant((int)Physics::Data::TypeRigidBody::E_STATIC));
		typeCollision->addItem("Kinematic", QVariant((int)Physics::Data::TypeRigidBody::E_KINEMATIC));

		connect(typeCollision, &QComboBox::currentIndexChanged,this, [typeCollision, pBody]
		{
			QVariant value = typeCollision->itemData(typeCollision->currentIndex());
			pBody->setType((Physics::Data::TypeRigidBody)value.toInt());

			typeCollision->clearFocus();
		});
		

		widLayout->addWidget(new QLabel("Type"), 1, 0, Qt::AlignLeft);
		widLayout->addWidget(new QLabel(" "), 1, 1, Qt::AlignRight);
		widLayout->addWidget(typeCollision, 1, 2, 1, 4, Qt::AlignLeft);
	}

	// Mass
	{
		WidgetDoubleSpinBox* spinMass = setSpinBox("Mass", pBody->mMass, widLayout, 2, 0);
		connect(spinMass, &WidgetDoubleSpinBox::valueChanged, this, [spinMass, pBody]
		{
			pBody->updateMass(spinMass->value());
		});
	}

	//Center
	{
		QLabel* position = new QLabel("Center");
		widLayout->addWidget(position, 3, 0, Qt::AlignLeft);

		setIndividualBoxAxisScrollbar("X", pBody->mPosition.x, widLayout, *pBody, 3, 1);
		setIndividualBoxAxisScrollbar("Y", pBody->mPosition.y, widLayout, *pBody, 3, 3);
		setIndividualBoxAxisScrollbar("Z", pBody->mPosition.z, widLayout, *pBody, 3, 5);
	}

	//Size
	{
		QLabel* position = new QLabel("Size");
		widLayout->addWidget(position, 4, 0, Qt::AlignLeft);

		setIndividualBoxAxisScrollbar("X", pBody->mSize.x, widLayout, *pBody, 4, 1);
		setIndividualBoxAxisScrollbar("Y", pBody->mSize.y, widLayout, *pBody, 4, 3);
		setIndividualBoxAxisScrollbar("Z", pBody->mSize.z, widLayout, *pBody, 4, 5);
	}

	wiz->setContentLayout(*widLayout);
}

WidgetDoubleSpinBox* WidgetInspectorData::setSpinBox(std::string pText, float& pAxisVal, QGridLayout* pLayout, int row, int column)
{
	pLayout->addWidget(new QLabel(pText.c_str()), row, column, Qt::AlignLeft);
	pLayout->addWidget(new QLabel(" "), row, column + 1, Qt::AlignRight);

	WidgetDoubleSpinBox* spinBox = new WidgetDoubleSpinBox(pAxisVal, 0, INT64_MAX);
	pLayout->addWidget(spinBox, row, column + 2, 1, 4, Qt::AlignLeft);
	
	return spinBox;
}

void WidgetInspectorData::setIndividualBoxAxisScrollbar(std::string pText, float& pAxisVal, QGridLayout* pLayout, Game::Component::CPBoxCollider& pBox, int row, int column)
{
	pLayout->addWidget(new QLabel(pText.c_str()), row, column, Qt::AlignRight);

	WidgetDoubleSpinBox* SpinBox = new WidgetDoubleSpinBox(pAxisVal, INT64_MIN, INT64_MAX);
	pLayout->addWidget(SpinBox, row, column + 1, Qt::AlignLeft);
	connect(SpinBox, &WidgetDoubleSpinBox::valueChanged, this, [SpinBox, &pAxisVal, &pBox, this]
	{
		pAxisVal = SpinBox->value();
		pBox.updateTransform();
	});
}
