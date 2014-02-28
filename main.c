#include "main.h"

/*

	data sheet  : http://www.nxp.com/documents/data_sheet/LPC1315_16_17_45_46_47.pdf
	user manual : http://www.nxp.com/documents/user_manual/UM10524.pdf


	led1 : pin 33 (0_12)
	led2 : pin 35 (0_14)

*/

#define NUM_LEDS 240

#include <stdio.h>

#include "drivers/usb_cdc.h"
#include "drivers/armmath.h"
#include "core/inc/lpc13uxx_uart.h"
//#include "core/inc/lpc13uxx_timer32.h"

volatile uint32_t msTicks = 0;
static uint8_t led_data[NUM_LEDS][3];
uint32_t T0H, T0L, T1H, T1L;
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
}

void delay_ms(uint32_t ms) {
	uint32_t now = msTicks;
	while ((msTicks-now) < ms);
}

int main(void) {

	SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock/1000);

    uint32_t i;

    T0H = 18;   // Actual: 18
    T1H = 43;   // Actual: 43.2
    T0L = 72;   // Actual: 72
    T1L = 47;   // Actual: 46.8

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
            led_data[i][1] = 32;
            led_data[i][2] = 0;
        }
    }
    LPC_GPIO->B0[12] = 0;
    output_stripe_data();
    LPC_GPIO->B0[14] = 0;
    UARTInit(1500000);

    uint32_t idx_led = 0;
    uint32_t idx_col = 0;

    while (1)
    {
        LPC_USART->FCR |= 0x2;  // Clear out FIFO
        LPC_GPIO->B0[12] = 0;
        //UARTSend(&pt, 1);       // Transmit ready signal
        while (idx_led < NUM_LEDS)
        {
            if (LPC_USART->LSR & 0x1)   // There is data available
                led_data[idx_led][idx_col++] = LPC_USART->RBR;
            if (idx_col == 3)
            {
                idx_led++;
                idx_col = 0;
            }
        }

        LPC_GPIO->B0[12] = 1;
        output_stripe_data();
        idx_led = 0;
    }
}

