#ifndef CONTROL_HPP_
#define CONTROL_HPP_

#include "Common.hpp"


using TargetId = uint8_t;
struct TargetItem
{
	TargetId id;
	Target t;
	bool valid;
	SYSTIM last_seen;
};

class TargetList
{
	public:
		TargetList();
		using Targets = std::array<TargetItem, 5>;

		void update ( Target t );
		void remove_old_targets();
		void remove ( TargetId id );

		Targets const & targets() const;

	private:
		TargetId next_id();

		Targets _targets;
		TargetId _max_id;
};

class Tower;

class Control
{
	public:
		explicit Control ( ID mutex_id, Tower & tower );
		void loop();
		void here_is_a_target ( ScannedTarget t );
		void every_1s();

		void lock_target ( TargetId id );

	private:
		TargetList _target_list;
		ID         _mutex_id;
		Tower    & _tower;
};


#endif // CONTROL_HPP_
