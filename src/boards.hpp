#ifndef BOARDS_HPP
#define BOARDS_HPP

#include <functional>
#include <unordered_map>
#include <string>

class _sunXi;
class _SOC;

extern std::unordered_map<std::string, int> _board_model;
extern std::unordered_map<int, std::unordered_map<int, int>> _pin_map;

extern _sunXi sunXi;
extern _SOC soc;

int get_gpio_pin(int board, int mode, int channel);
std::string get_name(int board);
std::unordered_map<std::string, std::string> get_info(int board);
int get_board();

#endif // BOARDS_HPP
