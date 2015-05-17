#ifndef SIMPLE_TEST_FRAMEWORK_H
#define SIMPLE_TEST_FRAMEWORK_H

/**
  *  The very purpose of this library to be as small and lightweight as possible.
  *  There are libraries with many more features, but might be unpractical for a project to depend on it.
  *  This library is originally created for https://github.com/treasurebox/sfl , shared under a MIT-like license
  *  Let me know if you use this. In that case, it might be beneficial for all parties involved
  *  to create a separate github repo. 
  */

#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <functional>
#include "sfl/Prelude.h"

extern std::vector<std::pair<std::string,std::function<void()>>> tests;
#define INIT_SFL \
	std::vector<std::pair<std::string,std::function<void()>>> tests;

#define TEST(x,y) \
	void TEST_##x##_##y##_run();\
	struct TEST_##x##_##y {\
		TEST_##x##_##y ()\
		{\
			tests.push_back(std::make_pair("" #x "." #y, &TEST_##x##_##y##_run));\
		}\
	};\
	static TEST_##x##_##y TEST_##x##_##y##_instance;\
	void TEST_##x##_##y##_run()

#define ASSERT_TRUE(x) \
	if (!(x)) { throw std::logic_error("assertion '" #x "' not true"); }

#define ASSERT_FALSE(x) \
	if ((x)) { throw std::logic_error("assertion '" #x "' not false"); }

#define ASSERT_EQ(x,y) \
	{\
		auto evalx = x;\
		auto evaly = y;\
		if (!(evalx == evaly)) { \
			std::stringstream ss;\
			ss << "assertion '" #x " == " #y "' not true because " << evalx << " != " << evaly;\
			throw std::logic_error(ss.str());\
		}\
	}

inline std::ostream & operator<< (std::ostream &out, const std::vector<std::string> &strings)
{
	using std::string;
	out << "[" << sfl::intercalate(string(","), sfl::map([](const string &s){return string("\"") + s + "\"";},strings)) << "]";
 	return out;
}

inline int runTests()
{
	size_t passes = 0;
	for (auto &test : tests) {
		try {
			test.second();
			std::cout << "PASS: " << test.first << std::endl;
			++passes;
		} catch (std::exception &e) {
			std::cout << "FAIL: " << test.first << " " << e.what() << std::endl;
		} catch (...) {
			std::cout << "FAIL: " << test.first << std::endl;
		}
	}

	std::cout << std::endl;
	std::cout << "Passed " << passes << " of " << tests.size() << " tests" << std::endl; 

	return passes < tests.size();
}

#endif