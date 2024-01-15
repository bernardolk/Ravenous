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
	uint Duration; // expressed in milliseconds
	vec3 FinalPosition;
	vec3 FinalRotation;
	vec3 FinalScale;
	vec3 StartingPosition;
	vec3 StartingRotation;
	vec3 StartingScale;

	uint Flags;
};

struct REntityAnimation
{
	string Description = "";
	bool Active = false;
	//@entityptr
	EEntity* Entity = nullptr;
	uint KeyframesCount = 0;
	REntityAnimationKeyframe Keyframes[AnMaxEntityAnimationKeyframes];

	float Runtime = 0; // expressed in milliseconds
	uint CurrentKeyframe = 0;
	float KeyframeRuntime = 0;

	void Update();
};

struct REntityAnimationBuffer
{
	constexpr static uint AnimationBufferArraySize = 16;
	REntityAnimation Animations[AnimationBufferArraySize];

	uint FindSlot();

	void StartAnimation(EEntity* Entity, REntityAnimation* Animation);

	static void UpdateAnimations();

};

inline REntityAnimationBuffer EntityAnimations{};

void AnCreateHardcodedAnimations();
