#include <stddef.h>
#include <stdlib.h>

#include <devices/timer.h>
#include <emul/emulregs.h>
#include <exec/execbase.h>
#include <exec/resident.h>
#include <exec/system.h>
#include <proto/exec.h>
#include <proto/socket.h>

#include <SDL.h>

#include "NET_version.h"
#include "NET_library.h"
#include "NET_startup.h"

STATIC CONST TEXT __TEXTSEGMENT__ verstring[] = VERSTAG;
STATIC CONST TEXT libname[] = "sdl2_net.library";

struct ExecBase   *SysBase  = NULL;
struct DosLibrary *DOSBase  = NULL;
struct Library 		*SDL2Base = NULL;
struct Library 		*SocketBase = NULL;
struct Library 		*SDL2NetBase = NULL;

/**********************************************************************
	LIB_Reserved
**********************************************************************/

STATIC ULONG LIB_Reserved(void)
{
	return 0;
}

/**********************************************************************
	comp_ctdt

	Sort constructors/destructors
**********************************************************************/

STATIC int comp_ctdt(struct CTDT *a, struct CTDT *b)
{
	if (a->priority == b->priority)
		return (0);
	if ((unsigned long)a->priority < (unsigned long) b->priority)
		return (-1);

	return (1);
}

STATIC VOID sort_ctdt(struct SDL2NetLibrary *LibBase)
{
	extern struct CTDT __ctdtlist;
	struct CTDT *ctdtlist = &__ctdtlist;

	struct HunkSegment *seg = (struct HunkSegment *)(((unsigned int)ctdtlist) - sizeof(struct HunkSegment));
	struct CTDT *_last_ctdt = (struct CTDT *)(((unsigned int)seg) + seg->Size);

	qsort((struct CTDT *)ctdtlist, _last_ctdt - ctdtlist, sizeof(*ctdtlist), (int (*)(const void *, const void *))comp_ctdt);

	LibBase->ctdtlist = ctdtlist;
	LibBase->last_ctdt = _last_ctdt;
}

/**********************************************************************
	init_libs
**********************************************************************/

static int init_libs(struct SDL2NetLibrary *base, struct ExecBase *SysBase)
{
	if ((DOSBase = base->MyDOSBase = (APTR)OpenLibrary("dos.library", 36)) != NULL)
	{
		sort_ctdt(base);
		return 1;
	}

	return 0;
}

/**********************************************************************
	data relocs
**********************************************************************/

#define R13_OFFSET 0x8000

extern int __datadata_relocs(void);

STATIC __inline int __dbsize(void)
{
	extern APTR __sdata_size, __sbss_size;
	STATIC CONST ULONG size[] =
	{
		(ULONG)&__sdata_size, (ULONG)&__sbss_size
	};
	return size[0] + size[1];
}

/**********************************************************************
	LIB_Init
**********************************************************************/

struct Library	*LIB_Init(struct SDL2NetLibrary *LibBase, BPTR SegList, struct ExecBase *sysBase)
{
	register char *r13;

	SysBase = sysBase;

	LibBase->Library.lib_Node.ln_Pri = -5;
#ifndef __MORPHOS__
	LibBase->Library.lib_Revision = COMPILE_REVISION;
#endif

	asm volatile ("lis %0,__r13_init@ha; addi %0,%0,__r13_init@l" : "=r" (r13));

	LibBase->SegList   = SegList;
	LibBase->DataSeg   = r13 - R13_OFFSET;
	LibBase->DataSize  = __dbsize();
	LibBase->Parent    = NULL;
	LibBase->MySysBase = sysBase;

	NEWLIST(&LibBase->TaskContext.TaskList);

	InitSemaphore(&LibBase->Semaphore);

	if (init_libs(LibBase, sysBase) == 0)
	{
		FreeMem((APTR)((ULONG)(LibBase) - (ULONG)(LibBase->Library.lib_NegSize)), LibBase->Library.lib_NegSize + LibBase->Library.lib_PosSize);
		LibBase	= NULL;
	}

	return (struct Library *)LibBase;
}

/**********************************************************************
	DeleteLib
**********************************************************************/

static BPTR DeleteLib(struct SDL2NetLibrary *LibBase, struct ExecBase *SysBase)
{
	BPTR SegList = 0;

	if (LibBase->Library.lib_OpenCnt == 0)
	{
		CloseLibrary((struct Library *)LibBase->MyDOSBase);

		SegList = LibBase->SegList;

		REMOVE(&LibBase->Library.lib_Node);
		FreeMem((APTR)((ULONG)(LibBase) - (ULONG)(LibBase->Library.lib_NegSize)), LibBase->Library.lib_NegSize + LibBase->Library.lib_PosSize);
	}

	return SegList;
}

/**********************************************************************
	UserLibClose
**********************************************************************/

static void UserLibClose(struct SDL2NetLibrary *LibBase, struct ExecBase *SysBase)
{
	//CloseLibrary(SDL2Base);
	//SDL2Base = NULL;
	if (SocketBase) {
		CloseLibrary(SocketBase);
		SocketBase = NULL;
	}
}

/**********************************************************************
	LIB_Expunge
**********************************************************************/

BPTR LIB_Expunge(void)
{
	struct SDL2NetLibrary *LibBase = (APTR)REG_A6;
	LibBase->Library.lib_Flags |= LIBF_DELEXP;
	return DeleteLib(LibBase, LibBase->MySysBase);
}

/**********************************************************************
	LIB_Close
*********************************************************************/

BPTR LIB_Close(void)
{
	struct SDL2NetLibrary *LibBase = (APTR)REG_A6;
	struct ExecBase *SysBase = LibBase->MySysBase;
	BPTR SegList = 0;

	if (LibBase->Parent)
	{
		struct SDL2NetLibrary *ChildBase = LibBase;

		if ((--ChildBase->Library.lib_OpenCnt) > 0)
			return 0;

		LibBase = ChildBase->Parent;

		REMOVE(&ChildBase->TaskContext.TaskNode.Node);

		AMIGA_Cleanup(ChildBase);
		FreeVecTaskPooled((APTR)((ULONG)(ChildBase) - (ULONG)(ChildBase->Library.lib_NegSize)));
	}

	ObtainSemaphore(&LibBase->Semaphore);

	LibBase->Library.lib_OpenCnt--;

	if (LibBase->Library.lib_OpenCnt == 0)
	{
		LibBase->Alloc = 0;
		UserLibClose(LibBase, SysBase);
	}

	ReleaseSemaphore(&LibBase->Semaphore);

	if (LibBase->Library.lib_Flags & LIBF_DELEXP)
		SegList	= DeleteLib(LibBase, SysBase);

	return SegList;
}

/**********************************************************************
	LIB_Open
**********************************************************************/

struct Library *LIB_Open(void)
{
	struct SDL2NetLibrary *LibBase = (APTR)REG_A6;
	struct SDL2NetLibrary	*newbase, *childbase;
	struct ExecBase *SysBase = LibBase->MySysBase;
	struct Task *MyTask = SysBase->ThisTask;
	struct TaskNode *ChildNode;
	ULONG MyBaseSize;

	/* Has this task already opened a child? */
	ForeachNode(&LibBase->TaskContext.TaskList, ChildNode)
	{
		if (ChildNode->Task == MyTask)
		{
			/* Yep, return it */
			childbase = (APTR)(((ULONG)ChildNode) - offsetof(struct SDL2NetLibrary, TaskContext.TaskNode.Node));
			childbase->Library.lib_Flags &= ~LIBF_DELEXP;
			childbase->Library.lib_OpenCnt++;

			return(&childbase->Library);
		}
	}

	childbase  = NULL;
	MyBaseSize = LibBase->Library.lib_NegSize + LibBase->Library.lib_PosSize;
	LibBase->Library.lib_Flags &= ~LIBF_DELEXP;
	LibBase->Library.lib_OpenCnt++;

	ObtainSemaphore(&LibBase->Semaphore);

	if (LibBase->Alloc == 0)
	{
		if (((SocketBase = OpenLibrary("bsdsocket.library",  4)) != NULL))
		{
			LibBase->Alloc = 1;		
		}
		else
		{
			goto error;
		}
	}

	if ((newbase = AllocVecTaskPooled(MyBaseSize + LibBase->DataSize + 15)) != NULL)
	{
		CopyMem((APTR)((ULONG)LibBase - (ULONG)LibBase->Library.lib_NegSize), newbase, MyBaseSize);

		childbase = (APTR)((ULONG)newbase + (ULONG)LibBase->Library.lib_NegSize);

		if (LibBase->DataSize)
		{
			char *orig   = LibBase->DataSeg;
			LONG *relocs = (LONG *) __datadata_relocs;
			int mem = ((int)newbase + MyBaseSize + 15) & (unsigned int) ~15;

			CopyMem(orig, (char *)mem, LibBase->DataSize);

			if (relocs[0] > 0)
			{
				int i, num_relocs = relocs[0];

				for (i = 0, relocs++; i < num_relocs; ++i, ++relocs)
				{
					*(long *)(mem + *relocs) -= (int)orig - mem;
				}
			}

			childbase->DataSeg = (char *)mem + R13_OFFSET;

			if (AMIGA_Startup(childbase) == 0)
			{
				AMIGA_Cleanup(childbase);
				FreeVecTaskPooled(newbase);
				childbase = 0;
				goto error;
			}
		}

		childbase->Parent = LibBase;
		childbase->Library.lib_OpenCnt = 1;

		/* Register which task opened this child */
		childbase->TaskContext.TaskNode.Task = MyTask;
		ADDTAIL(&LibBase->TaskContext.TaskList, &childbase->TaskContext.TaskNode.Node);
	}
	else
	{
error:
		LibBase->Library.lib_OpenCnt--;

		if (LibBase->Library.lib_OpenCnt == 0)
		{
			LibBase->Alloc	= 0;
			UserLibClose(LibBase, SysBase);
		}
	}

	ReleaseSemaphore(&LibBase->Semaphore);

	return (struct Library *)childbase;
}

/**********************************************************************
	Library data
**********************************************************************/

#include "NET_stubs.h"

extern void LIB_InitTGL();
extern void LIB_SetExitPointer();
extern void LIB_SDL_VSetError();

static const APTR FuncTable[] =
{
	(APTR)FUNCARRAY_BEGIN,
	(APTR)FUNCARRAY_32BIT_NATIVE, 

	(APTR)LIB_Open,
	(APTR)LIB_Close,
	(APTR)LIB_Expunge,
	(APTR)LIB_Reserved,
	(APTR)-1,

	(APTR)FUNCARRAY_32BIT_SYSTEMV,

	#define GENERATE_POINTERS
	#include "NET_stubs.h"

	(APTR)-1,
	(APTR)FUNCARRAY_END
};

static const size_t InitTable[] =
{
	sizeof(struct SDL2NetLibrary),
	(size_t)FuncTable,
	0,
	(size_t)LIB_Init
};

const struct Resident __TEXTSEGMENT__ RomTag =
{
	RTC_MATCHWORD,
	(struct Resident *)&RomTag,
	(struct Resident *)&RomTag+1,
	RTF_AUTOINIT | RTF_PPC | RTF_EXTENDED,
	VERSION,
	NT_LIBRARY,
	0,
	(char *)libname,
	(char *)&verstring[7],
	(APTR)&InitTable[0],
	REVISION, NULL
};

typedef struct {
	Uint32 host;			/* 32-bit IPv4 host address */
	Uint16 port;			/* 16-bit protocol port */
} IPaddress;
typedef struct _TCPsocket *TCPsocket;
typedef struct _UDPsocket *UDPsocket;
typedef struct {
	int channel;		/* The src/dst channel of the packet */
	Uint8 *data;		/* The packet data */
	int len;		/* The length of the packet data */
	int maxlen;		/* The size of the data buffer */
	int status;		/* packet status after sending */
	IPaddress address;		/* The source/dest address of an incoming/outgoing packet */
} UDPpacket;

typedef struct _SDLNet_SocketSet *SDLNet_SocketSet;

CONST ULONG __abox__ = 1;
__asm("\n.section \".ctdt\",\"a\",@progbits\n__ctdtlist:\n.long -1,-1\n");
