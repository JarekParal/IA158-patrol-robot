#include <algorithm> // std::reverse
#include <cassert>

#include "Scanner.hpp"

Scanner::Scanner(ePortS sonar_port) : _sonar(sonar_port) {
    on_target = nullptr;
    _scanned_map = {};
}

void Scanner::task() {}

void Scanner::received_position_message(PositionMessage msg) {
    Position position = msg.position;

    if (_scanned_map[position]) {
        scan_changes(position, msg.direction);
    } else {
        make_sample(position);
        _direction = msg.direction;
    }
}

void Scanner::make_sample(Position position) {
    _depth_map[position] = _sonar.getDistance();
    _scanned_map[position] = true;
}

void Scanner::scan_changes(Position position, Direction current_dir) {
    assert(_scanned_map[position]);

    Distance distance = _sonar.getDistance();

    if (detected_target) {
		bool moved = has_moved ( position, distance );
		bool changed_direction = (_direction != current_dir);
        if ( moved && !changed_direction ) {
			// still scanning target
			scanned_target.distances.push_back(distance);
            _depth_map[position] = distance; // update map
        } else {                             // target ended
            assert(!scanned_target.distances.empty());

			// Reorder them to grow
			if ( _direction == Direction::Left )
			{
				scanned_target._from -= scanned_target.distances.size();
				std::reverse(
						scanned_target.distances.begin(),
						scanned_target.distances.end()   );
			}

            if (on_target)
                on_target ( std::move ( scanned_target )  );
            scanned_target = ScannedTarget();
            detected_target = false;
        }


    } else {
        if (has_moved(position, distance)) { // target detected
			scanned_target._from = position;
			scanned_target.distances.push_back(distance);

            detected_target = true;
            _depth_map[position] = distance; // update map
        }
    }
}

bool Scanner::has_moved(Position position, Distance sample) const {
    assert(_scanned_map[position]);
    Distance expected = _depth_map[position];

    return (expected - allowed_error > sample ||
            expected + allowed_error < sample);
}
