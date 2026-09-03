#include "key.h"


volatile uint8_t car_start_request = 0;

void key_onExti(uint16_t GPIO_Pin) {
    if (GPIO_Pin != KEY1_Pin) {
        return;
    }
    car_start_request = 1;
}




