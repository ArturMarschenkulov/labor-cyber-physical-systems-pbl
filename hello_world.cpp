#include "ev3dev.h"
#include <iostream>

using namespace ev3dev;

int main() {
    std::cout << "Hello CPS Course." << std::endl;
    std::cout << "Press main button to leave ..." << std::endl;
    // Wait for main butclearton is pressed

    const uint32_t speed = 100;

    bool  should_shut_down = false;
    motor motor_left_wheel = motor(OUTPUT_A, motor::motor_large);
    motor motor_right_wheel = motor(OUTPUT_B, motor::motor_large);

    motor_left_wheel.set_duty_cycle_sp(speed);
    motor_right_wheel.set_duty_cycle_sp(speed);

    motor motor_head = motor(OUTPUT_D, motor::motor_medium);

    ultrasonic_sensor eye = ultrasonic_sensor(INPUT_4);

    touch_sensor touch = touch_sensor(INPUT_1);

    bool is_pressed_prev = false;
    bool is_pressed_curr = touch.is_pressed();
    bool is_on = false;
    do {

        auto centimeter = eye.distance_centimeters();
        auto angle = motor_head.position();
        // print out
        std::cout << "Distance: " << centimeter << " cm, Angle: " << angle << std::endl;

        is_pressed_curr = touch.is_pressed();
        if (is_pressed_curr && !is_pressed_prev) { is_on = !is_on; }
        is_pressed_prev = is_pressed_curr;

        if (is_on && eye.distance_centimeters() > 20) {

            is_pressed_curr = touch.is_pressed();
            if (eye.distance_centimeters() < 150) {
                motor_left_wheel.run_direct();
                motor_right_wheel.run_direct();
            }
        } else {
            motor_left_wheel.stop();
            motor_right_wheel.stop();
            is_pressed_curr = touch.is_pressed();
        }

    } while (!button::enter.pressed());

    motor_left_wheel.stop();
    motor_right_wheel.stop();
    return 0;
}
