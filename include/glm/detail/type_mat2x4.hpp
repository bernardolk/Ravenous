/// @ref core
/// @file glm/detail/type_mat2x4.hpp

#pragma once

#include "type_vec2.hpp"
#include "type_vec4.hpp"
#include <limits>
#include <cstddef>

namespace glm
{
	template<typename T, qualifier Q>
	struct mat<2, 4, T, Q>
	{
		using col_type = vec<4, T, Q>;
		using row_type = vec<2, T, Q>;
		using type = mat<2, 4, T, Q>;
		using transpose_type = mat<4, 2, T, Q>;
		using value_type = T;

	private:
		col_type value[2];

	public:
		// -- Accesses --

		using length_type = length_t;
		GLM_FUNC_DECL static GLM_CONSTEXPR length_type length() { return 2; }

		GLM_FUNC_DECL col_type &                     operator[](length_type i);
		GLM_FUNC_DECL GLM_CONSTEXPR const col_type & operator[](length_type i) const;

		// -- Constructors --

		GLM_FUNC_DECL GLM_CONSTEXPR mat() GLM_DEFAULT;
		template<qualifier P>
		GLM_FUNC_DECL GLM_CONSTEXPR mat(const mat<2, 4, T, P>& m);

		GLM_FUNC_DECL explicit GLM_CONSTEXPR mat(T scalar);
		GLM_FUNC_DECL GLM_CONSTEXPR          mat(
			T x0, T y0, T z0, T w0,
			T x1, T y1, T z1, T w1);
		GLM_FUNC_DECL GLM_CONSTEXPR mat(
			const col_type& v0,
			const col_type& v1);

		// -- Conversions --

		template<
			typename X1, typename Y1, typename Z1, typename W1,
			typename X2, typename Y2, typename Z2, typename W2>
		GLM_FUNC_DECL GLM_CONSTEXPR mat(
			X1 x1, Y1 y1, Z1 z1, W1 w1,
			X2 x2, Y2 y2, Z2 z2, W2 w2);

		template<typename U, typename V>
		GLM_FUNC_DECL GLM_CONSTEXPR mat(
			const vec<4, U, Q>& v1,
			const vec<4, V, Q>& v2);

		// -- Matrix conversions --

		template<typename U, qualifier P>
		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<2, 4, U, P>& m);

		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<2, 2, T, Q>& x);
		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<3, 3, T, Q>& x);
		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<4, 4, T, Q>& x);
		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<2, 3, T, Q>& x);
		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<3, 2, T, Q>& x);
		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<3, 4, T, Q>& x);
		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<4, 2, T, Q>& x);
		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<4, 3, T, Q>& x);

		// -- Unary arithmetic operators --

		template<typename U>
		GLM_FUNC_DECL mat<2, 4, T, Q> & operator=(const mat<2, 4, U, Q>& m);
		template<typename U>
		GLM_FUNC_DECL mat<2, 4, T, Q> & operator+=(U s);
		template<typename U>
		GLM_FUNC_DECL mat<2, 4, T, Q> & operator+=(const mat<2, 4, U, Q>& m);
		template<typename U>
		GLM_FUNC_DECL mat<2, 4, T, Q> & operator-=(U s);
		template<typename U>
		GLM_FUNC_DECL mat<2, 4, T, Q> & operator-=(const mat<2, 4, U, Q>& m);
		template<typename U>
		GLM_FUNC_DECL mat<2, 4, T, Q> & operator*=(U s);
		template<typename U>
		GLM_FUNC_DECL mat<2, 4, T, Q> & operator/=(U s);

		// -- Increment and decrement operators --

		GLM_FUNC_DECL mat<2, 4, T, Q> & operator++();
		GLM_FUNC_DECL mat<2, 4, T, Q> & operator--();
		GLM_FUNC_DECL mat<2, 4, T, Q>   operator++(int);
		GLM_FUNC_DECL mat<2, 4, T, Q>   operator--(int);
	};

	// -- Unary operators --

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 4, T, Q> operator+(const mat<2, 4, T, Q>& m);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 4, T, Q> operator-(const mat<2, 4, T, Q>& m);

	// -- Binary operators --

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 4, T, Q> operator+(const mat<2, 4, T, Q>& m, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 4, T, Q> operator+(const mat<2, 4, T, Q>& m1, const mat<2, 4, T, Q>& m2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 4, T, Q> operator-(const mat<2, 4, T, Q>& m, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 4, T, Q> operator-(const mat<2, 4, T, Q>& m1, const mat<2, 4, T, Q>& m2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 4, T, Q> operator*(const mat<2, 4, T, Q>& m, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 4, T, Q> operator*(T scalar, const mat<2, 4, T, Q>& m);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL typename mat<2, 4, T, Q>::col_type operator*(const mat<2, 4, T, Q>& m, const typename mat<2, 4, T, Q>::row_type& v);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL typename mat<2, 4, T, Q>::row_type operator*(const typename mat<2, 4, T, Q>::col_type& v, const mat<2, 4, T, Q>& m);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<4, 4, T, Q> operator*(const mat<2, 4, T, Q>& m1, const mat<4, 2, T, Q>& m2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 4, T, Q> operator*(const mat<2, 4, T, Q>& m1, const mat<2, 2, T, Q>& m2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<3, 4, T, Q> operator*(const mat<2, 4, T, Q>& m1, const mat<3, 2, T, Q>& m2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 4, T, Q> operator/(const mat<2, 4, T, Q>& m, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 4, T, Q> operator/(T scalar, const mat<2, 4, T, Q>& m);

	// -- Boolean operators --

	template<typename T, qualifier Q>
	GLM_FUNC_DECL bool operator==(const mat<2, 4, T, Q>& m1, const mat<2, 4, T, Q>& m2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL bool operator!=(const mat<2, 4, T, Q>& m1, const mat<2, 4, T, Q>& m2);
}//namespace glm

#ifndef GLM_EXTERNAL_TEMPLATE
#include "type_mat2x4.inl"
#endif
