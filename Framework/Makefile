
## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU) 

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
##CFLAGS += -x c -std=c99 -pedantic-errors -finline-functions -Wall $(OPTIMIZE) -g -fsigned-char
CFLAGS += -x c -std=c99 -finline-functions -Wall $(OPTIMIZE) -g -fsigned-char

## Linker flags
LDFLAGS = $(COMMON)

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump

#include dirs
INCLUDES = -I"$(OS_DIR)" \
					 -I"$(OS_DIR)/PDL/$(PLATFORM)"

include $(OS_DIR)/PDL/$(PLATFORM)/Make.Platform

#defines
DEFS += -DF_CPU=$(CPU_FREQUENCY)
DEFS += -DDEFAULT_PAN_ID=$(PAN_ID)
DEFS += -DMAC_DEFAULT_A_EXTENDED_ADDRESS=$(MAC_ADDR)
DEFS += -DDEFAULT_CHANNEL=$(CHANNEL)

DEFS += -DMAX_THREADS=$(NUM_THREADS) -DMAX_TIMERS=$(NUM_TIMERS) 
DEFS += -DMAX_NUM_PORTS=$(NUM_PORTS) 
DEFS += $(PLATFORM_DEFS)

#source files
SRC += $(APP_SRC) \
       $(OS_DIR)/PIL/Scheduler/Scheduler.c \
       $(OS_DIR)/PIL/Components.c \
       $(OS_DIR)/PIL/Guard.c \
       $(OS_DIR)/PIL/Utils.c \
       $(OS_DIR)/PIL/Main.c \
       $(PLATFORM_SRC)

all: $(TARGET) $(PRG).srec $(PRG).hex

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) $(SRC) -o $(TARGET) $(LIBS) 

# dependency:
app.o: 

clean:
	rm -rf *.o $(PRG).elf *.eps *.png *.pdf *.bak 
	rm -rf *.hex *.srec
	rm -rf *.lst *.map $(EXTRA_CLEAN_FILES)

lst:  $(PRG).lst

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

# Rules for building the .text rom images

text: hex bin srec

hex:  $(PRG).hex
bin:  $(PRG).bin
srec: $(PRG).srec

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@
	avr-size -td $(PRG).elf
%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@
