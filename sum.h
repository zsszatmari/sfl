#ifndef SFL_SUM_H
#define SFL_SUM_H

#include <cassert>
#include <cstdint>

namespace sfl
{
	/** 
	 * Algebraic data type, a.k.a. typesafe union. Now it can only have two members but it would be nice 
	 * if we would generalize this to more variables, while retaining vs2013 compatibility in the future. Use the match()
	 * function to get variable, this can be thought of a pseudo-guard. match() needs a template parameter which is the
	 * return value.
	 *
	 * usage example:
	 * sum<int, std::string> value = "this is a string"
	 *
	 * auto meaning = match<size_t>(value, [](int a){return 42 + al},
	 *									   [](const std::string &a){return a.length();});
	 */
	template<typename T0, typename T1>
	class sum final {
	public:
		sum(const T0 &value) :
			_currentType(0)
		{
			new (_storage) T0(value);
		}

		sum(const T1 &value) :
			_currentType(1)
		{
			new (_storage) T1(value);
		}

	public:
		sum(const sum<T0,T1> &rhs)
		{
			assign(rhs);
		}

		sum<T0,T1> &operator=(const sum<T0,T1> &rhs)
		{
			if (_currentType == rhs._currentType) {
				switch(_currentType) {
					case 0:
						*reinterpret_cast<T0 *>(&_storage) = *reinterpret_cast<const T0 *>(&rhs._storage);
						break;
					case 1:
						*reinterpret_cast<T1 *>(&_storage) = *reinterpret_cast<const T1 *>(&rhs._storage);
						break;
				}
			} else {
				resign();
				assign(rhs);
			}
			return *this;
		}

		~sum()
		{
			resign();
		}

		typedef T0 type0;
		typedef T1 type1;

		template<typename R, typename F0, typename F1>
		R match(F0 &&f0, F1 &&f1) const
		{
			switch (_currentType) {
				case 0:
					return f0(*reinterpret_cast<const T0 *>(&_storage));
				case 1:
					return f1(*reinterpret_cast<const T1 *>(&_storage));
			}
			assert(false);
		}

	private:
		void assign(const sum<T0,T1> &rhs)
		{
			_currentType = rhs._currentType;
			switch(_currentType) {
				case 0:
					new (_storage) T0(*reinterpret_cast<const T0 *>(&rhs._storage));
					break;
				case 1:
					new (_storage) T1(*reinterpret_cast<const T1 *>(&rhs._storage));
					break;
			}
		}

		void resign()
		{
			switch (_currentType) {
				case 0:
					(*reinterpret_cast<T0 *>(&_storage)).~T0();
					break;
				case 1:
					(*reinterpret_cast<T1 *>(&_storage)).~T1();
					break;
			}
		}
		
		// storage comes first for proper alignment
		uint8_t _storage[(sizeof(T0) > sizeof(T1)) ? sizeof(T0) : sizeof(T1)];
		uint8_t _currentType;
	};

	template<typename R, typename V, typename F0, typename F1>
	R match(const V &v, F0 &&f0, F1 &&f1)
	{
		return v.template match<R>(f0, f1);
	}
}

#endif