#include "stf.h"
#include "sfl/STM.h"

using namespace sfl;

TEST(STM, Basic)
{
	TVar<float> val(3);
	ASSERT_EQ(3, (float)val);

}