#include <algorithm>

#include "Tools/Utils/PathParser.h"
#include "Editor/Utils/Utils.h"

using namespace Tools::Utils;

std::string PathParser::makeWindowsStyle(const std::string& pPath)
{
	std::string result;
	result.resize(pPath.size());

	for (size_t i = 0; i < pPath.size(); ++i)
		result[i] = pPath[i] == '/' ? '\\' : pPath[i];

	return result;
}

std::string PathParser::makeNonWindowsStyle(const std::string& pPath)
{
	std::string result;
	result.resize(pPath.size());

	for (size_t i = 0; i < pPath.size(); ++i)
		result[i] = pPath[i] == '\\' ? '/' : pPath[i];

	return result;
}

std::string PathParser::getContainingFolder(const std::string& pPath)
{
	std::string result;

	bool extraction = false;

	for (auto it = pPath.rbegin(); it != pPath.rend(); ++it)
	{
		if (extraction)
			result += *it;

		if (!extraction && it != pPath.rbegin() && (*it == '\\' || *it == '/'))
			extraction = true;
	}

	std::reverse(result.begin(), result.end());

	if (!result.empty() && result.back() != '\\')
		result += '\\';

	return result;
}

std::string PathParser::getElementName(const std::string& pPath)
{
	std::string result;

	std::string path = pPath;
	if (!path.empty() && path.back() == '\\')
		path.pop_back();

	//Reverse search
	for (auto it = path.rbegin(); it != path.rend() && *it != '\\' && *it != '/'; ++it)
		result += *it;

	std::reverse(result.begin(), result.end());

	return result;
}

std::string PathParser::getExtension(const std::string& pPath)
{
	std::string result;

	//Reverse search
	for (auto it = pPath.rbegin(); it != pPath.rend() && *it != '.'; ++it)
		result += *it;

	std::reverse(result.begin(), result.end());

	return result;
}

std::string PathParser::fileTypeToString(EFileType pFileType)
{
	switch (pFileType)
	{
		case PathParser::EFileType::MODEL:		return "Model";
		case PathParser::EFileType::TEXTURE:	return "Texture";
		case PathParser::EFileType::SHADER:		return "Shader";
		case PathParser::EFileType::MATERIAL:	return "Material";
		case PathParser::EFileType::SOUND:		return "Sound";
		case PathParser::EFileType::SCENE:		return "Scene";
		case PathParser::EFileType::SCRIPT:		return "Script";
		case PathParser::EFileType::FONT:		return "Font";
	}

	return "Unknown";
}

PathParser::EFileType PathParser::getFileType(const std::string& pPath)
{
	std::string ext = getExtension(pPath);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if (ext == "fbx" || ext == "obj" || ext == "dae")
		return EFileType::MODEL;
	else if (ext == "png" || ext == "jpeg" || ext == "jpg" || ext == "bmp")
		return EFileType::TEXTURE;
	else if (ext == "glsl")
		return EFileType::SHADER;
	else if (ext == "mat")
		return EFileType::MATERIAL;
	else if (ext == "wav" || ext == "mp3" || ext == "ogg")
		return EFileType::SOUND;
	else if (ext == "scene")
		return EFileType::SCENE;
	else if (ext == "lua")
		return EFileType::SCRIPT;
	else if (ext == "ttf")
		return EFileType::FONT;

	return EFileType::UNKNOWN;
}

PathParser::EFileType PathParser::getFileType(const QString& pExtension)
{
	std::string pExt = Editor::Utils::qStringToStdString(pExtension);
	std::transform(pExt.begin(), pExt.end(), pExt.begin(), ::tolower);

	if (pExt == "fbx" || pExt == "obj" || pExt == "dae")
		return EFileType::MODEL;
	else if (pExt == "png" || pExt == "jpeg" || pExt == "jpg" || pExt == "bmp")
		return EFileType::TEXTURE;
	else if (pExt == "glsl")
		return EFileType::SHADER;
	else if (pExt == "mat")
		return EFileType::MATERIAL;
	else if (pExt == "wav" || pExt == "mp3" || pExt == "ogg")
		return EFileType::SOUND;
	else if (pExt == "map")
		return EFileType::SCENE;
	else if (pExt == "lua")
		return EFileType::SCRIPT;
	else if (pExt == "ttf")
		return EFileType::FONT;

	return EFileType::UNKNOWN;
}