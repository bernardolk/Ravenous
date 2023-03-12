#include "game/entities/player.h"
#include "engine/collision/cl_controller.h"


void Player::Update(World* world, bool update_collider)
{
	// perform updates to bounding boxes, colliders etc
	entity_ptr->UpdateModelMatrix();
	if(update_collider)
	{
		entity_ptr->UpdateCollider();
		entity_ptr->UpdateBoundingBox();
	}

	if(CL_update_player_world_cells(this, world))
	{
		CL_recompute_collision_buffer_entities(this);
	}
}

vec3 Player::GetLastTerrainContactPoint() const
{
	const vec3 player_btm_sphere_center = entity_ptr->position + vec3(0, radius, 0);
	return player_btm_sphere_center + -last_terrain_contact_normal * radius;
}

bool Player::MaybeHurtFromFall()
{
	float fall_height = height_before_fall - entity_ptr->position.y;
	fall_height_log = fall_height;
	if(fall_height >= hurt_height_2)
	{
		lives -= 2;
		return true;
	}
	if(fall_height >= hurt_height_1)
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

void Player::SetCheckpoint(Entity* entity)
{
	if(entity->type != EntityType_Checkpoint)
		assert(false);

	checkpoint_pos = entity_ptr->position;
	checkpoint = entity;
}

void Player::GotoCheckpoint()
{
	entity_ptr->position = checkpoint_pos;
}

void Player::Die()
{
	lives = initial_lives;
	entity_ptr->velocity = vec3(0);
	player_state = PlayerState::Standing;
	ForceInterruptPlayerAnimation(this);
	GotoCheckpoint();
}

void Player::BruteStop()
{
	// bypass deaceleration steps. Stops player right on his tracks.
	speed = 0;
}

void Player::MakeInvisible()
{
	entity_ptr->MakeInvisible();
}

void Player::MakeVisible()
{
	entity_ptr->MakeVisible();
}

Player* Player::ResetPlayer()
{
	auto* player = Get();
	*player = Player{};
	return player;
}