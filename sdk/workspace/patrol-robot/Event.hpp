/*
 * Very basic 'Event'. Does not care about copy/move semantics,
 * does not support perfect forwarding.
 */

#ifndef EVENT_HPP
#define EVENT_HPP

#include <functional> // std::function
#include <array>

template < typename... Args >
class Event
{
	public:
		using DelegateType = void(Args...);
		using Delegate = std::function<DelegateType>;

		Event() : _index(0) {}

		void insert(Delegate d)
		{
			_delegates[_index++] = d;
		}

		void invoke(Args... args) const
		{
			for (size_t i = 0; i != _index; i++)
				_delegates[i](args...);
		}

	private:
		std::array<Delegate, 10> _delegates;
		size_t _index;
};

#endif // EVENT_HPP
