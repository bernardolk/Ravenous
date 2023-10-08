#include "game/animation/AnUpdate.h"

#include "engine/core/logging.h"
#include "engine/rvn.h"
#include "engine/entities/Entity.h"

void REntityAnimation::Update()
{
	/* executes current keyframe in entity and updates runtimes, turns animation inactive once it ends. */
	auto& Frame = RavenousEngine::GetFrame();

	float FrameDurationMs = Frame.Duration * 1000;

	auto Kf = &Keyframes[CurrentKeyframe];

	KeyframeRuntime += FrameDurationMs;
	Runtime += FrameDurationMs;

	// --------------------
	// > Perform animation
	// --------------------

	float Speed;
	// Update entity position
	if (Kf->Flags & EntityAnimKfFlags_ChangePosition)
	{
		//x
		Speed = (Kf->FinalPosition.x - Kf->StartingPosition.x) / Kf->Duration;
		Entity->Position.x += Speed * FrameDurationMs;
		//y
		Speed = (Kf->FinalPosition.y - Kf->StartingPosition.y) / Kf->Duration;
		Entity->Position.y += Speed * FrameDurationMs;
		//z
		Speed = (Kf->FinalPosition.z - Kf->StartingPosition.z) / Kf->Duration;
		Entity->Position.z += Speed * FrameDurationMs;
	}

	// Update entity rotation
	if (Kf->Flags & EntityAnimKfFlags_ChangeRotation)
	{
		//x
		Speed = (Kf->FinalRotation.x - Kf->StartingRotation.x) / Kf->Duration;
		Entity->Rotation.x += Speed * FrameDurationMs;
		//y
		Speed = (Kf->FinalRotation.y - Kf->StartingRotation.y) / Kf->Duration;
		Entity->Rotation.y += Speed * FrameDurationMs;
		//z
		Speed = (Kf->FinalRotation.z - Kf->StartingRotation.z) / Kf->Duration;
		Entity->Rotation.z += Speed * FrameDurationMs;
	}

	// Update entity scale
	if (Kf->Flags & EntityAnimKfFlags_ChangeScale)
	{
		//x
		Speed = (Kf->FinalScale.x - Kf->StartingScale.x) / Kf->Duration;
		Entity->Scale.x += Speed * FrameDurationMs;
		//y
		Speed = (Kf->FinalScale.y - Kf->StartingScale.y) / Kf->Duration;
		Entity->Scale.y += Speed * FrameDurationMs;
		//z
		Speed = (Kf->FinalScale.z - Kf->StartingScale.z) / Kf->Duration;
		Entity->Scale.z += Speed * FrameDurationMs;
	}

	Entity->Update();

	// updates keyframe if necessary
	if (KeyframeRuntime >= Kf->Duration)
	{
		CurrentKeyframe++;
		if (CurrentKeyframe > KeyframesCount)
		{
			Active = false;
		}
	}
}


uint REntityAnimationBuffer::FindSlot()
{
	for (int i = 0; i < AnimationBufferArraySize; i++)
	{
		if (!Animations[i].Active)
		{
			// reset slot
			Animations[i] = REntityAnimation();
			return i;
		}
	}

	fatal_error("EntityAnimationBuffer overflow. Too many animations.");
	return 0;
}

void REntityAnimationBuffer::StartAnimation(EEntity* Entity, REntityAnimation* InAnimation)
{
	// makes a copy of the animation to the Entity_Animations buffer

	auto i = FindSlot();
	auto* Animation = &Animations[i];

	*Animation = *InAnimation;
	Animation->Active = true;
	Animation->Entity = Entity;
}

void REntityAnimationBuffer::UpdateAnimations()
{
	for (int i = 0; i < EntityAnimations.AnimationBufferArraySize; i++)
	{
		auto Anim = &EntityAnimations.Animations[i];

		if (Anim->Active)
			Anim->Update();
	}
}



void AnCreateHardcodedAnimations()
{
	// >> VERTICAL DOOR 
	{
		// > SLIDING UP
		{
			auto Kf = REntityAnimationKeyframe();
			Kf.Duration = 2000;
			Kf.StartingScale = vec3{0.24, 1.6, 2.350};
			Kf.FinalScale = Kf.StartingScale;
			Kf.FinalScale.z = 0.2;
			Kf.Flags |= EntityAnimKfFlags_ChangeScale;

			auto Anim = REntityAnimation();
			Anim.Description = "vertical_door_slide_up";
			Anim.KeyframesCount = 1;
			Anim.Keyframes[0] = Kf;

			AnimationCatalogue.insert({1, Anim});
		}

		// > SLIDING DOWN
		{
			auto Kf = REntityAnimationKeyframe();
			Kf.Duration = 2000;
			Kf.StartingScale = vec3{0.24, 1.6, 0.2};
			Kf.FinalScale = Kf.StartingScale;
			Kf.FinalScale.z = 2.350;
			Kf.Flags |= EntityAnimKfFlags_ChangeScale;

			auto Anim = REntityAnimation();
			Anim.Description = "vertical_door_slide_down";
			Anim.KeyframesCount = 1;
			Anim.Keyframes[0] = Kf;

			AnimationCatalogue.insert({2, Anim});
		}
	}
}
