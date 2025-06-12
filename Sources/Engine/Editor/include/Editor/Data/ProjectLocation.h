#pragma once
#include <QtCore/qstring.h>

namespace Editor::Data
{

	class ProjectLocation
	{
		public:
			QString mFolder;
			QString mName;

			ProjectLocation();
			ProjectLocation(QString pFolder, QString pName);

			bool operator==(const ProjectLocation& pOther);
	};

	QDataStream& operator<<(QDataStream& pOut, const ProjectLocation& pValue);

	QDataStream& operator>>(QDataStream& pIn, ProjectLocation& pValue);
}