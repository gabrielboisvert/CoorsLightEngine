#pragma once
#include "Renderer/VK/VKRenderer.h"
#include <string>
#include <vector>
#include <array>
#include <vulkan/vulkan_core.h>
#include "Maths/FVector3.h"
#include "Rendering/Buffers/VK/VKUniformBuffer.h"
#include "Rendering/Data/UniformData.h"
#include "Rendering/Buffers/VK/VKVertexBuffer.h"
#include "Rendering/Geometry/DebugVertex.h"

namespace Rendering
{
	class LineDrawer
	{
		public:
			VkPipeline mGraphicsPipeline = nullptr;
			VkPipelineLayout mPipelineLayout = nullptr;
			VkDescriptorSetLayout mObjectSetLayout;
			std::vector<Geometry::DebugVertex> mLineVertice;

			Buffers::VK::VKVertexBuffer* mVertexBuffer = nullptr;

			Buffers::VK::VKUniformBuffer<Rendering::Data::UniformData> mUniform;
			
			Renderer::VK::VKRenderer& mRenderer;
			Rendering::Data::UniformData& mData;

			LineDrawer(Renderer::VK::VKRenderer& pRenderer, Rendering::Data::UniformData& pData, const char* pVertex, const char* pFragment);
			~LineDrawer();

			static std::vector<char> readFile(const std::string& pFilename);
			VkShaderModule createShaderModule(const std::vector<char>& pCode);
			void createGraphicsPipeline(const char* pVertex, const char* pFragment);
			void createDescriptorSetLayout();

			void drawLine(const Maths::FVector3& pFrom, const Maths::FVector3& pTo, const Maths::FVector3& pColor);
			void flushLines();
			void setVP();

			void createVertexBuffer();
			void reset();
	};
}