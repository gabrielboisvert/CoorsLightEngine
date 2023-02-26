#pragma once

#include "Rendering/Renderer/IRenderer.h"
#include "vulkan/vulkan_core.h"
#include "EngineWindow/Core/Window.h"
#include "Rendering/Data/VKTypes.h"
#include "Rendering/Data/FrameData.h"
#include "Rendering/Data/UploadContext.h"
#include "Rendering/Context/Driver.h"
#include "Rendering/Data/QueueFamilyIndices.h"

namespace Rendering::Renderer::VK
{
	class VKRenderer : public IRenderer
	{
		public:
			static const unsigned int FRAME_OVERLAP = 3;
			bool mFramebufferResized = false;

			EngineWindow::Core::Window* mWindow = nullptr;

			int mFrameNumber { 0 };
			uint32_t mSwapchainImageIndex { 0 };

			VkDebugUtilsMessengerEXT mDebugMessenger = nullptr;
			VkSurfaceKHR mSurface = nullptr;
			VkSwapchainKHR mSwapchain;
			VkFormat mSwapchainImageFormat;
			std::vector<VkImage> mSwapchainImages;
			std::vector<VkImageView> mSwapchainImageViews;
			std::vector<VkFramebuffer> mFramebuffers;
			Data::FrameData mFrames[FRAME_OVERLAP];
			
			Data::UploadContext mUploadContext;

			VKRenderer();
			~VKRenderer();

			void init(EngineWindow::Core::Window& pWindow);
			void initVulkan();
			Rendering::Data::QueueFamilyIndices VKRenderer::findQueueFamilies(VkPhysicalDevice& pdevice);
			void getGraphicsQueue();
			void initSwapchain();
			void initCommands();
			void initDefaultRenderpass();
			void initFramebuffers();
			void initSyncStructures();
			void beginFrame();
			void endFrame();

			void cleanUp();
			void recreateSwapChain();
			void cleanupSwapChain();

			Data::FrameData& getCurrentFrame();
			VkCommandBuffer getCurrentCommandBuffer() override;
			static void immediateSubmit(std::function<void(VkCommandBuffer pCmd)>&& function);
	};
}