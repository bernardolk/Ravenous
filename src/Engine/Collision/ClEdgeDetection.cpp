#include "ClEdgeDetection.h"
#include "game/entities/EPlayer.h"
#include "engine/utils/utils.h"
#include "engine/collision/raycast.h"
#include "engine/collision/primitives/ray.h"
#include "engine/render/ImRender.h"
#include "engine/world/World.h"

RLedge ClPerformLedgeDetection(EPlayer* Player, RWorld* World)
{
	// concepts: front face - where the horizontal rays are going to hit
	//           top face - where the vertical ray (up towards down) is going to hit
	RLedge Ledge;

	// settings
	constexpr float FrontRayFirstRayDeltaY = 0.6f;
	constexpr float FrontRaySpacing = 0.03f;
	constexpr int FrontRayQty = 24;

	auto OrientationXz = ToXz(Player->Orientation);
	auto FirstRay = RRay{Player->GetEyePosition() - UnitY * FrontRayFirstRayDeltaY, OrientationXz};
	Ledge.DetectionDirection = FirstRay.Direction;

	
	if (auto FrontTest = World->LinearRaycastArray(FirstRay, FrontRayQty, FrontRaySpacing); FrontTest.Hit)
	{
		vec3 FrontalHitpoint = FrontTest.GetPoint();
		vec3 FrontFaceN = FrontTest.Triangle.GetNormal();

		if (dot(UnitY, FrontFaceN) > 0.0001f)
			return Ledge;

		constexpr float TopRayHeight = 2.0f;
		auto TopRay = RRay{FrontalHitpoint + FrontTest.Ray.Direction * 0.0001f + UnitY * TopRayHeight, -UnitY};

		auto TopTest = World->Raycast(TopRay, RayCast_TestOnlyFromOutsideIn, nullptr, TopRayHeight);

		if (TopTest.Hit)
		{
			vec3 TopHitpoint = TopTest.GetPoint();
			Ledge.SurfacePoint = TopHitpoint;

			if (TopTest.Distance <= Player->Height || TopHitpoint.y - FrontalHitpoint.y > FrontRaySpacing)
				return Ledge;

			RImDraw::AddLine(IMHASH, TopRay.Origin, FrontalHitpoint, 0, COLOR_PURPLE_1, 1.2f, false);
			RImDraw::AddPoint(IMHASH, TopHitpoint, 0, COLOR_PURPLE_1, 2.0, true);

			// test edges
			vec3 Edge1 = TopTest.Triangle.B - TopTest.Triangle.A; // 1
			vec3 Edge2 = TopTest.Triangle.C - TopTest.Triangle.B; // 2
			vec3 Edge3 = TopTest.Triangle.A - TopTest.Triangle.C; // 3

			// for debug: show face normal
			vec3 FrontFaceCenter = FrontTest.Triangle.GetBarycenter();
			RImDraw::AddLine(IMHASH, FrontFaceCenter, FrontFaceCenter + 1.f * FrontFaceN, 0.f, COLOR_BLUE_1, 2.0, false);

			if (abs(dot(Edge1, FrontFaceN)) < 0.0001f)
			{
				RImDraw::AddLine(IMHASH, TopTest.Triangle.A, TopTest.Triangle.A + Edge1, 0, COLOR_YELLOW_1, 2.0, true);
				RImDraw::AddPoint(IMHASH, TopTest.Triangle.A, 0, COLOR_YELLOW_1, 2.0, false);
				RImDraw::AddPoint(IMHASH, TopTest.Triangle.B, 0, COLOR_YELLOW_1, 2.0, false);

				Ledge.A = TopTest.Triangle.A;
				Ledge.B = TopTest.Triangle.B;

				Ledge.Empty = false;
				return Ledge;
			}
			if (abs(dot(Edge2, FrontFaceN)) < 0.0001f)
			{
				RImDraw::AddLine(IMHASH, TopTest.Triangle.B, TopTest.Triangle.B + Edge2, 0, COLOR_YELLOW_1, 2.0, true);
				RImDraw::AddPoint(IMHASH, TopTest.Triangle.B, 0,  COLOR_YELLOW_1, 2.0, false);
				RImDraw::AddPoint(IMHASH, TopTest.Triangle.C, 0,  COLOR_YELLOW_1, 2.0, false);

				Ledge.A = TopTest.Triangle.B;
				Ledge.B = TopTest.Triangle.C;

				Ledge.Empty = false;
				return Ledge;
			}
			if (abs(dot(Edge3, FrontFaceN)) < 0.0001f)
			{
				RImDraw::AddLine(IMHASH, TopTest.Triangle.C, TopTest.Triangle.C + Edge3, 0, COLOR_YELLOW_1, 2.0, true);
				RImDraw::AddPoint(IMHASH, TopTest.Triangle.C, 0, COLOR_YELLOW_1, 2.0, false);
				RImDraw::AddPoint(IMHASH, TopTest.Triangle.A, 0, COLOR_YELLOW_1, 2.0, false);

				Ledge.A = TopTest.Triangle.C;
				Ledge.B = TopTest.Triangle.A;

				Ledge.Empty = false;
				return Ledge;
			}
		}
	}

	Ledge.Empty = true;
	return Ledge;
}


vec3 ClGetFinalPositionLedgeVaulting(EPlayer* Player, RLedge Ledge)
{
	/* Returns the player's position after finishing vaulting across the given ledge */
	vec3 InwardNormal = normalize(Cross(Ledge.A - Ledge.B, UnitY));
	return Ledge.SurfacePoint + InwardNormal * Player->Radius * 2.f;
}
