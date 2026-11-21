/*
 * Name:        uwp_compat.h
 * Description: what a UWP app's C library leaves out, for the game's C (ports/uwp/uwp.cmake puts it before each of its
 *              files, /FI): the environment (getenv), which an app in its container has none of. The checks' variables
 *              (SIMPSONS_TRACE, SIMPSONS_WIDE_DEBUG, RECOMPLAUNCHER_MODE...) are then never set, as on a device
 */
#ifndef SIMPSONS_UWP_COMPAT_H
#define SIMPSONS_UWP_COMPAT_H

#include <stdlib.h>

static __inline char *simpsons_uwp_getenv(const char *name)
{
	(void)name;
	return NULL;
}
#define getenv(name) simpsons_uwp_getenv(name)

#endif
