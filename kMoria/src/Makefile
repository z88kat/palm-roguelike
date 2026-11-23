TARGET = kMoria
APPNAME = "kMoria"
APPID = "Mria"
# I have got this appid assigned.

# spells.c, magic.c, prayer.c .. have a few lines commented out..
UIOBJS = form_about.o form_inv.o form_equ.o form_recall.o form_chargen.o \
       form_store.o form_spell.o form_messagelog.o form_prefs.o \
       form_map.o form_death.o form_topscores.o form_invselect.o \
       form_scribe.o form_look.o form_hwbuttons.o form_dalek.o \
       form_snowcrash.o \
       form_repeat.o form_spellhwb.o form_beam.o form_user.o

#OBJS = main.o display.o lock.o generate.o sets.o desc.o recall.o util.o 
#       misc1.o misc2.o misc3.o misc4.o moria1.o moria2.o create.o 
#       eat.o potions.o staffs.o wands.o   store1.o store2.o 
#       magic.o prayer.o spells.o moria3.o moria4.o misc5.o 
#       creature.o 
# monsters, player, tables, treasure, variable: these are just data
# io: this will be partly implemented elsewhere partly ignored.
# rnd, wizard, signals: these will be ignored
# death dungeon files help save: these need to be munged.

OBJS = create.o creature.o desc.o \
       dungeon.o \
       eat.o generate.o magic.o main.o \
       misc1.o misc2.o misc3.o misc4.o moria1.o moria2.o moria3.o moria4.o \
       potions.o prayer.o recall.o sets.o scrolls.o spells.o staffs.o \
       store1.o store2.o wands.o \
       misc5.o display.o lock.o util.o save.o \
       color.o win.o handera.o \
       $(UIOBJS) \
       $(TARGET)-sections.o

INCS = kMoriaRsc.h kMoria.h

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
PILOT_XFER = pilot-xfer
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

# Getting tired of if'ing for OS2  - how many folks still have
# palm pilot professionals?
# To build for a different OS, uncomment its line and comment the others
# (Be sure to 'make clean' between different os builds!!)
#CFLAGS = -Wall -g -O2 -I. -Icompat -palmos2 -DI_AM_OS_2
##CFLAGS = -Wall -g -O2 -I. -Icompat -palmos3.1 -DNON_INTERNATIONAL
CFLAGS = -Wall -g -O2 -I. -Icompat -palmos3.5 -DNON_INTERNATIONAL -DI_AM_COLOR

all: kMoria.prc kMoriaColorDB.pdb

# Installs the app plus its required game-data database and colour scheme.
install: kMoria.prc kMoriaColorDB.pdb
	$(PILOT_XFER) -p $(PILOT_PORT) -i kMoria.prc kMoriaDB.pdb kMoriaColorDB.pdb

# Optional colour scheme; enables Set > Preferences > Display > Color.
kMoriaColorDB.pdb: ../../tools/make-kmoria-colors.py BuildDB/creaturenames.dat BuildDB/creature.dat
	python3 -I ../../tools/make-kmoria-colors.py BuildDB $@

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
	makedepend -Y  *.c

dingy:
	-rm -f *.[oa] $(TARGET)

clean:
	-rm -f *.[oa] $(TARGET) *.bin bin.stamp *.grc Makefile.bak $(TARGET)-sections.s $(TARGET)-sections.ld
	-rm -rf compat

veryclean: clean
	-rm -f $(TARGET).prc pilot.ram pilot.scratch *.ld *.s

# DO NOT DELETE

create.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
create.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
creature.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
creature.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
desc.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
desc.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
display.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
display.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
dungeon.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
dungeon.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
eat.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h prototypes.h
eat.o: records.h lock-externs.h misc-externs.h win.h
form_about.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_about.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_beam.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_beam.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_chargen.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_chargen.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_dalek.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_dalek.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_death.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_death.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_equ.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_equ.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_hwbuttons.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_hwbuttons.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_identchar.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_identchar.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_inv.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_inv.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_invselect.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_invselect.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_look.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_look.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_map.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_map.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_messagelog.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_messagelog.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_prefs.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_prefs.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_recall.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_recall.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_repeat.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_repeat.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_scribe.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_scribe.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_snowcrash.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_snowcrash.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_spell.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_spell.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_spellhwb.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_spellhwb.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_store.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_store.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_topscores.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_topscores.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
form_user.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
form_user.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
generate.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
generate.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
lock.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
lock.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
magic.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
magic.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
main.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
main.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
misc1.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
misc1.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
misc2.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
misc2.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
misc3.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
misc3.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
misc4.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
misc4.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
misc5.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
misc5.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
moria1.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
moria1.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
moria2.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
moria2.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
moria3.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
moria3.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
moria4.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
moria4.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
potions.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
potions.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
prayer.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
prayer.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
recall.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
recall.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
save.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
save.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
scrolls.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
scrolls.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
sets.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
sets.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
spells.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
spells.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
staffs.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
staffs.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
store1.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
store1.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
store2.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
store2.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
util.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
util.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
wands.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h
wands.o: prototypes.h records.h lock-externs.h misc-externs.h win.h
win.o: kMoria.h palm.h kMoriaRsc.h sections.h constant.h types.h prototypes.h
win.o: records.h lock-externs.h misc-externs.h win.h
