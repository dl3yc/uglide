/*
 * main tracker software
 *
 * Stefan Biereigel
 *
 */

#include <msp430.h>
#include <inttypes.h>
#include "main.h"

/* 
 * hardware initialisation
 *
 * GPIO init
 *   P2.0 UART TXD
 *   P2.1 UART RXD
 *   P2.2 SPI CLK
 *   P1.6 SPI MOSI
 *   P1.7 SPI MISO
 *   P1.3 SPI CS
 *   P1.1 SI4060 shutdown
 *   P1.2 SI4060 data
 *
 * init eUSCI_A to UART (9600/8N1)
 * init eUSCI_B to SPI 
 *
 */

void hw_init(void) {
	/* DEBUG */
	P3DIR |= BIT4 + BIT5 + BIT6 + BIT7;
	/* DCO init */
	/* SMCLK 5.37MHz, divided by 8 */
	CSCTL0_H = 0xA5;					/* write CS password */
	CSCTL1 = 0;						/* set DCO to 5.37MHz */
	CSCTL2 = SELA__DCOCLK + SELS__DCOCLK + SELM__DCOCLK;	/* DCO as ACLK, SMCLK, MCLK */
	CSCTL3 = DIVA__8 + DIVS__8 + DIVM__8;			/* divide all sources by 8 */
	CSCTL4 = XT1OFF + XT2OFF;				/* disable oscillators */
	/* GPIO init */
	P1DIR = SI_SHDN + SI_DATA + CS + MOSI;	/* GPIOs for output */
  	P1SEL1 |= MOSI + MISO;			/* USCI_B MOSI, MISO */ 
  	P1SEL0 &= ~(MOSI + MISO); 		/* USCI_B MOSI, MISO */
	
	P2DIR = TXD + SCLK;			/* GPIOs for output */
	P2SEL1 |= RXD + TXD + SCLK;		/* USCI_A RXD, TXD, USCI_B CLK */
	P2SEL0 &= ~(RXD + TXD + SCLK);		/* USCI_A RXD, TXD, USCI_B CLK */

	/* USCI_A init */
	UCA0CTL1 |= UCSWRST; 			/* reset USCI */
	UCA0CTL1 = UCSSEL_1;			/* SMCLK */ 
	UCA0BR0 = 4;				
	UCA0BR1 = 0;				
	UCA0MCTLW = (0xFD<<8)+(5<<4)+UCOS16;	/* set UCA0BRS */
	UCA0CTL1 &= ~UCSWRST;			/* release from reset */
	UCA0IE |= UCRXIE;			/* Enable RX interrupt */

	/* USCI_B init */
	//UCB0CTLW0 |= UCSWRST;			/* Put state machine in reset */
	//UCB0CTLW0 |= UCMST+UCSYNC+UCCKPL+UCMSB;	/* 3-pin, 8-bit SPI master */
						/* Clock polarity high, MSB */
	//UCB0CTLW0 |= UCSSEL_1;			/* SMCLK */
	//UCB0BR0 = 0x02;				/* divide by /2 */
	//UCB0BR1 = 0;
	//UCB0CTLW0 &= ~UCSWRST;			/* Initialize USCI state machine */

	/* Enable Interrupts */
	__bis_SR_register(GIE);			/* set interrupt enable bit */
}

uint16_t getBatteryVoltage(void) {
	uint16_t voltage;
	/* enable ADC */
	ADC10CTL0 = ADC10SHT_2 + ADC10ON;	/* ADC10ON, S&H=16 ADC clks */
	ADC10CTL1 = ADC10SHP;			/* ADCCLK = MODOSC; sampling timer */
	ADC10CTL2 = ADC10RES;			/* 10-bit conversion results */
	ADC10MCTL0 = ADC10INCH_1;		/* A1 ADC input select; Vref=AVCC */
	ADC10CTL0 = ADC10ENC + ADC10SC;		/* Sampling and conversion start */
	ADC10IE = ADC10IE0;			/* Enable ADC conv complete interrupt */
    	__bis_SR_register(CPUOFF + GIE);	/* LPM0, ADC10_ISR will force exit */
    	__no_operation();			
	
	/* take ADC reading */
	voltage = ADC10MEM0 * 32;		/* convert to mV */

	/* disable ADC */
	ADC10IE &= ~ADC10IE0;			/* Enable ADC conv complete interrupt */
	ADC10CTL0 &= ~ADC10ON;			/* ADC10 off */

	return voltage;
}

uint16_t getTemperature(void) {
	uint16_t temperature;

	/* enable ADC */
	// Configure ADC10 - Pulse sample mode; ADC10SC trigger
	ADC10CTL0 = ADC10SHT_8 + ADC10ON;	/* 16 ADC10CLKs; ADC ON,temperature sample period>30us */
	ADC10CTL1 = ADC10SHP + ADC10CONSEQ_0;	/* s/w trig, single ch/conv */
	ADC10CTL2 = ADC10RES;			/* 10-bit conversion results */
	ADC10MCTL0 = ADC10SREF_1 + ADC10INCH_10;/* ADC input ch A10 => temp sense */
  
	/* Configure internal reference */
	while(REFCTL0 & REFGENBUSY);		/* If ref generator busy, WAIT */
	REFCTL0 |= REFVSEL_0+REFON;		/* Select internal ref = 1.5V */
						/* Internal Reference ON */
	ADC10IE |= ADC10IE0;			/* enable the Interrupt */
  
	__delay_cycles(400);			/* Delay for Ref to settle */

	/* take ADC reading */
	ADC10CTL0 |= ADC10ENC + ADC10SC;        /* Sampling and conversion start */
	__bis_SR_register(CPUOFF + GIE);	/* CPU off with interrupts enabled */
	/* TODO was LPM4 - does that make a difference? */
	__no_operation();

	temperature = (ADC10MEM0 - CALADC10_15V_30C) *  (85-30) / (CALADC10_15V_85C-CALADC10_15V_30C) + 30;

	/* disable ADC */
	REFCTL0 &= ~REFON;			/* disable internal ref */
	ADC10IE &= ~ADC10IE0;			/* disable ADC conv complete interrupt */
	ADC10CTL0 &= ~ADC10ON;			/* ADC10 off */

	return temperature;
}

int main(void) {
	/* disable watchdog timer */
	WDTCTL = WDTPW + WDTHOLD;
	/* init all hardware components */
	hw_init();
	while(1);

}

/* USCI A0 ISR
 *
 * USCI A is UART. RX appends incoming bytes to the NMEA buffer 
 *
 */
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	switch(UCA0IV) {
		case 0:						/* Vector 0 - no interrupt */
			break;
		case 2:						/* Vector 2 - RXIFG */
			while (!(UCA0IFG&UCTXIFG));             /* USCI_A0 TX buffer ready */
			UCA0TXBUF = UCA0RXBUF;                  /* TX -> RXed character */
			break;
		case 4:						/* Vector 4 - TXIFG */
			break;                             
		default: 
			break;  
	}
}

/* ADC10 ISR
 *
 * just resumes CPU operation, as ADC conversions are done in CPUOFF-state 
 *
 */
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  switch(ADC10IV)
  {
    case  0: break;                          // No interrupt
    case  2: break;                          // conversion result overflow
    case  4: break;                          // conversion time overflow
    case  6: break;                          // ADC10HI
    case  8: break;                          // ADC10LO
    case 10: break;                          // ADC10IN
    case 12: __bic_SR_register_on_exit(CPUOFF);                                              
             break;                          // Clear CPUOFF bit from 0(SR)                         
    default: break; 
  }  
}

