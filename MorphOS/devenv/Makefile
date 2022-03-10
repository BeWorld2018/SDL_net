# Makefile to build the SDL2_net library 

CC      = ppc-morphos-gcc-9 -noixemul
INCLUDE =  -I../sdk
CFLAGS  =  -mcpu=750 -mtune=7450 -O2 $(INCLUDE) -Wall -Wno-pointer-sign -fno-strict-aliasing
AR	= ppc-morphos-ar
TARGET = libSDL2_net.a

ECHE = echo -e
BOLD = \033[1m
NRML = \033[22m

COMPILING = @$(ECHE) "compiling $(BOLD)$@$(NRML)..."
LINKING = @$(ECHE) "linking $(BOLD)$@$(NRML)..."
STRIPPING = @$(ECHE) "stripping $(BOLD)$@$(NRML)..."
ARCHIVING = @$(ECHE) "archiving $(BOLD)$@$(NRML)..."

all: lib/$(TARGET) lib/libb32/$(TARGET) lib/libb32/libSDL2_net-nc.a

sdl-startup.o: sdl-startup.c
	$(COMPILING)
	@$(CC) $(CFLAGS) -o $@ -c sdl-startup.c

sdl-startup-brel.o: sdl-startup.c
	$(COMPILING)
	@$(CC) $(CFLAGS) -mresident32 -o $@ -c sdl-startup.c

sdl-startup-brel-nc.o: sdl-startup.c
	$(COMPILING)
	@$(CC) $(CFLAGS) -mresident32 -o $@ -c sdl-startup.c -D__NO_SDL_CONSTRUCTORS

lib/$(TARGET): sdl-startup.o ../sdk/fd/sdl2_net_lib.fd
	$(ARCHIVING)
	@mkdir -p lib
	@-rm -f $@
	@cvinclude.pl --fd ../sdk/fd/sdl2_net_lib.fd --clib ../sdk/clib/sdl2_net_protos.h --gluelib=$@
	@$(AR) crus $@ sdl-startup.o

lib/libb32/$(TARGET): sdl-startup-brel.o  ../sdk/fd/sdl2_net_lib.fd
	$(ARCHIVING)
	@mkdir -p lib/libb32
	@-rm -f $@
	@cvinclude.pl --fd ../sdk/fd/sdl2_net_lib.fd --clib ../sdk/clib/sdl2_net_protos.h --brelgluelib=$@
	@$(AR) crus $@ sdl-startup-brel.o

lib/libb32/libSDL2_net-nc.a: sdl-startup-brel-nc.o ../sdk/fd/sdl2_net_lib.fd
	$(ARCHIVING)
	@mkdir -p lib/libb32
	@-rm -f $@
	@cvinclude.pl --fd ../sdk/fd/sdl2_net_lib.fd --clib ../sdk/clib/sdl2_net_protos.h --brelgluelib=$@
	@$(AR) crus $@ sdl-startup-brel-nc.o

clean:
	@-rm -rf lib *.s

dump:
	ppc-morphos-objdump --disassemble-all --reloc lib/$(TARGET).a >lib/libSDL2_net.s
