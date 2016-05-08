#include "Tower.hpp"
#include "Control.hpp"

TargetList::TargetList()
{
	_max_id = 0;
}

TargetList::Targets const & TargetList::targets() const
{
	return _targets;
}

TargetId TargetList::next_id()
{
	TargetId id = _max_id;
	_max_id = (_max_id + 1) % 20;
	return id;
}

void TargetList::update ( Target t )
{
	SYSTIM now;
	get_tim ( &now );

	// Update already existing target
	for ( TargetItem & i : _targets )
	{
		if ( !i.valid )
			continue;

		if ( i.t.position == t.position )
		{
			i.t.distance = t.distance;
			i.last_seen = now;
			
		}
	}

	// there is no such target in list
	// try to add a new one
	for ( TargetItem & i : _targets )
	{
		if ( i.valid )
			continue;

		i.id = next_id();
		i.t = t;
		i.valid = true;
		i.last_seen = now;
		return;
	}


	// Oops, target list is full.
	// We rather lost a new target
	// then some old one.
}

void TargetList::remove_old_targets()
{
	SYSTIM now;
	get_tim ( &now );

	for ( TargetItem & i : _targets )
	{
		if ( !i.valid )
			continue;

		// Keep only new targets
		if ( (now >= i.last_seen ) && ( now - i.last_seen < 10000) )
			continue;

		i.valid = false;
	}
}

void TargetList::remove ( TargetId id )
{
	for ( TargetItem & i : _targets )
	{
		if ( i.id == id )
			i.valid = false;
	}
}

Control::Control ( ID mutex_id, Tower & tower ) :
	_tower ( tower )
{
	_mutex_id = mutex_id;
}

void Control::here_is_a_target ( Target t )
{
	loc_mtx ( _mutex_id );
	_target_list.update ( t );
	unl_mtx ( _mutex_id );
}

void Control::every_1s()
{
	loc_mtx ( _mutex_id );
	_target_list.remove_old_targets();
	unl_mtx ( _mutex_id );
}

static bool is_prefix_of ( const char * prefix, const char * string )
{
	while ( *prefix == *string )
	{
		if ( *prefix == '\0' )
			return true;

		prefix++;
		string++;
	}

	if ( *prefix == '\0' )
		return true;

	return false;
}

static bool read_line(char * buf, size_t bufsz)
{
	while ( true ) {
		char c = fgetc ( bt );

		if ( (c == '\n') || (c == '\r') )
		{
			if (bufsz > 0)
			{
				*buf = '\0';
				return true;
			} else {
				return false;
			}
		}

		if (iscntrl(c))
			return false;

		fputc ( c, bt ); // echo

		if ( bufsz > 0 )
		{
			*buf = c;
			buf++;
			bufsz--;
		}
	}
}

void Control::loop()
{
	fprintf ( bt, "Robot started\n" );
	fprintf ( bt, 	"Commands:\n"
					"\tnext\n"
					"\tshoot [seconds]\n"
					"\tcalibrate-tower <angle>\n"
			
			);

	while(true)
	{
		fprintf( bt, "> " );
		char buff[80];
		buff[0] = '\0';
		bool valid = read_line (buff, 80 );
		if ( !valid )
		{
			fprintf ( bt, "--discarded\n");
			continue;
		}

		fprintf ( bt, "got: %s\n", buff );

		if ( is_prefix_of ( "calibrate-tower", buff ) )
		{
			fprintf ( bt, "OK, we will calibrate tower\n" );
			int angle;
			if (1 == sscanf ( buff, "calibrate-tower %d", &angle ) )
				_tower.calibrate(angle);
			else
				fprintf ( bt, "usage: calibrate-tower 45\n" );
		} else if ( is_prefix_of ( "", buff) )
		{

		}
	}
}

