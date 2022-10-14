/* helper for different display CVFD implementations */
#if HAVE_GENERIC_HARDWARE || HAVE_ARM_HARDWARE
#if BOXMODEL_OSMIO4KPLUS
#include <driver/lcdd.h>
#define CVFD CLCD
#else
#include <driver/simple_display.h>
#endif
#endif
#ifdef ENABLE_GRAPHLCD
#include <driver/glcd/glcd.h>
#endif
