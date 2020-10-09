#ifndef RS_UTILS_HPP
#define RS_UTILS_HPP

#include <common/core.hpp>
#include <inttypes.h>
#include <stddef.h>

#include <sys/types.h>
// #include <sys/uio.h>
#include <assert.h>
#include <string>

#define rs_assert(expression) assert(expression)

#define rs_freep(p) \
    if (p)          \
    {               \
        delete p;   \
        p = nullptr;\
    }               \
    (void)0         \

#define rs_freepa(pa) \
    if (pa)         \
    {                 \
        delete pa;    \
        pa = nullptr; \
    }                 \
    (void)0           \

#define rs_min(a, b) (((a) < (b)) ? (a) : (b))
#define rs_max(a, b) (((a) < (b)) ? (b) : (a))

class Utils
{
public:
    static std::string GetPeerIp(int32_t fd);
    static void RandomGenerate(char *bytes, int32_t size);
};

#endif
