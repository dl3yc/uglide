/*
 * main tracker software
 *
 * Stefan Biereigel
 *
 */

#ifndef MAIN_H_
#define MAIN_H_

/* payload name */
#define PAYLOAD_NAME "0x07"
/* payload telemetry interval */
#define TLM_INTERVAL 	44
/* #define TLM_DOMINOEX */
#define TLM_APRS
/* telemetry string prefix for RX syncronisation */
#define SYNC_PREFIX		"   $$"
/* telemetry string postfix for tlm parser */
#define TX_BUF_POSTFIX		"\n\n"

/* number of idle bits to transmit before beginning tlm string tx */
#define NUM_IDLE_BITS	32

/* length of non-GPS-telemetry fields */
#define VOLT_LENGTH		4
#define TEMP_LENGTH		2
#define CHECKSUM_LENGTH		4
/* sentence id is variable length */
#define SENT_ID_LENGTH_MAX	5

#define TX_BUF_POSTFIX_LENGTH	sizeof(TX_BUF_POSTFIX) - 1
/* offset from buffer start to telemetry data */
#define TX_BUF_START_OFFSET 	sizeof(SYNC_PREFIX "$$" PAYLOAD_NAME ",") - 1
#define TX_BUF_SENT_ID_START	TX_BUF_START_OFFSET
#define TX_BUF_TIME_START	TX_BUF_SENT_ID_START + tlm_sent_id_length + 1
#define TX_BUF_LAT_START	TX_BUF_TIME_START + TIME_LENGTH + 1
/* lat and lon fields are one char longer for +/- */
#define TX_BUF_LON_START	TX_BUF_LAT_START + LAT_LENGTH + 1 + 1
#define TX_BUF_ALT_START	TX_BUF_LON_START + LON_LENGTH + 1 + 1
#define TX_BUF_SAT_START	TX_BUF_ALT_START + tlm_alt_length + 1
#define TX_BUF_VOLT_START	TX_BUF_SAT_START + SAT_LENGTH + 1
/* temperature field is one char longer for +/- */
#define TX_BUF_TEMP_START	TX_BUF_VOLT_START + VOLT_LENGTH + 1
#define TX_BUF_CHECKSUM_START	TX_BUF_TEMP_START + TEMP_LENGTH + 1 + 1
#define TX_BUF_POSTFIX_START	TX_BUF_CHECKSUM_START + CHECKSUM_LENGTH
#define TX_BUF_FRAME_END	TX_BUF_POSTFIX_START + sizeof(TX_BUF_POSTFIX) - 1;
/* checksum parameters */
#define TX_BUF_CHECKSUM_BEGIN	sizeof(SYNC_PREFIX "$$") - 1
#define TX_BUF_CHECKSUM_END	TX_BUF_CHECKSUM_START - 1

#define TX_BUF_MAX_LENGTH	sizeof(SYNC_PREFIX "$$" PAYLOAD_NAME) - 1 + 1 + \
				SENT_ID_LENGTH_MAX + 1 + TIME_LENGTH + 1 + LAT_LENGTH + 1 + LON_LENGTH + 1 + \
				ALT_LENGTH_MAX + 1 + SAT_LENGTH + 1 + VOLT_LENGTH + 1 + TEMP_LENGTH + \
				sizeof("*") - 1 + CHECKSUM_LENGTH + TX_BUF_POSTFIX_LENGTH

/* buffer sizes */
#define NMEA_BUF_SIZE	83

/* Port 1 */
#define LED_A	BIT0
#define LED_K	BIT1
#define ADC_IN	BIT2
#define SI_SHDN	BIT3
#define SI_DATA	BIT4
#define MOSI	BIT6
#define MISO	BIT7

/* Port 2 */
#define RXD	BIT1
#define TXD	BIT0
#define SCLK	BIT2

/* Port J */
#define CS	BIT0

/* Timer compare defitions */
/* as some values need fine tuning to keep error in a margin and are constrained, */
/* these should be recalculated manually when the CPU frequency is changed by design */
#define N_APRS_NCO	208		/* prescaler for APRS sample clock, matlab script for min. error */
#define N_APRS_BAUD	852		/* 16367600/16/1200Hz = 852,48 */

#ifdef TLM_DOMINOEX
/* this should not be needed as the overflow interrupt of timer a is used anyway */
#define N_TLM	65536 - 1		/* 16367600/2^20 = 15,609 = DomEX baud rate (for coherency) */
#define TLM_HZ	16			/* tlm rate ~ 16 Hz */
#else
#define N_TLM	10230			/* 16367600/16/100 Hz = 10229,75 */
#define TLM_HZ	100			/* tlm rate ~ 100 Hz */
#endif

/* ADC calibration locations */
#define CALADC10_15V_30C  *((unsigned int *)0x1A1A)   // Temperature Sensor Calibration-30 C
#define CALADC10_15V_85C  *((unsigned int *)0x1A1C)   // Temperature Sensor Calibration-85 C

#endif
