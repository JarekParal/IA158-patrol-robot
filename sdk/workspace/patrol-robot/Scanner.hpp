#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <Port.h>
#include <SonarSensor.h>
#include <array>
#include <functional>

#include "Common.hpp"
#include "Event.hpp"
#include "Position.hpp"

class Scanner {
public:
    Scanner(ePortS sonar_port, PositionEvent& position_event,
            TargetEvent& target_event);
    void task();

private:
    enum class ScanningState { Init, Patrol };

    void received_position_message(PositionMessage msg);
    void make_sample(int16_t position);
    void scan_changes(int16_t position);
    bool check_state(Direction dir);

    static const size_t map_size = 256;
    static const size_t allowed_error = 1; // in cm
    ev3api::SonarSensor _sonar;
    ScanningState _state;
    TargetEvent const& _target_event;
    std::array<int16_t, map_size> _depth_map;
};

#endif // SCANNER_HPP
