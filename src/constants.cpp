#include "constants.hpp"

_const GPIO;

template <typename T>
void _const::setattr(const std::string& name, const T& value) {
    if (this->data.count(name)) {
        throw ConstError("Can't rebind const(" + name + ")");
    }
    this->data[name] = value;
}

template <typename T>
T _const::getattr(const std::string& name) {
    if (!this->data.count(name)) {
        throw std::out_of_range("Attribute '" + name + "' not found");
    }
    return std::any_cast<T>(this->data.at(name));
}

// From: https://sourceforge.net/p/raspberry-gpio-python/code/ci/default/tree/source/c_gpio.h#l42
void init_gpio() {
    GPIO.setattr("IN", 1);
    GPIO.setattr("OUT", 0);
    GPIO.setattr("ALT0", 4);

    GPIO.setattr("HIGH", 1);
    GPIO.setattr("LOW", 0);

    GPIO.setattr("PUD_OFF", 0);
    GPIO.setattr("PUD_DOWN", 1);
    GPIO.setattr("PUD_UP", 2);

    // From: https://sourceforge.net/p/raspberry-gpio-python/code/ci/default/tree/source/common.h
    GPIO.setattr("BOARD", 10);
    GPIO.setattr("BCM", 11);
    GPIO.setattr("SUNXI", 12);
    GPIO.setattr("SOC", 13);
    GPIO.setattr("SYSFS", GPIO.getattr<int>("SOC"));

    GPIO.setattr("NONE", 0);
    GPIO.setattr("RISING", 1);
    GPIO.setattr("FALLING", 2);
    GPIO.setattr("BOTH", 3);

    GPIO.setattr("PA", 0);
    GPIO.setattr("PC", 64);
    GPIO.setattr("PD", 96);
    GPIO.setattr("PE", 128);
    GPIO.setattr("PF", 160);
    GPIO.setattr("PG", 192);
    GPIO.setattr("PL", 352);

    //Поддерживаемые платы
    GPIO.setattr("REPKAPI3", 1);

    //Плата по умолчанию
    GPIO.setattr("DEFAULTBOARD", nullptr);
    // GPIO.setattr("DEFAULTBOARD", GPIO.getattr<int>("REPKAPI3")); // Repka Pi 3 выставлена по умолчанию

    //Автоматическое определение версии Repka Pi
    GPIO.setattr("AUTODETECT", true);
}
