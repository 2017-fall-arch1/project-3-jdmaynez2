#ifndef switches_included
#define switches_included

#include "msp430.h"
#define SW1 BIT0
#define SW2 BIT1
#define SW3 BIT2
#define SW4 BIT3

unsigned int p2sw_read();
void p2sw_init(unsigned char mask);

#endif // included
