#include "ferris_service.h"
#include "ble.h"
#include "ble_gatts.h"

#include <string.h>

const ble_uuid128_t ferris_uuid = {{0x9e, 0x5e, 0xaa, 0xf7, 0x4d, 0x9c, 0x47, 0xdc, 0x93, 0xad, 0x2a, 0xf9, 0x5b, 0x6b, 0x22, 0xa2}};
uint8_t char_acc_desc[] = "Acceleration raw data";
uint32_t ferris_add_acc_char(ferris_service_t *p_ferris_service) {

  uint32_t err_code;

  // CCCD metadata. Such as notification permission

  ble_gatts_attr_md_t cccd_md;
  memset(&cccd_md, 0, sizeof(cccd_md));

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

  cccd_md.vloc = BLE_GATTS_VLOC_STACK;

  // characteristic metadata

  ble_gatts_char_md_t char_md;
  memset(&char_md, 0, sizeof(char_md));
  char_md.char_props.read = 1;
  char_md.char_props.notify = 1;
  char_md.p_char_user_desc = char_acc_desc;
  char_md.char_user_desc_max_size = sizeof(char_acc_desc);
  char_md.char_user_desc_size = sizeof(char_acc_desc);
  // char_md.p_user_desc_md = &attr_md;
  char_md.p_cccd_md = &cccd_md;

  // characteristic uuid

  ble_uuid_t ble_uuid;
  ble_uuid.type = p_ferris_service->uuid_type;
  ble_uuid.uuid = 0x6050;

  // characteristic attrs. such as permission
  ble_gatts_attr_md_t attr_md;

  memset(&attr_md, 0, sizeof(attr_md));

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);

  attr_md.vloc = BLE_GATTS_VLOC_STACK;
  attr_md.vloc = BLE_GATTS_VLOC_USER;
  attr_md.rd_auth = 0;
  attr_md.wr_auth = 0;
  attr_md.vlen = 1;

  // characteristic value

  ble_gatts_attr_t attr_char_value;
  memset(&attr_char_value, 0, sizeof(attr_char_value));

  attr_char_value.p_uuid = &ble_uuid;
  attr_char_value.p_attr_md = &attr_md;
  attr_char_value.init_len = sizeof(uint8_t) * 6;
  attr_char_value.init_offs = 0;
  attr_char_value.max_len = sizeof(uint8_t) * 6;
  attr_char_value.p_value = p_ferris_service->p_acceleration_data;

  err_code = sd_ble_gatts_characteristic_add(p_ferris_service->service_handle, &char_md, &attr_char_value, &(p_ferris_service->acc_char_handle));
  return err_code;
}

uint32_t ferris_service_init(ferris_service_t *p_ferris_service, ferris_service_init_t *p_ferris_service_init) {
  uint32_t err_code;
  p_ferris_service->p_acceleration_data = p_ferris_service_init->p_acceleration_data;
  p_ferris_service->acceleration_notification = false;

  // Add a Vendor Specific base UUID.
  // Other uuids (both service and charistracter) are based on this uuid.
  err_code = sd_ble_uuid_vs_add(&ferris_uuid, &p_ferris_service->uuid_type);
  if (err_code) {
    return err_code;
  }
  ble_uuid_t ble_uuid;

  ble_uuid.type = p_ferris_service->uuid_type;
  ble_uuid.uuid = 0;

  // add service
  err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_ferris_service->service_handle);
  if (err_code) {
    return err_code;
  }

  // add characteristic
  err_code = ferris_add_acc_char(p_ferris_service);
  return err_code;
}