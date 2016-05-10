#include <algorithm> // std::reverse
#include <cassert>

#include "Scanner.hpp"

Scanner::Scanner(ePortS sonar_port) : _sonar(sonar_port) {
	on_target = nullptr;
	_scanned_map = {};
	change_detected = false;
	_direction = Direction::Right;
}

void Scanner::task() {}

void Scanner::received_position_message(PositionMessage msg) {
	Position position = msg.position;


	if (_scanned_map[position]) {
		scan_changes(position, msg.direction);
	} else {
		Distance distance = make_sample(position);
		if (distance != 255) {
			_depth_map[position] = distance;
			_scanned_map[position] = true;
		}
	}
	if ( _direction != msg.direction )
	{
		print_depth_map();
		_direction = msg.direction;
	}
}

void Scanner::print_depth_map()
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

Distance Scanner::make_sample(Position position) {
	std::vector<int16_t> samples;

	int16_t d = _sonar.getDistance();
	fprintf(bt, "%d\n", d);
	//fprintf(bt, "Position: %d, Distance: %d\n", position, d);
	return d;

	for (unsigned i = 0; i < sample_precision; ++i) {
		samples.push_back(_sonar.getDistance());
	}

	return median_distance(samples);

	/*if (distance != 255) {
	  _depth_map[position] = median_distance(samples);
	  _scanned_map[position] = true;
	  }*/

	// fprintf(bt, "Position: %d, Distance: %d\n", position,
	// _depth_map[position]);
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

void Scanner::detect_changes(Position position, Distance distance)
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

void Scanner::update_map_from_target()
{
	for ( size_t i = 0; i < scanned_target.distances.size(); i++ )
	{
		_depth_map[scanned_target.from() + i] = scanned_target.distances[i];
	}
}

void Scanner::reorder_target()
{
	if (_direction == Direction::Left) {
		scanned_target._from -= scanned_target.distances.size();
		std::reverse(scanned_target.distances.begin(),
				scanned_target.distances.end());
	}
}

void Scanner::update_changes(Position position, Distance distance, Direction current_dir)
{
	if ( distance = 255 )
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

void Scanner::scan_changes(Position position, Direction current_dir) {
	assert ( position >= 0 );
	assert(_scanned_map[position]);

	Distance distance = make_sample(position);

	if (change_detected)
		update_changes(position, distance, current_dir);
	else
		detect_changes(position, distance);
}

bool Scanner::has_moved(Position position, Distance sample) const {
	assert(_scanned_map[position]);
	Distance expected = _depth_map[position];

	return (expected - allowed_error > sample ||
			expected + allowed_error < sample);
}

