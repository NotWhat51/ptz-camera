#include "pwm.hpp"
#include "sysfs.hpp"
#include <cmath>
#include <cstdio>
#include <cerrno>
#include <cstring>

PWM_A::PWM_A(int chip, int pin, double frequency, double duty_cycle_percent, bool invert_polarity)
    : chip(chip), pin(pin), frequency(frequency), duty_cycle_percent(duty_cycle_percent), invert_polarity(invert_polarity) {
    try {
        PWM_Export(chip, pin);  // creates the pwm sysfs object
        if (invert_polarity) {
            PWM_Polarity(chip, pin, true);  // invert pwm i.e the duty cycle tells you how long the cycle is off
        } else {
            PWM_Polarity(chip, pin, false);  // don't invert the pwm signal. This is the normal way its used.
        }
        PWM_Enable(chip, pin);
        PWM_Frequency(chip, pin, frequency);
    } catch (const std::system_error& e) {
        if (e.code().value() == EBUSY) {  // Device or resource busy
            std::cerr << "Pin " << pin << " is already in use, continuing anyway." << std::endl;
            PWM_Unexport(chip, pin);
            PWM_Export(chip, pin);
        } else {
            throw;
        }
    }
}

void PWM_A::start_pwm() {
    // turn on pwm by setting the duty cycle to what the user specified
    PWM_Duty_Cycle_Percent(chip, pin, duty_cycle_percent);  // duty cycle controls the on-off
}

void PWM_A::stop_pwm() {
    // turn on pwm by setting the duty cycle to 0
    PWM_Duty_Cycle_Percent(chip, pin, 0);  // duty cycle at 0 is the equivalent of off
}

void PWM_A::change_frequency(double new_frequency) {
    // Order of operations:
    // 1. convert to period
    // 2. check if period is increasing or decreasing
    // 3. If increasing update pwm period and then update the duty cycle period
    // 4. If decreasing update the duty cycle period and then the pwm period
    // Why:
    // The sysfs rule for PWM is that PWM Period >= duty cycle period (in nanosecs)

    double pwm_period = (1 / new_frequency) * 1e9;
    int pwm_period_int = static_cast<int>(round(pwm_period));
    double duty_cycle = (duty_cycle_percent / 100) * pwm_period;
    int duty_cycle_int = static_cast<int>(round(duty_cycle));

    int old_pwm_period = static_cast<int>(round((1 / frequency) * 1e9));

    if (pwm_period_int > old_pwm_period) {  // if increasing
        PWM_Period(chip, pin, pwm_period_int);  // update the pwm period
        PWM_Duty_Cycle(chip, pin, duty_cycle_int);  // update duty cycle
    } else {
        PWM_Duty_Cycle(chip, pin, duty_cycle_int);  // update duty cycle
        PWM_Period(chip, pin, pwm_period_int);  // update pwm freq
    }

    frequency = new_frequency;  // update the frequency
}

void PWM_A::duty_cycle(double duty_cycle_percent) {
    // in percentage (0-100)
    if (0 <= duty_cycle_percent && duty_cycle_percent <= 100) {
        this->duty_cycle_percent = duty_cycle_percent;
        PWM_Duty_Cycle_Percent(chip, pin, this->duty_cycle_percent);
    } else {
        throw std::out_of_range("Duty cycle must be between 0 and 100. Current value: " + std::to_string(duty_cycle_percent) + " is out of bounds");
    }
}

void PWM_A::pwm_polarity() {
    // invert the polarity of the pwm
    PWM_Disable(chip, pin);
    PWM_Polarity(chip, pin, !invert_polarity);
    PWM_Enable(chip, pin);
}

void PWM_A::pwm_close() {
    // remove the object from the system
    PWM_Unexport(chip, pin);
}
