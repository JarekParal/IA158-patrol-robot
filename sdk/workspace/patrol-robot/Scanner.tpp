#include <algorithm> // std::reverse
#include <cmath>     // std::abs
#include <cassert>

template <class DistanceSensor>
Scanner<DistanceSensor>::Scanner(ePortS sonar_port) : _sonar(sonar_port) {
	on_target = nullptr;
    _scanned_map = {};
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
void Scanner<DistanceSensor>::received_position_message(PositionMessage msg) {
	update_map(msg.position, msg.direction);

	_previous_was_boundary_position = _is_boundary_position;

	_is_boundary_position = (_direction != msg.direction);
	Distance current_distance = make_sample();

	if ( _background_detected )
	{
		detect_object(msg.position, current_distance);
		if ( _is_boundary_position )
		{
			print_depth_map();
		}
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
	if ( distance_is_error(distance) )
	{
		// we assume previous value
		_current_object_distances.push_back(previous_distance());
		return;
	}

	bool background = distance_is_background(distance);
	bool continuous = continuous_change(previous_distance(), distance);

	if ( !continuous )
		fprintf ( bt, "not continuous: prev=%d, curr=%d\n", previous_distance(), distance );

	if ( !background && continuous )
	{
		_current_object_distances.push_back(distance);
	}

	if ( background || !continuous )
	{
		// Object ended
		fprintf ( bt, "Object ended at position %d, distance %d\n", position, distance );
		// TODO
		assert ( !_current_object_distances.empty() );

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
void Scanner<DistanceSensor>::update_map(Position position, Direction direction)
{
    // Out of bounds?
    if ( (position < 0) || (position >= (Position)map_size) )
        return;

    if (_scanned_map[position]) {
        scan_changes(position, direction);
    } else {
        Distance distance = make_sample();
        if (distance != 255) {
            _depth_map[position] = distance;
            _scanned_map[position] = true;
        }
    }
}

template <class DistanceSensor>
void Scanner<DistanceSensor>::print_depth_map()
{
    fprintf ( bt, "\n----------\n");

    for (size_t i = 0; i < map_size; i++)
    {
        if ( !_scanned_map[i] )
            fprintf ( bt, "--- " );
        else
            fprintf ( bt, "%3d ", _depth_map[i] );

    }
    fprintf ( bt, "\n----------\n");
}

template <class DistanceSensor>
Distance Scanner<DistanceSensor>::make_sample() {
    std::vector<int16_t> samples;

    int16_t d = _sonar.getDistance();
    //fprintf(bt, "%d\n", d);
    //fprintf(bt, "Position: %d, Distance: %d\n", position, d);
    return d;

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
void Scanner<DistanceSensor>::detect_changes(Position position, Distance distance)
{
    if ( distance == 255 )
        return;

    if (has_moved(position, distance)) { // target detected
        scanned_target._from = position;
        scanned_target.distances.push_back(distance);

        change_detected = true;
        _depth_map[position] = distance; // update map
    }

}

template <class DistanceSensor>
void Scanner<DistanceSensor>::update_map_from_target()
{
    for ( size_t i = 0; i < scanned_target.distances.size(); i++ )
    {
        _depth_map[scanned_target.from() + i] = scanned_target.distances[i];
    }
}

template <class DistanceSensor>
void Scanner<DistanceSensor>::reorder_target()
{
    if (_direction == Direction::Left) {
        scanned_target._from -= scanned_target.distances.size();
        std::reverse(scanned_target.distances.begin(),
                scanned_target.distances.end());
    }
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

template <class DistanceSensor>
void Scanner<DistanceSensor>::update_changes(Position position, Distance distance, Direction current_dir)
{
    if ( distance_is_error(distance) )
        distance = scanned_target.distances.back();

    bool moved = has_moved(position, distance);

    bool changed_direction = (_direction != current_dir);
    if (moved && !changed_direction) {
        // still scanning target
        scanned_target.distances.push_back(distance);
        // fprintf(bt, "Update Position: %d, Distance: %d\n", position,
        //        _depth_map[position]);
    } else { // target ended
        assert(!scanned_target.distances.empty());

        if ( scanned_target.distances.size() >= min_target_size )
        {
            reorder_target();
            update_map_from_target();
            if (on_target)
                on_target(std::move(scanned_target));
        }
        scanned_target = ScannedTarget();
        change_detected = false;
    }
}

template <class DistanceSensor>
void Scanner<DistanceSensor>::scan_changes(Position position, Direction current_dir) {
    assert ( position >= 0 );
    assert(_scanned_map[position]);

    Distance distance = make_sample();

    if (change_detected)
        update_changes(position, distance, current_dir);
    else
        detect_changes(position, distance);
}

template <class DistanceSensor>
bool Scanner<DistanceSensor>::has_moved(Position position, Distance sample) const {
    assert(_scanned_map[position]);
    Distance expected = _depth_map[position];

    return (expected - allowed_error > sample ||
            expected + allowed_error < sample);
}
