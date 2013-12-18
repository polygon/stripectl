#include "main.h"

/*

	data sheet  : http://www.nxp.com/documents/data_sheet/LPC1315_16_17_45_46_47.pdf
	user manual : http://www.nxp.com/documents/user_manual/UM10524.pdf


	led1 : pin 33 (0_12)
	led2 : pin 35 (0_14)

*/

#define NUM_LEDS 240
#define STARTLED 60
#define STOPLED 170

#include <stdio.h>

#include "drivers/usb_cdc.h"
#include "drivers/buttons.h"
#include "drivers/armmath.h"
#include "core/inc/lpc13uxx_uart.h"
//#include "core/inc/lpc13uxx_timer32.h"

volatile uint32_t msTicks = 0;
static uint16_t buttonsInitialized = 0;
static uint8_t mode  = 0;
static uint8_t led_data[NUM_LEDS][3];
uint32_t T0H, T0L, T1H, T1L, RES;
#define DATAPIN 11

void output_stripe_data()
{
    uint32_t led, color;
    uint8_t bit;
    uint32_t wait;
    __disable_irq();

    for (led = 0; led < NUM_LEDS; led++)
        for (color = 0; color < 3; color++)
            for (bit = 128; bit != 0; bit = bit >> 1)
                if (led_data[led][color] & bit)
                {
                    LPC_GPIO->B0[DATAPIN] = 1;
                    for (wait = 0; wait < T1H/3; wait++) {}
                    LPC_GPIO->B0[DATAPIN] = 0;
                    for (wait = 0; wait < T1L/3; wait++) {}
                }
                else
                {
                    LPC_GPIO->B0[DATAPIN] = 1;
                    for (wait = 0; wait < T0H/3; wait++) {}
                    LPC_GPIO->B0[DATAPIN] = 0;
                    for (wait = 0; wait < T0L/3; wait++) {}

                }

    __enable_irq();
}

void SysTick_Handler(void) {
	msTicks++;

	if(buttonsInitialized)
	{
		buttons_sample();
	}
}

void delay_ms(uint32_t ms) {
	uint32_t now = msTicks;
	while ((msTicks-now) < ms);
}

int main(void) {

	SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock/1000);

    uint32_t i;

    T0H = (SystemCoreClock * 5  ) / 20000000;
    T1H = (SystemCoreClock * 12 ) / 20000000;
    T0L = (SystemCoreClock * 20 ) / 20000000;
    T1L = (SystemCoreClock * 13 ) / 20000000;
    RES = (SystemCoreClock * 500) / 20000000;


	// clock to GPIO
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);

	// configure the two LEDs PINs as GPIO (they default to jtag)
	LPC_IOCON->TMS_PIO0_12  &= ~0x07;
	LPC_IOCON->TMS_PIO0_12  |= 0x01;
	LPC_IOCON->TRST_PIO0_14  &= ~0x07;
	LPC_IOCON->TRST_PIO0_14  |= 0x01;

    // Disable pull-up on LED Stripe pin (default enabled)
    LPC_IOCON->TDI_PIO0_11 &= ~0x1F;
    LPC_IOCON->TDI_PIO0_11 |= 0x01;

    //data direction: output
	LPC_GPIO->DIR[0] |= (1<<12);
	LPC_GPIO->DIR[0] |= (1<<14);
    LPC_GPIO->DIR[0] |= (1<<DATAPIN);

    LPC_GPIO->B0[DATAPIN] = 0;

    for (i = 0; i < NUM_LEDS; i++)
    {
        if (!(i % 10))
        {
            led_data[i][0] = 0;
            led_data[i][1] = 0;
            led_data[i][2] = 0;
        }
    }
    LPC_GPIO->B0[12] = 0;
    output_stripe_data();
    LPC_GPIO->B0[14] = 0;
    //UARTInit(9600);

	buttons_init();
	buttonsInitialized=1;
    buttons_get_press( KEY_C|KEY_B|KEY_A );
    int pos = STARTLED;
    int dir = 4;
    int ended = 0;

    while (1)
    {
        if (ended)
        {
            for (i = 0; i < NUM_LEDS; i++)
            {
                led_data[i][1] = 0;
            }
            output_stripe_data();
            delay_ms(100);
            ended = 0;
            for (i = 0; i < NUM_LEDS; i++)
            {
                led_data[i][1] = 0;
            }
            output_stripe_data();
            delay_ms(500);

        }

        for (i = 0; i < NUM_LEDS; i++)
        {
            led_data[i][1] = 0;
        }
        led_data[pos][1] = 4;
        led_data[pos+1][1] = 8;
        led_data[pos+2][1] = 16;
        led_data[pos+3][1] = 32;
        led_data[pos+4][1] = 64;
        led_data[pos+5][1] = 128;
        led_data[pos+6][1] = 255;
        led_data[pos+7][1] = 128;
        led_data[pos+8][1] = 64;
        led_data[pos+9][1] = 32;
        led_data[pos+10][1] = 16;
        led_data[pos+11][1] = 8;
        led_data[pos+12][1] = 4;
        pos += dir;
        if (pos >= STOPLED-1)
            dir = -6;
        if (pos <= STARTLED-6)
        {
            ended = 1;
            dir = 6;
            pos = STARTLED;
        }
        output_stripe_data();
    }
}

