#include "main.h"
#include "drivers/usb_cdc.h"

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
uint32_t T0H, T0L, T1H, T1L;
#define DATAPIN 11

volatile uint8_t curbyte;
volatile uint8_t disp_buf[NUM_BYTE];
volatile uint16_t outptr = 0;
volatile uint8_t bitptr = 0x80;
volatile uint8_t nextval = 1;
volatile uint8_t curbyte = 0;
volatile uint8_t nn = 0;

void CT32B0_OutputHandler(void)
{
    if (LPC_CT32B0->IR & 0x1) LPC_CT32B0->IR |= 0x1;
    if (LPC_CT32B0->IR & 0x2) LPC_CT32B0->IR |= 0x2;
    if (LPC_CT32B0->IR & 0x4) LPC_CT32B0->IR |= 0x4;
    if (LPC_CT32B0->IR & 0x8) LPC_CT32B0->IR |= 0x8;
    LPC_GPIO->B0[DATAPIN] = nextval;
    if (nextval)
    {
        if (curbyte & bitptr)
            LPC_CT32B0->MCR = 0x03 << 6;
        else
            LPC_CT32B0->MCR = 0x03;
        nextval = 0;
    }
    else
    {
        if (curbyte & bitptr)
            LPC_CT32B0->MCR = 0x03 << 9;
        else
            LPC_CT32B0->MCR = 0x03 << 3;
        bitptr = bitptr / 2;
        nextval = 1;
    }

    if (!bitptr)
    {
        LPC_GPIO->B0[14] = 1;
        bitptr = 128;
        outptr++;
        curbyte = disp_buf[outptr];
    }

    if (outptr == NUM_BYTE)
    {
        LPC_CT32B0->TCR = 0;
    }

    nn++;
    if (nn > 2)
        LPC_GPIO->B0[12] = 1;
}

void output_stripe_data(void)
{
    LPC_CT32B0->TC = 0;
    bitptr = 128;
    outptr = 0;
    nextval = 0;
    curbyte = disp_buf[0];

    // We need to transmit the first half-bit to bootstrap the timer
    if (curbyte & 0x80)
        LPC_CT32B0->MCR = 0x03 << 6;
    else
        LPC_CT32B0->MCR = 0x03;

    LPC_CT32B0->TCR = 1;
    LPC_GPIO->B0[DATAPIN] = 1;
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
        inp_buf[ledptr][colptr] = buffer[i];
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
    SysTick_Config(SystemCoreClock/1000);

    uint32_t i;

    T0H = 18;   // Actual: 18
    T1H = 43;   // Actual: 43.2
    T0L = 72;   // Actual: 72
    T1L = 47;   // Actual: 46.8

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

    NVIC_EnableIRQ(CT32B0_IRQn);
    NVIC_SetPriority(CT32B0_IRQn, 0);

    // Initialize USB CDC
    usb_init();

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
    UARTInit(1500000);

	buttons_init();
	buttonsInitialized=1;
    buttons_get_press( KEY_C|KEY_B|KEY_A );
    uint32_t idx_led = 0;
    uint32_t idx_col = 0;
    uint8_t pt = 48;

    while (1)
    {
        LPC_USART->FCR |= 0x2;  // Clear out FIFO
        LPC_GPIO->B0[12] = 0;
        UARTSend(&pt, 1);       // Transmit ready signal
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

