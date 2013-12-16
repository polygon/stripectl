#include "main.h"
#include "drivers/usb_cdc.h"

/*

	data sheet  : http://www.nxp.com/documents/data_sheet/LPC1315_16_17_45_46_47.pdf
	user manual : http://www.nxp.com/documents/user_manual/UM10524.pdf


	led1 : pin 33 (0_12)
	led2 : pin 35 (0_14)

*/

#define NUM_LEDS 240

#include <stdio.h>

#include "drivers/usb_cdc.h"
#include "drivers/buttons.h"
#include "drivers/armmath.h"
#include "drivers/usb_cdc.h"
//#include "core/inc/lpc13uxx_timer32.h"

volatile uint32_t msTicks = 0;
static uint16_t buttonsInitialized = 0;
static uint8_t led_data[NUM_LEDS][3];
static uint32_t ledptr = 0;
static uint32_t colptr = 0;
uint32_t T0H, T0L, T1H, T1L, RES;
#define DATAPIN 11

void output_stripe_data()
{
    uint32_t led, color;
    uint8_t bit;
    uint32_t wait;
    //__disable_irq();

    // Initialize timer counter
    LPC_CT32B0->TCR = 0;
    LPC_CT32B0->TC = 0;

    for (led = 0; led < NUM_LEDS; led++)
        for (color = 0; color < 3; color++)
            for (bit = 128; bit != 0; bit = bit >> 1)
                if (led_data[led][color] & bit)
                {
                    LPC_GPIO->B0[DATAPIN] = 1;
                    LPC_CT32B0->TCR = 1;
                    LPC_CT32B0->MCR = 0x6 << 6;
                    while (LPC_CT32B0->TCR & 0x1) {}

                    LPC_GPIO->B0[DATAPIN] = 0;
                    LPC_CT32B0->TCR = 1;
                    LPC_CT32B0->MCR = 0x6 << 9;
                    while (LPC_CT32B0->TCR & 0x1) {}
                }
                else
                {
                    LPC_GPIO->B0[DATAPIN] = 1;
                    LPC_CT32B0->TCR = 1;
                    LPC_CT32B0->MCR = 0x6;
                    while (LPC_CT32B0->TCR & 0x1) {}

                    LPC_GPIO->B0[DATAPIN] = 0;
                    LPC_CT32B0->TCR = 1;
                    LPC_CT32B0->MCR = 0x6 << 3;
                    while (LPC_CT32B0->TCR & 0x1) {}
                }

    //__enable_irq();
}

void SysTick_Handler(void) {
	msTicks++;
}

void delay_ms(uint32_t ms) {
	uint32_t now = msTicks;
	while ((msTicks-now) < ms);
}

void recv_data(uint8_t* buffer, uint32_t length)
{
    uint32_t i;
    for (i = 0; i < length; i++)
    {
        led_data[ledptr][colptr] = buffer[i];
        colptr++;
        if (colptr == 3)
        {
            ledptr++;
            colptr = 0;
        }
        if (ledptr == NUM_LEDS)
        {
            ledptr = 0;
            output_stripe_data();
            usb_send(46);
        }
    }
}

int main(void) {

	SystemCoreClockUpdate();
    //SysTick_Config(SystemCoreClock/1000);

    uint32_t i;

    T0H = (SystemCoreClock * 5   ) / 20000000;
    T1H = (SystemCoreClock * 12  ) / 20000000;
    T0L = (SystemCoreClock * 20  ) / 20000000;
    T1L = (SystemCoreClock * 13  ) / 20000000;

    RES = (SystemCoreClock * 1000) / 20000000;


	// clock to GPIO
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<9);

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

    // Configure high precision wait timer
    LPC_CT32B0->CTCR = 0x00;     /* binary: 00000000 */
    LPC_CT32B0->TC = 0x00000000;     /* decimal 0 */
    LPC_CT32B0->PR = 0x00000000;     /* decimal 0 */
    LPC_CT32B0->MCR = 0x0000;     /* binary: 00000000_00000000 */
    LPC_CT32B0->MR0 = 0x00000000;     /* decimal 0 */
    LPC_CT32B0->MR1 = 0x00000000;     /* decimal 0 */
    LPC_CT32B0->MR2 = 0x00000000;     /* decimal 0 */
    LPC_CT32B0->MR3 = 0x00000000;     /* decimal 0 */
    LPC_CT32B0->CCR = 0x0000;     /* binary: 00000000_00000000 */
    LPC_CT32B0->EMR = 0x0AAF;     /* binary: 00001010_10101111 */
    LPC_CT32B0->PWMC = 0x00000000;     /* binary: 00000000_00000000_00000000_00000000 */
    LPC_CT32B0->TCR = 0x00;     /* binary: 00000000 */
    LPC_CT32B0->MR0 = T0H;
    LPC_CT32B0->MR1 = T0L;
    LPC_CT32B0->MR2 = T1H;
    LPC_CT32B0->MR3 = T1L;

    // Initialize USB CDC
    usb_init();

    for (i = 0; i < NUM_LEDS; i++)
    {
        led_data[i][0] = 32;
        led_data[i][1] = 0;
        led_data[i][2] = 0;
    }

    output_stripe_data();

    //buttons_init();
    //buttonsInitialized=1;
    //buttons_get_press( KEY_C|KEY_B|KEY_A );

    LPC_GPIO->B0[12] = 0;
    LPC_GPIO->B0[14] = 1;

    while (1)
    {}
}

