#include <proto/exec.h>
#include "NET_library.h"

/*********************************************************************/

extern struct Library    *SDL2Base;
extern struct Library    *SDL2NetBase;

int ThisRequiresConstructorHandling = 0;

/* This function must preserve all registers except r13 */
asm
("\n"
"	.section \".text\"\n"
"	.align 2\n"
"	.type __restore_r13, @function\n"
"__restore_r13:\n"
"	lwz 13, 36(3)\n"
"	blr\n"
"__end__restore_r13:\n"
"	.size __restore_r13, __end__restore_r13 - __restore_r13\n"
);

APTR libnix_mempool;

/**********************************************************************
	Startup/Cleanup
**********************************************************************/

int SAVEDS AMIGA_Startup(struct SDL2NetLibrary *LibBase)
{

	SDL2NetBase = &LibBase->Library;

	if ((SDL2Base = OpenLibrary("sdl2.library", 53)) == NULL)
		return 0;
	
	return 1;
}

VOID SAVEDS AMIGA_Cleanup(struct SDL2NetLibrary *LibBase)
{
	
	CloseLibrary(SDL2Base);
	SDL2Base = NULL;

}

void __chkabort(void) { }
void abort(void) { for (;;) Wait(0); }
