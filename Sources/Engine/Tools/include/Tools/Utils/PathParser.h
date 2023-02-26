#pragma once

#include <string>
#include <QtCore/qstring.h>

namespace Tools::Utils
{

	class PathParser
	{
	public:
		enum class EFileType
		{
			UNKNOWN,
			MODEL,
			TEXTURE,
			SHADER,
			MATERIAL,
			SOUND,
			SCENE,
			SCRIPT,
			FONT
		};

		PathParser() = delete;

		static std::string makeWindowsStyle(const std::string& pPath);

		static std::string makeNonWindowsStyle(const std::string& pPath);

		static std::string getContainingFolder(const std::string& pPath);

		static std::string getElementName(const std::string& pPath);

		static std::string getExtension(const std::string& pPath);

		static std::string fileTypeToString(EFileType pFileType);

		static EFileType getFileType(const std::string& pPath);

		static EFileType getFileType(const QString& pExt);
	};
}