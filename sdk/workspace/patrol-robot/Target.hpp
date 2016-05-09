#ifndef TARGET_HPP_
#define TARGET_HPP_

#include "Common.hpp"

struct ScannedTarget {
	ScannedTarget() = default;
	ScannedTarget ( ScannedTarget const & orig ) = delete;
	ScannedTarget ( ScannedTarget && old ) = default;
	ScannedTarget & operator= ( ScannedTarget const & origin ) = delete;
	ScannedTarget & operator= ( ScannedTarget && old ) = default;

	Position _from;
	Position _to;
	std::vector<Distance> distances;

	Position from() const { return _from; }
	Position to() const { return _from + distances.size(); }
};

bool match ( ScannedTarget const & a, ScannedTarget const & b );

/**
 * @brief Removes leading and trailing zeroes
 * FIXME not implemented yet
 */
void strip ( ScannedTarget & target );

#endif // TARGET_HPP_
