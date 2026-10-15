/*
 * LPS22.c
 *
 *  Created on: Feb 17, 2026
 *      Author: marcr
 */

#include "stm32l4xx_hal.h"

extern I2C_HandleTypeDef hi2c2;

void LSM6DSL_Init(){

	uint8_t buffer[1];

	buffer[0] = 0x40;

	HAL_I2C_Mem_Write(&hi2c2, 0xD4, 0x10, I2C_MEMADD_SIZE_8BIT, buffer, 1, 1000);

}

int16_t LSM6DSL_ReadAccel(uint8_t axxis) {
    uint8_t buffer[2];
    uint8_t reg_addr;

    if(axxis == 0) reg_addr = 0x28;
    else if(axxis == 1) reg_addr = 0x2A;
    else if(axxis == 2) reg_addr = 0x2C;
    else return 0;

    HAL_I2C_Mem_Read(&hi2c2, 0xD4, reg_addr, I2C_MEMADD_SIZE_8BIT, buffer, 2, 100);

    int16_t accel = ((int16_t)(buffer[1] << 8) | buffer[0]) * 0.061f;
    return accel;
}
