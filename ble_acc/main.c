#include <stdbool.h>
#include <stdint.h>

#include "app_error.h"
#include "app_scheduler.h"
#include "app_timer.h"
#include "app_util_platform.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_bas.h"
#include "ble_conn_params.h"
#include "ble_conn_state.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "softdevice_handler.h"

#include "nrf_delay.h"
#include "nrf_drv_adc.h"
#include "nrf_drv_twi.h"
#include "nrf_gpio.h"

#define NRF_LOG_MODULE_NAME "FerrisApp"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "driver/mpu6050.h"

typedef __uint8_t uint8_t;
typedef __uint16_t uint16_t;
typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;

const int LED_R = 17;
const int LED_B = 19;
const int LED_G = 18;

const int LED_RGB[] = {17, 19, 18};
const uint8_t mpu6050_device_address = 0x69; // SENSOR_PRO board

const int CENTRAL_LINK_COUNT = 0;    /**< Number of central links used by the application. When changing this number
                                        remember to adjust the RAM settings*/
const int PERIPHERAL_LINK_COUNT = 1; /**< Number of peripheral links used by the application. When changing this number
                                        remember to adjust the RAM settings*/

// TWI config
#define TWI_INSTANCE_ID 0 // we are using TWI1
const int TWI_SCL_PIN = 10;
const int TWI_SDA_PIN = 9;

#define ADC_BUFFER_SIZE 10

#define APP_FEATURE_NOT_SUPPORTED BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2 /**< Reply when unsupported features are requested. */

#define DEVICE_NAME "Ferris V0.1"               /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME "NordicSemiconductor" /**< Manufacturer. Will be passed to Device Information Service. */

#define APP_ADV_FAST_INTERVAL 80   /**< The advertising interval (in units of 0.625 ms. This value corresponds to 50 ms). */
#define APP_ADV_SLOW_INTERVAL 3200 /**< Slow advertising interval (in units of 0.625 ms. This value corresponds to 2 seconds). */
#define APP_ADV_FAST_TIMEOUT 30    /**< The duration of the fast advertising period (in seconds). */
#define APP_ADV_SLOW_TIMEOUT 180   /**< The advertising timeout in units of seconds. */

// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC                                            \
  {                                                                   \
    .source = NRF_CLOCK_LF_SRC_XTAL, .rc_ctiv = 0, .rc_temp_ctiv = 0, \
    .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM                \
  }

#define APP_TIMER_PRESCALER 0     /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE 4 /**< Size of timer operation queues. */

#define MIN_CONN_INTERVAL MSEC_TO_UNITS(100, UNIT_1_25_MS) /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL MSEC_TO_UNITS(200, UNIT_1_25_MS) /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY 0                                    /**< Slave latency. */
#define CONN_SUP_TIMEOUT MSEC_TO_UNITS(4000, UNIT_10_MS)   /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT 3                                            /**< Number of attempts before giving up the connection parameter negotiation. */

static ble_uuid_t m_adv_uuids[] = {
    {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}}; /**< Universally unique service identifiers. */

static uint32_t last_error_code;
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; /**< Handle of the current connection. */
static ble_bas_t m_bas;
static nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
static nrf_adc_value_t adc_buffer[ADC_BUFFER_SIZE];

void check_error(volatile uint32_t err_code) {
  if (err_code) {
    last_error_code = err_code;
    for (int i = 0; i < 3; i++) {
      nrf_gpio_pin_set(LED_RGB[i]);
    }
    nrf_gpio_pin_clear(LED_R);

    for (;;) {
      nrf_gpio_pin_toggle(LED_R);
    }
  }
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt) {
  switch (ble_adv_evt) {
  case BLE_ADV_EVT_FAST:
    // NRF_LOG_INFO("Fast advertising\r\n");
    // nrf_gpio_pin_toggle(LED_B);
    break;
  case BLE_ADV_EVT_IDLE:
    check_error(10);
    break; // BLE_ADV_EVT_IDLE
  default:
    break;
  }
}

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void) {
  uint32_t err_code;

  ble_advdata_t advdata;
  ble_adv_modes_config_t options;

  // Build advertising data struct to pass into @ref ble_advertising_init.
  memset(&advdata, 0, sizeof(advdata));

  advdata.name_type = BLE_ADVDATA_FULL_NAME;
  advdata.include_appearance = true;
  advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
  advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
  advdata.uuids_complete.p_uuids = m_adv_uuids;

  memset(&options, 0, sizeof(options));
  options.ble_adv_fast_enabled = true;
  options.ble_adv_fast_interval = APP_ADV_FAST_INTERVAL;
  options.ble_adv_fast_timeout = APP_ADV_FAST_TIMEOUT;
  options.ble_adv_slow_enabled = true;
  options.ble_adv_slow_interval = APP_ADV_SLOW_INTERVAL;
  options.ble_adv_slow_timeout = APP_ADV_SLOW_TIMEOUT;

  err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, NULL);
  check_error(err_code);
}

