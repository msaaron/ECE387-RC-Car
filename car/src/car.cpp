// Copyright 2022 Matt Aaron
#include <pico/stdlib.h>
#include <hardware/pwm.h>
#include <RF24.h>

#define CE_PIN  7
#define CSN_PIN 8
#define IRQ_PIN 6

#define DRIVE_A_PIN    2
#define DRIVE_B_PIN    4
#define STEER_A_PIN   10
#define STEER_B_PIN    6

struct Controls {
    int8_t drive;
    int8_t steer;
};

namespace Car {
    RF24 radio(CE_PIN, CSN_PIN);
    Controls payload;
}

bool setup() {
    // Configure default LED (indicates radio power up)
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    // Configure drive pins
    gpio_init(DRIVE_A_PIN);
    gpio_init(DRIVE_B_PIN);
    gpio_set_dir(DRIVE_A_PIN, GPIO_OUT);
    gpio_set_dir(DRIVE_B_PIN, GPIO_OUT);
    gpio_put(DRIVE_A_PIN, 0);
    gpio_put(DRIVE_B_PIN, 0);

    // Configure steer pins
    gpio_init(STEER_A_PIN);
    gpio_init(STEER_B_PIN);
    gpio_set_dir(STEER_A_PIN, GPIO_OUT);
    gpio_set_dir(STEER_B_PIN, GPIO_OUT);
    gpio_put(STEER_A_PIN, 0);
    gpio_put(STEER_B_PIN, 0);

    // Configure radio hardware
    uint8_t address[6] = "00001";

    if (!Car::radio.begin()) {
        return false;
    }

    Car::radio.setPALevel(RF24_PA_MIN);
    Car::radio.setPayloadSize(sizeof(Car::payload));
    Car::radio.openReadingPipe(0, address);
    Car::radio.startListening();
    // Car::radio.stopListening();

    // Indicate successful radio startup
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    delay(500);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    // Set default control payload states
    Car::payload.drive = 0;
    Car::payload.steer = 0;

    return true;
}

void loop() {
    uint8_t pipe;
    if (Car::radio.available()) {
        uint8_t bytes = Car::radio.getPayloadSize();
        Car::radio.read(&Car::payload, bytes);
    }

    if (Car::payload.drive == 0) {  // Stop
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        gpio_put(DRIVE_A_PIN, 0);
        gpio_put(DRIVE_B_PIN, 0);
    } else if (Car::payload.drive > 0) {  // Forward
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        gpio_put(DRIVE_A_PIN, 1);
        gpio_put(DRIVE_B_PIN, 0);
    } else if (Car::payload.drive < 0) {  // Backward
        gpio_put(DRIVE_A_PIN, 0);
        gpio_put(DRIVE_B_PIN, 1);
    }

    if (Car::payload.steer == 0) {  // Center
        gpio_put(STEER_A_PIN, 0);
        gpio_put(STEER_B_PIN, 0);
    } else if (Car::payload.steer > 0) {  // Turn right?
        gpio_put(STEER_A_PIN, 1);
        gpio_put(STEER_B_PIN, 0);
    } else if (Car::payload.steer < 0) {  // Turn left?
        gpio_put(STEER_A_PIN, 0);
        gpio_put(STEER_B_PIN, 1);
    }
}

int main() {
    while (!setup()) { }
    while (true) {
        loop();
    }

    return 0;
}
