#ifndef SFL_SUM_H
#define SFL_SUM_H

#include <cstdint>

namespace sfl
{
	// it would be nice if we would generalize this to more variables, while retaining vs2013 compatibility
	template<typename T0, typename T1>
	class sum final {
	public:
		sum(const T0 &value)
		{
			_currentType = 0;
			new (_storage) T0(value);
		}

		sum(const T1 &value)
		{
			_currentType = 1;
			new (_storage) T1(value);
		}

		~sum()
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
		}

	private:
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