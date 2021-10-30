/* helper for different display CVFD implementations */
#if HAVE_COOL_HARDWARE || HAVE_DUCKBOX_HARDWARE
#include <driver/vfd.h>
#endif
#if HAVE_GENERIC_HARDWARE  || HAVE_ARM_HARDWARE || HAVE_MIPS_HARDWARE
#include <driver/simple_display.h>
#endif
#ifdef ENABLE_GRAPHLCD
#include <driver/glcd/glcd.h>
#endif
