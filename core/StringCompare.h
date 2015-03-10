#ifndef STRINGCOMPARE_H
#define STRINGCOMPARE_H

#include <cstdint>

namespace Gear
{
	class StringCompare
	{
	public:
		static int compare(const char *s1, const char *s2);
		static int compareWithLength(void *, int len1, const void *s1, int len2, const void *s2);
	};
}

#endif