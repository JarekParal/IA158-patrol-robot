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
			return;
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

Control::Control ( ID mutex_id )
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

void Control::loop()
{
	fprintf ( bt, "Robot started\n" );
	fprintf ( bt, 	"Commands:"
					"\tnext\n"
					"\tshoot [seconds]\n"
					""
			
			);

	while(true)
		tslp_tsk(50);
}

