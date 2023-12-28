// macros
#pragma once

// #define PRINT(x) std::cout << x << "\n";

#define DEPRECATED_BEGIN ;
#define DEPRECATED_END ;
#define DEPRECATED_BLOCK ;
#define DEPRECATED_LINE ;

#define TO_DEPRECATE_BEGIN
#define TO_DEPRECATE_END

#define DEBUG_BREAK __asm__ volatile("int $0x03");
#define FatalError(...) { printf(__VA_ARGS__); printf("\n"); assert(false); }
#define Assert(Expression, Message) { if (Expression){ printf(Message); printf("\n"); assert(false); } }
#define Break(...) { printf(__VA_ARGS__); printf("\n"); DEBUG_BREAK }
#define Log(...) { printf(__VA_ARGS__); printf("\n"); }

// #define TEXT1(x)								x
// #define TEXT2(x, x2)							x << x2
// #define TEXT3(x, x2, x3)						x << x2 << x3
// #define TEXT4(x, x2, x3, x4)					x << x2 << x3 << x4
// #define TEXT5(x, x2, x3, x4, x5)				x << x2 << x3 << x4 << x5
// #define TEXT6(x, x2, x3, x4, x5, x6)			x << x2 << x3 << x4 << x5 << x6
// #define TEXT(...) GET_MACRO_6(__VA_ARGS__, TEXT6, TEXT5, TEXT4, TEXT3, TEXT2, TEXT1)(__VA_ARGS__)

#define GET_MACRO_6(_1,_2,_3,_4, _5, _6, NAME, ...) NAME
#define GET_MACRO_4(_1,_2,_3,_4, NAME, ...) NAME
