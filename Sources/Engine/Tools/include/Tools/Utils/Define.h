#pragma once

class Define
{
	public:

		Define() = delete;

		static const char* PROJECT_EXTENSION;
		static const char* SCENE_EXTENSION;
		static const char* CLASS_EXTENSION;

		enum EEditorState
		{
			CLOSE_EDITOR,
			OPEN_EDITOR,
			OPEN_HUB,
			LOAD_EDITOR
		};
};