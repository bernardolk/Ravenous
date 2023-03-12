#include "game/animation/an_update.h"

#include "engine/core/logging.h"
#include "engine/rvn.h"
#include "engine/entities/entity.h"

void EntityAnimation::Update()
{
	/* executes current keyframe in entity and updates runtimes, turns animation inactive once it ends. */

	float frame_duration_ms = Rvn::frame.duration * 1000;

	auto kf = &keyframes[current_keyframe];

	keyframe_runtime += frame_duration_ms;
	runtime += frame_duration_ms;

	// --------------------
	// > Perform animation
	// --------------------

	float speed;
	// Update entity position
	if (kf->flags & EntityAnimKfFlags_ChangePosition)
	{
		//x
		speed = (kf->final_position.x - kf->starting_position.x) / kf->duration;
		entity->position.x += speed * frame_duration_ms;
		//y
		speed = (kf->final_position.y - kf->starting_position.y) / kf->duration;
		entity->position.y += speed * frame_duration_ms;
		//z
		speed = (kf->final_position.z - kf->starting_position.z) / kf->duration;
		entity->position.z += speed * frame_duration_ms;
	}

	// Update entity rotation
	if (kf->flags & EntityAnimKfFlags_ChangeRotation)
	{
		//x
		speed = (kf->final_rotation.x - kf->starting_rotation.x) / kf->duration;
		entity->rotation.x += speed * frame_duration_ms;
		//y
		speed = (kf->final_rotation.y - kf->starting_rotation.y) / kf->duration;
		entity->rotation.y += speed * frame_duration_ms;
		//z
		speed = (kf->final_rotation.z - kf->starting_rotation.z) / kf->duration;
		entity->rotation.z += speed * frame_duration_ms;
	}

	// Update entity scale
	if (kf->flags & EntityAnimKfFlags_ChangeScale)
	{
		//x
		speed = (kf->final_scale.x - kf->starting_scale.x) / kf->duration;
		entity->scale.x += speed * frame_duration_ms;
		//y
		speed = (kf->final_scale.y - kf->starting_scale.y) / kf->duration;
		entity->scale.y += speed * frame_duration_ms;
		//z
		speed = (kf->final_scale.z - kf->starting_scale.z) / kf->duration;
		entity->scale.z += speed * frame_duration_ms;
	}

	entity->Update();

	// updates keyframe if necessary
	if (keyframe_runtime >= kf->duration)
	{
		current_keyframe++;
		if (current_keyframe > keyframes_count)
		{
			active = false;
		}
	}
}


size_t EntityAnimationBuffer::FindSlot()
{
	For(animation_buffer_array_size)
	{
		if (!animations[i].active)
		{
			// reset slot
			animations[i] = EntityAnimation();
			return i;
		}
	}

	Quit_fatal("EntityAnimationBuffer overflow. Too many animations.");
	return 0;
}

void EntityAnimationBuffer::StartAnimation(Entity* entity, EntityAnimation* animation)
{
	// makes a copy of the animation to the Entity_Animations buffer

	auto i = FindSlot();
	auto _animation = &animations[i];

	*_animation = *animation;
	_animation->active = true;
	_animation->entity = entity;
}

void EntityAnimationBuffer::UpdateAnimations()
{
	For(EntityAnimations.animation_buffer_array_size)
	{
		auto anim = &EntityAnimations.animations[i];

		if (anim->active)
			anim->Update();
	}
}



void AN_CreateHardcodedAnimations()
{
	// >> VERTICAL DOOR 
	{
		// > SLIDING UP
		{
			auto kf = EntityAnimationKeyframe();
			kf.duration = 2000;
			kf.starting_scale = vec3{0.24, 1.6, 2.350};
			kf.final_scale = kf.starting_scale;
			kf.final_scale.z = 0.2;
			kf.flags |= EntityAnimKfFlags_ChangeScale;

			auto anim = EntityAnimation();
			anim.description = "vertical_door_slide_up";
			anim.keyframes_count = 1;
			anim.keyframes[0] = kf;

			AnimationCatalogue.insert({1, anim});
		}

		// > SLIDING DOWN
		{
			auto kf = EntityAnimationKeyframe();
			kf.duration = 2000;
			kf.starting_scale = vec3{0.24, 1.6, 0.2};
			kf.final_scale = kf.starting_scale;
			kf.final_scale.z = 2.350;
			kf.flags |= EntityAnimKfFlags_ChangeScale;

			auto anim = EntityAnimation();
			anim.description = "vertical_door_slide_down";
			anim.keyframes_count = 1;
			anim.keyframes[0] = kf;

			AnimationCatalogue.insert({2, anim});
		}
	}
}
