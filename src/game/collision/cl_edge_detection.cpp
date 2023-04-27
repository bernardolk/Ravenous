#include "cl_edge_detection.h"
#include "game/entities/player.h"
#include "engine/utils/utils.h"
#include "engine/collision/raycast.h"
#include "engine/collision/primitives/ray.h"
#include "engine/render/im_render.h"
#include "engine/world/world.h"


Ledge CL_PerformLedgeDetection(Player* player, T_World* world)
{
	// concepts: front face - where the horizontal rays are going to hit
	//           top face - where the vertical ray (up towards down) is going to hit
	Ledge ledge;

	// settings
	constexpr float _front_ray_first_ray_delta_y = 0.6f;
	constexpr float _front_ray_spacing = 0.03f;
	constexpr int _front_ray_qty = 24;

	auto orientation_xz = ToXz(player->orientation);
	auto first_ray = Ray{player->GetEyePosition() - UnitY * _front_ray_first_ray_delta_y, orientation_xz};
	ledge.detection_direction = first_ray.direction;

	auto front_test = world->LinearRaycastArray(first_ray, _front_ray_qty, _front_ray_spacing);
	if (front_test.hit)
	{
		vec3 frontal_hitpoint = CL_GetPointFromDetection(front_test.ray, front_test);
		vec3 front_face_n = front_test.t.GetNormal();

		if (dot(UnitY, front_face_n) > 0.0001f)
			return ledge;

		constexpr float _top_ray_height = 2.0f;
		auto top_ray = Ray{frontal_hitpoint + front_test.ray.direction * 0.0001f + UnitY * _top_ray_height, -UnitY};

		auto top_test = world->Raycast(top_ray, RayCast_TestOnlyFromOutsideIn, nullptr, _top_ray_height);

		if (top_test.hit)
		{
			vec3 top_hitpoint = CL_GetPointFromDetection(top_test.ray, top_test);
			ledge.surface_point = top_hitpoint;

			if (top_test.distance <= player->height || top_hitpoint.y - frontal_hitpoint.y > _front_ray_spacing)
				return ledge;

			ImDraw::AddLine(IMHASH, top_ray.origin, frontal_hitpoint, 1.2f, false, COLOR_PURPLE_1);
			ImDraw::AddPoint(IMHASH, top_hitpoint, 2.0, true, COLOR_PURPLE_1);

			// test edges
			vec3 edge1 = top_test.t.b - top_test.t.a; // 1
			vec3 edge2 = top_test.t.c - top_test.t.b; // 2
			vec3 edge3 = top_test.t.a - top_test.t.c; // 3

			// for debug: show face normal
			vec3 front_face_center = front_test.t.GetBarycenter();
			ImDraw::AddLine(IMHASH, front_face_center, front_face_center + 1.f * front_face_n, 2.0, false, COLOR_BLUE_1);

			if (abs(dot(edge1, front_face_n)) < 0.0001f)
			{
				ImDraw::AddLine(IMHASH, top_test.t.a, top_test.t.a + edge1, 2.0, true, COLOR_YELLOW_1);
				ImDraw::AddPoint(IMHASH, top_test.t.a, 2.0, false, COLOR_YELLOW_1);
				ImDraw::AddPoint(IMHASH, top_test.t.b, 2.0, false, COLOR_YELLOW_1);

				ledge.a = top_test.t.a;
				ledge.b = top_test.t.b;

				ledge.empty = false;
				return ledge;
			}
			if (abs(dot(edge2, front_face_n)) < 0.0001f)
			{
				ImDraw::AddLine(IMHASH, top_test.t.b, top_test.t.b + edge2, 2.0, true, COLOR_YELLOW_1);
				ImDraw::AddPoint(IMHASH, top_test.t.b, 2.0, false, COLOR_YELLOW_1);
				ImDraw::AddPoint(IMHASH, top_test.t.c, 2.0, false, COLOR_YELLOW_1);

				ledge.a = top_test.t.b;
				ledge.b = top_test.t.c;

				ledge.empty = false;
				return ledge;
			}
			if (abs(dot(edge3, front_face_n)) < 0.0001f)
			{
				ImDraw::AddLine(IMHASH, top_test.t.c, top_test.t.c + edge3, 2.0, true, COLOR_YELLOW_1);
				ImDraw::AddPoint(IMHASH, top_test.t.c, 2.0, false, COLOR_YELLOW_1);
				ImDraw::AddPoint(IMHASH, top_test.t.a, 2.0, false, COLOR_YELLOW_1);

				ledge.a = top_test.t.c;
				ledge.b = top_test.t.a;

				ledge.empty = false;
				return ledge;
			}
		}
	}

	ledge.empty = true;
	return ledge;
}


vec3 CL_GetFinalPositionLedgeVaulting(Player* player, Ledge ledge)
{
	/* Returns the player's position after finishing vaulting across the given ledge */
	vec3 inward_normal = normalize(Cross(ledge.a - ledge.b, UnitY));
	return ledge.surface_point + inward_normal * player->radius * 2.f;
}
