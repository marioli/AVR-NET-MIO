/*----------------------------------------------------------------------------
Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
Author:         Radig Ulrich
Remarks:
known Problems: none
Version:        24.10.2007
Description:    Timer Routinen

Dieses Programm ist freie Software. Sie können es unter den Bedingungen der 
GNU General Public License, wie von der Free Software Foundation veröffentlicht, 
weitergeben und/oder modifizieren, entweder gemäß Version 2 der Lizenz oder 
(nach Ihrer Option) jeder späteren Version. 

Die Veröffentlichung dieses Programms erfolgt in der Hoffnung, 
daß es Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, 
sogar ohne die implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT 
FÜR EINEN BESTIMMTEN ZWECK. Details finden Sie in der GNU General Public License. 

Sie sollten eine Kopie der GNU General Public License zusammen mit diesem 
Programm erhalten haben. 
Falls nicht, schreiben Sie an die Free Software Foundation, 
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA. 
------------------------------------------------------------------------------*/

#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart.h"
#include "stack.h"
#include "ntp.h"
#include "cmd.h"
#include "fader.h"
#include "dhcpc.h"
#include "timer.h"

volatile unsigned long time;
volatile unsigned long time_watchdog = 0;
static uint8_t postscaler = 252;

//----------------------------------------------------------------------------
//Diese Routine startet und initialisiert den Timer
void timer_init(void)
{

  /****** prepare timer for LED_GREEN (OC0) ******/
  TCCR0 = (1<<WGM00) | (1<<WGM01) | (0<<COM00) | (1<<COM01) | (1<<CS00) | (0<<CS01) | (0<<CS02);
  TCNT0 = 0;
  OCR0 = 0;

  /****** prepare timer for LED_RED/LED_BLUE (OC1A/OC1B) ******/
  TCCR1A = (1<<WGM10) | (0<<WGM11) | (0<<COM1A0) | (1<<COM1A1) | (0<<COM1B0) | (1<<COM1B1);
  TCCR1B = (1<<WGM12) | (0<<WGM13) | (1<<CS10) | (0<<CS11) | (0<<CS12);
  TCNT1 = 0;
  OCR1A = 0;
  OCR1B = 0;

  /****** prepare timer for TICKS of the system ******/
  /* Timer Counter Control Register 2
   *      CTC Mode                  Prescaler: 1024 Cycles */
  TCCR2 = (1<<WGM21) | (0<<WGM20) | ( 1<<CS22 | 1<<CS21 | 1<<CS20);
  TCNT2 = 0;
  OCR2 = ((F_CPU / 1024) / 252);
  TIMSK |= (1 << OCIE2);
  return;
};

//----------------------------------------------------------------------------
//Timer Interrupt
ISR (TIMER2_COMP_vect)
{
  /* the fader is high-res */
  fader_do_fade();

  /* everything else is needed once a second */
  if (postscaler--)
    return;

  /* reset postscaler */
  postscaler = 252;

  /* tick 1 second */
  time++;
  if((stack_watchdog++) > WTT)  //emergency reset of the stack
  {
    RESET();
  }
  eth.timer = 1;

  ntp_timer--;

  if ( dhcp_lease > 0 ) dhcp_lease--;
  if ( gp_timer   > 0 ) gp_timer--;
}
