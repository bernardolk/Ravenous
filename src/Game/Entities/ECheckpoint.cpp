#include "ECheckpoint.h"

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
