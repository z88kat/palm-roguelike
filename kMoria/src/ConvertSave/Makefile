CC = gcc
CFLAGS = -g -O2 -Wall
TARGET1 = pdb2moria
TARGET2 = moria2pdb
OBJS = swap.o xor.o header.o rec.c
OBJS1 = $(OBJS) to-umoria.o $(TARGET1).o
OBJS2 = $(OBJS) from-umoria.o $(TARGET2).o

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJS1)
	$(CC) -o $(TARGET1) $(OBJS1)

$(TARGET2): $(OBJS2)
	$(CC) -o $(TARGET2) $(OBJS2)

depend:
	makedepend -Y  *.c

clean:
	rm -f *.o $(TARGET1) $(TARGET2)

# Generic Rules

#%.o : %.c
#	$(CC) -c $(CFLAGS) $<
#	$(CC) -c $(FLAGS) $(IPATHS) $<
# DO NOT DELETE

from-umoria.o: defs.h xor.h swap.h rec.h
header.o: defs.h swap.h to-umoria.h
moria2pdb.o: defs.h swap.h header.h rec.h from-umoria.h
pdb2moria.o: defs.h swap.h header.h rec.h to-umoria.h
rec.o: defs.h xor.h swap.h
swap.o: swap.h
to-umoria.o: defs.h xor.h swap.h rec.h
xor.o: swap.h
