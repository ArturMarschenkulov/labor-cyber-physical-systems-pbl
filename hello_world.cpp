#include "ev3dev.h"
#include <ctime>
#include <iostream>
#include <algorithm>

using namespace ev3dev;

static auto std_clamp(int32_t val, int32_t min, int32_t max) -> int32_t {
    if (val < min) { return min; }
    if (val > max) { return max; }

    return val;
}

struct DriveModule {
public:
    static constexpr int32_t MAX_MOTOR_SPEED = 100;

    explicit DriveModule()
        : m_left_wheel(motor(OUTPUT_A, motor::motor_large))
        , m_right_wheel(motor(OUTPUT_B, motor::motor_large)) {

        m_base_speed = 30;
        m_left_wheel.set_duty_cycle_sp(m_base_speed);
        m_right_wheel.set_duty_cycle_sp(m_base_speed);
    }

    auto set_speed_left(int32_t speed) -> void {
        int32_t s = std_clamp(speed, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);
        m_left_wheel.set_duty_cycle_sp(s);
    }

    auto set_speed_right(int32_t speed) -> void {
        int32_t s = std_clamp(speed, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);
        m_right_wheel.set_duty_cycle_sp(s);
    }

    auto set_speed(int32_t speed) -> void {
        int32_t s = std_clamp(speed, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);
        m_left_wheel.set_duty_cycle_sp(s);
        m_right_wheel.set_duty_cycle_sp(s);
    }

    auto drive() -> void {
        m_left_wheel.run_direct();
        m_right_wheel.run_direct();
    }

    auto stop() -> void {
        m_left_wheel.stop();
        m_right_wheel.stop();
    }

    motor    m_left_wheel;
    motor    m_right_wheel;
    uint32_t m_base_speed;
};

struct HeadModule {
public:
    HeadModule()
        : m_head(motor(OUTPUT_D, motor::motor_medium)) {

        m_should_change_direction = false;
        m_movement_speed = 50;
        m_movement_direction = 1;

        m_head.set_position(0);
        m_head.set_duty_cycle_sp(m_movement_speed);
    }

    auto get_angle() const -> int { return m_head.position(); }

    auto move() -> void { m_head.run_direct(); }

    auto switch_head_movement() -> void {
        m_movement_direction = -m_movement_direction;
        m_head.set_duty_cycle_sp(m_movement_speed * m_movement_direction);
    }

    auto stop() -> void { m_head.stop(); }

public:
    motor m_head;

    bool m_should_change_direction;
    int  m_movement_speed;
    int  m_movement_direction; // -1 left, 1 right
    bool m_is_head_movement_done = false;
};

struct TouchModule {
public:
    explicit TouchModule()
        : m_sensor(touch_sensor(INPUT_1))
        , m_is_pressed_prev(this->is_down()) {}

    auto is_down() -> bool { return m_sensor.is_pressed(); }

    auto is_pressed() -> bool {
        m_is_pressed_curr = this->is_down();
        bool is_pressed = m_is_pressed_curr && !m_is_pressed_prev;
        m_is_pressed_prev = m_is_pressed_curr;
        return is_pressed;
    }

    auto is_released() -> bool {
        m_is_pressed_curr = this->is_down();
        bool is_released = !m_is_pressed_curr && m_is_pressed_prev;
        m_is_pressed_prev = m_is_pressed_curr;
        return is_released;
    }

    auto update() -> void {

        if (this->is_pressed()) { m_is_on = !m_is_on; }
    }

public:
    touch_sensor m_sensor;
    bool         m_is_pressed_prev = false;
    bool         m_is_pressed_curr = false;
    bool         m_is_on = false;
};

struct Robot {

    static constexpr int32_t MAX_MOTOR_SPEED = 100;

    Robot()
        : m_eye(ultrasonic_sensor(INPUT_4)) {

        const uint32_t speed = 30;
        m_drive_module.set_speed(speed);
    }

    ~Robot() { this->shutdown(); }

    auto shutdown() -> void {
        this->stop_drive();
        m_head.stop();
    }