static void on_ble_evt(ble_evt_t *p_ble_evt) {
  uint32_t err_code = NRF_SUCCESS;

  switch (p_ble_evt->header.evt_id) {
  case BLE_GAP_EVT_DISCONNECTED:
    NRF_LOG_INFO("Disconnected.\r\n");
    nrf_gpio_pin_set(LED_G);
    APP_ERROR_CHECK(err_code);
    break; // BLE_GAP_EVT_DISCONNECTED

  case BLE_GAP_EVT_CONNECTED:
    NRF_LOG_INFO("Connected.\r\n");
    nrf_gpio_pin_clear(LED_G);
    m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    break; // BLE_GAP_EVT_CONNECTED

  case BLE_GATTC_EVT_TIMEOUT:
    // Disconnect on GATT Client timeout event.
    NRF_LOG_DEBUG("GATT Client Timeout.\r\n");
    err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                     BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    APP_ERROR_CHECK(err_code);
    break; // BLE_GATTC_EVT_TIMEOUT

  case BLE_GATTS_EVT_TIMEOUT:
    // Disconnect on GATT Server timeout event.
    NRF_LOG_DEBUG("GATT Server Timeout.\r\n");
    err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                     BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    APP_ERROR_CHECK(err_code);
    break; // BLE_GATTS_EVT_TIMEOUT

  case BLE_EVT_USER_MEM_REQUEST:
    err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
    APP_ERROR_CHECK(err_code);
    break; // BLE_EVT_USER_MEM_REQUEST

  case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST: {
    ble_gatts_evt_rw_authorize_request_t req;
    ble_gatts_rw_authorize_reply_params_t auth_reply;

    req = p_ble_evt->evt.gatts_evt.params.authorize_request;

    if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID) {
      if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ) ||
          (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
          (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL)) {
        if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE) {
          auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
        } else {
          auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
        }
        auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
        err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                   &auth_reply);
        APP_ERROR_CHECK(err_code);
      }
    }
  } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST

#if (NRF_SD_BLE_API_VERSION == 3)
  case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
    err_code = sd_ble_gatts_exchange_mtu_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                               NRF_BLE_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
    break; // BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST
#endif

  default:
    // No implementation needed.
    break;
  }
}

/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t *p_ble_evt) {
  /** The Connection state module has to be fed BLE events in order to function correctly
     * Remember to call ble_conn_state_on_ble_evt before calling any ble_conns_state_* functions. */
  ble_conn_state_on_ble_evt(p_ble_evt);

  ble_conn_params_on_ble_evt(p_ble_evt);

  on_ble_evt(p_ble_evt);
  ble_advertising_on_ble_evt(p_ble_evt);
  /*YOUR_JOB add calls to _on_ble_evt functions from each service your application is using
       ble_xxs_on_ble_evt(&m_xxs, p_ble_evt);
       ble_yys_on_ble_evt(&m_yys, p_ble_evt);
     */
  // battery
  ble_bas_on_ble_evt(&m_bas, p_ble_evt);
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void) {
  uint32_t err_code;

  nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

  // Initialize the SoftDevice handler module.
  SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

  ble_enable_params_t ble_enable_params;
  err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT, PERIPHERAL_LINK_COUNT, &ble_enable_params);
  check_error(err_code);

  // Check the ram settings against the used number of links
  CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT, PERIPHERAL_LINK_COUNT);

  // Enable BLE stack.
  err_code = softdevice_enable(&ble_enable_params);
  check_error(err_code);

  // Register with the SoftDevice handler module for BLE events.
  err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
  check_error(err_code);
}
/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void) {
  uint32_t err_code;
  ble_gap_conn_params_t gap_conn_params;
  ble_gap_conn_sec_mode_t sec_mode;

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

  err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)DEVICE_NAME, strlen(DEVICE_NAME));
  check_error(err_code);

  /* YOUR_JOB: Use an appearance value matching the application's use case.
       err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_);
       APP_ERROR_CHECK(err_code); */

  memset(&gap_conn_params, 0, sizeof(gap_conn_params));

  gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
  gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
  gap_conn_params.slave_latency = SLAVE_LATENCY;
  gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

  err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
  check_error(err_code);
}

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void) {
  uint32_t err_code;

  ble_bas_init_t bas_init;
  memset(&bas_init, 0, sizeof(bas_init));

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.cccd_write_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init.battery_level_char_attr_md.write_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_report_read_perm);

  bas_init.support_notification = false;
  bas_init.initial_batt_level = 50;
  bas_init.p_report_ref = NULL;
  bas_init.evt_handler = NULL;

  err_code = ble_bas_init(&m_bas, &bas_init);
  check_error(err_code);
}

