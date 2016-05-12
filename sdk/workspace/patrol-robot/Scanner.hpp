#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <Port.h>
#include <SonarSensor.h>
#include <array>
#include <vector>
#include <functional>

#include "Common.hpp"
#include "Target.hpp"


class IScanner
{
	public:
		virtual void set_background ( Distance d ) = 0;
};

template <class DistanceSensor>
class Scanner : public IScanner
{
public:
    explicit Scanner(ePortS sonar_port);
    void task();

    void received_position_message(PositionMessage msg);
    Event<DepthObject> on_target;
	std::function<void()>        on_next_round;

	// < IScanner >
	virtual void set_background ( Distance d ) override;
	// </Iscanner >

private:
    Distance make_sample();
	Distance previous_distance() const;

    Distance median_distance(std::vector<int16_t>& samples);

	void detect_object(Position position, Distance distance);
	void start_detecting_object(Position position, Distance distance);
	void continue_detecting_object(Position position, Distance distance);
	bool detecting_object() const;


	static bool continuous_change(Distance prev, Distance curr);

	static bool distance_is_error ( Distance d );
	bool distance_is_background ( Distance d ) const;


    ScannedTarget scanned_target;
    bool change_detected;

    Direction _direction;

    static const unsigned sample_precision = 20; // number of samples
	static const size_t min_target_size = 3;
	static const bool   ignore_flat_objects = true;
	static const size_t max_flat_object_depth = 2;

    DistanceSensor _sonar;


	bool _background_detected;
	Distance _background_distance;
	bool _previous_was_boundary_position;
	bool _is_boundary_position;
	Distance _previous_distance;
	std::vector<Distance> _current_object_distances;
	Position _current_object_start;
};

#include "Scanner.tpp"

#endif // SCANNER_HPP
