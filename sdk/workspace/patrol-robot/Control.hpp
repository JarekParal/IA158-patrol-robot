#ifndef CONTROL_HPP_
#define CONTROL_HPP_

#include "Common.hpp"
#include "Target.hpp"

using TargetId = uint8_t;
struct TargetItem
{
	TargetId      id;
	DepthObject   target;
	SYSTIM        last_seen;
};

class TargetList
{
	public:
		using Targets = std::vector<TargetItem>;

		void insert(DepthObject o);
		void remove_old_targets(unsigned age);
		Targets const &targets() const;

	private:
		TargetId next_id();
		static double distance(DepthObject a, DepthObject b);

		Targets _targets;
		TargetId _max_id;

		using Tone = uint16_t;
		static const Tone tone_new_target     = 1000;
		static const Tone tone_new_target_len = 1000;

		static const Tone tone_updated_target     = 2000;
		static const Tone tone_updated_target_len =  250;

		static constexpr const double distance_threshold = 10;
};

class Tower;

class Control
{
	public:
		explicit Control ( ID mutex_id, Tower & tower );
		void loop();
		void here_is_a_target (DepthObject o);
		void every_1s();

		void lock_target ( TargetId id );

	private:
		TargetList _target_list;
		ID         _mutex_id;
		Tower    & _tower;

		static const unsigned max_age = 120;

};


#endif // CONTROL_HPP_
