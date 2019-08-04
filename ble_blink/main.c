#include <stdbool.h>
#include <stdint.h>

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "app_timer.h"
#include "softdevice_handler.h"

const int LED_R = 17;
const int LED_G = 18;
const int LED_B = 19;

const int LED_RGB[] = {17, 19, 18};

const int CENTRAL_LINK_COUNT = 0;    /**< Number of central links used by the application. When changing this number
                                        remember to adjust the RAM settings*/
const int PERIPHERAL_LINK_COUNT = 0; /**< Number of peripheral links used by the application. When changing this number
                                        remember to adjust the RAM settings*/

// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}

#define APP_TIMER_PRESCALER             0                                 /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                 /**< Size of timer operation queues. */


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);

    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);

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

    while (true) {

        nrf_delay_ms(500);
        nrf_gpio_pin_toggle(LED_G);
    }
}
