/*
 * Very basic 'Event'. Does not care about copy/move semantics,
 * does not support perfect forwarding.
 */

#ifndef EVENT_HPP
#define EVENT_HPP

#include <functional> // std::function
#include <vector>     // std::vector

template < typename... Args >
class Event
{
	public:
		using DelegateType = void(Args...);
		using Delegate = std::function<DelegateType>;

		void insert(Delegate d)
		{
			_delegates.push_back(d);
		}

		void invoke(Args... args) const
		{
			for ( Delegate d : _delegates )
			{
				d(args...);
			}
		}

	private:
		std::vector<Delegate> _delegates;
};

#endif // EVENT_HPP
