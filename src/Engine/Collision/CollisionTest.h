#pragma once
#include "Engine/Core/Core.h"
#include "ClTypes.h"

struct RCylinder;
struct RBoundingBox;

RCollisionResults TestCollisionBoxAgainstCylinder(const RBoundingBox& Box, const RCylinder& Cylinder);
