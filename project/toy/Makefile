COMMON_OBJECTS  = main.o buzzer.o 
CPU             = msp430g2553
CFLAGS          = -mmcu=${CPU} -Os -I../lcdLib -I../timerLib
LDFLAGS         = -L../lib -L/opt/ti/msp430_gcc/include/

CC              = msp430-elf-gcc
AS              = msp430-elf-as -mmcu=${CPU}

all: main.elf

main.elf: ${COMMON_OBJECTS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ -lLcd -lTimer

main.o: main.c
	${CC} ${CFLAGS} -c $< -o $@

buzzer.o: buzzer.c
	${CC} ${CFLAGS} -c $< -o $@

load: main.elf
	msp430loader.sh $^

clean:
	rm -f *.o *.elf.  
