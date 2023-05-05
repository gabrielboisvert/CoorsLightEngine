#include "Game/Component/CPCapsuleCollider.h"
#include <Game/Utils/ComponentType.h>

using namespace Game::Component;

CPCapsuleCollider::CPCapsuleCollider(Maths::FVector3 pPosition, float pRadius, float pHeight, void* pActor)
    : BulCapsuleCollider(pPosition, pRadius, pHeight, pActor)
{
}

void CPCapsuleCollider::UpdateTransform(Maths::FTransform& pTransform)
{
    setTransform(pTransform.mLocalPosition, pTransform.mLocalRotation,  pTransform.mLocalScale.x > pTransform.mLocalScale.z ? pTransform.mLocalScale.x : pTransform.mLocalScale.z, pTransform.mLocalScale.y);
}

void CPCapsuleCollider::recreateCollider()
{
    updateTransform();
}

Maths::FVector3 CPCapsuleCollider::getPhysicsPosition()
{
    return getPositionFromPhysics();
}

AComponent* CPCapsuleCollider::clone()
{
    return new CPCapsuleCollider(*this);
}


void CPCapsuleCollider::OnAnyCollisionEnter(void* pRigidbody, void* pOtherCollider, btPersistentManifold* persistent)
{
    IRigidbody* OtherCollider = (IRigidbody*)pRigidbody;

    if (mColliderType == Physics::Data::TypeRigidBody::E_TRIGGER || OtherCollider->mType == Physics::Data::TypeRigidBody::E_TRIGGER)
        ACollider::OnTriggerEnter(pOtherCollider, (ACollider*)pOtherCollider);
    else
        ACollider::OnCollisionEnter(pOtherCollider, (ACollider*)pOtherCollider);
}

void CPCapsuleCollider::OnAnyCollisionStay(void* pRigidbody, void* pOtherCollider, btPersistentManifold* persistent)
{
    IRigidbody* OtherCollider = (IRigidbody*)pRigidbody;

    if (mColliderType == Physics::Data::TypeRigidBody::E_TRIGGER || OtherCollider->mType == Physics::Data::TypeRigidBody::E_TRIGGER)
        ACollider::OnTriggerStay(pOtherCollider, (ACollider*)pOtherCollider);
    else
        ACollider::OnCollisionStay(pOtherCollider, (ACollider*)pOtherCollider);
}

void CPCapsuleCollider::OnAnyCollisionExit(void* pRigidbody, void* pOtherCollider, btPersistentManifold* persistent)
{
    IRigidbody* OtherCollider = (IRigidbody*)pRigidbody;

    if (mColliderType == Physics::Data::TypeRigidBody::E_TRIGGER || OtherCollider->mType == Physics::Data::TypeRigidBody::E_TRIGGER)
        ACollider::OnTriggerExit(pOtherCollider, (ACollider*)pOtherCollider);
    else
        ACollider::OnCollisionExit(pOtherCollider, (ACollider*)pOtherCollider);
}

Maths::FQuaternion CPCapsuleCollider::getPhysicsRotation()
{
    return getRotationFromPhysics();
}

void CPCapsuleCollider::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter)
{
    pWriter.StartObject();

    pWriter.Key("Type");
    pWriter.Int((int)Game::Utils::ComponentType::CapsuleCollider);

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

    pWriter.Key("Radius");
    pWriter.Double(mRadius);

    pWriter.Key("Height");
    pWriter.Double(mHeight);

    pWriter.EndObject();
}

void Game::Component::CPCapsuleCollider::addForce(Maths::FVector3 Force)
{
    addForcePhysics(Force);
}

void Game::Component::CPCapsuleCollider::clearForces()
{
    clearForcesPhysics();
}

void Game::Component::CPCapsuleCollider::setLinearVelocity(Maths::FVector3 pVelocity)
{
    setLinearVelocityPhysics(pVelocity);
}

void Game::Component::CPCapsuleCollider::setAngularVelocity(Maths::FVector3 pVelocity)
{
    setAngularVelocityPhysics(pVelocity);
}

void Game::Component::CPCapsuleCollider::clearLinearVelocity()
{
    clearLinearVelocityPhysics();
}
