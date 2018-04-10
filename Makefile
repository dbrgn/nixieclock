# Devices
MCU_GCC    = atmega16
MCU_DUDE   = m16
PROGRAMMER = usbasp

# GCC config
CC      = avr-gcc
OBJDUMP = avr-objdump
OBJCOPY = avr-objcopy


# Atmega16
# To calculate the fuses, see http://www.engbedded.com/fusecalc
LFUSE    = 0xff
HFUSE    = 0xc9
BITCLOCK = 4
F_CPU    = 8000000  # Main clock frequency (8.0Mhz)

# Compile flags
OPTLEVEL = s
CFLAGS   = -I. -std=gnu99 -O$(OPTLEVEL)
CFLAGS   += -mmcu=$(MCU_GCC) -DF_CPU=$(F_CPU)
CFLAGS   += -funsigned-char -funsigned-bitfields
CFLAGS   += -Wall -Wextra

# Target
BIN    = main
TARGET = ${BIN}.hex
OBJS   = main.o clock.o dcf77.o display.o uart.o

# Avrdude config
DUDE = avrdude


${BIN}.hex: ${BIN}.elf
	$(OBJCOPY) -O ihex -j .text -j .data $< $@

${BIN}.elf: ${OBJS}
	$(CC) $(CFLAGS) -o $@ $^

all: $(TARGET)

flash: $(TARGET)
	$(DUDE) -v -c $(PROGRAMMER) -p $(MCU_DUDE) -P usb -B $(BITCLOCK) -F -U flash:w:$(TARGET):i

setfuses:
	$(DUDE) -v -c $(PROGRAMMER) -p $(MCU_DUDE) -P usb -B $(BITCLOCK) -F -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m

clean:
	rm -f ${BIN}.elf ${BIN}.hex ${OBJS}

.PHONY: clean setfuses
