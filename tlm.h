#ifndef __tlm_h__
#define __tlm_h__

#include <inttypes.h>
#include "main.h"

#define	BLIP_FACTOR	5

void tx_blips(uint8_t reset);
void init_tx_buffer(void);
void prepare_tx_buffer(void);

#endif
