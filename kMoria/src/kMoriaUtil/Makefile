TARGET = kMoriaUtil
APPNAME = "kMoriaUtil"
APPID = "MriA"

OBJS = main.o

CC = m68k-palmos-gcc
CFLAGS = -Wall -g -O2 -I. -I.. -palmos3.5 -DNON_INTERNATIONAL -DI_AM_COLOR

PILRC = pilrc
BUILDPRC = build-prc

all: $(TARGET).prc

.S.o:
	$(CC) $(TARGETFLAGS) -c $<

.c.s:
	$(CC) $(CSFLAGS) $<

$(TARGET).prc: $(TARGET) bin.stamp
	$(BUILDPRC) $(TARGET).def $(TARGET) *.bin

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

bin.stamp: $(TARGET).rcp $(TARGET).pbm
	$(PILRC) $(TARGET).rcp .
	touch bin.stamp


send: $(TARGET).prc
	$(PILOTXFER) -i $(TARGET).prc

depend:
	makedepend -Y -I.. *.c

dingy:
	-rm -f *.[oa] $(TARGET)

clean:
	-rm -f *.[oa] $(TARGET) *.bin bin.stamp *.grc Makefile.bak

veryclean: clean
	-rm -f $(TARGET).prc pilot.ram pilot.scratch

# DO NOT DELETE

main.o: ../palm.h kMoriaUtilRsc.h ../kMoria.h ../sections.h ../constant.h
main.o: ../types.h ../prototypes.h ../records.h
