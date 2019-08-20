#include <string.h>

#include "ble.h"
#include "ble_gatts.h"
#include "ble_srv_common.h"
#include "ferris_service.h"

const ble_uuid128_t ferris_uuid = {{0x9e, 0x5e, 0xaa, 0xf7, 0x4d, 0x9c, 0x47, 0xdc, 0x93, 0xad, 0x2a, 0xf9, 0x5b, 0x6b, 0x22, 0xa2}};
const uint16_t acc_data_len     = 6;
#define sin5deg = 0.08715574274765817;

const uint8_t char_acc_desc[]             = "Acceleration raw data, [-2G, 2G], in {X_H, X_L, Y_H, Y_L, Z_H, Z_L} format";
const uint8_t char_sample_interval_desc[] = "Sample interval in ms.";
const uint8_t char_battery_voltage_desc[] = "Battery voltage in mV.";

// Acceleration characteristic
uint32_t ferris_add_accel_char(ferris_service_t *p_ferris_service) {

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
  char_md.char_props.read         = 1;
  char_md.char_props.notify       = 1;
  char_md.p_char_user_desc        = (uint8_t *)char_acc_desc;
  char_md.char_user_desc_max_size = sizeof(char_acc_desc);
  char_md.char_user_desc_size     = sizeof(char_acc_desc);
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

  attr_md.vloc    = BLE_GATTS_VLOC_USER;
  attr_md.rd_auth = 0;
  attr_md.wr_auth = 0;
  attr_md.vlen    = 0;

  // characteristic value

  ble_gatts_attr_t attr_char_value;
  memset(&attr_char_value, 0, sizeof(attr_char_value));

  attr_char_value.p_uuid    = &ble_uuid;
  attr_char_value.p_attr_md = &attr_md;
  attr_char_value.init_len  = sizeof(uint8_t) * acc_data_len;
  attr_char_value.init_offs = 0;
  attr_char_value.max_len   = sizeof(uint8_t) * acc_data_len;
  attr_char_value.p_value   = p_ferris_service->p_acceleration_data;

  err_code = sd_ble_gatts_characteristic_add(p_ferris_service->service_handle, &char_md, &attr_char_value, &(p_ferris_service->acc_char_handle));
  return err_code;
}

// Add normal value characteristic
uint32_t ferris_add_normal_characteristic(ferris_service_t *p_ferris_service, ble_gatts_char_handles_t *p_handles,
                                          uint8_t *p_value, uint16_t value_len,
                                          uint16_t uuid, const uint8_t *char_user_desc, uint16_t char_user_desc_size,
                                          bool readonly, uint8_t format) {

  uint32_t err_code;

  // characteristic metadata

  ble_gatts_char_md_t char_md;
  memset(&char_md, 0, sizeof(char_md));
  char_md.char_props.read         = 1;
  char_md.char_props.notify       = 0;
  char_md.char_props.write        = readonly ? 0 : 1;
  char_md.p_char_user_desc        = (uint8_t *)char_user_desc;
  char_md.char_user_desc_max_size = char_user_desc_size;
  char_md.char_user_desc_size     = char_user_desc_size;

  ble_gatts_char_pf_t char_pf;
  memset(&char_pf, 0, sizeof(char_pf));
  if (format != 0) {
    char_pf.format    = format;
    char_md.p_char_pf = &char_pf;
  }

  // characteristic uuid

  ble_uuid_t ble_uuid;
  ble_uuid.type = p_ferris_service->uuid_type;
  ble_uuid.uuid = uuid;

  // characteristic attrs. such as permission
  ble_gatts_attr_md_t attr_md;

  memset(&attr_md, 0, sizeof(attr_md));

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
  if (readonly) {
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
  } else {
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
  }

  attr_md.vloc    = BLE_GATTS_VLOC_USER;
  attr_md.rd_auth = 0;
  attr_md.wr_auth = 0;
  attr_md.vlen    = false;

  // characteristic value

  ble_gatts_attr_t attr_char_value;
  memset(&attr_char_value, 0, sizeof(attr_char_value));

  attr_char_value.p_uuid    = &ble_uuid;
  attr_char_value.p_attr_md = &attr_md;
  attr_char_value.init_len  = value_len;
  attr_char_value.init_offs = 0;
  attr_char_value.max_len   = value_len;
  attr_char_value.p_value   = p_value;

  err_code = sd_ble_gatts_characteristic_add(p_ferris_service->service_handle, &char_md, &attr_char_value, p_handles);
  return err_code;
}

