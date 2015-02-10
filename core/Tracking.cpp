#include <sstream>
#include "Tracking.h"
#include "HttpDownloader.h"
#include "UrlEncode.h"
#include "BackgroundExecutor.h"
#include "IApp.h"
#include "ThemeManager.h"
#include "ITheme.h"
#include "IPreferences.h"
#include "SerialExecutor.h"

namespace Gear
{
	using std::string;

#define method Tracking::


	method Tracking()
	{
	}

	void method trackStart()
	{
        track("Lifecycle", "Start");
        track("Lifecycle", "Theme", IApp::instance()->themeManager()->current()->name());
        track("Lifecycle", "Purchased Version", IApp::instance()->purchasedVersion());
        track("Lifecycle", "Run Count", IApp::instance()->runCount());
        auto trial = IApp::instance()->trialRemaining();
		if (trial > 0) {
    		track("Lifecycle", "Trial Remaining", trial);
    	}
	}

	static void doTrack(const string &eventCategory, const string &eventAction, const string &extras)
	{
		if (!IApp::instance()->preferences().boolForKey("SendUsageStatistics")) {
			return;
		}

		static Base::SerialExecutor _executor;

		// don't use the backgroundexecutor, it is easiliy exhaustable
    	_executor.addTask([eventCategory, eventAction, extras]{

    		std::stringstream ss;
    		ss << "v=1&tid=" << IApp::instance()->trackingId();
    		ss << "&cid=" << IApp::instance()->trackingClientId();
    		ss << "&t=event";
    		ss << "&an=G-Ear";
    		ss << "&av=" << IApp::instance()->version();
    		ss << "&aid=com.treasurebox.gear";
    		ss << "&aiid=com.treasurebox.gear";

    		ss << "&ec=" << eventCategory;
    		ss << "&ea=" << eventAction;
    		ss << "&sr=" << IApp::instance()->screenWidth() << "x" << IApp::instance()->screenHeight();

    		static bool startSession = true;
    		if (startSession) {
    			startSession = false;
    			ss << "&sc=start";
    		}

    		ss << extras;

            string bodyPure = ss.str();
			string body = UrlEncode::encode(bodyPure);

			map<string,string> headers;
			headers["User-Agent"] = IApp::instance()->trackingAgent();
			headers["Content-Type"] = "application/x-www-form-urlencoded";

			auto downloader = HttpDownloader::create("https://www.google-analytics.com/collect", headers, "POST", body);
            //std::cout << HttpDownloader::curlDump("https://www.google-analytics.com/collect", headers, "POST", body) << std::endl;
			downloader->start();
			downloader->waitUntilFinished();
			//auto failCode = downloader->failed();
			//string result = *downloader;
    	});
	}

	void method track(const string &eventCategory, const string &eventAction, const string &eventLabel)
	{
		if (eventLabel.empty()) {
			doTrack(eventCategory, eventAction, "");
		} else {
			doTrack(eventCategory, eventAction, "&el=" + eventLabel);
		}
	}

	void method track(const string &eventCategory, const string &eventAction, const int64_t eventNumber)
	{
        std::stringstream ss;
        ss << "&el=" << eventNumber;
        string s = ss.str();
		doTrack(eventCategory, eventAction, s);
	}
}