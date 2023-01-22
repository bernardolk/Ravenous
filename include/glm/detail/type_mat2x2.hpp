/// @ref core
/// @file glm/detail/type_mat2x2.hpp

#pragma once

#include "type_vec2.hpp"
#include <limits>
#include <cstddef>

namespace glm
{
	template<typename T, qualifier Q>
	struct mat<2, 2, T, Q>
	{
		using col_type = vec<2, T, Q>;
		using row_type = vec<2, T, Q>;
		using type = mat<2, 2, T, Q>;
		using transpose_type = mat<2, 2, T, Q>;
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
		GLM_FUNC_DECL GLM_CONSTEXPR mat(const mat<2, 2, T, P>& m);

		GLM_FUNC_DECL explicit GLM_CONSTEXPR mat(T scalar);
		GLM_FUNC_DECL GLM_CONSTEXPR          mat(
			const T& x1, const T& y1,
			const T& x2, const T& y2);
		GLM_FUNC_DECL GLM_CONSTEXPR mat(
			const col_type& v1,
			const col_type& v2);

		// -- Conversions --

		template<typename U, typename V, typename M, typename N>
		GLM_FUNC_DECL GLM_CONSTEXPR mat(
			const U& x1, const V& y1,
			const M& x2, const N& y2);

		template<typename U, typename V>
		GLM_FUNC_DECL GLM_CONSTEXPR mat(
			const vec<2, U, Q>& v1,
			const vec<2, V, Q>& v2);

		// -- Matrix conversions --

		template<typename U, qualifier P>
		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<2, 2, U, P>& m);

		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<3, 3, T, Q>& x);
		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<4, 4, T, Q>& x);
		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<2, 3, T, Q>& x);
		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<3, 2, T, Q>& x);
		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<2, 4, T, Q>& x);
		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<4, 2, T, Q>& x);
		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<3, 4, T, Q>& x);
		GLM_FUNC_DECL GLM_EXPLICIT GLM_CONSTEXPR mat(const mat<4, 3, T, Q>& x);

		// -- Unary arithmetic operators --

		template<typename U>
		GLM_FUNC_DECL mat<2, 2, T, Q> & operator=(const mat<2, 2, U, Q>& m);
		template<typename U>
		GLM_FUNC_DECL mat<2, 2, T, Q> & operator+=(U s);
		template<typename U>
		GLM_FUNC_DECL mat<2, 2, T, Q> & operator+=(const mat<2, 2, U, Q>& m);
		template<typename U>
		GLM_FUNC_DECL mat<2, 2, T, Q> & operator-=(U s);
		template<typename U>
		GLM_FUNC_DECL mat<2, 2, T, Q> & operator-=(const mat<2, 2, U, Q>& m);
		template<typename U>
		GLM_FUNC_DECL mat<2, 2, T, Q> & operator*=(U s);
		template<typename U>
		GLM_FUNC_DECL mat<2, 2, T, Q> & operator*=(const mat<2, 2, U, Q>& m);
		template<typename U>
		GLM_FUNC_DECL mat<2, 2, T, Q> & operator/=(U s);
		template<typename U>
		GLM_FUNC_DECL mat<2, 2, T, Q> & operator/=(const mat<2, 2, U, Q>& m);

		// -- Increment and decrement operators --

		GLM_FUNC_DECL mat<2, 2, T, Q> & operator++();
		GLM_FUNC_DECL mat<2, 2, T, Q> & operator--();
		GLM_FUNC_DECL mat<2, 2, T, Q>   operator++(int);
		GLM_FUNC_DECL mat<2, 2, T, Q>   operator--(int);
	};

	// -- Unary operators --

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 2, T, Q> operator+(const mat<2, 2, T, Q>& m);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 2, T, Q> operator-(const mat<2, 2, T, Q>& m);

	// -- Binary operators --

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 2, T, Q> operator+(const mat<2, 2, T, Q>& m, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 2, T, Q> operator+(T scalar, const mat<2, 2, T, Q>& m);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 2, T, Q> operator+(const mat<2, 2, T, Q>& m1, const mat<2, 2, T, Q>& m2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 2, T, Q> operator-(const mat<2, 2, T, Q>& m, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 2, T, Q> operator-(T scalar, const mat<2, 2, T, Q>& m);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 2, T, Q> operator-(const mat<2, 2, T, Q>& m1, const mat<2, 2, T, Q>& m2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 2, T, Q> operator*(const mat<2, 2, T, Q>& m, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 2, T, Q> operator*(T scalar, const mat<2, 2, T, Q>& m);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL typename mat<2, 2, T, Q>::col_type operator*(const mat<2, 2, T, Q>& m, const typename mat<2, 2, T, Q>::row_type& v);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL typename mat<2, 2, T, Q>::row_type operator*(const typename mat<2, 2, T, Q>::col_type& v, const mat<2, 2, T, Q>& m);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 2, T, Q> operator*(const mat<2, 2, T, Q>& m1, const mat<2, 2, T, Q>& m2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<3, 2, T, Q> operator*(const mat<2, 2, T, Q>& m1, const mat<3, 2, T, Q>& m2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<4, 2, T, Q> operator*(const mat<2, 2, T, Q>& m1, const mat<4, 2, T, Q>& m2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 2, T, Q> operator/(const mat<2, 2, T, Q>& m, T scalar);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 2, T, Q> operator/(T scalar, const mat<2, 2, T, Q>& m);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL typename mat<2, 2, T, Q>::col_type operator/(const mat<2, 2, T, Q>& m, const typename mat<2, 2, T, Q>::row_type& v);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL typename mat<2, 2, T, Q>::row_type operator/(const typename mat<2, 2, T, Q>::col_type& v, const mat<2, 2, T, Q>& m);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL mat<2, 2, T, Q> operator/(const mat<2, 2, T, Q>& m1, const mat<2, 2, T, Q>& m2);

	// -- Boolean operators --

	template<typename T, qualifier Q>
	GLM_FUNC_DECL bool operator==(const mat<2, 2, T, Q>& m1, const mat<2, 2, T, Q>& m2);

	template<typename T, qualifier Q>
	GLM_FUNC_DECL bool operator!=(const mat<2, 2, T, Q>& m1, const mat<2, 2, T, Q>& m2);
} //namespace glm

#ifndef GLM_EXTERNAL_TEMPLATE
#include "type_mat2x2.inl"
#endif
