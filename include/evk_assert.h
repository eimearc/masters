#ifndef EVK_ASSERT_H_
#define EVK_ASSERT_H_

// Used for assert with message at failure.
// #define assertm(exp, msg) assert(((void)msg, exp))

#ifndef EVK_NDEBUG

    #include <iostream>
    #define evk_assert(x) \
        if(!(x)) {assert(x);}

#else

    #define evk_assert(x)

#endif

#endif