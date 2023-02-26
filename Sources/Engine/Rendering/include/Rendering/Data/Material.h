#pragma once
#include "Rendering/Data/Shader.h"
#include "Rendering/Data/VKTypes.h"

namespace Rendering::Data
{
	struct BufferWriteDescriptor
	{
		int mDstSet;
		int mDstBinding;
		VkDescriptorType mDescriptorType;
		VkDescriptorBufferInfo mBufferInfo{ nullptr, 0, 0 };
		VkDescriptorImageInfo mImageInfo{ nullptr, nullptr, VK_IMAGE_LAYOUT_UNDEFINED };
		uint32_t mDynamicOffset;
	};

	struct DynOffsets
	{
		std::array<uint32_t, 16> offsets;
		uint32_t count{ 0 };
	};

	struct Material
	{
		VkPipeline mPipeline;
		Shader* mShader = nullptr;

		std::vector<BufferWriteDescriptor> mBufferWrites;
		std::array<VkDescriptorSet, 4> mCachedDescriptorSets;
		std::array<DynOffsets, 4> mSetOffsets;

		~Material();
		void bindBuffer(const char* pName, const VkDescriptorBufferInfo& pBufferInfo);
		void bindDynamicBuffer(const char* pName, uint32_t pOffset, const VkDescriptorBufferInfo& pBufferInfo);
		void bindImage(const char* pName, const VkDescriptorImageInfo& pImageInfo);
		void buildSets();

		void bindDescriptor(const char* pName, const VkDescriptorSet& pImageInfo);
		void applyBinds(VkCommandBuffer cmd);
		void bindPipeLine(VkCommandBuffer cmd);
	};
}