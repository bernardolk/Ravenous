#include "game/entities/player.h"
#include "engine/collision/cl_controller.h"


void Player::Update(T_World* world, bool update_collider)
{
	// perform updates to bounding boxes, colliders etc
	UpdateModelMatrix();
	if (update_collider)
	{
		UpdateCollider();
		UpdateBoundingBox();
	}

	if (CL_UpdatePlayerWorldCells(this, world))
	{
		CL_RecomputeCollisionBufferEntities(this);
	}
}

vec3 Player::GetLastTerrainContactPoint() const
{
	const vec3 player_btm_sphere_center = position + vec3(0, radius, 0);
	return player_btm_sphere_center + -last_terrain_contact_normal * radius;
}

bool Player::MaybeHurtFromFall()
{
	float fall_height = height_before_fall - position.y;
	fall_height_log = fall_height;
	if (fall_height >= hurt_height_2)
	{
		lives -= 2;
		return true;
	}
	if (fall_height >= hurt_height_1)
	{
		lives -= 1;
		return true;
	}
	return false;
}

void Player::RestoreHealth()
{
	lives = initial_lives;
}

void Player::SetCheckpoint(E_Entity* entity)
{
	return;
	/*
	if (entity->type != EntityType_Checkpoint)
		assert(false);

	checkpoint_pos = position;
	checkpoint = entity;
	*/
}

void Player::GotoCheckpoint()
{
	position = checkpoint_pos;
}

void Player::Die()
{
	lives = initial_lives;
	velocity = vec3(0);
	player_state = PlayerState::Standing;
	ForceInterruptPlayerAnimation(this);
	GotoCheckpoint();
}

void Player::BruteStop()
{
	// bypass deaceleration steps. Stops player right on his tracks.
	velocity = vec3(0);
}

Player* Player::ResetPlayer()
{
	auto* player = Get();
	*player = Player{};
	return player;
}
