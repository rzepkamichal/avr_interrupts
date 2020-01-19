//////////////////////////////////////////////////////////////////////////////
// Laboratory AVR Microcontrollers Part1
// Program template for lab 7
// Please fill in this information before starting coding
// Authors:
//		Rzepka Micha�
//		Wa�ny �ukasz
//		Wolny Dominik
//
// Group:	1
// Section:	2
//
// Task:	a
//
// Todo:
// Write a program which endlessly copies data from ROMTAB to RAMTAB. Coping should occur only if
// at least one button is pressed. If the program runs into the end of one of the tables, it should begin from the
// beginning of the table. There is a 200 millisecond delay between each byte. After each cycle, LED on PORTB.6
// must switch on for 1 second.
//
//
// Version: 5.0
//////////////////////////////////////////////////////////////////////////////

#include <avr/io.h>
// please look into header pgmspace.h functions to properly read from ROM
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
// please change this to proper value given by the teacher
#define F_CPU 16000000L
// please correct this line according to the guidelines given by the teacher
#define nLength 256

uint8_t TAB_RAM[nLength];
// please correct end of this table according to the guidelines given by the teacher

#define PROGMEM_TEST_1   	 __attribute__((used, section(".test_1")))
#define PROGMEM_TEST_2   	 __attribute__((used, section(".test_2")))

uint8_t const TAB_ROM[] PROGMEM = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x1F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1F, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaA, 0xaB, 0xaC, 0xaD, 0xAE, 0xAF,
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
	0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8, 0xF7, 0xF6, 0xF5, 0xF4, 0xF3, 0xF2, 0xF1, 0xF0,
0x00, 0x00};


//timer prescaler
#define presc 1024

//max 16-bit int value stored in timer
#define intSize 65535

//amount of timer ticks, after which 200ms pass
#define timer_200_ms (intSize - ((F_CPU) / presc) * 0.2)

//timer flag
//indicates the number of timer interrupts the main program should wait after certain events
//volatile - to access the variable from timer overflow function and from main function
volatile int overflowCounter;

//handles timer1 overflow interrupt
ISR(TIMER1_OVF_vect){
	
	overflowCounter--;
	
	//blink LED's on PORTC when the timer hits the preset amount of overflows
	if(overflowCounter == 0){
		
		if(PORTC != 0x00)
			PORTC = 0x00;
		else
			PORTC = 0xFF;
	}
	
	//reset the timer register value
	TCNT1 = timer_200_ms;
}

//---------------------------------------------------------------------
int main (void)
{
	//---------------------------------------------------------------------
	// Main program code place here
	// 1. Place here code related to initialization of ports and interrupts
	// for instance:
	//    port A as input and switching Pull-up resistors on
	//DDRA=0x00;
	//PORTA=0xFF;
	//    port C as output and initial value FF
	
	DDRA = 0x00;
	
	//turn on internal pull-ups if none external are present
	if(PINA != 0xFF){
		PORTA = 0xFF;
	}
	
	DDRB=0xFF;
	PORTB=0x00;
	
	DDRC=0xFF;
	PORTC=0xFF;

	//
	// Writing a logic one to PINxn toggles the value of PORTxn, independent on the value of DDRxn
	// Note that the SBI instruction can be used to toggle one single bit in a port.


	// 2. Enable interrupts if needed
	TCCR1A = 0x00;
	TCCR1B = (1<<CS10) | (1<<CS12);	// set timer1 prescaler to 1024
	TIMSK1 = (1 << TOIE1);			// enable timer1 overflow interrupt
	
	//disable global interrupts
	cli();

	// 3. Place here main code
	
	int iROM = 0;
	int iRAM = 0;
	
	while(1)
	{
		if(PINA != 0xFF) {
			
			if (iRAM < nLength) {
				
				if (0x0000 == pgm_read_word_far(pgm_get_far_address(TAB_ROM) + iROM)) {
					
					iROM = 0;
				
					//wait for 1000 ms
					overflowCounter = 5;
					TCNT1 = timer_200_ms;
					sei();
					while(overflowCounter > 0){}
					cli();
					continue;
				}
				
				TAB_RAM[iRAM] = pgm_read_byte_far(pgm_get_far_address (TAB_ROM) + iROM);
				iROM++;
				iRAM++;
				
				//wait for 200ms
				overflowCounter = 1;
				TCNT1 = timer_200_ms;
				sei();
				while(overflowCounter > 0){}
				cli();
				
			} else {
				
				iRAM = 0;
				
				//wait for 1000 ms
				overflowCounter = 5;
				TCNT1 = timer_200_ms;
				sei();
				while(overflowCounter > 0){}
				cli();
			}
		}
	}
	
	//----------------------------------------------------------------------
	// Program end
	//----------------------------------------------------------------------
}
// -------------------------------------------------------------------