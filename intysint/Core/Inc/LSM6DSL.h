#ifndef INC_LSM6DSL_H_
#define INC_LSM6DSL_H_

#include "stm32l4xx_hal.h"

typedef struct {
    float x;
    float y;
    float z;
} AxesRaw_t;

void LSM6DSL_Init(void);
int16_t LSM6DSL_ReadAccel(uint8_t axxis);

#endif /* INC_LSM6DSL_H_ */
