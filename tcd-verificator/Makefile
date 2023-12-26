
CC65_HOME   = E:/devtools/cc65
MAKEFLAGS   = -j

SYS	    = nes

croper	= $(CC65_HOME)/bin/croper
CL	= $(CC65_HOME)/bin/cl65
CC	= $(CC65_HOME)/bin/cc65
AS	= $(CC65_HOME)/bin/ca65
LD	= $(CC65_HOME)/bin/ld65
MR	= ./tools/memreport	
TS	= ./tools/timestamp	

INC	= $(CC65_HOME)/include
ASMINC	= $(CC65_HOME)/asminc
# This one comes with VICE

# --static-locals
CFLAGS		:= --cpu HuC6280 -O2
AFLAGS		:= --cpu HuC6280 -t $(SYS) 
BDIR		:=build

vpath %.o $(BDIR)/


SRC_S := crt0.s sys_asm.s
SRC_C := $(wildcard *.c)
#SRC_C := $(filter-out app-%.c, $(SRC_C))

OBJ	= $(SRC_S:.s=.o)
OBJ	+= $(SRC_C:.c=.o)
OFILES	= $(OBJ:%.o=$(BDIR)/%.o)
	
# --------------------------------------------------------------------------
all: $(OBJ) 
	$(LD) -C pce.cfg -o verificator.pce $(OFILES) nes.lib -m $(BDIR)/mem.map
	$(TS) -f verificator.pce -a 0x00000
	$(MR) verificator.pce
	

%.o: %.s
	$(AS) $(AFLAGS) -o $(basename $(BDIR)/$<).o $<

%.o: %.c
	$(CC) $(CFLAGS) -o $(basename $(BDIR)/$<).s $<
	$(AS) $(AFLAGS) $(basename $(BDIR)/$<).s


clean:
	$(RM) $(BDIR)/*.o
	$(RM) $(BDIR)/*.s
	$(RM) $(APDIR)/*.o
