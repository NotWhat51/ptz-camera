#include <iostream>
#include <map>
#include <vector>
#include <assert.h>
#include "constants.hpp"
#include "sysfs.hpp"
#include "event.hpp"
#include "boards.hpp"

bool _gpio_warnings = true;
int _mode = -1;
int _board = GPIO.getattr<int>("DEFAULTBOARD");
std::map<int, int> _exports;
std::vector<int> _boards = {GPIO.getattr<int>("REPKAPI3")};
std::string RPI_INFO = "Не выбранна модель платы. Для выбора модели платы используйте метод setboard()";

void _check_configured(int channel, int direction = -1) {
    auto it = _exports.find(channel);
    if (it == _exports.end()) {
        throw std::runtime_error("Channel " + std::to_string(channel) + " is not configured");
    }
    if (direction != -1 && it->second != direction) {
        std::string descr = (it->second == GPIO.getattr<int>("IN")) ? "input" : "output";
        throw std::runtime_error("Channel " + std::to_string(channel) + " is configured for " + descr);
    }
}

void setboard(int board) {
    assert(board == GPIO.getattr<int>("REPKAPI3"));
    _board = board;
}

std::string getboardmodel() {
    if (_board != GPIO.getattr<int>("REPKAPI3")) {
        throw std::runtime_error("Не выбранна модель платы. Для выбора модели платы используйте метод setboard()");
    }
    return "Repka-Pi3-H5";
}

int getmode() {
    return _mode;
}

void setmode(int mode) {
    assert(mode == GPIO.getattr<int>("BCM") || mode == GPIO.getattr<int>("BOARD"));
    _mode = mode;
}

void setwarnings(bool enabled) {
    _gpio_warnings = enabled;
}

void setup(int channel, int direction, int initial = -1, int pull_up_down = -1) {
    if (_board != GPIO.getattr<int>("REPKAPI3")) {
        throw std::runtime_error("Не выбранна модель платы. Для выбора модели платы используйте метод setboard()");
    }
    if (_mode == -1) {
        throw std::runtime_error("Mode has not been set");
    }
    if (pull_up_down != -1) {
        if (_gpio_warnings) {
            std::cerr << "Pull up/down пока не поддерживаются, но выполнение продолжается. Используйте GPIO.setwarnings(False) что бы отключить предупреждение." << std::endl;
        }
    }
    if (std::find(_boards.begin(), _boards.end(), _board) == _boards.end()) {
        throw std::runtime_error("Не выбранна модель платы. Для выбора модели платы используйте метод setboard()");
    }
    if (channel in _exports) {
        throw std::runtime_error("Channel " + std::to_string(channel) + " is already configured");
    }
    int pin = get_gpio_pin(_board, _mode, channel);
    try {
        export_pin(pin);
    } catch (const std::exception& e) {
        if (e.code() == 16) {   // Device or resource busy
            if (_gpio_warnings) {
                std::cerr << "Channel " + std::to_string(channel) + " is already in use, continuing anyway. Use GPIO.setwarnings(False) to disable warnings." << std::endl;
            }
            unexport_pin(pin);
            export_pin(pin);
        } else {
            throw e;
        }
    }
    direction(pin, direction);
    _exports[channel] = direction;
    if (direction == GPIO.getattr<int>("OUT") && initial != -1) {
        output(pin, initial);
    }
}

int input(int channel) {
    _check_configured(channel);  // Can read from a pin configured for output
    int pin = get_gpio_pin(_board, _mode, channel);
    return input(pin);
}

void output(int channel, int state) {
    _check_configured(channel, direction = GPIO.getattr<int>("OUT"));
    int pin = get_gpio_pin(_board, _mode, channel);
    output(pin, state);
}

int wait_for_edge(int channel, int trigger, int timeout = -1) {
    _check_configured(channel, direction = GPIO.getattr<int>("IN"));
    int pin = get_gpio_pin(_board, _mode, channel);
    if (blocking_wait_for_edge(pin, trigger, timeout) != -1) {
        return channel;
    }
    return -1;
}

void add_event_detect(int channel, int trigger, std::function<void(int)> callback = nullptr, int bouncetime = -1) {
    _check_configured(channel, direction = GPIO.getattr<int>("IN"));

    if (bouncetime != -1) {
        if (_gpio_warnings) {
            std::cerr << "bouncetime is not (yet) fully supported, continuing anyway. Use GPIO.setwarnings(False) to disable warnings." << std::endl;
        }
    }

    int pin = get_gpio_pin(_board, _mode, channel);
    add_edge_detect(pin, trigger, [=](int p) { callback(channel); }, bouncetime);
}

void remove_event_detect(int channel) {
    _check_configured(channel, direction = GPIO.getattr<int>("IN"));
    int pin = get_gpio_pin(_board, _mode, channel);
    remove_edge_detect(pin);
}

bool event_detected(int channel) {
    _check_configured(channel, direction = GPIO.getattr<int>("IN"));
    int pin = get_gpio_pin(_board, _mode, channel);
    return edge_detected(pin);
}

void cleanup(int channel = -1) {
    if (channel == -1) {
        for (auto& it : _exports) {
            cleanup(it.first);
        }
        setwarnings(true);
        _mode = -1;
    } else {
        _check_configured(channel);
        int pin = get_gpio_pin(_board, _mode, channel);
        cleanup(pin);
        unexport_pin(pin);
        _exports.erase(channel);
    }
}
