#include <stdbool.h>
#include <stdint.h>

#include "app_timer.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_srv_common.h"

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "softdevice_handler.h"

const int LED_R = 17;
const int LED_G = 18;
const int LED_B = 19;

const int LED_RGB[] = {17, 19, 18};

const int CENTRAL_LINK_COUNT = 0;    /**< Number of central links used by the application. When changing this number
                                        remember to adjust the RAM settings*/
const int PERIPHERAL_LINK_COUNT = 0; /**< Number of peripheral links used by the application. When changing this number
                                        remember to adjust the RAM settings*/

#define DEVICE_NAME "Ferris_PRO"                /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME "NordicSemiconductor" /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL \
    300 /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS 180 /**< The advertising timeout in units of seconds. */

// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC                                                \
    {                                                                     \
        .source = NRF_CLOCK_LF_SRC_XTAL, .rc_ctiv = 0, .rc_temp_ctiv = 0, \
        .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM                \
    }

#define APP_TIMER_PRESCALER 0     /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE 4 /**< Size of timer operation queues. */

static ble_uuid_t m_adv_uuids[] = {
    {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}}; /**< Universally unique service identifiers. */

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
        nrf_gpio_pin_toggle(LED_G);
        break;
    default:
        break;
    }
}

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void) {
    uint32_t               err_code;
    ble_advdata_t          advdata;
    ble_adv_modes_config_t options;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = m_adv_uuids;

    memset(&options, 0, sizeof(options));
    options.ble_adv_fast_enabled  = true;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
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
    APP_ERROR_CHECK(err_code);

    // Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT, PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
}

int main(void) {
    // init LEDS

    for (int i = 0; i < 3; i++) {
        nrf_gpio_cfg_output(LED_RGB[i]);
    }
    for (int i = 0; i < 3; i++) {
        nrf_gpio_pin_set(LED_RGB[i]);
    }

    // uint32_t err_code;
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);

    // Initialize SoftDevice.
    ble_stack_init();
    advertising_init();

    while (true) {

        nrf_delay_ms(500);
        nrf_gpio_pin_toggle(LED_G);
    }
}
