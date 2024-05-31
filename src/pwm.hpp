#ifndef PWM_HPP
#define PWM_HPP

#include <stdexcept>
#include "sysfs.hpp"

class PWM_A {
public:
    PWM_A(int chip, int pin, double frequency, double duty_cycle_percent, bool invert_polarity = false);

    void start_pwm();
    void stop_pwm();
    void change_frequency(double new_frequency);
    void duty_cycle(double duty_cycle_percent);
    void pwm_polarity();
    void pwm_close();

private:
    int chip;
    int pin;
    double frequency;
    double duty_cycle_percent;
    bool invert_polarity;
};

#endif // PWM_HPP
