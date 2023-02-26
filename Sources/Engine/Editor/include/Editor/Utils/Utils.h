#pragma once
#include <QtCore/qstring.h>
#include <string>
#include <QtWidgets/qwidget.h>

namespace Editor
{
	class Utils
	{
		public:
			Utils() = delete;

			static QString uniqueString(const QString& pBase, const char* pExtension = nullptr);
			static void copyAndReplaceFolderContents(const QString& pFromDir, const QString& pToDir, bool pCopyAndRemove = false);
			static void copyFileAndDir(const QString& pFromDir, const QString& pToDir, QWidget* pParent, bool pCopyAndRemove = false);
			static std::string qStringToStdString(const QString& pString);
	};
}