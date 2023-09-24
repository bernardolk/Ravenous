// Catalogue
#pragma once

#include "engine/core/core.h"

struct REntityAnimation;
inline map<uint, REntityAnimation> AnimationCatalogue;

constexpr static uint AnMaxEntityAnimationKeyframes = 16;

enum NEntityAnimationKeyframeFlags
{
	EntityAnimKfFlags_ChangePosition = 1 << 0,
	EntityAnimKfFlags_ChangeRotation = 1 << 1,
	EntityAnimKfFlags_ChangeScale    = 1 << 2,
};

struct REntityAnimationKeyframe
{
	uint duration; // expressed in milliseconds
	vec3 final_position;
	vec3 final_rotation;
	vec3 final_scale;
	vec3 starting_position;
	vec3 starting_rotation;
	vec3 starting_scale;

	uint flags;
};

struct REntityAnimation
{
	std::string description = "";
	bool active = false;
	EEntity* entity = nullptr;
	uint keyframes_count = 0;
	REntityAnimationKeyframe keyframes[AnMaxEntityAnimationKeyframes];

	float runtime = 0; // expressed in milliseconds
	uint current_keyframe = 0;
	float keyframe_runtime = 0;

	void Update();
};

struct REntityAnimationBuffer
{
	constexpr static uint animation_buffer_array_size = 16;
	REntityAnimation animations[animation_buffer_array_size];

	uint FindSlot();

	void StartAnimation(EEntity* entity, REntityAnimation* animation);

	static void UpdateAnimations();

};

inline REntityAnimationBuffer EntityAnimations{};

void AN_CreateHardcodedAnimations();
