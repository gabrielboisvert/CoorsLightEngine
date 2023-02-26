#pragma once
#include <Bullet/LinearMath/btIDebugDraw.h>
#include "Rendering/LineDrawer.h"
#include "Rendering/Geometry/DebugVertex.h"
#include "Rendering/Data/UniformData.h"
#include "Rendering/Buffers/VK/VKUniformBuffer.h"
#include "Rendering/Data/Material.h"
#include "Rendering/Buffers/VK/VKDynamicVertexBuffer.h"

namespace Physics::Renderer
{
	class PhysicsDebugDrawer : public btIDebugDraw
	{
		public:
			std::vector<Rendering::Geometry::DebugVertex> mLineVertices;
			int size = 0;
			Rendering::Data::Material mMat;
			Rendering::Buffers::VK::VKUniformBuffer<Rendering::Data::UniformData> mUniformBuffer;
			Rendering::Buffers::VK::VKDynamicVertexBuffer* mVertexBuffer = nullptr;
			int mDebugMode = 1;

	
			PhysicsDebugDrawer();

			virtual ~PhysicsDebugDrawer();

			virtual void   drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor) override;

			virtual void   drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

			virtual void   drawSphere(const btVector3& p, btScalar radius, const btVector3& color) override;

			virtual void   drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha) override;

			virtual void   drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;

			virtual void   reportErrorWarning(const char* warningString) override;

			virtual void   draw3dText(const btVector3& location, const char* textString) override;

			virtual void setDebugMode(int debugMode) override;

			virtual int getDebugMode() const override;

			void flushLines() override;


	};
}