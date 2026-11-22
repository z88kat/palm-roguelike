TARGET = iRogue
APPNAME = "iRogue"
APPID = "RoGe"

OBJS = Globals.o color.o dead.o display.o hit.o inventory.o level.o main.o \
	message.o monster.o move.o my_object.o object.o pack.o random.o \
	ring.o room.o save.o spec_hit.o throw.o trap.o use.o win.o zap.o \
	util.o \
	form_about.o form_call.o form_hwbuttons.o \
        form_map.o form_messagelog.o \
	form_prefs.o form_revivify.o form_save.o form_topscores.o form_wiz.o \
	$(TARGET).o $(TARGET)-sections.o

#INCS = Globals.h rogue.h rogue_defines.h rogue_prototypes.h librogue.h
INCS = constant.h globals.h handera.h iRogue.h iRogueRsc.h palm.h \
	prototypes.h sections.h types.h

# --- Toolchain locations (modern macOS / palmdev-macos) -------------
# Override on the command line if yours live elsewhere, e.g.
#   make PALMDEV_TOOLCHAIN=/path/to/toolchain
PALMDEV_TOOLCHAIN ?= $(HOME)/Development/palmdev-macos/toolchain
PALMDEV_SDK ?= /usr/local/palmdev

# prc-tools needs this to find its specs/libraries
export GCC_EXEC_PREFIX = $(PALMDEV_TOOLCHAIN)/lib/gcc-lib/

CC = $(PALMDEV_TOOLCHAIN)/bin/m68k-palmos-gcc
MULTIGEN = $(PALMDEV_TOOLCHAIN)/bin/m68k-palmos-multigen
BUILDPRC = $(PALMDEV_TOOLCHAIN)/bin/build-prc
# Stock pilrc 3.2 can't read .bmp files on 64-bit hosts; use the patched
# one built by tools/build-pilrc.sh.
PILRC = ../../tools/pilrc/pilrc
PILOTXFER = pilot-xfer
PILOT_PORT ?= usb:

ifeq ($(wildcard $(CC)),)
  $(error m68k-palmos-gcc not found in $(PALMDEV_TOOLCHAIN)/bin - set PALMDEV_TOOLCHAIN)
endif
ifeq ($(wildcard $(PILRC)),)
  $(error Patched pilrc not found - run ../../tools/build-pilrc.sh first)
endif

# The HandEra SDK headers ship with CRLF line endings, which break
# gcc 2.95's handling of backslash-continued macros.  We make LF copies
# under compat/Handera/ so that #include "Handera/Vga.h" works.
HANDERA_SDK_INC = $(PALMDEV_SDK)/handera-105/include
HANDERA_STAMP = compat/Handera/.stamp

#CFLAGS = -Wall -g -O2 -Icompat -palmos2 -DI_AM_OS_2
# can compile without color, but it doesn't really save any space
CFLAGS = -Wall -g -O2 -Icompat -palmos3.5 -DNON_INTERNATIONAL -DI_AM_COLOR
##CFLAGS = -Wall -g -O2 -Icompat -palmos3.1 -DNON_INTERNATIONAL

all: iRogue.prc

install: $(TARGET).prc
	$(PILOTXFER) -p $(PILOT_PORT) -i $(TARGET).prc

$(HANDERA_STAMP): $(wildcard $(HANDERA_SDK_INC)/*.h)
	@mkdir -p compat/Handera
	@for f in $(HANDERA_SDK_INC)/*.h; do \
	  tr -d '\r' < "$$f" > "compat/Handera/$$(basename "$$f")"; \
	done
	@touch $@

$(OBJS): $(HANDERA_STAMP)

.S.o:
	$(CC) $(TARGETFLAGS) -c $<

.c.s:
	$(CC) $(CSFLAGS) $<

$(TARGET).prc: $(TARGET) bin.stamp
#	$(BUILDPRC) $(TARGET).prc $(APPNAME) $(APPID) $(TARGET) *.bin
	$(BUILDPRC) $(TARGET).def $(TARGET) *.bin

$(TARGET): $(OBJS) $(TARGET)-sections.ld
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(TARGET)-sections.ld

bin.stamp: $(TARGET).rcp $(TARGET).pbm Font56.txt
	$(PILRC) $(TARGET).rcp .
	touch bin.stamp

$(TARGET)-sections.o: $(TARGET)-sections.s
	$(CC) -c $(TARGET)-sections.s

$(TARGET)-sections.s $(TARGET)-sections.ld: $(TARGET).def
	$(MULTIGEN) $(TARGET).def



depend:
#	makedepend -Y -I/usr/local/pilot/m68k-palmos/include/ *.c
	makedepend -Y  *.c

dingy:
	-rm -f *.[oa] $(TARGET)

clean:
	-rm -f *.[oa] $(TARGET) *.bin bin.stamp *.grc Makefile.bak $(TARGET)-sections.s $(TARGET)-sections.ld
	-rm -rf compat

veryclean: clean
	-rm -f $(TARGET).prc pilot.ram pilot.scratch

# DO NOT DELETE

Globals.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h
Globals.o: types.h prototypes.h globals.h
color.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
color.o: prototypes.h globals.h
dead.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
dead.o: prototypes.h globals.h
display.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h
display.o: types.h prototypes.h globals.h
form_about.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h
form_about.o: types.h prototypes.h globals.h
form_hwbuttons.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h
form_hwbuttons.o: types.h prototypes.h globals.h
form_map.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h
form_map.o: types.h prototypes.h globals.h
form_messagelog.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h
form_messagelog.o: constant.h types.h prototypes.h globals.h
form_prefs.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h
form_prefs.o: types.h prototypes.h globals.h
form_revivify.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h
form_revivify.o: types.h prototypes.h globals.h
form_save.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h
form_save.o: types.h prototypes.h globals.h
form_topscores.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h
form_topscores.o: types.h prototypes.h globals.h
form_wiz.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h
form_wiz.o: types.h prototypes.h globals.h
hit.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
hit.o: prototypes.h globals.h
iRogue.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
iRogue.o: prototypes.h globals.h
inventory.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h
inventory.o: types.h prototypes.h globals.h
level.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
level.o: prototypes.h globals.h
main.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
main.o: prototypes.h globals.h
message.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h
message.o: types.h prototypes.h globals.h
monster.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h
monster.o: types.h prototypes.h globals.h
move.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
move.o: prototypes.h globals.h
my_object.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h
my_object.o: types.h prototypes.h globals.h
object.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
object.o: prototypes.h globals.h
pack.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
pack.o: prototypes.h globals.h
random.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
random.o: prototypes.h globals.h
ring.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
ring.o: prototypes.h globals.h
room.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
room.o: prototypes.h globals.h
save.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
save.o: prototypes.h globals.h
spec_hit.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h
spec_hit.o: types.h prototypes.h globals.h
throw.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
throw.o: prototypes.h globals.h
trap.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
trap.o: prototypes.h globals.h
use.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
use.o: prototypes.h globals.h
util.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
util.o: prototypes.h globals.h
win.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
win.o: prototypes.h globals.h
zap.o: iRogue.h palm.h handera.h iRogueRsc.h sections.h constant.h types.h
zap.o: prototypes.h globals.h
