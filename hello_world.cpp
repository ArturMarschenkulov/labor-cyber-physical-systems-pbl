#include "ev3dev.h"
#include <iostream>

using namespace ev3dev;

int main() {
    std::cout << "Hello CPS Course." << std::endl;
    std::cout << "Press main button to leave ..." << std::endl;
    // Wait for main butclearton is pressed

    bool  should_shut_down = false;
    motor motor_left_wheel = motor(OUTPUT_A);
    motor motor_right_wheel = motor(OUTPUT_B);
    motor motor_head = motor(OUTPUT_D);

    ultrasonic_sensor eye = ultrasonic_sensor(INPUT_4);

    touch_sensor touch = touch_sensor(INPUT_1);

    bool is_down_prev = false;
    bool is_down_curr = false;

    auto speed = 20;

    bool is_on = false;
    do {

        auto centimeter = eye.distance_centimeters();
        auto angle = motor_head.position();
        // print out
        std::cout << "Distance: " << centimeter << " cm, Angle: " << angle << std::endl;
        if (touch.is_pressed()) {
            bool is_changed = (is_down_prev != is_down_curr);

            if (!is_changed) {
                // noop
            } else if (is_changed && is_on == true) {
                is_on = false;
            } else if (is_changed && is_on == false) {
                is_on = true;
            } else {
                // noop
            }
        }

        if (is_on) {
            motor_left_wheel.set_duty_cycle_sp(speed);
            motor_left_wheel.run_direct();

            motor_right_wheel.set_duty_cycle_sp(speed);
            motor_right_wheel.run_direct();
        } else {
            motor_left_wheel.stop();
            motor_right_wheel.stop();
        }

    } while (!button::enter.pressed());

    motor_left_wheel.stop();
    motor_right_wheel.stop();
    return 0;
}
