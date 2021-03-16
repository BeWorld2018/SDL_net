/*
 * Copyright (C) 2004-2015 MorphOS Development Team
 *
 * $Id: sdl-startup.c,v 1.2 2015/01/24 18:18:22 itix Exp $
 */

#include <constructor.h>
#include <stdio.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/muimaster.h>

#include "../NET_version.h"

extern void __SDL2_OpenLibError(ULONG version, const char *name);

#if defined(__NO_SDL_CONSTRUCTORS)
extern struct Library *SDL2NetBase;
#else
int _INIT_4_SDL2NetBase(void) __attribute__((alias("__CSTP_init_SDL2NetBase")));
void _EXIT_4_SDL2NetBase(void) __attribute__((alias("__DSTP_cleanup_SDL2NetBase")));

struct Library *SDL2NetBase;

static CONSTRUCTOR_P(init_SDL2NetBase, 100)
{
	static const char libname[] = "sdl2_net.library";
	struct Library *base = OpenLibrary((STRPTR)libname, VERSION);
	SDL2NetBase = base;

	if (base == NULL)
	{
		__SDL2_OpenLibError(VERSION, libname);
	}

	return (base == NULL);
}

static DESTRUCTOR_P(cleanup_SDL2NetBase, 100)
{
	CloseLibrary(SDL2NetBase);
}
#endif
