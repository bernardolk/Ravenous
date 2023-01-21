#pragma once

#include "setup.hpp"

#if GLM_COMPILER == GLM_COMPILER_VC12
#	pragma warning(push)
#	pragma warning(disable: 4512) // assignment operator could not be generated
#endif

namespace glm
{
	namespace detail
	{
		template<typename T>
		union float_t
		{};

		// https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
		template<>
		union float_t<float>
		{
			using int_type = int;
			using float_type = float;

			GLM_CONSTEXPR float_t(float_type Num = 0.0f) : f(Num) {}

			GLM_CONSTEXPR float_t & operator=(const float_t& x)
			{
				f = x.f;
				return *this;
			}

			// Portable extraction of components.
			GLM_CONSTEXPR bool     negative() const { return i < 0; }
			GLM_CONSTEXPR int_type mantissa() const { return i & ((1 << 23) - 1); }
			GLM_CONSTEXPR int_type exponent() const { return (i >> 23) & ((1 << 8) - 1); }

			int_type   i;
			float_type f;
		};

		template<>
		union float_t<double>
		{
			using int_type = int64;
			using float_type = double;

			GLM_CONSTEXPR float_t(float_type Num = 0) : f(Num) {}

			GLM_CONSTEXPR float_t & operator=(const float_t& x)
			{
				f = x.f;
				return *this;
			}

			// Portable extraction of components.
			GLM_CONSTEXPR bool     negative() const { return i < 0; }
			GLM_CONSTEXPR int_type mantissa() const { return i & ((static_cast<int_type>(1) << 52) - 1); }
			GLM_CONSTEXPR int_type exponent() const { return (i >> 52) & ((static_cast<int_type>(1) << 11) - 1); }

			int_type   i;
			float_type f;
		};
	}//namespace detail
}//namespace glm

#if GLM_COMPILER == GLM_COMPILER_VC12
#	pragma warning(pop)
#endif
