// macros
#pragma once

#define For(x) for (int i = 0; i < x; i ++)
#define Forj(x) for (int j = 0; j < x; j ++)
#define ForLess(x) for (int i = 0; i < x; i ++)
#define ForLessEqual(x) for (int i = 0; i <+ x; i ++)
#define ForIt(x) for (auto it = x.begin(); it != x.end(); it++)

/**
 *  Static Helpers are methods that execute at static initialization time
 *  and are used by low level systems for automating certain code processes.
 *  It can also be used in properties when we such systems need data that is specific
 *  per type and that gets initialized when the type is first initialized (such as for
 *  reflection, using macros in conjunction)
 */
#define STATIC_HELPER static

#define StaticHelperByte static inline char

// #define PRINT(x) std::cout << x << "\n";

#define DEPRECATED_BEGIN ;
#define DEPRECATED_END ;
#define DEPRECATED_BLOCK ;
#define DEPRECATED_LINE ;

#define TO_DEPRECATE_BEGIN
#define TO_DEPRECATE_END

#define check(x, msg) assert(x)

#define fatal_error(...) { printf(__VA_ARGS__); printf("\n"); assert(false); }

#define print(...) { printf(__VA_ARGS__); printf("\n"); }

// #define TEXT1(x)								x
// #define TEXT2(x, x2)							x << x2
// #define TEXT3(x, x2, x3)						x << x2 << x3
// #define TEXT4(x, x2, x3, x4)					x << x2 << x3 << x4
// #define TEXT5(x, x2, x3, x4, x5)				x << x2 << x3 << x4 << x5
// #define TEXT6(x, x2, x3, x4, x5, x6)			x << x2 << x3 << x4 << x5 << x6
// #define TEXT(...) GET_MACRO_6(__VA_ARGS__, TEXT6, TEXT5, TEXT4, TEXT3, TEXT2, TEXT1)(__VA_ARGS__)

#define GET_MACRO_6(_1,_2,_3,_4, _5, _6, NAME, ...) NAME
#define GET_MACRO_4(_1,_2,_3,_4, NAME, ...) NAME



#define DeclSingleton(Type) \
	public:	\
		static Type* Get() \
		{ \
			static Type instance; \
			return &instance; \
		} \
	private: \
		Type(){}; \
	public: