/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#undef STUB

#if defined(GENERATE_STUBS)
#define STUB(name) extern int name(); int __saveds LIB_##name() { return name(); }
#elif defined(GENERATE_POINTERS)
#define STUB(name) (APTR)&LIB_##name ,
#else
#define STUB(name) extern int LIB_##name();
#endif

	STUB(SDLNet_Linked_Version)
	STUB(SDLNet_Init)
	STUB(SDLNet_Quit)
	STUB(SDLNet_ResolveHost)
	STUB(SDLNet_ResolveIP)
	STUB(SDLNet_GetLocalAddresses)
	STUB(SDLNet_TCP_Open)
	STUB(SDLNet_TCP_Accept)
	STUB(SDLNet_TCP_GetPeerAddress)
	STUB(SDLNet_TCP_Send)
	STUB(SDLNet_TCP_Recv)
	STUB(SDLNet_TCP_Close)
	STUB(SDLNet_AllocPacket)
	STUB(SDLNet_ResizePacket)
	STUB(SDLNet_FreePacket)
	STUB(SDLNet_AllocPacketV)
	STUB(SDLNet_FreePacketV)
	STUB(SDLNet_UDP_Open)
	STUB(SDLNet_UDP_SetPacketLoss)
	STUB(SDLNet_UDP_Bind)
	STUB(SDLNet_UDP_Unbind)
	STUB(SDLNet_UDP_GetPeerAddress)
	STUB(SDLNet_UDP_SendV)
	STUB(SDLNet_UDP_Send)
	STUB(SDLNet_UDP_RecvV)
	STUB(SDLNet_UDP_Recv)
	STUB(SDLNet_UDP_Close)
	STUB(SDLNet_AllocSocketSet)
	STUB(SDLNet_AddSocket)
	STUB(SDLNet_DelSocket)
	STUB(SDLNet_CheckSockets)
	STUB(SDLNet_FreeSocketSet)
	STUB(SDLNet_TCP_OpenClient)
	STUB(SDLNet_TCP_OpenServer)