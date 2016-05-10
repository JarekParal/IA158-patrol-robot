#include <utility>
#include "Target.hpp"

static void strip_trailing ( ScannedTarget & target )
{
	while ( !target.distances.empty() && target.distances.back() <= 0 )
		target.distances.pop_back();
}

static void strip_leading ( ScannedTarget & target )
{

}

void strip ( ScannedTarget & target )
{
	strip_trailing ( target );
	strip_leading ( target );
}

bool match ( ScannedTarget const & _a, ScannedTarget const & _b )
{
	ScannedTarget const * a = &_a;
	ScannedTarget const * b = &_b;

	if ( a->from() > b->from() )
		std::swap ( a, b );

	size_t total = a->distances.size() + b->distances.size();
	size_t cover = 0;
	if ( a->to() >= b->to() )
		cover = b->to() - b->from();
	else
		cover = a->to() - b->from();

	cover = cover * 2;
	if ( cover >= total / 2 )
		return true;
	else
		return false;
}

