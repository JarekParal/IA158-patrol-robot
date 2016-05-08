#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <Port.h>
#include <SonarSensor.h>
#include <array>
#include <vector>
#include <functional>

#include "Common.hpp"

class Scanner {
public:
    explicit Scanner(ePortS sonar_port);
    void task();

    void received_position_message(PositionMessage msg);
    Event<Target> on_target;

private:
    void make_sample(Position position);
    void scan_changes(Position position, Direction current_dir);

    bool has_moved(Position position, Distance sample) const;

    struct ScannedTarget {
        Position from;
        Position to;
        std::vector<Distance> distances;

        void detect(Position pos, Distance dist) {
            assert(distances.empty());

            from = pos;
            to = pos;
            distances.push_back(dist);
        }
        void extend(Position pos, Distance dist) {
            to = pos;
            distances.push_back(dist);
        }
        void reset() { distances.clear(); }

        Position get_mid() const { return from + (to - from) / 2; }
    };

    ScannedTarget scanned_target;
    bool detected_target = false;

    Direction _direction;

    static const size_t map_size = 256;
    static const size_t allowed_error = 1; // in cm

    ev3api::SonarSensor _sonar;
    std::array<bool, map_size> _scanned_map;
    std::array<Distance, map_size> _depth_map;
};

#endif // SCANNER_HPP
