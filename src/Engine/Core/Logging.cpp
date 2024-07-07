#include "Logging.h"
#include "Macros.h"
#include "Engine/Geometry/Quad.h"
#include "engine/collision/primitives/ray.h"
#include "engine/utils/utils.h"


void QuickLog(RQuad& Quad)
{
	Log("Plane: Top Right (%f, %f, %f) - Top Right (%f, %f, %f)", Quad.T1.A.x, Quad.T1.A.y, Quad.T1.A.z, Quad.T2.C.x, Quad.T2.C.y, Quad.T2.C.z);
}

void QuickLog(RRay& Ray)
{
	Log("Ray: Origin (%f, %f, %f) - Direction (%f, %f, %f)", Ray.Origin.x, Ray.Origin.y, Ray.Origin.z, Ray.Direction.x, Ray.Direction.y, Ray.Direction.z);
}

