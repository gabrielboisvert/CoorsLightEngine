#include "Rendering/LineDrawer.h"
#include "Rendering/Resources/VK/PipeLineBuilder.h"
#include "Rendering/Buffers/VK/VKVertexBuffer.h"

using namespace Rendering;

LineDrawer::LineDrawer(Rendering::Renderer::VK::VKRenderer& pRenderer)
	: mUniformBuffer(VK_SHADER_STAGE_VERTEX_BIT), mMat(Rendering::Renderer::Resources::VK::PipeLineBuilder()
		.initPipeLine("Resources/Engine/Shaders/LineVertex.vert.spv",
			"Resources/Engine/Shaders/LineFrag.frag.spv", pRenderer.mRenderPass, true, true, true)), mRenderer(pRenderer)
{
	mUniformBuffer.mData.mModel = Maths::FMatrix4::createTransformMatrix({}, {}, { Maths::FVector3::One });

	mMat.bindDescriptor("ubo", mUniformBuffer.mDescriptorSets);

	mLineVertices.reserve(MAX_LINE);
	mVertexBuffer = new Rendering::Buffers::VK::VKDynamicVertexBuffer(mLineVertices.data(), mLineVertices.capacity() * sizeof(Rendering::Geometry::DebugVertex));
}

LineDrawer::~LineDrawer()
{
	delete mVertexBuffer;
}

void LineDrawer::drawLine(const Maths::FVector3& pFrom, const Maths::FVector3& pTo, const Maths::FVector3& pColor)
{
	mLineVertices.emplace_back(Rendering::Geometry::DebugVertex(pFrom.x, pFrom.y, pFrom.z, pColor.x, pColor.y, pColor.z));
	mLineVertices.emplace_back(Rendering::Geometry::DebugVertex(pTo.x, pTo.y, pTo.z, pColor.x, pColor.y, pColor.z));
}

void LineDrawer::flushLines()
{
	if (mLineVertices.size() == 0)
		return;

	Rendering::Buffers::VK::VKBuffer::mapBuffer(mLineVertices.data(), mLineVertices.size() * sizeof(Rendering::Geometry::DebugVertex), mVertexBuffer->mVertexBuffer.mAllocation);

	VkCommandBuffer cmd = mRenderer.getCurrentCommandBuffer();

	mMat.bindPipeLine(cmd);

	mVertexBuffer->bind(cmd);
	vkCmdDraw(cmd, mLineVertices.size(), 1, 0, 0);
}

void LineDrawer::updateViewProj(Maths::FMatrix4 pViewProj)
{
	mUniformBuffer.mData.mViewProjection = pViewProj;
	mUniformBuffer.updateData();
}

void LineDrawer::reset()
{
	mLineVertices.resize(0);
}