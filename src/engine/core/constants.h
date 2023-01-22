#pragma once

#include "types.h"

constexpr static float PI = 3.141592;

const static glm::mat4 Mat4Identity(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);

inline const float VecComparePrecision = 0.00001f;
inline const float MaxFloat = std::numeric_limits<float>::max();
inline const float MinFloat = -MaxFloat;