/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t *p_evt) {
  uint32_t err_code;

  if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
    err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
    APP_ERROR_CHECK(err_code);
  }
}

/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error) {
  APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void) {
  uint32_t err_code;
  ble_conn_params_init_t cp_init;

  memset(&cp_init, 0, sizeof(cp_init));

  cp_init.p_conn_params = NULL;
  cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
  cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
  cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
  cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
  cp_init.disconnect_on_fail = false;
  cp_init.evt_handler = on_conn_params_evt;
  cp_init.error_handler = conn_params_error_handler;

  err_code = ble_conn_params_init(&cp_init);
  APP_ERROR_CHECK(err_code);
}
/**@brief Function for the Power manager.
 */
static void power_manage(void) {
  uint32_t err_code = sd_app_evt_wait();
  check_error(err_code);
}

/**
 * @brief ADC interrupt handler.
 */
static void adc_event_handler(nrf_drv_adc_evt_t const *p_event) {
  if (p_event->type == NRF_DRV_ADC_EVT_DONE) {

    int sum = 0;
    for (int i = 0; i < p_event->data.done.size; i++) {
      sum += p_event->data.done.p_buffer[i];
    }
    ble_bas_battery_level_update(&m_bas, sum / p_event->data.done.size);
  }
}

/**
 * @brief ADC initialization.
 */
static void adc_config(void) {
  ret_code_t ret_code;
  nrf_drv_adc_config_t config = NRF_DRV_ADC_DEFAULT_CONFIG;
  static nrf_drv_adc_channel_t adc_channel_config = NRF_DRV_ADC_DEFAULT_CHANNEL(NRF_ADC_CONFIG_INPUT_DISABLED); //Get default ADC channel configuration
  adc_channel_config.config.config.resolution = NRF_ADC_CONFIG_RES_8BIT;
  adc_channel_config.config.config.input = NRF_ADC_CONFIG_SCALING_SUPPLY_ONE_THIRD;
  adc_channel_config.config.config.reference = NRF_ADC_CONFIG_REF_VBG;

  ret_code = nrf_drv_adc_init(&config, adc_event_handler);
  check_error(ret_code);

  nrf_drv_adc_channel_enable(&adc_channel_config);
}

/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const *p_event, void *p_context) {
  switch (p_event->type) {
  case NRF_DRV_TWI_EVT_DONE:
    // if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX)
    // {
    //     data_handler(m_sample);
    // }
    // m_xfer_done = true;
    break;
  default:
    break;
  }
}

/**
 * @brief UART initialization.
 */
void twi_init(void) {
  ret_code_t err_code;

  const nrf_drv_twi_config_t twi_config = {
      .scl = TWI_SCL_PIN,
      .sda = TWI_SDA_PIN,
      .frequency = NRF_TWI_FREQ_400K,
      .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
      .clear_bus_init = true,
  };

  err_code = nrf_drv_twi_init(&m_twi, &twi_config, twi_handler, NULL);
  check_error(err_code);

  nrf_drv_twi_enable(&m_twi);
}

int main(void) {
  uint32_t err_code = 0;

  // init LEDS
  for (int i = 0; i < 3; i++) {
    nrf_gpio_cfg_output(LED_RGB[i]);
  }
  for (int i = 0; i < 3; i++) {
    nrf_gpio_pin_set(LED_RGB[i]);
  }
  // enable adc
  adc_config();

  err_code = NRF_LOG_INIT(NULL);
  check_error(err_code);

  APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);

  // Initialize SoftDevice.
  ble_stack_init();

  // Enable internal DCDC to reduce power consumption
  err_code = sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
  check_error(err_code);

  // init gap_params
  gap_params_init();
  advertising_init();
  services_init();

  conn_params_init();
  nrf_gpio_pin_clear(LED_B);

  // err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
  // check_error(err_code);

  // Battery ADC
  err_code = nrf_drv_adc_buffer_convert(adc_buffer, ADC_BUFFER_SIZE);
  check_error(err_code);

  for (uint16_t i = 0; i < ADC_BUFFER_SIZE; i++) {
    nrf_drv_adc_sample();
    __SEV();
    __WFE();
    __WFE();
  }

  // init twi
  twi_init();
  nrf_gpio_pin_clear(LED_G);

  // init mpu6050
  nrf_delay_ms(100);
  while (!mpu6050_init(&m_twi, mpu6050_device_address)) {
    // nrf_gpio_pin_clear(LED_B);
    // nrf_gpio_pin_clear(LED_B);
    check_error(10);
    nrf_gpio_pin_toggle(LED_G);
  }

  nrf_gpio_pin_set(LED_B);

  // init MPU6050
  // while (twi_master_init() != true) {
  //   nrf_gpio_pin_clear(LED_B);
  // }

  while (true) {
    app_sched_execute();
    power_manage();
  }
}
