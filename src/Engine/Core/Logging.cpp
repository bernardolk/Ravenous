#include "Logging.h"
#include "Macros.h"
#include "Engine/Geometry/Quad.h"


void QuickLog(RQuad& Quad)
{
	Log("Plane: Top Right (%f, %f, %f) - Top Right (%f, %f, %f)", Quad.T1.A.x, Quad.T1.A.y, Quad.T1.A.z, Quad.T2.C.x, Quad.T2.C.y, Quad.T2.C.z);
}
