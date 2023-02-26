#define VMA_IMPLEMENTATION

#include "Rendering/Renderer/VK/VKRenderer.h"
#include "Rendering/Data/VKTypes.h"
#include "Rendering/Renderer/VK/VKInitializers.h"
#include "EngineCore/Service/ServiceLocator.h"

#include <VkBootstrap/VkBootstrap.h>
#include <VulkanMemoryAllocator/vk_mem_alloc.h>
#include <set>
#include <iostream>

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

using namespace Rendering::Renderer::VK;

VKRenderer::VKRenderer()
{
	{
		std::unique_lock lock(IRenderer::mInstanceLock);
		IRenderer::mInstance++;
		
		if (IRenderer::mInstance == 1)
		{
			provideService(VkDevice, mDriver.mDevice);
			provideService(VkPhysicalDevice, mDriver.mPhysicalDevice);
			provideService(VmaAllocator, mAllocator);
			provideService(Data::DeletionQueue, mMainDeletionQueue);
			provideService(VKRenderer, *this);
		}
	}
}

VKRenderer::~VKRenderer()
{
	cleanUp();
}

void VKRenderer::init(EngineWindow::Core::Window& pWindow)
{
	mWindow = &pWindow;
	mWindow->setWindowUserPointer(this);

	initVulkan();
	getGraphicsQueue();
	initSwapchain();
	initCommands();
	initDefaultRenderpass();
	initFramebuffers();
	initSyncStructures();
}

void VKRenderer::initVulkan()
{
	vkb::InstanceBuilder builder;
	vkb::Instance vkb_inst;
	
	{
		std::unique_lock lock(IRenderer::mInstanceLock);
		auto inst_ret = builder.set_app_name("Vukan Renderer")
			.request_validation_layers(true/*enableValidationLayers && mInstance == 1*/)
			.require_api_version(1, 1, 0)
			.use_default_debug_messenger()
			.build();

		vkb_inst = inst_ret.value();
		
		
		if (mDriver.mInstance == nullptr)
		{
			mDriver.mInstance = vkb_inst.instance;
			mDebugMessenger = vkb_inst.debug_messenger;
		}
		else
			vkb_inst.instance = mDriver.mInstance;
	}

	int res = mWindow->createWindowSurface<VkInstance, VkSurfaceKHR*>(mDriver.mInstance, &mSurface);

	vkb::PhysicalDeviceSelector selector{ vkb_inst };
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_surface(mSurface)
		.select()
		.value();
		
	vkb::DeviceBuilder deviceBuilder{ physicalDevice };

	{
		std::unique_lock lock(IRenderer::mInstanceLock);
		if (mDriver.mDevice == nullptr)
			mDriver.mPhysicalDevice = physicalDevice.physical_device;
	}
}

Rendering::Data::QueueFamilyIndices VKRenderer::findQueueFamilies(VkPhysicalDevice& pdevice)
{
	Rendering::Data::QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.mGraphicsFamily = i;

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(pdevice, i, mSurface, &presentSupport);

		if (presentSupport)
			indices.mPresentFamily = i;

		if (indices.isComplete())
			break;

		i++;
	}

	return indices;
}

