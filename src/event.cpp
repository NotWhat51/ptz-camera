#include "event.hpp"
#include <thread>
#include <mutex>
#include <vector>
#include <stdexcept>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include "sysfs.hpp"

std::unordered_map<int, std::thread> _threads;

class _worker {
public:
    _worker(int pin, int trigger, std::function<void(int)> callback = nullptr)
        : _pin(pin), _trigger(trigger), _event_detected(false), _finished(false) {
        if (callback) {
            add_callback(callback);
        }
    }

    void add_callback(std::function<void(int)> callback) {
        _callbacks.push_back(callback);
    }

    bool event_detected() {
        std::lock_guard<std::mutex> lock(_lock);
        if (_event_detected) {
            _event_detected = false;
            return true;
        } else {
            return false;
        }
    }

    void cancel() {
        _finished = true;
        if (_thread.joinable()) {
            _thread.join();
        }
    }

    void start() {
        _thread = std::thread(&::_worker::run, this);
    }

private:
    void run() {
        try {
            edge(_pin, _trigger);
            bool initial_edge = true;

            int fd = ValueDescriptor(_pin);
            int efd = epoll_create1(0);
            struct epoll_event event;
            event.events = EPOLLIN | EPOLLET | EPOLLPRI;
            event.data.fd = fd;
            epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event);

            try {
                while (!_finished) {
                    struct epoll_event events[1];
                    int n = epoll_wait(efd, events, 1, 100);
                    if (initial_edge) {
                        initial_edge = false;
                    } else if (n > 0) {
                        std::lock_guard<std::mutex> lock(_lock);
                        _event_detected = true;
                        notify_callbacks();
                    }
                }
            } catch (...) {
                epoll_ctl(efd, EPOLL_CTL_DEL, fd, &event);
                close(efd);
                throw;
            }

            epoll_ctl(efd, EPOLL_CTL_DEL, fd, &event);
            close(efd);
        } catch (const std::exception& e) {
            _exc = std::current_exception();
        }

        edge(_pin, GPIO.getattr<int>("NONE"));
    }

    void notify_callbacks() {
        for (const auto& cb : _callbacks) {
            cb(_pin);
        }
    }

    int _pin;
    int _trigger;
    bool _event_detected;
    bool _finished;
    std::mutex _lock;
    std::vector<std::function<void(int)>> _callbacks;
    std::thread _thread;
    std::exception_ptr _exc;
};

int blocking_wait_for_edge(int pin, int trigger, int timeout) {
    if (trigger != GPIO.getattr<int>("RISING") && trigger != GPIO.getattr<int>("FALLING") && trigger != GPIO.getattr<int>("BOTH")) {
        throw std::invalid_argument("Invalid trigger");
    }

    if (_threads.find(pin) != _threads.end()) {
        throw std::runtime_error("Conflicting edge detection events already exist for this GPIO channel");
    }

    try {
        edge(pin, trigger);

        bool finished = false;
        bool initial_edge = true;

        int fd = ValueDescriptor(pin);
        int efd = epoll_create1(0);
        struct epoll_event event;
        event.events = EPOLLIN | EPOLLET | EPOLLPRI;
        event.data.fd = fd;
        epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event);

        try {
            while (!finished) {
                struct epoll_event events[1];
                int n = epoll_wait(efd, events, 1, timeout);
                if (initial_edge) {
                    initial_edge = false;
                } else {
                    finished = true;
                }

                if (n == 0) {
                    return -1;
                } else {
                    return pin;
                }
            }
        } catch (...) {
            epoll_ctl(efd, EPOLL_CTL_DEL, fd, &event);
            close(efd);
            throw;
        }

        epoll_ctl(efd, EPOLL_CTL_DEL, fd, &event);
        close(efd);
    } catch (...) {
        edge(pin, GPIO.getattr<int>("NONE"));
        throw;
    }

    edge(pin, GPIO.getattr<int>("NONE"));
    return -1;
}

bool edge_detected(int pin) {
    if (_threads.find(pin) != _threads.end()) {
        return _threads[pin].event_detected();
    } else {
        return false;
    }
}

void add_edge_detect(int pin, int trigger, std::function<void(int)> callback) {
    if (trigger != GPIO.getattr<int>("RISING") && trigger != GPIO.getattr<int>("FALLING") && trigger != GPIO.getattr<int>("BOTH")) {
        throw std::invalid_argument("Invalid trigger");
    }

    if (_threads.find(pin) != _threads.end()) {
        throw std::runtime_error("Conflicting edge detection already enabled for this GPIO channel");
    }

    _worker worker(pin, trigger, callback);
    _threads[pin] = std::thread(&::_worker::start, &worker);
}

void remove_edge_detect(int pin) {
    if (_threads.find(pin) != _threads.end()) {
        _threads[pin].cancel();
        _threads.erase(pin);
    }
}

void add_edge_callback(int pin, std::function<void(int)> callback) {
    if (_threads.find(pin) != _threads.end()) {
        _threads[pin].add_callback(callback);
    } else {
        throw std::runtime_error("Add event detection before adding a callback");
    }
}

void cleanup(int pin) {
    if (pin == -1) {
        for (auto& kv : _threads) {
            kv.second.cancel();
        }
        _threads.clear();
    } else {
        remove_edge_detect(pin);
    }
}
