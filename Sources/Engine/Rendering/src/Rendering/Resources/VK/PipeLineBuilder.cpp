#include "Rendering/Resources/VK/PipeLineBuilder.h"
#include "Rendering/Geometry/VertexDescriptor.h"
#include "Rendering/Renderer/VK/VKInitializers.h"
#include "EngineCore/Service/ServiceLocator.h"
#include <vulkan/vulkan.h>
#include <iostream>
#include "Rendering/Geometry/DebugVertexDescriptor.h"

using namespace Rendering::Renderer::Resources::VK;

Rendering::Data::ShaderCache PipeLineBuilder::mModuleCache;

VkPipeline PipeLineBuilder::buildPipeline(VkRenderPass pPass)
{
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &mViewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &mScissor;


	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = nullptr;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &mColorBlendAttachment;

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;
	pipelineInfo.stageCount = mShaderStages.size();
	pipelineInfo.pStages = mShaderStages.data();
	pipelineInfo.pVertexInputState = &mVertexInputInfo;
	pipelineInfo.pInputAssemblyState = &mInputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &mRasterizer;
	pipelineInfo.pMultisampleState = &mMultisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = mPipelineLayout;
	pipelineInfo.renderPass = pPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.pDepthStencilState = &mDepthStencil;

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

	std::vector<VkDynamicState> dynamicStates;
	dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
	dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
	dynamicState.pDynamicStates = dynamicStates.data();
	dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();

	pipelineInfo.pDynamicState = &dynamicState;

	VkPipeline newPipeline = nullptr;

	if (vkCreateGraphicsPipelines(service(VkDevice), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS)
	{
		std::cout << "failed to create pipeline\n";
		return VK_NULL_HANDLE;
	}
	else
		return newPipeline;
}

void PipeLineBuilder::clearVertexInput()
{
	mVertexInputInfo.pVertexAttributeDescriptions = nullptr;
	mVertexInputInfo.vertexAttributeDescriptionCount = 0;
	mVertexInputInfo.pVertexBindingDescriptions = nullptr;
	mVertexInputInfo.vertexBindingDescriptionCount = 0;
}

Rendering::Data::Material PipeLineBuilder::initPipeLine(const std::string& pVertex, const std::string& pfrag, VkRenderPass pPass, bool pClearVertex, bool pDebugVertex)
{
	Data::Shader* sh = new Data::Shader;
	sh->addStage(mModuleCache.getShader(pVertex.c_str()), VK_SHADER_STAGE_VERTEX_BIT);
	sh->addStage(mModuleCache.getShader(pfrag.c_str()), VK_SHADER_STAGE_FRAGMENT_BIT);
	sh->reflectLayout();

	return initPipeLine(*sh, pPass, pClearVertex, pDebugVertex);
}

Rendering::Data::Material PipeLineBuilder::initPipeLine(Data::Shader& pShader, VkRenderPass pPass, bool pClearVertex, bool pDebugVertex)
{
	PipeLineBuilder pipelineBuilder;
	

	{
		auto bindingDescription = Rendering::Geometry::getBindingDescription();
		auto attributeDescriptions = Rendering::Geometry::getAttributeDescriptions();

		pipelineBuilder.mVertexInputInfo = Rendering::Renderer::VK::VKInit::vertexInputStateCreateInfo();
		pipelineBuilder.mVertexInputInfo.vertexBindingDescriptionCount = 1;
		pipelineBuilder.mVertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
		pipelineBuilder.mVertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		pipelineBuilder.mVertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	}

	pipelineBuilder.mShaderStages.clear();
	pShader.fillStages(pipelineBuilder.mShaderStages);
	pipelineBuilder.mPipelineLayout = pShader.mBuiltLayout;

	if (pClearVertex)
		pipelineBuilder.clearVertexInput();

	if (pDebugVertex)
	{
		auto bindingDescription = Rendering::Geometry::getDebugBindingDescription();
		auto attributeDescriptions = Rendering::Geometry::getDebugAttributeDescriptions();

		pipelineBuilder.mVertexInputInfo = Rendering::Renderer::VK::VKInit::vertexInputStateCreateInfo();
		pipelineBuilder.mVertexInputInfo.vertexBindingDescriptionCount = 1;
		pipelineBuilder.mVertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
		pipelineBuilder.mVertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		pipelineBuilder.mVertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		pipelineBuilder.mInputAssembly = Rendering::Renderer::VK::VKInit::inputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
	}
	else
		pipelineBuilder.mInputAssembly = Rendering::Renderer::VK::VKInit::inputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);


	pipelineBuilder.mRasterizer = Rendering::Renderer::VK::VKInit::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
	pipelineBuilder.mRasterizer.cullMode = VK_CULL_MODE_NONE;
	pipelineBuilder.mMultisampling = Rendering::Renderer::VK::VKInit::multisamplingStateCreateInfo();
	
	pipelineBuilder.mColorBlendAttachment = Rendering::Renderer::VK::VKInit::colorBlendAttachmentState();
	pipelineBuilder.mColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	pipelineBuilder.mColorBlendAttachment.blendEnable = VK_TRUE;
	pipelineBuilder.mColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	pipelineBuilder.mColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	pipelineBuilder.mColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	pipelineBuilder.mColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	pipelineBuilder.mColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	pipelineBuilder.mColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;


	pipelineBuilder.mDepthStencil = Rendering::Renderer::VK::VKInit::depthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	VkPipeline result = pipelineBuilder.buildPipeline(pPass);

	service(Rendering::Data::DeletionQueue).pushFunction([=]()
	{
		vkDestroyPipeline(service(VkDevice), result, nullptr);
	});

	return { result, &pShader };
}