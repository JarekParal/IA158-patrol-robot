#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <Port.h>
#include <SonarSensor.h>
#include <vector>

#include "Position.hpp"

class Scanner {
public:
    Scanner(ePortS sonar_port, PositionEvent& position_event);
    void init();
    void task();

private:
    void received_position_message(PositionMessage msg);
    ev3api::SonarSensor _sonar;
    std::vector<int16_t> depth_map;
};

#endif // SCANNER_HPP
