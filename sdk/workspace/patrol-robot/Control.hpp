#ifndef CONTROL_HPP_
#define CONTROL_HPP_

#include "Common.hpp"
#include "Target.hpp"

using TargetId = uint8_t;
struct TargetItem
{
	TargetId      id;
	ScannedTarget target;
	bool          valid;
	SYSTIM        last_seen;
};

class TargetList
{
	public:
		TargetList();
		using Targets = std::array<TargetItem, 5>;

		void update ( ScannedTarget t );
		void remove_old_targets();
		void remove ( TargetId id );

		Targets const & targets() const;

	private:
		TargetId next_id();
		void insert ( TargetItem & item, ScannedTarget target, SYSTIM now );


		Targets _targets;
		TargetId _max_id;

		using Tone = uint16_t;
		static const Tone tone_new_target     = 1000;
		static const Tone tone_new_target_len = 2000;

		static const Tone tone_updated_target     = 2000;
		static const Tone tone_updated_target_len =  500;
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
