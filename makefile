BASE=pomodoro
MYLIBS=wdtasks.h wdtasks.c $(BASE).h

CC=avr-gcc
CC_MCU=attiny85
CC_FLAGS=-save-temps -Os -g -Wall -mmcu=$(CC_MCU)

E2H=avr-objcopy
E2H_FFLAGS=-j .text -j .data -O ihex
E2H_EFLAGS=-j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex

ISP=avrdude
ISP_MCU=t85
ISP_TYPE=usbasp

ISP_FLAGS=-p $(ISP_MCU) -c $(ISP_TYPE) -i 2

DEFINE:=$(shell perl -e '\
  while (<STDIN>) {\
    if (/^\s*\#define\s+(WDTASKS_\S+)\s*(.*)$$/) {\
      print(" -D ",$$1) ;\
      print("=\"",$$2,"\"") if ($$2 ne "") ;\
    }\
  }' <$(BASE).c)



all: $(BASE).hex $(BASE).eep.hex

$(BASE).elf: $(BASE).c $(MYLIBS)
	$(CC) $(DEFINE) $(CC_FLAGS) -o $@ $^

$(BASE).hex: $(BASE).elf
	$(E2H) $(E2H_FFLAGS) $< $@

$(BASE).eep.hex: $(BASE).elf
	$(E2H) $(E2H_EFLAGS) $< $(BASE).eep.hex

symbol: $(BASE).elf
	avr-nm -n $< >$(BASE).sym

size: $(BASE).elf
	avr-size -C $<

desass: $(BASE).elf
	avr-objdump -d -S $< | less

clean:
	@rm -f *.hex *.elf *.sym *.i *.o *.s

rfuse:
	@$(ISP) $(ISP_FLAGS) -U lfuse:r:-:b -U hfuse:r:-:b 2>/dev/null

flash: $(BASE).hex
	@echo "[flash] Connecter l'Attiny puis taper ENTREE"
	@read X
	$(ISP) $(ISP_FLAGS) -U flash:w:$<

eeprom: $(BASE).eep.hex
	@echo "[eeprom] Connecter l'Attiny puis taper ENTREE"
	@read X
	$(ISP) $(ISP_FLAGS) -U eeprom:w:$<

eeread:
	@echo "[eeread] Connecter l'Attiny puis taper ENTREE"
	@read X
	$(ISP) $(ISP_FLAGS) -U eeprom:r:-:r | xxd -g1
