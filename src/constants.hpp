#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <any>

class _const {
public:
    class ConstError : public std::runtime_error {
    public:
        ConstError(const std::string& message) : std::runtime_error(message) {}
    };

    template <typename T>
    void setattr(const std::string& name, const T& value);

    template <typename T>
    T getattr(const std::string& name);

private:
    std::unordered_map<std::string, std::any> data;
};

extern _const GPIO;

void init_gpio();

#endif // CONSTANTS_HPP
