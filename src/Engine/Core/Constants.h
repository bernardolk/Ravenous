#pragma once

constexpr static float PI = 3.141592;

const static glm::mat4 Mat4Identity(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);

inline constexpr float VecComparePrecision = FLT_EPSILON;
inline constexpr float MaxFloat = FLT_MAX; // 3.40282347e+38F
// inline constexpr float MinFloat = FLT_MIN;
inline constexpr float MinFloat = -FLT_MAX;
inline constexpr float Epsilon = FLT_EPSILON;        //1.19209290e-7F
inline constexpr double EpsilonDouble = DBL_EPSILON; //2.2204460492503131e-16
inline constexpr double MaxDouble = DBL_MAX;
// inline constexpr double MinDouble = DBL_MIN; // 2.225073858507201e-308
inline constexpr double MinDouble = -DBL_MAX;