void VKRenderer::getGraphicsQueue()
{
	Rendering::Data::QueueFamilyIndices indices = findQueueFamilies(mDriver.mPhysicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.mGraphicsFamily.value(), indices.mPresentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(mDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = mDeviceExtensions.data();

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
		createInfo.ppEnabledLayerNames = mValidationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;

	{
		std::unique_lock lock(IRenderer::mInstanceLock);

		if (mDriver.mDevice == nullptr)
			if (vkCreateDevice(mDriver.mPhysicalDevice, &createInfo, nullptr, &mDriver.mDevice) != VK_SUCCESS)
				throw std::runtime_error("failed to create logical device!");

		

		vkGetDeviceQueue(mDriver.mDevice, indices.mGraphicsFamily.value(), 0, &mGraphicsQueue);
		mGraphicsQueueFamily = indices.mGraphicsFamily.value();

		if (mAllocator == nullptr)
		{
			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.physicalDevice = mDriver.mPhysicalDevice;
			allocatorInfo.device = mDriver.mDevice;
			allocatorInfo.instance = mDriver.mInstance;
			vmaCreateAllocator(&allocatorInfo, &mAllocator);
		}
	}
}

void VKRenderer::initSwapchain()
{
	int width, height;
	mWindow->getFramebufferSize(&width, &height);
	mWindowExtent = { (unsigned int)width, (unsigned int)height };

	vkb::SwapchainBuilder swapchainBuilder{ mDriver.mPhysicalDevice, mDriver.mDevice, mSurface };

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.use_default_format_selection()
		.set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR) //VK_PRESENT_MODE_FIFO_KHR -- vsync 60 fps max / VK_PRESENT_MODE_MAILBOX_KHR
		.set_desired_extent(mWindowExtent.width, mWindowExtent.height)
		.build()
		.value();

	mSwapchain = vkbSwapchain.swapchain;
	mSwapchainImages = vkbSwapchain.get_images().value();
	mSwapchainImageViews = vkbSwapchain.get_image_views().value();

	mSwapchainImageFormat = vkbSwapchain.image_format;

	VkExtent3D depthImageExtent = 
	{
		mWindowExtent.width,
		mWindowExtent.height,
		1
	};

	VkImageCreateInfo dimg_info = VKInit::imageCreateInfo(mDepthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

	VmaAllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vmaCreateImage(mAllocator, &dimg_info, &dimg_allocinfo, &mDepthImage.mImage, &mDepthImage.mAllocation, nullptr);

	VkImageViewCreateInfo dview_info = VKInit::imageviewCreateInfo(mDepthFormat, mDepthImage.mImage, VK_IMAGE_ASPECT_DEPTH_BIT);
	vkCreateImageView(mDriver.mDevice, &dview_info, nullptr, &mDepthImageView);
}

void VKRenderer::initCommands()
{
	VkCommandPoolCreateInfo commandPoolInfo = VKInit::commandPoolCreateInfo(mGraphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	
	for (int i = 0; i < FRAME_OVERLAP; i++) 
	{
		vkCreateCommandPool(mDriver.mDevice, &commandPoolInfo, nullptr, &mFrames[i].mCommandPool);


		VkCommandBufferAllocateInfo cmdAllocInfo = VKInit::commandBufferAllocateInfo(mFrames[i].mCommandPool, 1);
		vkAllocateCommandBuffers(mDriver.mDevice, &cmdAllocInfo, & mFrames[i].mMainCommandBuffer);

		mMainDeletionQueue.pushFunction([=]() 
		{
			vkDestroyCommandPool(mDriver.mDevice, mFrames[i].mCommandPool, nullptr);
		});
	}

	VkCommandPoolCreateInfo uploadCommandPoolInfo = VKInit::commandPoolCreateInfo(mGraphicsQueueFamily);
	vkCreateCommandPool(mDriver.mDevice, &uploadCommandPoolInfo, nullptr, &mUploadContext.mCommandPool);

	mMainDeletionQueue.pushFunction([=]() 
	{
		vkDestroyCommandPool(mDriver.mDevice, mUploadContext.mCommandPool, nullptr);
	});

	VkCommandBufferAllocateInfo cmdAllocInfo = VKInit::commandBufferAllocateInfo(mUploadContext.mCommandPool, 1);
	vkAllocateCommandBuffers(mDriver.mDevice, &cmdAllocInfo, &mUploadContext.mCommandBuffer);
}

void VKRenderer::initDefaultRenderpass()
{
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = mSwapchainImageFormat;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depth_attachment = {};
	depth_attachment.flags = 0;
	depth_attachment.format = mDepthFormat;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	VkAttachmentDescription attachments[2] = { color_attachment, depth_attachment };

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency depth_dependency = {};
	depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	depth_dependency.dstSubpass = 0;
	depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depth_dependency.srcAccessMask = 0;
	depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency dependencies[2] = { dependency, depth_dependency };

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 2;
	render_pass_info.pAttachments = &attachments[0];
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 2;
	render_pass_info.pDependencies = &dependencies[0];

	vkCreateRenderPass(mDriver.mDevice, &render_pass_info, nullptr, &mRenderPass);

	mMainDeletionQueue.pushFunction([=]() 
	{
		vkDestroyRenderPass(mDriver.mDevice, mRenderPass, nullptr);
	});
}

void VKRenderer::initFramebuffers()
{
	VkFramebufferCreateInfo fb_info = VKInit::framebufferCreateInfo(mRenderPass, mWindowExtent);

	const uint32_t swapchain_imagecount = mSwapchainImages.size();
	mFramebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

	for (int i = 0; i < swapchain_imagecount; i++) 
	{
		VkImageView attachments[2];
		attachments[0] = mSwapchainImageViews[i];
		attachments[1] = mDepthImageView;

		fb_info.pAttachments = attachments;
		fb_info.attachmentCount = 2;

		vkCreateFramebuffer(mDriver.mDevice, &fb_info, nullptr, &mFramebuffers[i]);
	}
}

void VKRenderer::initSyncStructures()
{
	VkFenceCreateInfo fenceCreateInfo = VKInit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semaphoreCreateInfo = VKInit::semaphoreCreateInfo();

	for (int i = 0; i < FRAME_OVERLAP; i++) 
	{
		vkCreateFence(mDriver.mDevice, &fenceCreateInfo, nullptr, &mFrames[i].mRenderFence);

		mMainDeletionQueue.pushFunction([=]() 
		{
			vkDestroyFence(mDriver.mDevice, mFrames[i].mRenderFence, nullptr);
		});


		vkCreateSemaphore(mDriver.mDevice, &semaphoreCreateInfo, nullptr, &mFrames[i].mPresentSemaphore);
		vkCreateSemaphore(mDriver.mDevice, &semaphoreCreateInfo, nullptr, &mFrames[i].mRenderSemaphore);

		mMainDeletionQueue.pushFunction([=]() 
		{
			vkDestroySemaphore(mDriver.mDevice, mFrames[i].mPresentSemaphore, nullptr);
			vkDestroySemaphore(mDriver.mDevice, mFrames[i].mRenderSemaphore, nullptr);
		});
	}

	VkFenceCreateInfo uploadFenceCreateInfo = VKInit::fenceCreateInfo();

	vkCreateFence(mDriver.mDevice, &uploadFenceCreateInfo, nullptr, &mUploadContext.mUploadFence);
	mMainDeletionQueue.pushFunction([=]()
	{
		vkDestroyFence(mDriver.mDevice, mUploadContext.mUploadFence, nullptr);
	});
}

void VKRenderer::beginFrame()
{
	vkWaitForFences(mDriver.mDevice, 1, &getCurrentFrame().mRenderFence, true, UINT64_MAX);
	
	VkResult result = vkAcquireNextImageKHR(mDriver.mDevice, mSwapchain, UINT64_MAX, getCurrentFrame().mPresentSemaphore, nullptr, &mSwapchainImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		throw std::runtime_error("failed to acquire swap chain image!");
	
	vkResetFences(mDriver.mDevice, 1, &getCurrentFrame().mRenderFence);

	vkResetCommandBuffer(getCurrentFrame().mMainCommandBuffer, 0);

	VkCommandBufferBeginInfo cmdBeginInfo = VKInit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	vkBeginCommandBuffer(getCurrentFrame().mMainCommandBuffer, &cmdBeginInfo);

	VkClearValue clearValue;
	clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	VkClearValue clearValues[] = { clearValue, depthClear };

	VkRenderPassBeginInfo rpInfo = VKInit::renderpassBeginInfo(mRenderPass, mWindowExtent, mFramebuffers[mSwapchainImageIndex]);
	rpInfo.clearValueCount = 2;
	rpInfo.pClearValues = &clearValues[0];

	vkCmdBeginRenderPass(getCurrentFrame().mMainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)mWindowExtent.width;
	viewport.height = (float)mWindowExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = mWindowExtent;

	vkCmdSetViewport(getCurrentFrame().mMainCommandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(getCurrentFrame().mMainCommandBuffer, 0, 1, &scissor);
	vkCmdSetDepthBias(getCurrentFrame().mMainCommandBuffer, 0, 0, 0);
}

void VKRenderer::endFrame()
{
	std::unique_lock lock(IRenderer::mInstanceLock);

	vkCmdEndRenderPass(getCurrentFrame().mMainCommandBuffer);
	vkEndCommandBuffer(getCurrentFrame().mMainCommandBuffer);
	
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submit = VKInit::submitInfo(&getCurrentFrame().mMainCommandBuffer);
	submit.pWaitDstStageMask = &waitStage;
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &getCurrentFrame().mPresentSemaphore;
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &getCurrentFrame().mRenderSemaphore;

	vkQueueSubmit(mGraphicsQueue, 1, &submit, getCurrentFrame().mRenderFence);


	VkPresentInfoKHR presentInfo = VKInit::presentInfo();
	presentInfo.pSwapchains = &mSwapchain;
	presentInfo.swapchainCount = 1;
	presentInfo.pWaitSemaphores = &getCurrentFrame().mRenderSemaphore;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pImageIndices = &mSwapchainImageIndex;

	VkResult result = vkQueuePresentKHR(mGraphicsQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFramebufferResized)
	{
		mFramebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("failed to present swap chain image!");

	mFrameNumber = (mFrameNumber + 1) % FRAME_OVERLAP;
}

void VKRenderer::cleanUp()
{
	vkWaitForFences(mDriver.mDevice, 1, &getCurrentFrame().mRenderFence, true, UINT64_MAX);

	cleanupSwapChain();

	mMainDeletionQueue.flush();
	
	vkDestroySurfaceKHR(mDriver.mInstance, mSurface, nullptr);
	
	{
		std::unique_lock lock(IRenderer::mInstanceLock);
		IRenderer::mInstance--;

		if (IRenderer::mInstance == 0)
		{
			vmaDestroyAllocator(mAllocator);
			vkDestroyDevice(mDriver.mDevice, nullptr);
			vkb::destroy_debug_utils_messenger(mDriver.mInstance, mDebugMessenger);
			vkDestroyInstance(mDriver.mInstance, nullptr);
		}
	}
}



void VKRenderer::recreateSwapChain()
{
	int width = 0, height = 0;
	mWindow->getFramebufferSize(&width, &height);
	while (width == 0 || height == 0)
		mWindow->getFramebufferSize(&width, &height);
	
	
	vkDeviceWaitIdle(mDriver.mDevice);
	cleanupSwapChain();

	initSwapchain();
	initFramebuffers();
}

void VKRenderer::cleanupSwapChain()
{
	for (int i = 0; i < mFramebuffers.size(); i++)
	{
		vkDestroyFramebuffer(mDriver.mDevice, mFramebuffers[i], nullptr);
		vkDestroyImageView(mDriver.mDevice, mSwapchainImageViews[i], nullptr);
	}

	vkDestroyImageView(mDriver.mDevice, mDepthImageView, nullptr);
	vmaDestroyImage(mAllocator, mDepthImage.mImage, mDepthImage.mAllocation);

	vkDestroySwapchainKHR(mDriver.mDevice, mSwapchain, nullptr);
}

Rendering::Data::FrameData& VKRenderer::getCurrentFrame()
{
	return mFrames[mFrameNumber];
}

VkCommandBuffer VKRenderer::getCurrentCommandBuffer()
{
	return getCurrentFrame().mMainCommandBuffer;
}

void VKRenderer::immediateSubmit(std::function<void(VkCommandBuffer pCmd)>&& function)
{
	std::unique_lock lock(IRenderer::mInstanceLock);

	VKRenderer& renderer = service(VKRenderer);

	VkCommandBufferBeginInfo cmdBeginInfo = VKInit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	vkBeginCommandBuffer(renderer.mUploadContext.mCommandBuffer, &cmdBeginInfo);

	function(renderer.mUploadContext.mCommandBuffer);

	vkEndCommandBuffer(renderer.mUploadContext.mCommandBuffer);

	VkSubmitInfo submit = VKInit::submitInfo(&renderer.mUploadContext.mCommandBuffer);

	vkQueueSubmit(renderer.mGraphicsQueue, 1, &submit, renderer.mUploadContext.mUploadFence);

	vkWaitForFences(renderer.mDriver.mDevice, 1, &renderer.mUploadContext.mUploadFence, true, UINT64_MAX);
	vkResetFences(renderer.mDriver.mDevice, 1, &renderer.mUploadContext.mUploadFence);

	vkResetCommandPool(renderer.mDriver.mDevice, renderer.mUploadContext.mCommandPool, 0);
}