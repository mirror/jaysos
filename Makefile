TARGET = jaysos.gba
ELF    = jaysos.elf
KERNOBJ   = kernel.o main.o uart.o semaphore.o cond_lock.o msg_queue.o ui_mgr.o util.o malloc.o printf.o libgbfs.o apps/shell.o apps/breakout.o apps/life.o apps/stats.o 
KERNSRC   = kernel.c kernel.h main.c gba.h uart.c semaphore.c cond_lock.c msg_queue.c ui_mgr.c util.c malloc.c printf.c libgfs.c apps/shell.c apps/breakout.c apps/life.c apps/stats.c

WABASRC = waba/vm/waba.c waba/vm/jaysos/nmjaysos_a.c waba/vm/jaysos/nmjaysos_b.c waba/vm/jaysos/nmjaysos_c.c
WABAOBJ = waba/vm/waba.o waba/vm/jaysos/nmjaysos_a.o waba/vm/jaysos/nmjaysos_b.o waba/vm/jaysos/nmjaysos_c.o


# relative to waba/examples
WABADEMOS = Controls/*.java ImageSplit/*.java ImageView/*.java CoreTest/*.java HelloWorldWABA/*.java Life/*.java Lines/*.java PocketWatch/*.java


SRC = $(KERNSRC) $(WABASRC)
OBJ = $(KERNOBJ) $(WABAOBJ)


INCLUDE = -I. -Iwaba/vm -Iwaba/vm/jaysos


### crt, script and libraries
CRT    = lowlevel.o
LDSCRIPT = gba.ld.script
LIBS = -lgcc

### programs
AS = arm-thumb-elf-as -marm7 -mall
CC = arm-thumb-elf-gcc 
ELF2BIN = ./gba-elf2bin

### program options
CFLAGS = -Wall -mcpu=arm7tdmi  -ffreestanding -fomit-frame-pointer -O3 $(INCLUDE)
LFLAGS = -nostartfiles -nostdlib -Wl,-Ttext=0x08000000,-Tdata=0x03000000,-T,$(LDSCRIPT) $(CRT) 

$(TARGET): $(ELF) waba/classfiles.gbfs
	$(ELF2BIN) $(ELF) $(TARGET)
	cat waba/classfiles.gbfs >> $(TARGET)

lowlevel.o: lowlevel.s
	$(AS) -o lowlevel.o lowlevel.s

waba/classfiles.gbfs: $(addprefix waba/examples/, $(WABADEMOS)) 
	cd waba/examples; \
	javac $(WABADEMOS) -classpath ../classfiles -d ../classfiles

	cd waba/classfiles; \
	../../tools/gbfs ../classfiles.gbfs * ; \
	cd ../..


$(ELF): $(OBJ) $(CRT)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJ) -o $(ELF) $(LIBS)

clean:
	rm -f $(OBJ) $(TARGET) $(BINARY) $(ELF) *~ *.log *.bak *.o

depend:
	@makedepend $(INCLUDE) $(SRC)

# DO NOT DELETE

