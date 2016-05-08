#include "Scanner.hpp"

Scanner::Scanner(ePortS sonar_port)
    : _sonar(sonar_port)
    , _state(ScanningState::Init) {
		on_target = nullptr;
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

		if ( on_target )
			on_target ( Target{position, distance} );
    }
}
