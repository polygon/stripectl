#include "main.h"

/*

	data sheet  : http://www.nxp.com/documents/data_sheet/LPC1315_16_17_45_46_47.pdf
	user manual : http://www.nxp.com/documents/user_manual/UM10524.pdf


	led1 : pin 33 (0_12)
	led2 : pin 35 (0_14)

*/

#define NUM_LEDS 240

#include <stdio.h>
#include <string.h>

#include "drivers/usb_cdc.h"
#include "drivers/armmath.h"
//#include "core/inc/lpc13uxx_timer32.h"

volatile uint32_t msTicks = 0;
static uint8_t led_data[NUM_LEDS][3];
uint32_t T0H, T0L, T1H, T1L;
#define DATAPIN 9

uint32_t ledptr = 0;
uint32_t colptr = 0;

void output_stripe_data()
{
    uint32_t led, color;
    uint8_t bit;
    uint8_t out;

    for (led = 0; led < NUM_LEDS; led++)
        for (color = 0; color < 3; color++)
            for (bit = 128; bit != 0; bit = bit >> 1)
            {
                if (led_data[led][color] & bit)
                    out = 0b11110000;
                else
                    out = 0b11000000;

                while (!(LPC_SSP0->SR & 0x1)) {}  // Wait for non-full FIFO

                LPC_SSP0->DR = out; // Write next byte to FIFO
            }

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
    //y3SysTick_Config(SystemCoreClock/1000);

    uint32_t i;

    T0H = 18;   // Actual: 18
    T1H = 43;   // Actual: 43.2
    T0L = 72;   // Actual: 72
    T1L = 47;   // Actual: 46.8

    // clock to GPIO, USB and SSP0
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6) | (1<<9) | (1<<11);

    // Enable clock to SSP0 and de-assert the reset signal
    LPC_SYSCON->SSP0CLKDIV = 1;
    LPC_SYSCON->PRESETCTRL = 1;

    // Configure pin as SSP0 output
    LPC_IOCON->PIO0_9 = (0x1);

    // Initialize SSP0
    // 72MHz system clock, 400kHz target clock
    // DIV=90, MUL=4 gives 64kHz = 8*target clock
    // Set MUL=4 for 800kHz high speed mode
    // Two bits high for 0, Four bits high for 1
    LPC_SSP0->CR0 |= (0<<8);    // Prescaler 4

    // 8 Bit frames, TI format, Bus clock low between frames
    LPC_SSP0->CR0 |= (0x7) | (1<<4);

    // Enable controller in master mode
    LPC_SSP0->CR1 = 0;

    LPC_SSP0->CPSR = 22; // 15
    LPC_SSP0->IMSC = 0;
    LPC_SSP0->CR1 = (LPC_SSP0->CR1 & 0xf)|(1<<1);

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

    for (i = 0; i < NUM_LEDS; i++)
    {
        led_data[i][0] = 0;
        led_data[i][1] = 0;
        led_data[i][2] = 0;
    }
    LPC_GPIO->B0[12] = 0;
    LPC_GPIO->B0[14] = 0;

    usb_init();

    while (1)
    {}
}

