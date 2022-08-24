/*******************************************************************************
* Copyright (c) 2020, STMicroelectronics - All Rights Reserved
*
* This file is part of the VL53L5CX Ultra Lite Driver and is dual licensed,
* either 'STMicroelectronics Proprietary license'
* or 'BSD 3-clause "New" or "Revised" License' , at your option.
*
********************************************************************************
*
* 'STMicroelectronics Proprietary license'
*
********************************************************************************
*
* License terms: STMicroelectronics Proprietary in accordance with licensing
* terms at www.st.com/sla0081
*
* STMicroelectronics confidential
* Reproduction and Communication of this document is strictly prohibited unless
* specifically authorized in writing by STMicroelectronics.
*
*
********************************************************************************
*
* Alternatively, the VL53L5CX Ultra Lite Driver may be distributed under the
* terms of 'BSD 3-clause "New" or "Revised" License', in which case the
* following provisions apply instead of the ones mentioned above :
*
********************************************************************************
*
* License terms: BSD 3-clause "New" or "Revised" License.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*
*******************************************************************************/
#include "platform.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "global_defines.h"

uint8_t RdByte(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t *p_value)
{
    //printf("RdBBtick\n");
	uint8_t status = 0;
	uint8_t register_address_arr[2];
	uint8_t data_read[1];
	
    register_address_arr[0] = (RegisterAdress >> 8);
	register_address_arr[1] = RegisterAdress & 0xFF;

    i2c_cmd_handle_t i2c_handle = i2c_cmd_link_create();
    status |= i2c_master_start(i2c_handle);
    status |= i2c_master_write_byte(i2c_handle, p_platform->address | I2C_MASTER_WRITE, true);
    status |= i2c_master_write(i2c_handle, register_address_arr, 2, true);
	status |= i2c_master_stop(i2c_handle);
	status |= i2c_master_cmd_begin(p_platform->port, i2c_handle, 1000 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(i2c_handle);

	i2c_handle = i2c_cmd_link_create();
    status |= i2c_master_start(i2c_handle);
	status |= i2c_master_write_byte(i2c_handle, p_platform->address | I2C_MASTER_READ, true);
    status |= i2c_master_read_byte(i2c_handle, (uint8_t *) &data_read, I2C_MASTER_NACK);
    status |= i2c_master_stop(i2c_handle);
    status |= i2c_master_cmd_begin(p_platform->port, i2c_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(i2c_handle);

	*p_value = data_read[0];

	return status;
}

uint8_t RdMulti(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t *p_values,
		uint32_t size)
{
	//printf("Rd_Multi_tick\n");
	uint8_t status = 0;
	uint8_t register_address_arr[2];
  
    register_address_arr[0] = (RegisterAdress >> 8);
	register_address_arr[1] = RegisterAdress & 0xFF;

    i2c_cmd_handle_t i2c_handle = i2c_cmd_link_create();
    status |= i2c_master_start(i2c_handle);
    status |= i2c_master_write_byte(i2c_handle, p_platform->address | I2C_MASTER_WRITE, true);
    status |= i2c_master_write(i2c_handle, register_address_arr, 2, true);
	status |= i2c_master_stop(i2c_handle);
	status |= i2c_master_cmd_begin(p_platform->port, i2c_handle, 1000 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(i2c_handle);
	
	i2c_handle = i2c_cmd_link_create();
    status |= i2c_master_start(i2c_handle);
	status |= i2c_master_write_byte(i2c_handle, p_platform->address | I2C_MASTER_READ, true);
    status |= i2c_master_read(i2c_handle, p_values, size,  I2C_MASTER_LAST_NACK);
    status |= i2c_master_stop(i2c_handle);
    status |= i2c_master_cmd_begin(p_platform->port, i2c_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(i2c_handle);

	return status;
}


uint8_t WrByte(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t value)
{
	uint8_t register_address_arr_and_value[3];
	uint8_t status = 0;

    //printf("wrBtick\n");

    register_address_arr_and_value[0] = (RegisterAdress >> 8) & 0xFF; //Address at device
	register_address_arr_and_value[1] = RegisterAdress & 0xFF; //Address at device
	register_address_arr_and_value[2] = value & 0xFF; //Data to send

    i2c_cmd_handle_t i2c_handle = i2c_cmd_link_create();
    status |= i2c_master_start(i2c_handle);
    status |= i2c_master_write_byte(i2c_handle, p_platform->address | I2C_MASTER_WRITE, true);
    status |= i2c_master_write(i2c_handle, register_address_arr_and_value, 3, true);
    status |= i2c_master_stop(i2c_handle);
    status |= i2c_master_cmd_begin(p_platform->port, i2c_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(i2c_handle);

	return status;
}

uint8_t WrMulti(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t *p_values,
		uint32_t size)
{
	//printf("Wr_Multi_tick\n");

    uint8_t status = 0;
	uint8_t register_address_arr[2];

    register_address_arr[0] = (RegisterAdress >> 8) & 0xFF; //Address at device
	register_address_arr[1] = RegisterAdress & 0xFF; //Address at device

    i2c_cmd_handle_t i2c_handle = i2c_cmd_link_create();
    status |= i2c_master_start(i2c_handle);
	status |= i2c_master_write_byte(i2c_handle, p_platform->address | I2C_MASTER_WRITE, true);
    status |= i2c_master_write(i2c_handle, register_address_arr, 2, true);
	status |= i2c_master_write(i2c_handle, p_values, size, true);
    status |= i2c_master_stop(i2c_handle);
    status |= i2c_master_cmd_begin(p_platform->port, i2c_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(i2c_handle);

	return status;
}


uint8_t Reset_Sensor(
		VL53L5CX_Platform *p_platform)
{
	uint8_t status = 0;
		
    status |= gpio_set_level(LP_1_PIN, OFF);
    status |= gpio_set_level(PWR_ENABLE_1_PIN, OFF);
	WaitMs(p_platform, 100);

    status |= gpio_set_level(LP_1_PIN, ON);
    gpio_set_level(PWR_ENABLE_1_PIN, ON);
	WaitMs(p_platform, 100);

	return status;
}

void SwapBuffer(
		uint8_t 		*buffer,
		uint16_t 	 	 size)
{
	uint32_t i, tmp;
	
	/* Example of possible implementation using <string.h> */
	for(i = 0; i < size; i = i + 4) 
	{
		tmp = (
		  buffer[i]<<24)
		|(buffer[i+1]<<16)
		|(buffer[i+2]<<8)
		|(buffer[i+3]);
		
		memcpy(&(buffer[i]), &tmp, 4);
	}
}	

uint8_t WaitMs(
		VL53L5CX_Platform *p_platform,
		uint32_t TimeMs)
{
    vTaskDelay(TimeMs / portTICK_PERIOD_MS);
	return 0;
}
