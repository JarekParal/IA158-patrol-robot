#ifndef CONTROL_HPP_
#define CONTROL_HPP_

#include "Common.hpp"
#include "Target.hpp"

using TargetId = uint8_t;
using TTL = uint8_t;
struct TargetItem
{
	TargetId      id;
	DepthObject   target;
	TTL           ttl;
};

class TargetList
{
	public:
		TargetList() : _max_id(0) {}
		using Targets = std::vector<TargetItem>;

		TargetId insert(DepthObject o);
		void remove_old_targets();
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

		static constexpr const double distance_threshold = 30;
};

class Tower;
class IScanner;

class Control
{
	public:
		explicit Control ( ID mutex_id, Tower & tower, IScanner & scanner );
		void loop();
		void here_is_a_target (DepthObject o);
		void next_round();

		void lock_target ( TargetId id );

	private:
		void usage() const;

		TargetList _target_list;
		ID         _mutex_id;
		Tower    & _tower;
		IScanner  & _scanner;
		TargetId   _locked_id;

		static const unsigned max_age = 120;

};


#endif // CONTROL_HPP_
