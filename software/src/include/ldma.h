#ifndef __LDMA_H__
#define __LDMA_H__

#include <em_device.h>
#include "nvic.h"

typedef struct ldma_descriptor_t ldma_descriptor_t;

struct ldma_descriptor_t
{
	uint32_t CTRL;
	uint32_t SRC;
	uint32_t DST;
	uint32_t LINK;
};

void ldma_init();

#endif