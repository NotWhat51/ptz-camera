#ifndef EVENT_HPP
#define EVENT_HPP

#include <functional>
#include <thread>
#include "constants.hpp"

class _worker;

extern std::unordered_map<int, std::thread> _threads;

int blocking_wait_for_edge(int pin, int trigger, int timeout = -1);
bool edge_detected(int pin);
void add_edge_detect(int pin, int trigger, std::function<void(int)> callback = nullptr);
void remove_edge_detect(int pin);
void add_edge_callback(int pin, std::function<void(int)> callback);
void cleanup(int pin = -1);

#endif // EVENT_HPP
