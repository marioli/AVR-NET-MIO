#include <avr/io.h>
#include <avr/eeprom.h>
#include "config.h"
#include "fader.h"

#define FADER_STEPS 100
static unsigned char need_fade;
static unsigned char fade_delay_ticks;
static unsigned char fader_wait;
static unsigned char fader_step;

static float factor_r;
static float factor_g;
static float factor_b;
static unsigned char from_r;
static unsigned char from_g;
static unsigned char from_b;
static unsigned char to_r;
static unsigned char to_g;
static unsigned char to_b;

void fader_init(void)
{
  need_fade = 0;
  fade_delay_ticks = 2;
}

unsigned char fader_set_delay(unsigned char ticks)
{
  if(ticks)
  {
    fade_delay_ticks = ticks;
  }
  return fade_delay_ticks;
}


void fader_fade_to(unsigned char r, unsigned char g, unsigned char b)
{
  from_r = OCR1A;
  from_g = OCR0;
  from_b = OCR1B;
  to_r = r;
  to_g = g;
  to_b = b;
  if(to_r != from_r || to_g != from_g || to_b != from_b)
  {
    factor_r = ((float)(to_r - from_r)/((float) FADER_STEPS));
    factor_g = ((float)(to_g - from_g)/((float) FADER_STEPS));
    factor_b = ((float)(to_b - from_b)/((float) FADER_STEPS));

    fader_wait = fade_delay_ticks;
    fader_step = 0;
    need_fade = 1;
  }
}


void fader_do_fade(void)
{
  if(need_fade && 0 == fader_wait--)
  {
    fader_step++;
    OCR1A = from_r + (factor_r * ((float)fader_step));
    OCR0 = from_g + (factor_g * ((float)fader_step));
    OCR1B = from_b + (factor_b * ((float)fader_step));

    /* set last precise step if finished */
    if(fader_step == FADER_STEPS)
    {
      OCR1A = to_r;
      OCR0 = to_g;
      OCR1B = to_b;
      need_fade = 0;
    }
    fader_wait = fade_delay_ticks;
  }
}


