#include "boards.hpp"
#include "constants.hpp"
#include <string>
#include <cassert>
#include <unordered_map>

class _sunXi {
public:
    int operator[](const std::string& value) {
        int offset = value[1] - 'A';
        int pin = std::stoi(value.substr(2));

        assert(value[0] == 'P');
        assert(0 <= offset && offset <= 25);
        assert(0 <= pin && pin <= 31);

        return (offset * 32) + pin;
    }
};

class _SOC {
public:
    int operator[](int value) {
        return value;
    }
};

std::unordered_map<std::string, int> _board_model = {
    {"Repka-Pi3-H5", GPIO.getattr<int>("REPKAPI3")}
};

std::unordered_map<int, std::unordered_map<int, int>> _pin_map = {
    {GPIO.getattr<int>("REPKAPI3"), {
        {0, "Repka Pi 3"},
        {1, {{"P1_REVISION", 3}, {"TYPE", "Repka Pi 3"}, {"MANUFACTURER", "ИНТЕЛЛЕКТ"}, {"RAM", "1024M"}, {"REVISION", ""}, {"PROCESSOR", "Allwinner H5"}}},
        {GPIO.getattr<int>("BOARD"), {
            {3, 12},    // PA12/TWI1_SDA/DI_RX/PA_EINT12
            {5, 11},    // PA11/TWI1_SCK/DI_TX/PA_EINT11
            {7, 7},     // PA7
            {8, 4},     // PA4/UART0_TX
            {10, 5},    // PA5/UART0_RX
            {11, 8},    // PA8
            {12, 6},    // PA14
            {13, 9},    // PA9
            {15, 10},   // PA10
            {16, 354},  // PL2/S_UART_TX
            {18, 355},  // PL3/S_UART_RX
            {19, 64},   // PA15/SPI0_MOSI
            {21, 65},   // PA16/SPI0_MISO
            {22, 2},    // PA2
            {23, 66},   // PA14/SPI0_CLK
            {24, 67},   // PC3/SPI0_CS0
            {26, 3},    // PA3/SPI0_CS0
            {27, 19},   // PA19/TWI2_SDA
            {28, 18},   // PA18/TWI2_SCK
            {29, 0},    // PA0/UART2_TX
            {31, 1},    // PA1/UART2_RX
            {32, 363},  // PL11
            {33, 362},  // PL10/PWM0
            {35, 16},   // PA16/SPI1_MISO
            {36, 13},   // PA13/SPI1_CS0
            {37, 21},   // PA21
            {38, 15},   // PG6/SPI1_MOSI
            {40, 14}    // PG7/SPI1_CLK
        }},
        {GPIO.getattr<int>("BCM"), {
            {2, 12},
            {3, 11},
            {4, 7},
            {14, 4},
            {15, 5},
            {17, 8},
            {18, 6},
            {27, 9},
            {22, 10},
            {23, 354},
            {24, 355},
            {10, 64},
            {9, 65},
            {25, 2},
            {11, 66},
            {8, 67},
            {7, 3},
            {5, 0},
            {6, 1},
            {12, 363},
            {13, 362},
            {19, 16},
            {16, 13},
            {26, 21},
            {20, 15},
            {21, 14}
        }},
        {GPIO.getattr<int>("SUNXI"), _sunXi()},
        {GPIO.getattr<int>("SOC"), _SOC()}
    }}
};

int get_gpio_pin(int board, int mode, int channel) {
    assert(mode == GPIO.getattr<int>("BOARD") || mode == GPIO.getattr<int>("BCM") || mode == GPIO.getattr<int>("SUNXI") || mode == GPIO.getattr<int>("SOC"));
    assert(board == GPIO.getattr<int>("REPKAPI3"));
    if (mode == GPIO.getattr<int>("SUNXI")) {
        return sunXi[std::to_string(channel)];
    } else if (mode == GPIO.getattr<int>("SOC")) {
        return soc[channel];
    } else {
        return _pin_map[board][mode][channel];
    }
}

std::string get_name(int board) {
    assert(board == GPIO.getattr<int>("REPKAPI3"));
    return "Repka Pi 3";
}

std::unordered_map<std::string, std::string> get_info(int board) {
    if (board != GPIO.getattr<int>("REPKAPI3")) {
        throw std::runtime_error("Не выбранна модель платы. Для выбора модели платы используйте метод setboard()");
    }

    std::unordered_map<std::string, std::string> info = {
        {"P1_REVISION", "3"},
        {"TYPE", "Repka Pi 3"},
        {"MANUFACTURER", "ИНТЕЛЛЕКТ"},
        {"RAM", "1024M"},
        {"REVISION", ""},
        {"PROCESSOR", "Allwinner H5"}
    };

    std::string mem;
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("cat /proc/meminfo | grep -i 'memtotal' | grep -o '[[:digit:]]*'", "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        mem += buffer.data();
    }

    int ram = std::stoi(mem) / 1024;
    if (ram > 1024 && ram < 2048) {
        info["RAM"] = "2GB";
    } else {
        info["RAM"] = "1GB";
    }

    return info;
}

int get_board() {
    std::ifstream file("/proc/device-tree/model");
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open /proc/device-tree/model");
    }

    std::string model;
    std::getline(file, model);
    model.erase(std::remove(model.end() - 1, model.end(), '\x00'), model.end());

    if (_board_model.find(model) == _board_model.end()) {
        throw std::runtime_error("Не выбранна модель платы. Для выбора модели платы используйте метод setboard()");
    }

    return _board_model[model];
}

