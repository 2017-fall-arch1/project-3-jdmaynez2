#include <msp430.h>
#include "libTimer.h"
#include "buzzer.h"

static unsigned int period = 2000;
static signed int rate = 300;	
static signed int note = 0;

#define MIN_PERIOD 2000
#define MAX_PERIOD 5000
#define a 1255
#define b 1118
#define c 1055
#define cs 996
#define d 940
#define eb 887
#define e 837
#define fs 746
#define g 704
#define a2 627

void buzzer_init()
{
    /* 
       Direct timer A output "TA0.1" to P2.6.  
        According to table 21 from data sheet:
          P2SEL2.6, P2SEL2.7, anmd P2SEL.7 must be zero
          P2SEL.6 must be 1
        Also: P2.6 direction must be output
    */
    timerAUpmode();		/* used to drive speaker */
    P2SEL2 &= ~(BIT6 | BIT7);
    P2SEL &= ~BIT7; 
    P2SEL |= BIT6;
    P2DIR = BIT6;		/* enable output to speaker (P2.6) */

    //buzzer_set_period(200);	/* start buzzing!!! */
    //buzzer_set_period(1000);

}

void buzzer_advance_frequency() 
{
  period += rate;
  /*if ((rate > 0 && (period > MAX_PERIOD)) || 
      (rate < 0 && (period < MIN_PERIOD))) {
    rate = -rate;
    period += (rate << 1);
  //}*/
  if(rate > 0 && (period > MAX_PERIOD)){
      period = MIN_PERIOD;
      
}
  buzzer_set_period(period);
}

void cannon() 
{//cannon
    if(note == 0 || note == 3 || note == 6){buzzer_set_period(a2);}
    else if(note == 1 || note == 4 || note == 12 || note == 14){buzzer_set_period(fs);}
    else if(note == 2 || note == 5 || note == 13){buzzer_set_period(g);}
    else if(note == 7){buzzer_set_period(a);}
    else if(note == 8){buzzer_set_period(b);}
    else if(note == 9){buzzer_set_period(cs);}
    else if(note == 10){buzzer_set_period(d);}
    else if(note == 11){buzzer_set_period(e);}
    else if(note == 15 || note == 16){buzzer_set_period(0);}
    
    note +=1;
    if(note == 17){note = 0;}
}

void furElise()
{//fur elise
    if(note == 0){buzzer_set_period(e);}
    else if(note == 1){buzzer_set_period(eb);}
    else if(note == 2){buzzer_set_period(e);}
    else if(note == 3){buzzer_set_period(eb);}
    else if(note == 4){buzzer_set_period(e);}
    else if(note == 5){buzzer_set_period(b);}
    else if(note == 6){buzzer_set_period(d);}
    else if(note == 7){buzzer_set_period(c);}
    else if(note == 8){buzzer_set_period(a);}
    else if(note == 9){buzzer_set_period(0);}
    else if(note == 10){buzzer_set_period(0);}
    note +=1;
    if(note == 13){note = 0;}
}

void buzzer_set_period(short cycles)
{
  CCR0 = cycles; 
  CCR1 = cycles >> 1;		/* one half cycle */
}
