#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <Port.h>
#include <SonarSensor.h>
#include <vector>
#include <functional>

#include "Common.hpp"
#include "Event.hpp"
#include "Position.hpp"

class Scanner {
public:
    Scanner(ePortS sonar_port, PositionEvent& position_event);
    void init();
    void task();

    void subscribe_target(std::function<void(Target)> callback);
private:
    void received_position_message(PositionMessage msg);
    ev3api::SonarSensor _sonar;
    std::vector<int16_t> depth_map;
    Event<Target> target_msg_dispatch;
};

#endif // SCANNER_HPP
