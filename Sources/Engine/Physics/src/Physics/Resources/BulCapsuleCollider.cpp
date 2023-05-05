#include "Physics\Resources\BulCapsuleCollider.h"
#include "EngineCore/Service/ServiceLocator.h"
#include <Bullet/btBulletDynamicsCommon.h>
Physics::Resources::BulCapsuleCollider::BulCapsuleCollider(Maths::FVector3 pPosition, float pRadius, float pHeight, void* pActor)
	: IRigidbody(pPosition), mHeight(pHeight), mRadius(pRadius), mCapsule(mRadius, mHeight)
{
	createCollider();
	service(Core::BulPhysicsEngine).addCollider(*mCollider);

	std::function<void(void* pRigidbody,void* pOtherCollider, btPersistentManifold* persistent)> collisionEnterCallback = [this](void* pRigidbody, void* pOtherCollider, btPersistentManifold* pPersistent) {
		OnAnyCollisionEnter(pRigidbody, pOtherCollider, pPersistent);
	};

	std::function<void(void* pRigidbody, void* pOtherCollider, btPersistentManifold* persistent)> collisionStayCallback = [this](void* pRigidbody, void* pOtherCollider, btPersistentManifold* pPersistent) {
		OnAnyCollisionStay(pRigidbody, pOtherCollider, pPersistent);
	};

	std::function<void(void* pRigidbody, void* pOtherCollider, btPersistentManifold* manifold)> collisionExitCallback = [this](void* pRigidbody, void* pOtherCollider, btPersistentManifold* pPersistent) {
		OnAnyCollisionExit(pRigidbody, pOtherCollider, pPersistent);
	};

	mCallback.physicsObject = pActor;
	mCallback.enter = collisionEnterCallback;
	mCallback.stay = collisionStayCallback;
	mCallback.exit = collisionExitCallback;
	mCallback.rigidbody = this;
}

Physics::Resources::BulCapsuleCollider::BulCapsuleCollider(BulCapsuleCollider& pOther)
	: IRigidbody(pOther), mHeight(pOther.mHeight), mRadius(pOther.mRadius), mCapsule(pOther.mCapsule), mMotion(pOther.mMotion), mCallback(pOther.mCallback)
{
	mCollider = std::move(pOther.mCollider);
}

Physics::Resources::BulCapsuleCollider::~BulCapsuleCollider()
{
	service(Core::BulPhysicsEngine).removeCollider(*mCollider);
	delete mCollider;
}

void Physics::Resources::BulCapsuleCollider::setTransform(Maths::FVector3 pPosition, Maths::FQuaternion pRotation, float pRadius, float pHeight)
{
	mPosition = pPosition;
	mRotation = pRotation;
	mRadius = pRadius;
	mHeight = pHeight;

	service(Core::BulPhysicsEngine).removeCollider(*mCollider);

	delete mCollider;
	createCollider();

	service(Core::BulPhysicsEngine).addCollider(*mCollider);
}

void Physics::Resources::BulCapsuleCollider::updateTransform()
{
	service(Core::BulPhysicsEngine).removeCollider(*mCollider);

	delete mCollider;
	createCollider();

	service(Core::BulPhysicsEngine).addCollider(*mCollider);
}

void Physics::Resources::BulCapsuleCollider::createCollider()
{
	btVector3 inertia;
	mCapsule = btCapsuleShape(mRadius, mHeight);
	mCapsule.calculateLocalInertia(mMass, inertia);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(mPosition.x + mCenter.x, mPosition.y + mCenter.y, mPosition.z + mCenter.z));
	Maths::FMatrix4 mat4 = mRotation.toMatrix4();
	transform.setBasis(btMatrix3x3(mat4.data[0][0], mat4.data[1][0], mat4.data[2][0], mat4.data[0][1], mat4.data[1][1], mat4.data[2][1], mat4.data[0][2], mat4.data[1][2], mat4.data[2][2]));
	mMotion = btDefaultMotionState(transform);

	btRigidBody::btRigidBodyConstructionInfo info(mMass, &mMotion, &mCapsule, inertia);

	mCollider = new btRigidBody(info);
	mCollider->setUserPointer(&mCallback);
	mCollider->setCollisionFlags(mCollider->getCollisionFlags() | (btCollisionObject::CollisionFlags)mType);
}
