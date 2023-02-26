#include <QtCore/qfileinfo.h>
#include "Editor/Widget/WidgetDropInput.h"
#include <QtCore/qmimedata.h>
#include "Editor/Utils/Utils.h"
#include "Tools/Utils/PathParser.h"
#include "Game/Component/CPModel.h"

using namespace Editor::Widget;

WidgetDropInput::WidgetDropInput(QLabel& pName, Game::Component::AComponent& pComponent)
	: mName(pName), mComponent(pComponent)
{
	setAcceptDrops(true);
}

void WidgetDropInput::dragEnterEvent(QDragEnterEvent* e)
{
	e->acceptProposedAction();
}

void WidgetDropInput::dropEvent(QDropEvent* e)
{
	QFileInfo info = QFileInfo(e->mimeData()->text());
	if (Tools::Utils::PathParser::getFileType(info.suffix()) != Tools::Utils::PathParser::EFileType::MODEL)
		return;

	mName.setText(info.baseName());
	setText(info.absoluteFilePath());

	((Game::Component::CPModel*)&mComponent)->setModel(Utils::qStringToStdString(info.baseName())
		, Utils::qStringToStdString(info.absoluteFilePath()).c_str());
}