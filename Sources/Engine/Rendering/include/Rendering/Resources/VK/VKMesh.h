#pragma once

#include "Rendering/Resources/IMesh.h"
#include "Rendering/Data/VKTypes.h"
#include "Rendering/Buffers/VK/VKVertexBuffer.h"
#include "Rendering/Buffers/VK/VKIndiceBuffer.h"

namespace Rendering::Resources::VK
{
	class VKMesh : public IMesh
	{
		public:
			Buffers::VK::VKVertexBuffer mVertexBuffer;
			Buffers::VK::VKIndiceBuffer mIndicesBuffer;

			VKMesh(const std::vector<Geometry::Vertex>& pVertices, const std::vector<uint32_t>& pIndices);
			~VKMesh();

			void draw(void* pCommandBuffer) override;
	};
}