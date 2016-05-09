#include "Scanner.hpp"

#include <cassert>

Scanner::Scanner(ePortS sonar_port) : _sonar(sonar_port) {
    on_target = nullptr;
    _scanned_map = {};
}

void Scanner::task() {}

void Scanner::received_position_message(PositionMessage msg) {
    Position position = msg.position;
    int16_t d = _sonar.getDistance();
    fprintf(bt, "Position: %d, Distance: %d\n", position, d);

    /*if (_scanned_map[position]) {
        scan_changes(position, msg.direction);
    } else {
        make_sample(position);
        _direction = msg.direction;
    }*/
}

void Scanner::make_sample(Position position) {
    std::vector<int16_t> samples;

    for (int i = 0; i < sample_precision; ++i) {
        samples.push_back(_sonar.getDistance());
    }

    _depth_map[position] = median_distance(samples);
    _scanned_map[position] = true;
    fprintf(bt, "Position: %d, Distance: %d\n", position, _depth_map[position]);
}

Distance Scanner::median_distance(std::vector<int16_t>& samples) {
    sort(samples.begin(), samples.end());

    Distance median;
    size_t size = samples.size();

    if (size % 2 == 0) {
        median = (samples[size / 2 - 1] + samples[size / 2]) / 2;
    } else {
        median = samples[size / 2];
    }
    return median;
}

void Scanner::scan_changes(Position position, Direction current_dir) {
    assert(_scanned_map[position]);

    Distance distance = _sonar.getDistance();

    if (detected_target) {
        if (has_moved(position, distance) &&
            _direction == current_dir) { // still scanning target
            scanned_target.extend(position, distance);
            _depth_map[position] = distance; // update map
            fprintf(bt, "Update Position: %d, Distance: %d\n", position,
                    _depth_map[position]);
        } else { // target ended
            assert(!scanned_target.distances.empty());

            Position target_mid = scanned_target.get_mid();
            if (on_target)
                on_target(Target{target_mid,
                                 scanned_target.distances[target_mid]});
            scanned_target.reset();
            detected_target = false;
        }
    } else {
        if (has_moved(position, distance)) { // target detected
            scanned_target.detect(position, distance);
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
