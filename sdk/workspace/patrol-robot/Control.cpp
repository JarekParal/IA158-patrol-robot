#include "Tower.hpp"
#include "Control.hpp"
#include <cmath>
#include <algorithm>

static void print_tabs ( FILE * fw, size_t tabs )
{
	while ( tabs-- )
		fputc ( '\t', fw );
}

static void print ( FILE * fw, size_t tabs, DepthObject const & target )
{
	print_tabs ( fw, tabs );
	fprintf ( fw, "{ %d, %d }", target.coordinates.x, target.coordinates.y);
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

double TargetList::distance(DepthObject a, DepthObject b) {
	double xx = a.coordinates.x - b.coordinates.x;
	double yy = a.coordinates.y - b.coordinates.y;
	return sqrt(xx * xx + yy * yy);
}

void TargetList::insert(DepthObject target)
{
	for (auto& existing_target : _targets) {
		if (distance(target, existing_target.target) < distance_threshold) {
			// Update
			existing_target.target = target;
			ev3_speaker_play_tone(tone_updated_target, tone_updated_target_len);
			return;
		}
	}

	SYSTIM now;
	get_tim (&now);
	_targets.push_back({next_id(), target, now});
	ev3_speaker_play_tone(tone_new_target, tone_new_target_len);
}

void TargetList::remove_old_targets(unsigned age)
{
	SYSTIM now;
	get_tim (&now);

	auto begin = std::remove_if(_targets.begin(), _targets.end(),
		[&](const TargetItem& i) {
			return now >= i.last_seen && now - i.last_seen < age * 1000;
		});

	_targets.erase(begin, _targets.end());
}

Control::Control ( ID mutex_id, Tower & tower ) :
	_tower ( tower )
{
	_mutex_id = mutex_id;
}

void Control::here_is_a_target(DepthObject o)
{
	loc_mtx ( _mutex_id );
	_target_list.insert(std::move(o));
	unl_mtx ( _mutex_id );
}

void Control::every_1s()
{
	loc_mtx ( _mutex_id );
	_target_list.remove_old_targets(max_age);
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
		if ( it.id == id )
		{
			unl_mtx ( _mutex_id );
			fprintf ( bt, "locking at [%d, %d]\n", it.target.coordinates.x,
				it.target.coordinates.y );
			_tower.lock_at ( it.target.coordinates );
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
		else if (is_prefix_of("lockat", buff)) {
			int x,y;
			if ( 2 == sscanf(buff, "lockat %d %d", &x, &y ) )
				_tower.lock_at ( Coordinates { x, y } );
			else
				fprintf (bt, "usage: lockat 8 15\n");
				
		} else if (is_prefix_of("lock", buff)) {
			unsigned int target_id;
			if (1 == sscanf(buff, "lock %u", &target_id))
				lock_target(TargetId(target_id));
			else
				fprintf(bt, "usage: lock 12\n");
		} else if (is_prefix_of("unlock", buff)) {
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

