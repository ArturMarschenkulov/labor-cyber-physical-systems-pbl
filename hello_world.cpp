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

struct Robot {

    Robot()
        : m_left_wheel(motor(OUTPUT_A, motor::motor_large))
        , m_right_wheel(motor(OUTPUT_B, motor::motor_large))
        , m_head(motor(OUTPUT_D, motor::motor_medium))
        , m_eye(ultrasonic_sensor(INPUT_4))
        , m_touch(touch_sensor(INPUT_1)) {

        const uint32_t speed = 30;
        m_left_wheel.set_duty_cycle_sp(speed);
        m_right_wheel.set_duty_cycle_sp(speed);

        m_head_should_change_direction = false;
        m_head_movement_speed = 100;
        m_head_movement_direction = 1;

        m_head.set_position(0);
        m_head.set_duty_cycle_sp(m_head_movement_speed);

        m_touch_state.init(&m_touch);
    }

    auto set_drive_speed(uint32_t speed) -> void {
        m_left_wheel.set_duty_cycle_sp(speed);
        m_right_wheel.set_duty_cycle_sp(speed);
    }

    auto drive() -> void {
        m_left_wheel.run_direct();
        m_right_wheel.run_direct();
    }

    auto stop_drive() -> void {
        m_left_wheel.stop();
        m_right_wheel.stop();
    }

    auto shutdown() -> void {
        this->stop_drive();
        m_head.stop();
    }

    motor             m_left_wheel;
    motor             m_right_wheel;
    motor             m_head;
    ultrasonic_sensor m_eye;
    touch_sensor      m_touch;

    TouchState m_touch_state;

    bool m_head_should_change_direction;
    int  m_head_movement_speed;
    int  m_head_movement_direction; // -1 left, 1 right
};

int main() {
    std::cout << "Hello CPS Course." << std::endl;
    std::cout << "Press main button to leave ..." << std::endl;
    // Wait for main butclearton is pressed

    Robot robot;

    do {

        auto centimeter = robot.m_eye.distance_centimeters();
        auto angle = robot.m_head.position();
        // print out
        std::cout << "Distance: " << centimeter << " cm, Angle: " << angle << std::endl;

        robot.m_touch_state.update();

        if (centimeter > 100) {
            robot.set_drive_speed(100);
        } else {
            robot.set_drive_speed(centimeter);
        }

        robot.m_head.run_direct();

        const auto head_angle = 170;
        if (abs(angle) > head_angle && !robot.m_head_should_change_direction) {
            robot.m_head_should_change_direction = true;

            robot.m_head_movement_direction = -robot.m_head_movement_direction;
            auto sign = robot.m_head_movement_direction;
            auto head_speed = robot.m_head_movement_speed;
            robot.m_head.set_duty_cycle_sp(sign * head_speed);
        }
        if (abs(angle) <= head_angle) { robot.m_head_should_change_direction = false; }


        if (angle) {

            if (robot.m_touch_state.is_on && robot.m_eye.distance_centimeters() > 20) {

                robot.m_touch_state.is_pressed_curr = robot.m_touch.is_pressed();
                if (robot.m_eye.distance_centimeters() < 150) { robot.drive(); }
            } else {
                robot.stop_drive();
                robot.m_touch_state.is_pressed_curr = robot.m_touch.is_pressed();
            }
        }

    } while (!button::enter.pressed());

    robot.shutdown();
    return 0;
}
