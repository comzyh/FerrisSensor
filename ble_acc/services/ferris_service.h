#ifndef FERRIS_SERVICE_H
#define FERRIS_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "ble.h"
#include "ble_gatts.h"

typedef struct {
  uint8_t uuid_type;       /**< UUID type for Ferris Service Base UUID. */
  uint16_t service_handle; /**< Handle of Ferris Service (as provided by the BLE stack). */
  ble_gatts_char_handles_t acc_char_handle;
  uint16_t conn_handle;

  bool acceleration_notification;

  ble_gatts_char_handles_t sample_interval_char_handle;
  uint16_t sample_interval;

  uint8_t *p_acceleration_data;
} ferris_service_t;

typedef struct {
  uint8_t *p_acceleration_data;
} ferris_service_init_t;

uint32_t ferris_service_init(ferris_service_t *p_ferris_service, ferris_service_init_t *p_ferris_service_init);

void ferris_on_ble_evt(ferris_service_t *p_nus, ble_evt_t *p_ble_evt);

uint32_t ferris_acceleration_send(ferris_service_t *p_ferris_service);

#endif