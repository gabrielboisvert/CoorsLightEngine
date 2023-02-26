#pragma once
#include "Rendering/Renderer/IRenderer.h"
#include "Rendering/Data/FrameData.h"

namespace Rendering::Renderer::VK
{
	class VKOffScreenRenderer : public IRenderer
	{
		public:
			VkFramebuffer mFramebuffer;
			Data::FrameData mFrame;

			VkImageView mColorImageView;
			Data::AllocatedImage mColorImage;
			VkFormat mColorFormat = VK_FORMAT_R8G8B8A8_UNORM;

			VKOffScreenRenderer(unsigned int pWidth, unsigned int pHeight);
			~VKOffScreenRenderer();
			void initGraphicsQueue();
			void createCommandPool();
			void createFrameBufferAttachement();
			void createRenderPass();
			void createFrameBuffer();
			void beginFrame();
			void endFrame();

			VkCommandBuffer getCurrentCommandBuffer() override;
			const void* pixelToArray();

			void saveToPNG(const char* pFile);

			void saveToJPG(const char* pFile);
	};
}