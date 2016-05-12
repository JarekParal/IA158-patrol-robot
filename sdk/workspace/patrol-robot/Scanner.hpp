#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <Port.h>
#include <SonarSensor.h>
#include <array>
#include <vector>
#include <functional>

#include "Common.hpp"
#include "Target.hpp"

template <class DistanceSensor>
class Scanner {
public:
    explicit Scanner(ePortS sonar_port);
    void task();

    void received_position_message(PositionMessage msg);
    Event<ScannedTarget> on_target;

private:
    Distance make_sample();
    void scan_changes(Position position, Direction current_dir);
	void print_depth_map();
	Distance previous_distance() const;

    Distance median_distance(std::vector<int16_t>& samples);

    bool has_moved(Position position, Distance sample) const;
	void detect_changes(Position position, Distance distance);
	void update_changes(Position position, Distance distance, Direction current_dir);
	void update_map_from_target();
	void update_map(Position position, Direction direction);
	void detect_object(Position position, Distance distance);
	void start_detecting_object(Position position, Distance distance);
	void continue_detecting_object(Position position, Distance distance);

	void reorder_target();
	

	static bool continuous_change(Distance prev, Distance curr);

	static bool distance_is_error ( Distance d );
	bool distance_is_background ( Distance d ) const;


    ScannedTarget scanned_target;
    bool change_detected;

    Direction _direction;

    static const size_t map_size = 256;
    static const size_t allowed_error = 10;      // in cm
    static const unsigned sample_precision = 20; // number of samples
	static const size_t min_target_size = 3;

    DistanceSensor _sonar;
    std::array<bool, map_size> _scanned_map;
    std::array<Distance, map_size> _depth_map;

	bool _background_detected;
	Distance _background_distance;
	bool _previous_was_boundary_position;
	bool _is_boundary_position;
	Distance _previous_distance;
	bool _detecting_object;
	std::vector<Distance> _current_object_distances;
	Position _current_object_start;
};

#include "Scanner.tpp"

#endif // SCANNER_HPP
