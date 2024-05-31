#ifndef SYSFS_HPP
#define SYSFS_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <cassert>

class ValueDescriptor {
public:
    ValueDescriptor(int pin, const std::string& mode = "r");
    ~ValueDescriptor();
    std::fstream& get();

private:
    std::string path;
    std::fstream fp;
};

void await_permissions(const std::string& path);

void export_pin(int pin);
void unexport_pin(int pin);
void direction(int pin, int dir);
int input(int pin);
void output(int pin, int value);
void edge(int pin, int trigger);

void PWM_Export(int chip, int pin);
void PWM_Unexport(int chip, int pin);
void PWM_Enable(int chip, int pin);
void PWM_Disable(int chip, int pin);
void PWM_Polarity(int chip, int pin, bool invert = false);
void PWM_Period(int chip, int pin, int pwm_period);
void PWM_Frequency(int chip, int pin, double pwm_frequency);
void PWM_Duty_Cycle_Percent(int chip, int pin, double Duty_cycle);
void PWM_Duty_Cycle(int chip, int pin, int Duty_cycle);

#endif // SYSFS_HPP
