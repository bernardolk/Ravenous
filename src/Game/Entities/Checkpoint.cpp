#include "Checkpoint.h"
#include "Player.h"

ECheckpoint::ECheckpoint()
{
	Cylinder.Height = 1.f;
	Cylinder.Radius = 1.f;
}

void ECheckpoint::Interact()
{
	auto* Player = EPlayer::Get();
	Player->Checkpoint = MakeHandle<ECheckpoint>(this);
	Player->Lives = 2;
}

vec3 ECheckpoint::GetPlayerSpawnPosition()
{
	return Position + vec3(MatModel[2]) * 2.f;
}
