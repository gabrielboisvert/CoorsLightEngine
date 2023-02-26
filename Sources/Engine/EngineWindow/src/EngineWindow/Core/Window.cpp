#include "EngineWindow/Core/Window.h"
#include <cstdlib>
#include <mutex>
#include "Rendering/Renderer/VK/VKRenderer.h"

using namespace EngineWindow::Core;

int Window::mInstance = 0;
std::mutex Window::mInstanceLock;

void Window::framebufferSizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight)
{
	Rendering::Renderer::VK::VKRenderer* renderer = (Rendering::Renderer::VK::VKRenderer*)glfwGetWindowUserPointer(pWindow);
	renderer->mFramebufferResized = true;
	//app->mCamera.changeViewPortSize(lm::mat4::perspectiveProjection(-45, float(pWidth) / float(pHeight), 0.1f, 500.f));
}

void Window::mousePosition(GLFWwindow* pWindow, double pX, double pY)
{
	Rendering::Renderer::VK::VKRenderer* renderer = (Rendering::Renderer::VK::VKRenderer*)glfwGetWindowUserPointer(pWindow);
	renderer->mWindow->mMouse.mOldPosition = renderer->mWindow->mMouse.mPosition;
	renderer->mWindow->mMouse.mPosition = Maths::FVector2(pX, pY);
}

void Window::mouseButton(GLFWwindow* pWindow, int pButton, int pAction, int pMods)
{
	Rendering::Renderer::VK::VKRenderer* renderer = (Rendering::Renderer::VK::VKRenderer*)glfwGetWindowUserPointer(pWindow);
	if (pButton == GLFW_MOUSE_BUTTON_LEFT)
		renderer->mWindow->mMouse.mLeft = { pAction == GLFW_PRESS };
	else if (pButton == GLFW_MOUSE_BUTTON_RIGHT)
		renderer->mWindow->mMouse.mRight = { pAction == GLFW_PRESS };
}

void Window::scrollCallback(GLFWwindow* pWindow, double /*pX*/, double pY)
{
	Rendering::Renderer::VK::VKRenderer* renderer = (Rendering::Renderer::VK::VKRenderer*)glfwGetWindowUserPointer(pWindow);
	renderer->mWindow->mMouse.mScroll = pY;
}

void Window::keyCallback(GLFWwindow* pWindow, int pKey, int pScancode, int pAction, int pMods)
{
	bool a = true;
	/*Application* app = (Application*)glfwGetWindowUserPointer(pWindow);
	app->keyCallback(pKey, pScancode, pAction, pMods);*/
}

Window::Window()
{
}

Window::Window(const char* pTitle, const unsigned int& pWidth, const unsigned int& pHeight) : mWidth(pWidth), mHeight(pHeight)
{
	{
		std::unique_lock lock(mInstanceLock);
		if (mInstance == 0)
			glfwInit();

		mInstance++;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	mWindow = glfwCreateWindow(pWidth, pHeight, pTitle, nullptr, nullptr);
	if (mWindow == nullptr)
	{
		std::unique_lock lock(mInstanceLock);
		if (mInstance == 1)
		{
			glfwTerminate();
			exit(EXIT_FAILURE);
		}
		else
		{
			mInstance--;
			return;
		}
	}

	// Center window
	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	if (primary != nullptr)
	{
		const GLFWvidmode* desktop = glfwGetVideoMode(primary);
		glfwSetWindowPos(mWindow, (desktop->width - pWidth) / 2, (desktop->height - pHeight) / 2);
	}

	glfwMakeContextCurrent(mWindow);
	glfwSetFramebufferSizeCallback(mWindow, Window::framebufferSizeCallback);
	glfwSetCursorPosCallback(mWindow, Window::mousePosition);
	glfwSetMouseButtonCallback(mWindow, Window::mouseButton);
	glfwSetKeyCallback(mWindow, Window::keyCallback);
	glfwSetScrollCallback(mWindow, Window::scrollCallback);
}

Window::~Window()
{
	if (!mShutdown)
		shutDown();
}

void Window::close()
{
	glfwSetWindowShouldClose(mWindow, true);
}

void Window::shutDown()
{
	glfwDestroyWindow(mWindow);
	mShutdown = true;

	{
		std::unique_lock lock(mInstanceLock);
		mInstance--;

		if (mInstance == 0)
			glfwTerminate();
	}
}

bool Window::shouldClose()
{
	return glfwWindowShouldClose(mWindow);
}

void Window::pollEvents()
{
	glfwPollEvents();
}

void Window::waitEvent()
{
	glfwWaitEvents();
}

void Window::setWindowUserPointer(void* pointer)
{
	glfwSetWindowUserPointer(mWindow, pointer);
}

int Window::getKey(const unsigned int& key)
{
	return glfwGetKey(mWindow, key);
}

void Window::bindContext(bool pBinded)
{
	if (pBinded)
		glfwMakeContextCurrent(mWindow);
	else
		glfwMakeContextCurrent(nullptr);
}

const char** Window::getRequiredInstanceExtensions(uint32_t& pExtensionCount)
{
	return glfwGetRequiredInstanceExtensions(&pExtensionCount);
}

void Window::getFramebufferSize(int* pWidth, int* pHeight)
{
	glfwGetFramebufferSize(mWindow, pWidth, pHeight);

	mWidth = *pWidth;
	mHeight = *pHeight;
}

void Window::updateFramebufferSize()
{
	glfwGetFramebufferSize(mWindow, &mWidth, &mHeight);
}

HWND Window::Window::getWindowId()
{
	return glfwGetWin32Window(mWindow);
}

void Window::hideCursorView(bool pState)
{
	if (mMouse.mCursorHidden == pState)
		return;

	mMouse.mCursorHidden = pState;
	glfwSetInputMode(mWindow, GLFW_CURSOR, pState ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
}

Maths::FVector2 Window::getMousePositionDifference()
{
	return mMouse.getPositionDifference();
}