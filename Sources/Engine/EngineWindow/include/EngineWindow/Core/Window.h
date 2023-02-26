#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#include <PlatformHeader/qwglnativecontext.h>
#include "EngineWindow/Data/Mouse.h"
#include <mutex>

namespace EngineWindow::Core
{
	class Window
	{
	public:
		static int mInstance;
		static std::mutex mInstanceLock;

		GLFWwindow* mWindow;
		int mWidth;
		int mHeight;
		Data::Mouse mMouse;
		bool mShutdown = false;

		static void framebufferSizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight);
		static void mousePosition(GLFWwindow* pWindow, double pX, double pY);
		static void mouseButton(GLFWwindow* pWindow, int pButton, int pAction, int pMods);
		//static void mouseScroll(GLFWwindow* pWindow, double pX, double pY);
		static void scrollCallback(GLFWwindow* pWindow, double pX, double pY);
		static void keyCallback(GLFWwindow* pWindow, int pKey, int pScancode, int pAction, int pMods);

		Window();
		Window(const char* pTitle, const unsigned int& pWidth, const unsigned int& pHeight);
		~Window();

		void close();
		void shutDown();
		bool shouldClose();
		void pollEvents();
		void waitEvent();
		void setWindowUserPointer(void* pPointer);
		int getKey(const unsigned int& pKey);
		void bindContext(bool pBinded);

		static const char** getRequiredInstanceExtensions(uint32_t& pExtensionCount);
		void getFramebufferSize(int* pWidth, int* pHeight);
		void updateFramebufferSize();

		HWND getWindowId();
		void hideCursorView(bool pState);
		Maths::FVector2 getMousePositionDifference();

		template<typename T, typename K> int createWindowSurface(void* pInstance, void* pSurface)
		{
			if (pInstance == nullptr)
				return -1;
			return glfwCreateWindowSurface((T)pInstance, mWindow, nullptr, (K)pSurface);
		}
	};
}