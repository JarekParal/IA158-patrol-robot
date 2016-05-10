#include "Tower.hpp"
#include "Control.hpp"

static void print_tabs ( FILE * fw, size_t tabs )
{
	while ( tabs-- )
		fputc ( '\t', fw );
}

static void print ( FILE * fw, size_t tabs, ScannedTarget const & target )
{
	print_tabs ( fw, tabs );
	fprintf ( fw, "{ from: %d, distances: [ ", target.from() );
	for ( size_t i = 0; i < target.distances.size(); i++ )
	{
		if ( i != 0 )
			fprintf ( fw, ", " );
		fprintf ( fw, " %d", target.distances[i] );

	}
	fprintf ( fw, "] }\n" );
}


TargetList::TargetList()
{
	_max_id = 0;
	for ( TargetItem & i : _targets )
	{
		i.valid = false;
	}
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

void TargetList::insert ( TargetItem & item, ScannedTarget target, SYSTIM now )
{
	fprintf ( bt, "updating %u:\n", item.id );
	print ( bt, 1, target );
	strip ( target );
	if ( target.distances.size() == 0 )
	{
		item.valid = false;
		return;
	}

	item.valid = true;
	item.target = std::move ( target );
	item.last_seen = now;
}

void TargetList::update ( ScannedTarget target )
{
	SYSTIM now;
	get_tim ( &now );

	// Update already existing target
	for ( TargetItem & i : _targets )
	{
		if ( !i.valid )
			continue;

		if ( !match ( i.target, target ) )
			continue;

		insert ( i, std::move ( target ), now );
		ev3_speaker_play_tone(tone_updated_target, tone_updated_target_len);
		return;
	}

	// there is no such target in list
	// try to add a new one
	for ( TargetItem & i : _targets )
	{
		if ( i.valid )
			continue;

		i.id = next_id();
		insert ( i, std::move ( target ), now );
		ev3_speaker_play_tone(tone_new_target, tone_new_target_len);
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
		if ( (now >= i.last_seen ) && ( now - i.last_seen < 120*1000) )
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

void Control::here_is_a_target ( ScannedTarget t )
{
	loc_mtx ( _mutex_id );
	_target_list.update ( std::move(t) );
	unl_mtx ( _mutex_id );
}

void Control::every_1s()
{
	loc_mtx ( _mutex_id );
	_target_list.remove_old_targets();
	unl_mtx ( _mutex_id );
}

static void print ( FILE * fw, TargetItem const & item, SYSTIM now )
{
	fprintf ( fw, "{ ID: %d, last_seen: %.3f, target:",
			item.id, double (now - item.last_seen) / 1000 );
	print ( fw, 1, item.target );
	fprintf ( fw, "}\n" );
}

static void print ( FILE *fw, TargetList const & tl )
{
	SYSTIM now;
	get_tim ( &now );

	fprintf ( fw, "{\n" );
	auto const & targets = tl.targets();
	for ( TargetItem const & item : targets )
	{
		if ( !item.valid )
			continue;
		fprintf ( fw, "\t" );
		print   ( fw, item, now );
		fprintf ( fw, "\n" );
	}
	fprintf ( fw, "}\n");
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

void Control::lock_target ( TargetId id )
{
	loc_mtx ( _mutex_id );

	for ( TargetItem const & it : _target_list.targets() )
	{
		if ( it.valid && (it.id == id) )
		{
			size_t mid = it.target.distances.size() / 2;
			int16_t y = it.target.distances[mid];
			int16_t x = it.target.from() + mid;
			unl_mtx ( _mutex_id );
			_tower.lock_at ( Coordinates { x, y } );
			return;
		}
	}

	unl_mtx ( _mutex_id );
	return;
}

void Control::loop()
{
	fprintf ( bt, "Robot started\n" );
	fprintf ( bt, 	"Commands:\n"
					"\tshoot [rounds]\n"
					"\tcalibrate-tower <angle>\n"
					"\tlist\n"
					"\tlock\n"
					"\tunlock\n"
			
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

		fputc ( '\n', bt );

		if (is_prefix_of("calibrate-tower", buff)) {
			fprintf ( bt, "OK, we will calibrate tower\n" );
			int angle;
			if (1 == sscanf(buff, "calibrate-tower %d", &angle))
				_tower.calibrate(angle);
			else
				fprintf(bt, "usage: calibrate-tower 45\n");
		}
		else if (is_prefix_of("list", buff)) {
			print(bt, _target_list);
		}
		else if (is_prefix_of("lock", buff)) {
			unsigned int target_id;
			if (1 == sscanf(buff, "lock %u", &target_id))
				lock_target(TargetId(target_id));
			else
				fprintf(bt, "usage: lock 12\n");
		}
		else if (is_prefix_of("unlock", buff)) {
			_tower.unlock();
		} 
		else if (is_prefix_of("shoot", buff)) {
			fprintf (bt, "OK, shoot!\n");
			int shots;
			if (1 == sscanf (buff, "shoot %d", &shots))
				_tower.shoot(shots);
			else
				_tower.shoot(1);
		}
	}
}

