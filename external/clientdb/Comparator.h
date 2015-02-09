#ifndef COMPARATOR_H
#define COMPARATOR_H

namespace ClientDb
{
	typedef int (*Comparator)(void *, int len1, const void *s1, int len2, const void *s2);
}

#endif