#ifndef RS_CORE_HPP
#define RS_CORE_HPP

#include <inttypes.h>
#include <stddef.h>

#include <sys/types.h>
// #include <sys/uio.h>
#include <assert.h>

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_REVISION 0

#define RS_XSTR(v) INTERNAL_STR(v)
#define INTRENAL_STR(v) #v

#define SIG_RS_KEY "RS"
#define SIG_RS_CODE "ppking"

#define SIG_RS_VERSION RS_XSTR(VERSION_MAJOR) "." RS_XSTR(VERSION_MINOR) "." RS_XSTR(VERSION_REVISION)
#define SIG_RS_SERVER SIG_RS_KEY "/" SIG_RS_VERSION "(" SIG_RS_CODE ")"

#endif
