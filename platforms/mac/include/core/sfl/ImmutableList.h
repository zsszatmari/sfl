#ifndef IMMUTABLELIST_H
#define IMMUTABLELIST_H

#include <memory>

namespace sfl
{
	/**
	  * A reference counted list with immutable values. Appending elements to the front of
	  * the list does not require a copy, which may be beneficial to performance.
	  */
	template<typename T>
	class ImmutableList final
	{
	private:
		struct Node final
		{
			Node(const T &aValue, const std::shared_ptr<Node> &aNext) :
				next(aNext),
				value(aValue)
			{
			}

			const std::shared_ptr<Node> next;
			const T value;
		};

		size_t _size;
		std::shared_ptr<Node> _node;

		ImmutableList(const std::shared_ptr<Node> &node, size_t size) :
				_size(size),
				_node(node)
		{
		}

	public:
		ImmutableList() :
			_size(0),
			_node(nullptr)
		{
		}

		ImmutableList(const T &t) :
			_size(1),
			_node(std::make_shared<Node>(t, nullptr))
		{
		}

		size_t size() const
		{
			return _size;
		}

		T at(size_t index) const
		{
			std::shared_ptr<Node> node = _node;
			while (index > 0) {
				node = node->next;
				--index;
			}
			return node->value;
		}

		ImmutableList<T> cons(const T& lhs) const
		{
			return ImmutableList(std::make_shared<Node>(lhs, _node), _size+1);
		}

		typedef T value_type;

		class const_iterator final : public std::iterator<std::forward_iterator_tag,T> 
		{
		public:
			const_iterator(const std::shared_ptr<Node> &node, size_t size) :
				_size(size),
				_node(node)
			{
			}

			bool operator==(const const_iterator &rhs) const
		    {
		    	return _size == rhs._size;
		    }

		    bool operator!=(const const_iterator &rhs) const
		    {
		    	return _size != rhs._size;
		    }

		    const_iterator & operator++()
		    {
		    	--_size;
		    	_node = _node->next;
		    	return *this;
		    }

		    const T& operator*() const
			{
				return _node->value;
			}

		    const T* operator->() const
		    {
		    	return &_node->value;
		    }

		private:
			size_t _size;
			std::shared_ptr<Node> _node;

			friend class ImmutableList;
		};

		const_iterator begin() const
		{
			return const_iterator(_node, _size);
		}

		const_iterator end() const
		{
			return const_iterator(nullptr, 0);
		}
	};

	namespace List
	{
		template<typename T>
		ImmutableList<T> singleton(const T &t)
		{
			return ImmutableList<T>(t);
		}
	}

	template<typename R, typename T = typename R::value_type>
	R cons(const T &lhs, const R &r)
	{
		return r.cons(lhs);
	}
}

#endif