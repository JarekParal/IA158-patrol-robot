#include <Clock.h>
#include "Walker.hpp"

/*
 * Robot starts on leftmost cell, which is red and has index '0'.
 * The rightmost cell is red, too, and has index 'k' for some k \in \Nats.
 * There must be at least one non-red cell, so k has to be >= 2;
 */


Walker::Walker (
		ePortM motor_port,
		ePortS color_port,
		PositionEvent const & position_event )
:
	_motor          ( motor_port     ),
	_color_sensor   ( color_port     ),
	_position_event ( position_event )
{
}

void Walker::init()
{
	_current_position  = 0;
	_current_direction = Direction::Right;
	_current_color     = PositionColor::Red;
	_motor.setPWM(50);
}

Walker::PositionColor Walker::next_color(rgb_raw_t const & rgb) const
{
	uint16_t red = rgb.r;
	uint16_t other = uint16_t((uint32_t(rgb.g) + uint32_t(rgb.b)) / 2);

	PositionColor c = _current_color;

	// White
	if ( red >= 50000 && other >= 50000 )
	{
		c = PositionColor::White;
	}

	if ( red >= 50000 && other <= 25000 )
	{
		c = PositionColor::Red;
	}

	if ( red <= 25000 && other <= 25000 )
	{
		c = PositionColor::Black;
	}

	return c;
}

void Walker::update_position(PositionColor c)
{
	if ( c == _current_color )
		return;

	step();

	if ( c == PositionColor::Red )
		change_direction();

	_current_color = c;
	_position_event.invoke(PositionMessage{_current_direction, _current_position});
}

void Walker::step()
{
	if ( _current_direction == Direction::Right )
		_current_position++;

	if ( _current_direction == Direction::Left )
		_current_position--;
}

void Walker::change_direction()
{
	switch ( _current_direction )
	{
		case Direction::Right:
			_current_direction = Direction::Left;
			_motor.setPWM(-50);
			break;

		case Direction::Left:
			_current_direction = Direction::Right;
			_motor.setPWM(50);
			break;
	}
}

void Walker::task()
{
	rgb_raw_t rgb;
	_color_sensor.getRawColor(rgb);
	PositionColor c = next_color(rgb);
	update_position(c);
}

