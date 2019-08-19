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
#include "mpu_reg.h"
#include "nrf_drv_twi.h"

typedef __uint8_t uint8_t;
typedef __uint16_t uint16_t;
typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;

#define ADDRESS_WHO_AM_I (0x75U)          // !< WHO_AM_I register identifies the device. Expected value is 0x68.
#define ADDRESS_SIGNAL_PATH_RESET (0x68U) // !<

static const uint8_t expected_who_am_i = 0x68U; // !< Expected value to get from WHO_AM_I register.
static uint8_t m_device_address;                // !< Device address in bits [7:1]
static nrf_drv_twi_t *m_p_twi;

uint32_t check_retcode(uint32_t ret_code) {
  if (ret_code != NRF_SUCCESS) {
    // for (;;) {}
  }
  return ret_code;
}

bool mpu6050_init(nrf_drv_twi_t *p_twi, uint8_t device_address) {
  uint32_t ret_code;

  m_p_twi          = p_twi;
  m_device_address = device_address;

  // Do a reset on signal paths
  uint8_t reset_value = 0x04U | 0x02U | 0x01U; // Resets gyro, accelerometer and temperature sensor signal paths.
  ret_code            = mpu6050_register_write(ADDRESS_SIGNAL_PATH_RESET, reset_value);

  if (ret_code != NRF_SUCCESS) {
    return false;
  }
  // initialize MPU6050
  ret_code = check_retcode(mpu6050_register_write(SMPLRT_DIV, SAMPLE_50HZ));
  ret_code = check_retcode(mpu6050_register_write(CONFIG, DLPF_21HZ));
  ret_code = check_retcode(mpu6050_register_write(ACCEL_CONFIG, ACCEL_FS_2g));
  // MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2 
  // The MPU-60X0 can be put into Accelerometer Only Low Power Mode using the following steps:
  // (i)   Set CYCLE bit to 1
  // (ii)  Set SLEEP bit to 0
  // (iii) Set TEMP_DIS bit to 1
  // (iv)  Set STBY_XG, STBY_YG, STBY_ZG bits to 1
  ret_code = check_retcode(mpu6050_register_write(PWR_MGMT_1, CLKSEL_PllGyroX | TEMP_DIS | CYCLE));
  ret_code = check_retcode(mpu6050_register_write(PWR_MGMT_2, gyroscope_STBY | LP_WAKE_CTRL_5));
  if (ret_code != NRF_SUCCESS) {
    return false;
  }
  // Read and verify product ID
  return mpu6050_verify_product_id();
}

bool mpu6050_verify_product_id(void) {
  uint8_t who_am_i;

  if (mpu6050_register_read(ADDRESS_WHO_AM_I, &who_am_i, 1) == NRF_SUCCESS) {
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
  return nrf_drv_twi_tx(m_p_twi, m_device_address, w2_data, 2, false);
}

uint32_t mpu6050_register_read(uint8_t register_address, uint8_t *destination, uint8_t number_of_bytes) {
  uint32_t ret_code = 0;
  ret_code          = nrf_drv_twi_tx(m_p_twi, m_device_address, &register_address, 1, false);

  if (ret_code != NRF_SUCCESS) {
    return ret_code;
  }
  ret_code = nrf_drv_twi_rx(m_p_twi, m_device_address, destination, number_of_bytes);
  return ret_code;
}

uint32_t mpu6050_read_acceleration(uint8_t *dest) {
  return mpu6050_register_read(ACCEL_XOUT_H, dest, 6);
}

uint32_t mpu6050_enter_sleep() {
  return mpu6050_register_write(PWR_MGMT_1, SLEEP);
}
uint32_t mpu6050_wake_up() {
  return mpu6050_register_write(PWR_MGMT_1, CLKSEL_PllGyroX | TEMP_DIS | CYCLE);
}

uint32_t mpu6050_set_wake_up_freq(MPU6050_WAKEUP_FREQ freq) {
  return mpu6050_register_write(PWR_MGMT_2, gyroscope_STBY | ((uint8_t)(freq & 0x3) << 6));
}
