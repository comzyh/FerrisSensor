#ifndef FERRIS_SERVICE_H
#define FERRIS_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "ble_gatts.h"

typedef struct {
  uint8_t uuid_type;       /**< UUID type for Ferris Service Base UUID. */
  uint16_t service_handle; /**< Handle of Ferris Service (as provided by the BLE stack). */
  ble_gatts_char_handles_t acc_char_handle;
  uint8_t *p_acceleration_data;
  bool acceleration_notification;

} ferris_service_t;

typedef struct {
  uint8_t *p_acceleration_data;
} ferris_service_init_t;

uint32_t ferris_service_init(ferris_service_t *p_ferris_service, ferris_service_init_t *p_ferris_service_init);

#endif