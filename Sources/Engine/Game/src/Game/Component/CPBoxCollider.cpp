#include "Game/Component/CPBoxCollider.h"

using namespace Game::Component;

CPBoxCollider::CPBoxCollider(Maths::FVector3 pPosition, Maths::FVector3 pSize)
    : BulBoxCollider(pPosition, pSize)
{
}

AComponent* CPBoxCollider::clone()
{
    return new CPBoxCollider(*this);
}