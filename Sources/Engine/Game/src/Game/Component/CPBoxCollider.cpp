#include "Game/Component/CPBoxCollider.h"
#include "Physics/Data/TypeRigidBody.h"
#include "Game/Component/CPCapsuleCollider.h"
#include <Game/Utils/ComponentType.h>
using namespace Game::Component;

CPBoxCollider::CPBoxCollider(Maths::FVector3 pPosition, Maths::FVector3 pSize, void* pActor)
    : BulBoxCollider(pPosition, pSize, pActor)
{
}

void CPBoxCollider::UpdateTransform(Maths::FTransform& pTransform)
{
    setTransform(pTransform.mLocalPosition, pTransform.mLocalRotation, pTransform.mLocalScale);
}

void CPBoxCollider::recreateCollider()
{
    updateTransform();
}

Maths::FVector3 CPBoxCollider::getPhysicsPosition()
{
    return getPositionFromPhysics();
}

Maths::FQuaternion CPBoxCollider::getPhysicsRotation()
{
    return getRotationFromPhysics();
}

AComponent* CPBoxCollider::clone()
{
    return new CPBoxCollider(*this);
}

void CPBoxCollider::OnAnyCollisionEnter(void* pRigidbody, void* pOtherCollider, btPersistentManifold* persistent)
{
    IRigidbody* OtherCollider = (IRigidbody*)pRigidbody;

    if (mColliderType == Physics::Data::TypeRigidBody::E_TRIGGER || OtherCollider->mType == Physics::Data::TypeRigidBody::E_TRIGGER)
        ACollider::OnTriggerEnter(pOtherCollider, (ACollider*)pRigidbody);
    else
        ACollider::OnCollisionEnter(pOtherCollider, (ACollider*)pOtherCollider);
}

void CPBoxCollider::OnAnyCollisionStay(void* pRigidbody, void* pOtherCollider, btPersistentManifold* persistent)
{
    IRigidbody* OtherCollider = (IRigidbody*)pRigidbody;

    if (mColliderType == Physics::Data::TypeRigidBody::E_TRIGGER || OtherCollider->mType == Physics::Data::TypeRigidBody::E_TRIGGER)
        ACollider::OnTriggerStay(pOtherCollider, (ACollider*)pOtherCollider);
    else
        ACollider::OnCollisionStay(pOtherCollider, (ACollider*)pOtherCollider);
}

void CPBoxCollider::OnAnyCollisionExit(void* pRigidbody, void* pOtherCollider, btPersistentManifold* persistent)
{
    IRigidbody* OtherCollider = (IRigidbody*)pRigidbody;

    if (mColliderType == Physics::Data::TypeRigidBody::E_TRIGGER || OtherCollider->mType == Physics::Data::TypeRigidBody::E_TRIGGER)
        ACollider::OnTriggerExit(pOtherCollider, (ACollider*)pOtherCollider);
    else
        ACollider::OnCollisionExit(pOtherCollider, (ACollider*)pOtherCollider);
}

void CPBoxCollider::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter)
{
    pWriter.StartObject();

    pWriter.Key("Type");
    pWriter.Int((int)Game::Utils::ComponentType::BoxCollider);

    pWriter.Key("TypeCollision");
    pWriter.Int((int)mType);

    pWriter.Key("Mass");
    pWriter.Double(mMass);

    pWriter.Key("Center");
    pWriter.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
    pWriter.StartArray();
    pWriter.Double(mCenter.x);
    pWriter.Double(mCenter.y);
    pWriter.Double(mCenter.z);
    pWriter.EndArray();
    pWriter.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatDefault);

    pWriter.Key("Size");
    pWriter.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
    pWriter.StartArray();
    pWriter.Double(mSize.x);
    pWriter.Double(mSize.y);
    pWriter.Double(mSize.z);
    pWriter.EndArray();
    pWriter.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatDefault);

    pWriter.EndObject();
}

void Game::Component::CPBoxCollider::addForce(Maths::FVector3 Force)
{
    addForcePhysics(Force);
}

void Game::Component::CPBoxCollider::clearForces()
{
    clearForcesPhysics();
}

void Game::Component::CPBoxCollider::setLinearVelocity(Maths::FVector3 pVelocity)
{
    setLinearVelocityPhysics(pVelocity);
}

void Game::Component::CPBoxCollider::setAngularVelocity(Maths::FVector3 pVelocity)
{
    setAngularVelocityPhysics(pVelocity);
}

void Game::Component::CPBoxCollider::clearLinearVelocity()
{
    clearLinearVelocityPhysics();
}
