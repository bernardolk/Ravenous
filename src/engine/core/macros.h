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

#define PRINT(x) std::cout << (x) << "\n";

#define DEPRECATED_BEGIN ;
#define DEPRECATED_END ;
#define DEPRECATED_BLOCK ;
#define DEPRECATED_LINE ;

#define TO_DEPRECATE_BEGIN
#define TO_DEPRECATE_END

#define check(x, msg) assert(x)