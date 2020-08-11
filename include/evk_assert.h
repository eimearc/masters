#ifndef EVK_ASSERT_H_
#define EVK_ASSERT_H_

#include <iostream>
#define EVK_ABORT(msg) \
    {std::cerr << "FATAL ERROR: " << msg; std::abort();}

// Used for assert with message at failure.
#ifndef EVK_NDEBUG

    #define _ASSERTM(exp, msg) \
        {if(!(exp)) \
            {std::cerr << "Assertion failed: line " << __LINE__ \
                << " of file " << __FILE__ << " -- " << msg;std::abort();}}
    #define _EXPECTM(msg) \
        {std::cerr << "Warning: Expect true failed: line " << __LINE__ \
                << " of file " << __FILE__ << " -- " << msg;}
    #define EVK_ASSERT(x,msg) \
        {_ASSERTM((x==VK_SUCCESS),msg);}
    #define EVK_ASSERT_TRUE(x,msg) \
        {_ASSERTM(x,msg);}
    #define EVK_EXPECT_TRUE(x,msg) \
        {if(!(x)) _EXPECTM(msg);}
    #define EVK_ASSERT_IMAGE_VALID(x,msg) \
        {_ASSERTM( \
            ((x==VK_ERROR_OUT_OF_DATE_KHR) || \
            (x==VK_SUBOPTIMAL_KHR) || \
            (x==VK_SUCCESS)),msg);}
    #define EVK_EXPECT_PRESENT_VALID(x,msg) \
        {if( \
            x!=VK_ERROR_OUT_OF_DATE_KHR && \
            x!=VK_SUBOPTIMAL_KHR && \
            x!=VK_SUCCESS) \
            _EXPECTM(msg);}

#else

    #define EVK_ASSERT(x,msg)
    #define EVK_ASSERT_TRUE(x,msg)
    #define EVK_EXPECT_TRUE(x,msg)
    #define EVK_ASSERT_IMAGE_VALID(x,msg)
    #define EVK_EXPECT_PRESENT_VALID(x,msg)
    #define EVK_ASSERT_PRESENT_VALID(x,msg)

#endif

#endif