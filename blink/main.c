#include <stdbool.h>
#include <stdint.h>

#include "nrf_delay.h"
#include "nrf_gpio.h"

const int LED_R = 17;
const int LED_B = 18;
const int LED_G = 19;

const int LED_RGB[] = {17, 19, 18};

int main(void) {
    for (int i = 0; i < 3; i++) {
        nrf_gpio_cfg_output(LED_RGB[i]);
        nrf_gpio_cfg_output(26);
        nrf_gpio_cfg_output(26);
    }

    while (true) {

        nrf_delay_ms(500);
        nrf_gpio_pin_toggle(LED_B);
    }
}
