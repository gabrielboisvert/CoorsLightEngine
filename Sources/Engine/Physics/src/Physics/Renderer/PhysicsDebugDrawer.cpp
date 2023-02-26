#include "Physics/Renderer/PhysicsDebugDrawer.h"
#include "Rendering/Resources/VK/PipeLineBuilder.h"
#include "Rendering/Renderer/VK/VKRenderer.h"
#include "Rendering/Buffers/VK/VKVertexBuffer.h"

using namespace Physics::Renderer;

PhysicsDebugDrawer::PhysicsDebugDrawer() 
	: mUniformBuffer(VK_SHADER_STAGE_VERTEX_BIT), mMat(Rendering::Renderer::Resources::VK::PipeLineBuilder()
		.initPipeLine("Resources/Editor/Shaders/LineVertex.vert.spv",
			"Resources/Editor/Shaders/LineFrag.frag.spv", service(Rendering::Renderer::VK::VKRenderer).mRenderPass, true, true))
{
	mUniformBuffer.mData.mModel = Maths::FMatrix4::createTransformMatrix({}, { }, { Maths::FVector3::One });
	mUniformBuffer.mData.mViewProjection = Maths::FMatrix4::createPerspective(-45, 1, 0.01f, 500.0f)
		* Maths::FMatrix4::lookAt({ 0, 0, -10 }, { 0, 0, 0 }, Maths::FVector3::Up);
	mUniformBuffer.updateData();

	mMat.bindDescriptor("ubo", mUniformBuffer.mDescriptorSets);

	mLineVertices.resize(10000);
	mVertexBuffer = new Rendering::Buffers::VK::VKDynamicVertexBuffer(mLineVertices.data(), 10000 * sizeof(Rendering::Geometry::DebugVertex));
}

PhysicsDebugDrawer::~PhysicsDebugDrawer()
{
	delete mVertexBuffer;
}

void PhysicsDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor)
{
}

void PhysicsDebugDrawer::drawLine(const btVector3& pFrom, const btVector3& pTo, const btVector3& pColor)
{
	mLineVertices.emplace_back(Rendering::Geometry::DebugVertex(pFrom.x(), pFrom.y(), pFrom.z(), pColor.x(), pColor.y(), pColor.z()));
	mLineVertices.emplace_back(Rendering::Geometry::DebugVertex(pTo.x(), pTo.y(), pTo.z(), pColor.x(), pColor.y(), pColor.z()));

	size += 2;
}

void PhysicsDebugDrawer::drawSphere(const btVector3& p, btScalar radius, const btVector3& color)
{
}

void PhysicsDebugDrawer::drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha)
{
}

void PhysicsDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
}

void PhysicsDebugDrawer::reportErrorWarning(const char* warningString)
{
}

void PhysicsDebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
}

void PhysicsDebugDrawer::setDebugMode(int debugMode)
{
	mDebugMode = debugMode;
}

int PhysicsDebugDrawer::getDebugMode() const
{
	return mDebugMode;
}

void PhysicsDebugDrawer::flushLines()
{
	if (mLineVertices.size() == 0)
		return;

	Rendering::Buffers::VK::VKBuffer::mapBuffer(mLineVertices.data(), size * sizeof(Rendering::Geometry::DebugVertex), mVertexBuffer->mVertexBuffer.mAllocation);

	VkCommandBuffer cmd = service(Rendering::Renderer::VK::VKRenderer).getCurrentCommandBuffer();

	mMat.bindPipeLine(cmd);

	mVertexBuffer->bind(cmd);
	vkCmdDraw(cmd, size, 1, 0, 0);

	mLineVertices.resize(0);
	size = 0;
}