    auto set_drive_speed(int32_t speed) -> void {
        int32_t s = std_clamp(speed, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);
        m_drive_module.set_speed(s);
    }

    auto drive() -> void { m_drive_module.drive(); }

    auto stop_drive() -> void {
        m_drive_module.stop();
        m_drive_module.set_speed(0);
    }

    DriveModule m_drive_module;

    HeadModule m_head;

    ultrasonic_sensor m_eye;

    TouchModule m_touch_module;
};

auto main() -> int {

    std::cout << "Hello CPS Course." << std::endl;
    std::cout << "Press main button to leave ..." << std::endl;
    // Wait for main butclearton is pressed

    Robot          robot;
    constexpr auto distance_slots_num = 5;
    constexpr auto MIN_DISTANCE = 10;
    constexpr auto MAX_DISTANCE = 200;
    constexpr auto MAX_SECS_OF_ROTATING = 5;

    int last_min_distance = 100000;
    // int curr_min_distance = last_min_distance;
    int angle_for_min_distance = 0;

    enum class STATE {
        PERFECT,
        TOO_FAR,
        TOO_CLOSE
    };
    STATE state = STATE::PERFECT;

    bool is_timing_on = false;
    do {

        auto distance_in_cm = robot.m_eye.distance_centimeters();
        auto angle = robot.m_head.get_angle();

        if (distance_in_cm < last_min_distance) {
            last_min_distance = distance_in_cm;
            angle_for_min_distance = angle;
        }

        if (last_min_distance > MAX_DISTANCE) {
            state = STATE::TOO_FAR;
        } else if (last_min_distance < MIN_DISTANCE) {
            state = STATE::TOO_CLOSE;
        } else {
            state = STATE::PERFECT;
        }

        std::cout << "dist: " << distance_in_cm << " cm, Angle: " << angle
                  << " min dist: " << last_min_distance
                  << " angle: " << angle_for_min_distance << std::endl;

        // print out

        robot.m_touch_module.update();

        // robot.set_drive_speed(distance_in_cm);

        robot.m_head.move();

        const auto MAX_HEAD_ANGLE = 90;

        // head movement logic
        if (abs(angle) > MAX_HEAD_ANGLE && !robot.m_head.m_should_change_direction) {
            robot.m_head.m_should_change_direction = true;

            robot.m_head.switch_head_movement();
        }
        if (abs(angle) <= MAX_HEAD_ANGLE && robot.m_head.m_should_change_direction) {
            auto curr_base_speed = robot.m_drive_module.m_base_speed;
            switch (state) {
                case STATE::PERFECT: {

                    if (angle_for_min_distance > 0) {
                        robot.m_drive_module.set_speed_right(curr_base_speed);
                        robot.m_drive_module.set_speed_left(curr_base_speed + 40);

                    } else {
                        robot.m_drive_module.set_speed_left(curr_base_speed);
                        robot.m_drive_module.set_speed_right(curr_base_speed + 40);
                    }
                } break;
                case STATE::TOO_CLOSE: {
                    robot.m_drive_module.set_speed_right(0);
                    robot.m_drive_module.set_speed_left(0);
                } break;
                case STATE::TOO_FAR: {
                    robot.m_drive_module.set_speed_right(70);
                    robot.m_drive_module.set_speed_left(-70);
                } break;
            }
            robot.m_head.m_should_change_direction = false;
            last_min_distance = 100000;
        }

        if (robot.m_touch_module.m_is_on && robot.m_eye.distance_centimeters() > 20) {

            robot.m_touch_module.m_is_pressed_curr =
                robot.m_touch_module.m_sensor.is_pressed();
            if (robot.m_eye.distance_centimeters() < 150) { robot.drive(); }
        } else {
            robot.stop_drive();
            robot.m_touch_module.m_is_pressed_curr =
                robot.m_touch_module.m_sensor.is_pressed();
        }

    } while (!button::enter.pressed());

    std::cout << "Program ended" << std::endl;
    return 0;
}
