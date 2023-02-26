#include "Physics/Resources/BulBoxCollider.h"
#include "EngineCore/Service/ServiceLocator.h"
#include <Bullet/btBulletDynamicsCommon.h>

using namespace Physics::Resources;

BulBoxCollider::BulBoxCollider(Maths::FVector3 pPosition, Maths::FVector3 pSize) 
	: mPosition(pPosition), mSize(pSize), mBox(btVector3(mSize.x, mSize.y, mSize.z))
{
	createCollider();
	service(Core::BulPhysicsEngine).addBoxCollider(*mCollider);
}

BulBoxCollider::BulBoxCollider(BulBoxCollider& pOther)
	: mType(pOther.mType), mPosition(pOther.mPosition), mSize(pOther.mSize), mMass(pOther.mMass), 
	mBox(pOther.mBox), mMotion(pOther.mMotion)
{
	mCollider = std::move(pOther.mCollider);
}

BulBoxCollider::~BulBoxCollider()
{
	service(Core::BulPhysicsEngine).removeCollider(*mCollider);
	delete mCollider;
}

Maths::FVector3 BulBoxCollider::getPositionFromPhysics()
{
	btTransform transform;
	btScalar worldmatrix[16];
	transform = mCollider->getWorldTransform();
	mPosition = Maths::FVector3(transform.getOrigin().x(), transform.getOrigin().y(), transform.getOrigin().z());

	return mPosition;
}

void BulBoxCollider::setTransform(Maths::FVector3 pPosition, Maths::FQuaternion pRotation, Maths::FVector3 pSize)
{
	mPosition = pPosition;
	mRotation = pRotation;
	mSize = pSize;

	service(Core::BulPhysicsEngine).removeCollider(*mCollider);

	delete mCollider;
	createCollider();

	service(Core::BulPhysicsEngine).addBoxCollider(*mCollider);
}

void BulBoxCollider::setType(Data::TypeRigidBody pType)
{
	mType = pType;

	service(Core::BulPhysicsEngine).removeCollider(*mCollider);

	delete mCollider;
	createCollider();

	service(Core::BulPhysicsEngine).addBoxCollider(*mCollider);
}

void BulBoxCollider::updateMass(double pValue)
{
	mMass = pValue;
	
	service(Core::BulPhysicsEngine).removeCollider(*mCollider);

	delete mCollider;
	createCollider();

	service(Core::BulPhysicsEngine).addBoxCollider(*mCollider);
}

void BulBoxCollider::updateTransform()
{
	service(Core::BulPhysicsEngine).removeCollider(*mCollider);

	delete mCollider;
	createCollider();

	service(Core::BulPhysicsEngine).addBoxCollider(*mCollider);
	
}

void BulBoxCollider::createCollider()
{
	btVector3 inertia;
	mBox = btBoxShape(btVector3(mSize.x, mSize.y, mSize.z));
	mBox.calculateLocalInertia(mMass, inertia);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(mPosition.x, mPosition.y, mPosition.z));
	Maths::FMatrix4 mat4 = mRotation.toMatrix4();
	transform.setBasis(btMatrix3x3(mat4.data[0][0], mat4.data[1][0], mat4.data[2][0], mat4.data[0][1], mat4.data[1][1], mat4.data[2][1], mat4.data[0][2], mat4.data[1][2], mat4.data[2][2]));
	mMotion = btDefaultMotionState(transform);

	btRigidBody::btRigidBodyConstructionInfo info(mMass, &mMotion, &mBox, inertia);

	mCollider = new btRigidBody(info);
	mCollider->setUserPointer(&mCallback);
	mDefaultflag = mCollider->getCollisionFlags();
	mCollider->setCollisionFlags(mDefaultflag | (btCollisionObject::CollisionFlags)mType);
}