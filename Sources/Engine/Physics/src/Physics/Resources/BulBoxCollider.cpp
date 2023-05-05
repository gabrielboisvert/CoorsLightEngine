#include "Physics/Resources/BulBoxCollider.h"
#include "EngineCore/Service/ServiceLocator.h"
#include <Bullet/btBulletDynamicsCommon.h>

using namespace Physics::Resources;

BulBoxCollider::BulBoxCollider(Maths::FVector3 pPosition, Maths::FVector3 pSize, void* pActor) 
	: IRigidbody(pPosition), mSize(pSize), mBox(btVector3(mSize.x, mSize.y, mSize.z))
{
	createCollider();
	service(Core::BulPhysicsEngine).addCollider(*mCollider);

	std::function<void(void* pRigidbody, void* pOtherCollider, btPersistentManifold* persistent)> collisionEnterCallback = [this](void* pRigidbody, void* pOtherCollider, btPersistentManifold* pPersistent) {
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

BulBoxCollider::BulBoxCollider(BulBoxCollider& pOther)
	: IRigidbody(pOther),  mSize(pOther.mSize), mBox(pOther.mBox), mMotion(pOther.mMotion), mCallback(pOther.mCallback)
{
	mCollider = std::move(pOther.mCollider);
}

BulBoxCollider::~BulBoxCollider()
{
	service(Core::BulPhysicsEngine).removeCollider(*mCollider);
	delete mCollider;
}

void BulBoxCollider::setTransform(Maths::FVector3 pPosition, Maths::FQuaternion pRotation, Maths::FVector3 pSize)
{
	mPosition = pPosition;
	mRotation = pRotation;
	mSize = pSize;

	service(Core::BulPhysicsEngine).removeCollider(*mCollider);

	delete mCollider;
	createCollider();

	service(Core::BulPhysicsEngine).addCollider(*mCollider);
}

void BulBoxCollider::updateTransform()
{
	service(Core::BulPhysicsEngine).removeCollider(*mCollider);

	delete mCollider;
	createCollider();

	service(Core::BulPhysicsEngine).addCollider(*mCollider);
}

void BulBoxCollider::createCollider()
{
	btVector3 inertia;
	mBox = btBoxShape(btVector3(mSize.x / 2, mSize.y / 2, mSize.z / 2));
	mBox.calculateLocalInertia(mMass, inertia);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(mPosition.x + mCenter.x, mPosition.y + mCenter.y, mPosition.z + mCenter.z));
	Maths::FMatrix4 mat4 = mRotation.toMatrix4();
	transform.setBasis(btMatrix3x3(mat4.data[0][0], mat4.data[1][0], mat4.data[2][0], mat4.data[0][1], mat4.data[1][1], mat4.data[2][1], mat4.data[0][2], mat4.data[1][2], mat4.data[2][2]));
	mMotion = btDefaultMotionState(transform);
	
	btRigidBody::btRigidBodyConstructionInfo info(mMass, &mMotion, &mBox, inertia);

	mCollider = new btRigidBody(info);
	mCollider->setUserPointer(&mCallback);
	mDefaultflag = mCollider->getCollisionFlags();
	mCollider->setCollisionFlags(mDefaultflag | (btCollisionObject::CollisionFlags)mType);
}