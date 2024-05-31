#include "sysfs.hpp"
#include "constants.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cmath>

const double WAIT_PERMISSION_TIMEOUT = 1.0;

ValueDescriptor::ValueDescriptor(int pin, const std::string& mode) {
    path = "/sys/class/gpio/gpio" + std::to_string(pin) + "/value";
    await_permissions(path);
    if (mode == "r") {
        fp.open(path, std::ios::in);
    } else {
        fp.open(path, std::ios::out);
    }
}

ValueDescriptor::~ValueDescriptor() {
    if (fp.is_open()) {
        fp.close();
    }
}

std::fstream& ValueDescriptor::get() {
    return fp;
}

void await_permissions(const std::string& path) {
    auto start_time = std::chrono::steady_clock::now();

    auto timed_out = [&]() {
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - start_time).count() >= WAIT_PERMISSION_TIMEOUT;
    };

    while (access(path.c_str(), W_OK) != 0 && !timed_out()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void export_pin(int pin) {
    std::string path = "/sys/class/gpio/export";
    await_permissions(path);
    std::ofstream fp(path);
    fp << pin;
}

void unexport_pin(int pin) {
    std::string path = "/sys/class/gpio/unexport";
    await_permissions(path);
    std::ofstream fp(path);
    fp << pin;
}

void direction(int pin, int dir) {
    assert(dir == GPIO.getattr<int>("IN") || dir == GPIO.getattr<int>("OUT"));
    std::string path = "/sys/class/gpio/gpio" + std::to_string(pin) + "/direction";
    await_permissions(path);
    std::ofstream fp(path);
    if (dir == GPIO.getattr<int>("IN")) {
        fp << "in";
    } else {
        fp << "out";
    }
}

int input(int pin) {
    ValueDescriptor vd(pin);
    std::string value;
    vd.get() >> value;
    return (value == std::to_string(GPIO.getattr<int>("LOW"))) ? GPIO.getattr<int>("LOW") : GPIO.getattr<int>("HIGH");
}

void output(int pin, int value) {
    std::string str_value = value ? "1" : "0";
    ValueDescriptor vd(pin, "w");
    vd.get() << str_value;
}

void edge(int pin, int trigger) {
    assert(trigger == GPIO.getattr<int>("NONE") || trigger == GPIO.getattr<int>("RISING") || trigger == GPIO.getattr<int>("FALLING") || trigger == GPIO.getattr<int>("BOTH"));
    std::string path = "/sys/class/gpio/gpio" + std::to_string(pin) + "/edge";
    await_permissions(path);
    std::ofstream fp(path);
    switch (trigger) {
        case 0: fp << "none"; break;
        case 1: fp << "rising"; break;
        case 2: fp << "falling"; break;
        case 3: fp << "both"; break;
    }
}

void PWM_Export(int chip, int pin) {
    std::string path = "/sys/class/pwm/pwmchip" + std::to_string(chip) + "/export";
    await_permissions(path);
    std::ofstream fp(path);
    fp << pin;
}

void PWM_Unexport(int chip, int pin) {
    std::string path = "/sys/class/pwm/pwmchip" + std::to_string(chip) + "/unexport";
    await_permissions(path);
    std::ofstream fp(path);
    fp << pin;
}

void PWM_Enable(int chip, int pin) {
    std::string path = "/sys/class/pwm/pwmchip" + std::to_string(chip) + "/pwm" + std::to_string(pin) + "/enable";
    await_permissions(path);
    std::ofstream fp(path);
    fp << 1;
}

void PWM_Disable(int chip, int pin) {
    std::string path = "/sys/class/pwm/pwmchip" + std::to_string(chip) + "/pwm" + std::to_string(pin) + "/enable";
    await_permissions(path);
    std::ofstream fp(path);
    fp << 0;
}

void PWM_Polarity(int chip, int pin, bool invert) {
    std::string path = "/sys/class/pwm/pwmchip" + std::to_string(chip) + "/pwm" + std::to_string(pin) + "/polarity";
    await_permissions(path);
    std::ofstream fp(path);
    if (invert) {
        fp << "inversed";
    } else {
        fp << "normal";
    }
}

void PWM_Period(int chip, int pin, int pwm_period) {
    std::string duty_cycle_path = "/sys/class/pwm/pwmchip" + std::to_string(chip) + "/pwm" + std::to_string(pin) + "/duty_cycle";
    std::ifstream fp(duty_cycle_path);
    int current_duty_cycle_period;
    fp >> current_duty_cycle_period;
    fp.close();

    if (current_duty_cycle_period > pwm_period) {
        std::cerr << "Error: the new duty cycle period must be less than or equal to the PWM Period: " << pwm_period << std::endl;
        std::cerr << "New Duty Cycle = " << current_duty_cycle_period << " Current PWM Period = " << pwm_period << std::endl;
        std::terminate();
    }

    std::string path = "/sys/class/pwm/pwmchip" + std::to_string(chip) + "/pwm" + std::to_string(pin) + "/period";
    await_permissions(path);
    std::ofstream fp_out(path);
    fp_out << pwm_period;
}

void PWM_Frequency(int chip, int pin, double pwm_frequency) {
    int pwm_period = static_cast<int>(round((1 / pwm_frequency) * 1e9));
    std::string path = "/sys/class/pwm/pwmchip" + std::to_string(chip) + "/pwm" + std::to_string(pin) + "/period";
    await_permissions(path);
    std::ofstream fp(path);
    fp << pwm_period;
}

void PWM_Duty_Cycle_Percent(int chip, int pin, double Duty_cycle) {
    std::string PWM_period_path = "/sys/class/pwm/pwmchip" + std::to_string(chip) + "/pwm" + std::to_string(pin) + "/period";
    std::ifstream fp(PWM_period_path);
    int current_period;
    fp >> current_period;
    fp.close();

    int new_duty_cycle = static_cast<int>(round(Duty_cycle / 100 * current_period));

    std::string path = "/sys/class/pwm/pwmchip" + std::to_string(chip) + "/pwm" + std::to_string(pin) + "/duty_cycle";
    std::ofstream fp_out(path);
    fp_out << new_duty_cycle;
}

void PWM_Duty_Cycle(int chip, int pin, int Duty_cycle) {
    std::string PWM_period_path = "/sys/class/pwm/pwmchip" + std::to_string(chip) + "/pwm" + std::to_string(pin) + "/period";
    std::ifstream fp(PWM_period_path);
    int current_period;
    fp >> current_period;
    fp.close();

    if (Duty_cycle > current_period) {
        std::cerr << "Error: the new duty cycle period must be less than or equal to the PWM Period: " << current_period << std::endl;
        std::cerr << "New Duty Cycle = " << Duty_cycle << " Current PWM Period = " << current_period << std::endl;
        std::terminate();
    }

    std::string path = "/sys/class/pwm/pwmchip" + std::to_string(chip) + "/pwm" + std::to_string(pin) + "/duty_cycle";
    await_permissions(path);
    std::ofstream fp_out(path);
    fp_out << Duty_cycle;
}
