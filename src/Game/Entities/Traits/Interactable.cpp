#include "interactable.h"
#include "Engine/Collision/CollisionTest.h"
#include "Game/Entities/Player.h"
#include "engine/camera/camera.h"

bool TInteractable::IsVolumeCollidingWithPlayer()
{
	auto& Box = EPlayer::Get()->BoundingBox;
	auto Test = TestCollisionBoxAgainstCylinder(Box, Cylinder);
	return Test.Collision;
}

void TInteractable::Draw()
{
	RRay Ray = CastFirstPersonRay();
	RImDraw::AddLine(IMHASH, Ray.Origin, Ray.Origin + Ray.Direction * 20.f, 2000, COLOR_BLUE_2, 3, false);
}

bool TInteractable::IsPlayerLookingAtEntity(EEntity* Entity)
{
	RRay Ray = CastFirstPersonRay();
	
	RImDraw::AddLine(IMHASH, Ray.Origin, Ray.Origin + Ray.Direction * 20.f);
	RImDraw::AddBoundingBox(IMHASH, Entity->BoundingBox);
	
	auto HitResults = TestRayAgainstEntity(Ray, Entity, RayCast_TestOnlyFromOutsideIn);
	return HitResults.Hit;
}

bool TInteractable::IsPlayerInteracting()
{
	return EPlayer::Get()->bInteractButton;
}
