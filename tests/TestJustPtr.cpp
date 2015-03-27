#include "stf.h"
#include "sfl/just_ptr.h"

using namespace sfl;

class Parent
{
public:
	int a;
};

class Child : public Parent
{
public:
	Child()
	{
		a = 6;
	}

	Child(int value)
	{
		a = value;
	}
};

TEST(just_ptr, Hierarchy)
{
	{
		just_ptr<Child> value = sfl::make_just<Child>();
		just_ptr<Parent> p = value;
		ASSERT_EQ(6, p->a);
	}
	{
		just_ptr<Child> value = sfl::make_just<Child>(3);
		just_ptr<Parent> p = value;
		ASSERT_EQ(3, p->a);
	}
}