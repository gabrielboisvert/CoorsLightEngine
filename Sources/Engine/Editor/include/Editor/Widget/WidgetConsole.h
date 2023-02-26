#pragma once
#include <QtWidgets/qplaintextedit.h>
#include <QtWidgets/qwidget.h>
#include <QtDocking/DockWidget.h>
#include <QtCore/qsettings.h>
#include <mutex>
#include "Tools/Utils/String.h"

namespace Editor::Widget
{
	class WidgetConsole : public QPlainTextEdit
	{
	public:
		enum class SEVERITY_LEVEL
		{
			DEBUG,
			INFO,
			WARNING,
			ERROR
		};

		bool mClearPlay = true;
		bool mShowDebug = true;
		bool mShowInfo = true;
		bool mShowWarning = true;
		bool mShowError = true;

		static WidgetConsole* gInstance;
		std::condition_variable mCv;
		bool mCanPrint = true;

		QColor pLastColor;
		WidgetConsole(QWidget* pParent = nullptr);
		~WidgetConsole();

		template <typename ... T>
			static void print(SEVERITY_LEVEL pLevel, const char* str, const T ... args);
		
		template <typename ... T>
			static void debugPrint(const char* str, const T ... args);

		template <typename ... T> 
			static void infoPrint(const char* str, const T ... args);

		template <typename ... T>
			static void warningPrint(const char* str, const T ... args);

		template <typename ... T>
			static void errorPrint(const char* str, const T ... args);

		static void invokeMsg(const std::string& pMessage);
		
		void setColor(const Qt::GlobalColor& pColor);
		void setColor(int pR, int pG, int pB);

		void play();

		void lockPrint();
		void unlockPrint();
	};


	class WidgetConsoleApp : public ads::CDockWidget
	{
		public:
			WidgetConsole mConsole;

			WidgetConsoleApp(QSettings& pSettings, QWidget* pParent = nullptr);
			void play();
	};

}


template <typename ... T> void Editor::Widget::WidgetConsole::WidgetConsole::print(SEVERITY_LEVEL pLevel, const char* str, const T ... args)
{
	switch (pLevel)
	{
	case Editor::Widget::WidgetConsole::SEVERITY_LEVEL::DEBUG: debugPrint(str, args...);
		break;
	case Editor::Widget::WidgetConsole::SEVERITY_LEVEL::INFO: infoPrint(str, args...);
			break;
	case Editor::Widget::WidgetConsole::SEVERITY_LEVEL::WARNING: warningPrint(str, args...);
		break;
	case Editor::Widget::WidgetConsole::SEVERITY_LEVEL::ERROR: errorPrint(str, args...);
		break;
	default:
		break;
	}
}

template <typename ... T> void Editor::Widget::WidgetConsole::infoPrint(const char* str, const T ... args)
{
	if (!gInstance->mShowInfo)
		return;

	std::string fStr = Tools::Utils::String::formatString(str, args...);

	gInstance->lockPrint();
	gInstance->setColor(4, 211, 5);
	invokeMsg(fStr);
}

template <typename ... T> void Editor::Widget::WidgetConsole::debugPrint(const char* str, const T ... args)
{
	if (!gInstance->mShowDebug)
		return;

	std::string fStr = Tools::Utils::String::formatString(str, args...);

	gInstance->lockPrint();
	gInstance->setColor(5, 204, 205);
	invokeMsg(fStr);
}

template <typename ... T> void Editor::Widget::WidgetConsole::warningPrint(const char* str, const T ... args)
{
	if (!gInstance->mShowWarning)
		return;

	std::string fStr = Tools::Utils::String::formatString(str, args...);

	gInstance->lockPrint();
	gInstance->setColor(227, 183, 6);
	invokeMsg(fStr);
}

template <typename ... T> void Editor::Widget::WidgetConsole::errorPrint(const char* str, const T ... args)
{
	if (!gInstance->mShowError)
		return;

	std::string fStr = Tools::Utils::String::formatString(str, args...);

	gInstance->lockPrint();
	gInstance->setColor(Qt::GlobalColor::red);
	invokeMsg(fStr);
}