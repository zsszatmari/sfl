#include <string>
#include <stdint.h>

namespace Gear
{
	using std::string;

	class Tracking final
	{
	public:
		static void trackStart();
		static void track(const string &eventCategory, const string &eventAction, const string &eventLabel = "");
		static void track(const string &eventCategory, const string &eventAction, const int64_t eventNumber);

	private:
		Tracking();
	};
}