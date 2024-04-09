#include "ev3dev.h"
#include <iostream>

using namespace ev3dev;

struct TouchState {
    bool          is_pressed_prev = false;
    bool          is_pressed_curr = false;
    bool          is_on = false;
    touch_sensor* sensor = nullptr;

    auto init(touch_sensor* sensor) -> void {
        is_pressed_prev = false;
        is_pressed_curr = sensor->is_pressed();
        is_on = false;
        this->sensor = sensor;
    }

    auto update() -> void {
        is_pressed_curr = sensor->is_pressed();
        if (is_pressed_curr && !is_pressed_prev) { is_on = !is_on; }
        is_pressed_prev = is_pressed_curr;
    }
};

int main() {
    std::cout << "Hello CPS Course." << std::endl;
    std::cout << "Press main button to leave ..." << std::endl;
    // Wait for main butclearton is pressed

    const uint32_t speed = 30;

    bool  should_shut_down = false;
    motor left_wheel = motor(OUTPUT_A, motor::motor_large);
    motor right_wheel = motor(OUTPUT_B, motor::motor_large);

    left_wheel.set_duty_cycle_sp(speed);
    right_wheel.set_duty_cycle_sp(speed);

    motor head = motor(OUTPUT_D, motor::motor_medium);

    ultrasonic_sensor eye = ultrasonic_sensor(INPUT_4);

    touch_sensor touch = touch_sensor(INPUT_1);
    TouchState   touch_state = {};
    touch_state.init(&touch);

    do {

        auto centimeter = eye.distance_centimeters();
        auto angle = head.position();
        // print out
        std::cout << "Distance: " << centimeter << " cm, Angle: " << angle << std::endl;

        touch_state.update();

        if (touch_state.is_on && eye.distance_centimeters() > 20) {

            touch_state.is_pressed_curr = touch.is_pressed();
            if (eye.distance_centimeters() < 150) {
                left_wheel.run_direct();
                right_wheel.run_direct();
            }
        } else {
            left_wheel.stop();
            right_wheel.stop();
            touch_state.is_pressed_curr = touch.is_pressed();
        }

    } while (!button::enter.pressed());

    left_wheel.stop();
    right_wheel.stop();
    return 0;
}
