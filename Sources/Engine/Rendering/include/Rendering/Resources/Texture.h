#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "Rendering/Resources/IResource.h"
#include "Rendering/Data/VKTypes.h"

namespace Rendering::Resources
{
	class Texture : public IResource
	{
		public:
			Data::AllocatedImage mTexture;
			VkImageView mImageView;
			VkSampler mTextureSampler;
			VkDescriptorSet mTextureSets;

			Texture(const std::string& pFileName);
			~Texture() override;

			void createTextureImage(const std::string& pFileName);
			void createTextureImageView();
			void createTextureSampler();
			void createDescritorSet();
	};
}