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
    printf("RdBBtick\n");
	uint8_t status = 0;
	uint8_t data_write[2];
	uint8_t data_read[1];
	
	/* Need to be implemented by customer. This function returns 0 if OK */
	//data_write[0] = (RegisterAdress >> 8) & 0xFF;
    data_write[0] = (RegisterAdress >> 8);
	data_write[1] = RegisterAdress & 0xFF;

	//status = HAL_I2C_Master_Transmit(&hi2c1, p_platform->address, data_write, 2, T_OUT);
	//status = HAL_I2C_Master_Receive(&hi2c1, p_platform->address, data_read, 1, T_OUT);
    i2c_cmd_handle_t i2c_handle = i2c_cmd_link_create();
    status |= i2c_master_start(i2c_handle);
    status |= i2c_master_write_byte(i2c_handle, p_platform->address | I2C_MASTER_READ, true);
    status |= i2c_master_write(i2c_handle, data_write, 2, true);
    status |= i2c_master_read_byte(i2c_handle, (uint8_t *) &data_read, I2C_MASTER_NACK);
    status |= i2c_master_stop(i2c_handle);
    status |= i2c_master_cmd_begin(p_platform->port, i2c_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(i2c_handle);

	*p_value = data_read[0];

	return status;
}

uint8_t WrByte(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t value)
{
	uint8_t data_write[3];
	uint8_t status = 0;

    printf("wrBtick\n");

	/* Need to be implemented by customer. This function returns 0 if OK */
    data_write[0] = (RegisterAdress >> 8) & 0xFF; //Address at device
	data_write[1] = RegisterAdress & 0xFF; //Address at device
	data_write[2] = value & 0xFF; //Data to send

	//status = HAL_I2C_Master_Transmit(&hi2c1,p_platform->address, data_write, 3, T_OUT);
    i2c_cmd_handle_t i2c_handle = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(i2c_handle));
    ESP_ERROR_CHECK(i2c_master_write_byte(i2c_handle, p_platform->address | I2C_MASTER_WRITE, true));
    ESP_ERROR_CHECK(i2c_master_write(i2c_handle, data_write, 3, true));
    ESP_ERROR_CHECK(i2c_master_stop(i2c_handle));
    ESP_ERROR_CHECK(i2c_master_cmd_begin(p_platform->port, i2c_handle, 1000 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(i2c_handle);

	return status;
}

uint8_t WrMulti(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t *p_values,
		uint32_t size)
{
    uint8_t status = 0;

    uint8_t send_buffer[65535];

    send_buffer[0] = (uint8_t) (p_platform->address | I2C_MASTER_WRITE); //Address of device
    send_buffer[1] = (RegisterAdress >> 8) & 0xFF; //Address at device
	send_buffer[2] = RegisterAdress & 0xFF; //Address at device
    memcpy(&send_buffer[3], p_values, size);
		/* Need to be implemented by customer. This function returns 0 if OK */
	//uint8_t status = HAL_I2C_Mem_Write(&hi2c1, p_platform->address, RegisterAdress,
	//								I2C_MEMADD_SIZE_16BIT, p_values, size, 65535);

    i2c_cmd_handle_t i2c_handle = i2c_cmd_link_create();
    status |= i2c_master_start(i2c_handle);
    status |= i2c_master_write(i2c_handle, send_buffer, size + 3, true);
    status |= i2c_master_stop(i2c_handle);
    status |= i2c_master_cmd_begin(p_platform->port, i2c_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(i2c_handle);

	return status;
}

uint8_t RdMulti(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t *p_values,
		uint32_t size)
{
    uint8_t status = 0;
    uint8_t data_write[3];
	/* Need to be implemented by customer. This function returns 0 if OK */
    data_write[0] = (uint8_t) (p_platform->address | I2C_MASTER_READ);
	data_write[1] = (RegisterAdress >> 8) & 0xFF;
	data_write[2] = RegisterAdress & 0xFF;
	//status = HAL_I2C_Master_Transmit(&hi2c1, p_platform->address, data_write, 2, T_OUT);
	//status += HAL_I2C_Master_Receive(&hi2c1, p_platform->address, p_values, size, T_OUT);
    i2c_cmd_handle_t i2c_handle = i2c_cmd_link_create();
    status |= i2c_master_start(i2c_handle);
    status |= i2c_master_write(i2c_handle, data_write, 4, true);
    status |= i2c_master_read(i2c_handle, p_values, size - 1,  I2C_MASTER_ACK);
    status |= i2c_master_read_byte(i2c_handle, p_values + size - 1, I2C_MASTER_NACK);
    status |= i2c_master_stop(i2c_handle);
    status |= i2c_master_cmd_begin(p_platform->port, i2c_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(i2c_handle);

	return status;
}

uint8_t Reset_Sensor(
		VL53L5CX_Platform *p_platform)
{
	uint8_t status = 0;
	
	/* (Optional) Need to be implemented by customer. This function returns 0 if OK */
	
    gpio_set_level(LP_1_PIN, OFF);
    gpio_set_level(PWR_ENABLE_1_PIN, OFF);
	WaitMs(p_platform, 100);

    gpio_set_level(LP_1_PIN, ON);
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
	uint8_t status = 255;

	/* Need to be implemented by customer. This function returns 0 if OK */
    vTaskDelay(1 / portTICK_PERIOD_MS);
    status = 0;

	return 0;
}
