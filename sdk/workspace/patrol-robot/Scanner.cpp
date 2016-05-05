#include "Scanner.hpp"

Scanner::Scanner(ePortS sonar_port, PositionEvent& position_event,
                 TargetEvent& target_event)
    : _sonar(sonar_port)
    , _state(ScanningState::Init)
    , _target_event(target_event) {
    position_event.insert(
            [this](PositionMessage msg) { received_position_message(msg); });
}

void Scanner::task() {}

void Scanner::received_position_message(PositionMessage msg) {
    switch (_state) {
    case ScanningState::Init:
        if (check_state(msg.direction))
            make_sample(msg.position);
        break;
    case ScanningState::Patrol:
        scan_changes(msg.position);
        break;
    }
}

bool Scanner::check_state(Direction direction) {
    if (direction == Direction::Left && _state == ScanningState::Init) {
        _state = ScanningState::Patrol;
        return false;
    }
    return true;
}

void Scanner::make_sample(int16_t position) {
    _depth_map[position] = _sonar.getDistance();
}

void Scanner::scan_changes(int16_t position) {
    int16_t distance = _sonar.getDistance();
    int16_t expected = _depth_map[position];

    if (expected - allowed_error > distance ||
        expected + allowed_error < distance) {
        _target_event.invoke(Target{position, distance});
    }
}