uint32_t ferris_service_init(ferris_service_t *p_ferris_service, ferris_service_init_t *p_ferris_service_init) {
  uint32_t err_code;
  p_ferris_service->p_acceleration_data       = p_ferris_service_init->p_acceleration_data;
  p_ferris_service->p_battery_voltage         = p_ferris_service_init->p_battery_voltage;
  p_ferris_service->acceleration_notification = false;
  p_ferris_service->sample_interval           = 200;

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

  // add acceleration characteristic
  err_code = ferris_add_accel_char(p_ferris_service);
  if (err_code) {
    return err_code;
  }

  // add sample_interval
  err_code = ferris_add_normal_characteristic(p_ferris_service, &(p_ferris_service->sample_interval_char_handle),
                                              (uint8_t *)(&p_ferris_service->sample_interval), 2,
                                              ((uint16_t)('I') << 8) + 'T',
                                              char_sample_interval_desc, sizeof(char_sample_interval_desc), false,
                                              BLE_GATT_CPF_FORMAT_UINT16);
  if (err_code) {
    return err_code;
  }
  // add battery voltage
  if (p_ferris_service->p_battery_voltage != NULL) {
    err_code = ferris_add_normal_characteristic(p_ferris_service, &(p_ferris_service->battery_voltage_handle),
                                                (uint8_t *)(p_ferris_service->p_battery_voltage), 2,
                                                ((uint16_t)('B') << 8) + 'V',
                                                char_battery_voltage_desc, sizeof(char_battery_voltage_desc), true,
                                                BLE_GATT_CPF_FORMAT_UINT16);
    if (err_code) {

      return err_code;
    }
  }

  return 0;
}
void decode_acc(uint8_t *p_value, float *p_acc) {
  for (int i = 0; i < 3; i++) {
    p_acc[i] = ((float)((int16_t)uint16_big_decode(p_value + i * 2))) / 32768 * 20;
  }
}

float cross_product_length(float U[3], float V[3]) {
  float x = U[1] * V[2] - U[2] * V[1];
  float y = U[2] * V[0] - V[2] * U[0];
  float z = U[0] * V[1] - U[1] * V[0];
  return x * x + y * y + z * z;
}

uint32_t ferris_acceleration_send(ferris_service_t *p_ferris_service) {
  ble_gatts_hvx_params_t hvx_params;

  if (p_ferris_service == NULL) {
    return 0;
  }

  if ((p_ferris_service->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_ferris_service->acceleration_notification)) {
    return NRF_ERROR_INVALID_STATE;
  }

  // Skip report if the rotated angle is too low
  float acc[3];
  decode_acc(p_ferris_service->p_acceleration_data, acc);
  bool large_angle = cross_product_length(acc, p_ferris_service->last_report_acc) > 10 * 10 * 0.08715574274765817;

  if (large_angle) {
    // We send some more acceleration value after a big rotate
    p_ferris_service->mandatory_report_remain = 4;
  }

  // We skip report if angle change is not large
  if (p_ferris_service->mandatory_report_remain <= 0 && !large_angle && p_ferris_service->skiped_report < 5 * 10) {
    p_ferris_service->skiped_report++;
    return 0;
  }

  if (p_ferris_service->mandatory_report_remain) {
    p_ferris_service->mandatory_report_remain--;
  }
  p_ferris_service->skiped_report = 0;

  memcpy(p_ferris_service->last_report_acc, acc, sizeof(acc));

  // send notification
  memset(&hvx_params, 0, sizeof(hvx_params));

  hvx_params.handle = p_ferris_service->acc_char_handle.value_handle;
  hvx_params.p_data = p_ferris_service->p_acceleration_data;
  hvx_params.p_len  = (uint16_t *)&acc_data_len;
  hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

  return sd_ble_gatts_hvx(p_ferris_service->conn_handle, &hvx_params);
}

/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_ferris_service     Ferris Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_connect(ferris_service_t *p_ferris_service, ble_evt_t *p_ble_evt) {
  p_ferris_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_ferris_service     Ferris Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ferris_service_t *p_ferris_service, ble_evt_t *p_ble_evt) {
  p_ferris_service->conn_handle = BLE_CONN_HANDLE_INVALID;
}

/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the S110 SoftDevice.
 *
 * @param[in] p_ferris_service     Ferris Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ferris_service_t *p_ferris_service, ble_evt_t *p_ble_evt) {
  ble_gatts_evt_write_t *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

  if (
      (p_evt_write->handle == p_ferris_service->acc_char_handle.cccd_handle) &&
      (p_evt_write->len == 2)) {
    if (ble_srv_is_notification_enabled(p_evt_write->data)) {
      p_ferris_service->acceleration_notification = true;
      p_ferris_service->mandatory_report_remain   = 5;
      p_ferris_service->skiped_report             = 0;
    } else {
      p_ferris_service->acceleration_notification = false;
    }
  } else if ( // sample_interval
      (p_evt_write->handle == p_ferris_service->sample_interval_char_handle.value_handle) &&
      (p_evt_write->len == 2)) {
    p_ferris_service->sample_interval = *((uint16_t *)p_evt_write->data);
  }
}

void ferris_on_ble_evt(ferris_service_t *p_ferris_service, ble_evt_t *p_ble_evt) {
  if ((p_ferris_service == NULL) || (p_ble_evt == NULL)) {
    return;
  }

  switch (p_ble_evt->header.evt_id) {
  case BLE_GAP_EVT_CONNECTED:
    on_connect(p_ferris_service, p_ble_evt);
    break;

  case BLE_GAP_EVT_DISCONNECTED:
    on_disconnect(p_ferris_service, p_ble_evt);
    break;

  case BLE_GATTS_EVT_WRITE:
    on_write(p_ferris_service, p_ble_evt);
    break;

  default:
    // No implementation needed.
    break;
  }
}