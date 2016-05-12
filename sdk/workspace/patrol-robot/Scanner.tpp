#include <algorithm> // std::reverse
#include <cmath>     // std::abs
#include <cassert>

template <class DistanceSensor>
Scanner<DistanceSensor>::Scanner(ePortS sonar_port) : _sonar(sonar_port) {
	on_target = nullptr;
    change_detected = false;
    _direction = Direction::Right;
	_is_boundary_position = true;
	_background_detected = false;
	_background_distance = 0;
}

template <class DistanceSensor>
void Scanner<DistanceSensor>::task() {}

template <class DistanceSensor>
bool Scanner<DistanceSensor>::distance_is_background ( Distance dist ) const
{
	return dist >= _background_distance;
}

template <class DistanceSensor>
void Scanner<DistanceSensor>::received_position_message(PositionMessage msg)
{
	_previous_was_boundary_position = _is_boundary_position;

	_is_boundary_position = (_direction != msg.direction);
	Distance current_distance = make_sample();

	if ( _background_detected )
	{
		detect_object(msg.position, current_distance);
	} else {
		if ( !distance_is_error(current_distance) )
		{
			_background_distance = std::max(_background_distance, current_distance);
		}
		if ( _is_boundary_position && (msg.direction == Direction::Left) ) {
			if ( _background_distance >= 1 )
				_background_distance = std::max ( 15, _background_distance - 15 );
			else
				_background_distance = 254; // only errors were detected
			fprintf ( bt, "background: %d\n", _background_distance );
			_background_detected = true;
		}
	}

	_direction = msg.direction;
	_previous_distance = current_distance;
}

template <class DistanceSensor>
Distance Scanner<DistanceSensor>::previous_distance() const
{
	// Background on boundaries,
	// previous distance elsewhere

	if ( _previous_was_boundary_position )
		return _background_distance;
	else
		return _previous_distance;
}

template <class DistanceSensor>
bool Scanner<DistanceSensor>::continuous_change(Distance prev, Distance curr)
{
	return std::abs(prev - curr) <= 5;
}

template <class DistanceSensor>
void Scanner<DistanceSensor>::continue_detecting_object(Position position, Distance distance)
{
	// _is_boundary_position
	if ( distance_is_error(distance) )
	{
		// we assume previous value
		_current_object_distances.push_back(previous_distance());
		return;
	}

	bool background = distance_is_background(distance);
	bool continuous = continuous_change(previous_distance(), distance);

	if ( !background && continuous && !_is_boundary_position )
	{
		_current_object_distances.push_back(distance);
	} else {
		// Object ended
		fprintf ( bt, "Object ended at position %d, distance %d\n", position, distance );

		auto min_distance_it = std::min_element(
				_current_object_distances.begin(),
				_current_object_distances.end()   );
		Distance min_distance = *min_distance_it;
		size_t min_position_diff = min_distance_it - _current_object_distances.begin();
		Position position_of_minimal_distance;
		if ( _direction == Direction:: Right)
			position_of_minimal_distance = position - _current_object_distances.size() + min_position_diff;
		else
			position_of_minimal_distance = position + _current_object_distances.size() - min_position_diff;
		fprintf ( bt, "Object center at position %d, distance %d\n", position_of_minimal_distance, min_distance );
		fprintf ( bt, "Object distances: " );
		for ( Distance d : _current_object_distances )
		{
			fprintf ( bt, "%d ", d );
		}
		fprintf ( bt, "\n");
		on_target(DepthObject{Coordinates{position_of_minimal_distance, min_distance}});
		_current_object_distances.clear();
	}
}

template <class DistanceSensor>
void Scanner<DistanceSensor>::start_detecting_object(Position position, Distance distance)
{
	assert (!detecting_object());

	bool background = distance_is_background(distance);
	bool continuous = continuous_change(previous_distance(), distance);

	if ( distance_is_error(distance) || background || !continuous )
		return;

	fprintf ( bt, "Object started at %d, distance %d\n", position, distance );

	_current_object_distances.clear();
	_current_object_distances.push_back(distance);
	_current_object_start = position;	
}

template <class DistanceSensor>
bool Scanner<DistanceSensor>::detecting_object() const
{
	return !_current_object_distances.empty();
}

template <class DistanceSensor>
void Scanner<DistanceSensor>::detect_object(Position position, Distance distance)
{
	if (detecting_object() )
		continue_detecting_object(position, distance);
	else
		start_detecting_object(position, distance);
}



template <class DistanceSensor>
Distance Scanner<DistanceSensor>::make_sample()
{
    int16_t d = _sonar.getDistance();
    return d;

	std::vector<int16_t> samples;
    for (unsigned i = 0; i < sample_precision; ++i) {
        samples.push_back(_sonar.getDistance());
    }

    return median_distance(samples);
}

template <class DistanceSensor>
Distance Scanner<DistanceSensor>::median_distance(std::vector<int16_t>& samples) {
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



template <class DistanceSensor>
bool Scanner<DistanceSensor>::distance_is_error(Distance d)
{
	if (d >= 255)
		return true;
	if (d < 0 )
		return true;

	return false;
}

