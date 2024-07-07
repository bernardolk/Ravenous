#include "CollisionTest.h"

#include "Engine/Geometry/Cylinder.h"
#include "engine/utils/utils.h"
#include "primitives/BoundingBox.h"

RCollisionResults TestCollisionBoxAgainstCylinder(const RBoundingBox& Box, const RCylinder& Cylinder)
{
	RCollisionResults Results;
    // Calculate closest point on AABB to cylinder's center
    float ClosestX = Max(Box.MinX, Min(Cylinder.Position.x, Box.MaxX));
    float ClosestY = Max(Box.MinY, Min(Cylinder.Position.y, Box.MaxY));
    float ClosestZ = Max(Box.MinZ, Min(Cylinder.Position.z, Box.MaxZ));

    // Calculate distance between closest point and cylinder's center
    float DistanceX = ClosestX - Cylinder.Position.x;
    float DistanceY = ClosestY - Cylinder.Position.y;
    float DistanceZ = ClosestZ - Cylinder.Position.z;

    // Check if the distance is less than the cylinder's radius
    float DistanceSquared = DistanceX * DistanceX + DistanceY * DistanceY + DistanceZ * DistanceZ;
    float RadiusSquared = Cylinder.Radius * Cylinder.Radius;

    if (DistanceSquared <= RadiusSquared) {
        // Check if closest point is within the height of the cylinder
        float MinHeight = Cylinder.Position.y - Cylinder.Height / 2.0f;
        float MaxHeight = Cylinder.Position.y + Cylinder.Height / 2.0f;
        if (ClosestY >= MinHeight && ClosestY <= MaxHeight) {
        	Results.Collision = true;
        }
    }

    return Results;
}
