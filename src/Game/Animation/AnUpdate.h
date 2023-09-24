// Catalogue
#pragma once

#include "engine/core/core.h"

struct EntityAnimation;
inline map<u32, EntityAnimation> AnimationCatalogue;

constexpr static u32 AnMaxEntityAnimationKeyframes = 16;

enum EntityAnimationKeyframeFlags
{
	EntityAnimKfFlags_ChangePosition = 1 << 0,
	EntityAnimKfFlags_ChangeRotation = 1 << 1,
	EntityAnimKfFlags_ChangeScale    = 1 << 2,
};

struct EntityAnimationKeyframe
{
	u32 duration; // expressed in milliseconds
	vec3 final_position;
	vec3 final_rotation;
	vec3 final_scale;
	vec3 starting_position;
	vec3 starting_rotation;
	vec3 starting_scale;

	u32 flags;
};

struct EntityAnimation
{
	std::string description = "";
	bool active = false;
	EEntity* entity = nullptr;
	u32 keyframes_count = 0;
	EntityAnimationKeyframe keyframes[AnMaxEntityAnimationKeyframes];

	float runtime = 0; // expressed in milliseconds
	u32 current_keyframe = 0;
	float keyframe_runtime = 0;

	void Update();
};

struct EntityAnimationBuffer
{
	constexpr static u32 animation_buffer_array_size = 16;
	EntityAnimation animations[animation_buffer_array_size];

	u32 FindSlot();

	void StartAnimation(EEntity* entity, EntityAnimation* animation);

	static void UpdateAnimations();

};

inline EntityAnimationBuffer EntityAnimations{};

void AN_CreateHardcodedAnimations();
