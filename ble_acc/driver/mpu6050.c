/**
 * Copyright (c) 2009 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <stdbool.h>
#include <stdint.h>

#include "mpu6050.h"
#include "nrf_drv_twi.h"

typedef __uint8_t uint8_t;
typedef __uint16_t uint16_t;
typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;

#define TWI_READ_BIT (0x01) //!< If this bit is set in the address field, transfer direction is from slave to master.

#define TWI_ISSUE_STOP ((bool)true)       //!< Parameter for @ref twi_master_transfer
#define TWI_DONT_ISSUE_STOP ((bool)false) //!< Parameter for @ref twi_master_transfer

/*lint ++flb "Enter library region" */

#define ADDRESS_WHO_AM_I (0x75U)          // !< WHO_AM_I register identifies the device. Expected value is 0x68.
#define ADDRESS_SIGNAL_PATH_RESET (0x68U) // !<

static const uint8_t expected_who_am_i = 0x68U; // !< Expected value to get from WHO_AM_I register.
static uint8_t m_device_address;                // !< Device address in bits [7:1]
static nrf_drv_twi_t *m_p_twi;

bool mpu6050_init(nrf_drv_twi_t *p_twi, uint8_t device_address) {
  uint32_t ret_code;

  m_p_twi = p_twi;
  m_device_address = (uint8_t)(device_address << 1);

  // Do a reset on signal paths
  uint8_t reset_value = 0x04U | 0x02U | 0x01U; // Resets gyro, accelerometer and temperature sensor signal paths.
  ret_code = mpu6050_register_write(ADDRESS_SIGNAL_PATH_RESET, reset_value) == 0;

  if (!ret_code != NRF_SUCCESS)
    for (;;) {
    }
  // Read and verify product ID
  ret_code = mpu6050_verify_product_id();

  return ret_code == NRF_SUCCESS;
}

bool mpu6050_verify_product_id(void) {
  uint8_t who_am_i;

  if (mpu6050_register_read(ADDRESS_WHO_AM_I, &who_am_i, 1)) {
    if (who_am_i != expected_who_am_i) {
      return false;
    } else {
      return true;
    }
  } else {
    return false;
  }
}

uint32_t mpu6050_register_write(uint8_t register_address, uint8_t value) {
  uint8_t w2_data[2];

  w2_data[0] = register_address;
  w2_data[1] = value;
  return nrf_drv_twi_tx(m_p_twi, m_device_address, w2_data, 2, TWI_ISSUE_STOP);
}

uint32_t mpu6050_register_read(uint8_t register_address, uint8_t *destination, uint8_t number_of_bytes) {
  uint32_t ret_code;
  ret_code = nrf_drv_twi_tx(m_p_twi, m_device_address, &register_address, 1, TWI_DONT_ISSUE_STOP);
  if (ret_code != NRF_SUCCESS) {
    return ret_code;
  }
  return nrf_drv_twi_rx(m_p_twi, m_device_address, destination, number_of_bytes);
}

/*lint --flb "Leave library region" */
