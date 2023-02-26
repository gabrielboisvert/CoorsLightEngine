#pragma once
#include <QtWidgets/qwidget.h>
#include "Game/Data/Actor.h"
#include <QtCore/qsettings.h>
#include <QtGui/qstandarditemmodel.h>
#include <QtWidgets/qboxlayout.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qpushbutton.h>
#include "Game/Component/AComponent.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include "Game/Component/CPModel.h"
#include <QtWidgets/qfiledialog.h>
#include "Game/Component/CPBoxCollider.h"
#include "Editor/Widget/WidgetDoubleSpinBox.h"

namespace Editor::Widget
{
	class WidgetDropInput;

	class WidgetInspectorData : public QWidget
	{
		public:
			QStandardItem* mItem = nullptr;
			QVBoxLayout* mLayout = new QVBoxLayout();
			QSettings& mSetting;
			QPushButton* mCreateComponent;
			QLineEdit* mGameObjectName = nullptr;

			QLabel* mModelName;
			WidgetDropInput* mModelPath;

			WidgetInspectorData(QSettings& pSetting, QStandardItem* pItem);
			QAction* createAction(const QIcon& pIcon, const QString& pTitle, const QString& pStatusTip, const QKeySequence::StandardKey& pShortcut, QWidget& pWidget);
			
			void setObjectNameEdit();
			void setCreateButton();

			bool eventFilter(QObject* obj, QEvent* event) override;
	
			void initTransforms();
			void setIndividualTransformAxisScrollbar(std::string Text, float& pAxisVal, QGridLayout* layout, Game::Component::CPTransform& pTransform, int row, int column);
	
			void setIndividualTransformAxisScrollbarWithRotation(std::string Text, float pAxisVal, int AxisNum, QGridLayout* layout, Game::Component::CPTransform& pTransform, int row, int column);

			void createModel();
			void addModel(Game::Component::CPModel* pModel);
			void setModel(Game::Component::AComponent* pComponent, QLabel* name, QLineEdit* mProjectPath);
			void setModel(QFileInfo& info);

			void createBoxCollider();
			void addBoxCollider(Game::Component::CPBoxCollider* pBody);
			WidgetDoubleSpinBox* setSpinBox(std::string pText, float& pAxisVal, QGridLayout* pLayout, int row, int column);
			void WidgetInspectorData::setIndividualBoxAxisScrollbar(std::string pText, float& pAxisVal, QGridLayout* pLayout, Game::Component::CPBoxCollider& pBox, int row, int column);
	};
}