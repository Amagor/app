PRG      = app
TARGET   = app.elf
MCU      = atmega128
PLATFORM = sbn128
OPTIMIZE = -Os
OS_DIR   = Framework

# You should not have to change anything below here.
CC       = avr-gcc

CPU_FREQUENCY = 8000000
NUM_TIMERS  = 9
NUM_THREADS = 32
NUM_PORTS   = 2
PAN_ID     = 0xb21
MAC_ADDR   = 2
CHANNEL    = 11

#defines
DEFS  = -DFIRST=2

LIBS = 
SRC  = app.c

include $(OS_DIR)/Makefile
include $(OS_DIR)/Make.LEDs
include $(OS_DIR)/Make.Timers
include $(OS_DIR)/Make.SPI
include $(OS_DIR)/Make.UART
include $(OS_DIR)/Make.NWK
include $(OS_DIR)/Make.TWI
include $(OS_DIR)/Make.OWI
include $(OS_DIR)/Make.Sensors
include $(OS_DIR)/Make.PWR
include $(OS_DIR)/Make.Buttons
