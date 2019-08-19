#ifndef FERRIS_SERVICE_H
#define FERRIS_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "ble.h"
#include "ble_gatts.h"

typedef struct {
  uint8_t uuid_type;       /**< UUID type for Ferris Service Base UUID. */
  uint16_t service_handle; /**< Handle of Ferris Service (as provided by the BLE stack). */
  uint16_t conn_handle;

  // sample interval
  uint16_t sample_interval;
  ble_gatts_char_handles_t sample_interval_char_handle;
  int16_t skiped_report;
  int16_t mandatory_report_remain;

  // battery voltage
  uint16_t *p_battery_voltage;
  ble_gatts_char_handles_t battery_voltage_handle;

  // acceleration
  uint8_t *p_acceleration_data;
  ble_gatts_char_handles_t acc_char_handle;
  bool acceleration_notification;
  float last_report_acc[3];

} ferris_service_t;

typedef struct {
  uint8_t *p_acceleration_data;
  uint16_t *p_battery_voltage;
} ferris_service_init_t;

uint32_t ferris_service_init(ferris_service_t *p_ferris_service, ferris_service_init_t *p_ferris_service_init);

void ferris_on_ble_evt(ferris_service_t *p_nus, ble_evt_t *p_ble_evt);

uint32_t ferris_acceleration_send(ferris_service_t *p_ferris_service);

#endif