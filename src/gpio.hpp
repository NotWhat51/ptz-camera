#ifndef GPIO_H
#define GPIO_H

#include <iostream>
#include <map>
#include <vector>
#include <assert.h>
#include "constants.hpp"
#include "sysfs.hpp"
#include "event.hpp"

extern bool _gpio_warnings;
extern int _mode;
extern int _board;
extern std::map<int, int> _exports;
extern std::vector<int> _boards;
extern std::string RPI_INFO;

void _check_configured(int channel, int direction = -1);

void setboard(int board);
std::string getboardmodel();
int getmode();
void setmode(int mode);
void setwarnings(bool enabled);
void setup(int channel, int direction, int initial = -1, int pull_up_down = -1);
int input(int channel);
void output(int channel, int state);
int wait_for_edge(int channel, int trigger, int timeout = -1);
void add_event_detect(int channel, int trigger, std::function<void(int)> callback = nullptr, int bouncetime = -1);
void remove_event_detect(int channel);
bool event_detected(int channel);
void cleanup(int channel = -1);

#endif // GPIO_H
